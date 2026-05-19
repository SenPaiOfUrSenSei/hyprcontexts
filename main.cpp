#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/desktop/Workspace.hpp>
#include <string>
#include <set>
#include <fstream>

inline HANDLE PHANDLE = nullptr;

int g_iCurrentContext = 0;
int g_iOffsetMultiplier = 100;

void pluginLog(std::string msg) {
    std::ofstream logFile("/tmp/hyprcontexts.log", std::ios::app);
    logFile << msg << std::endl;
}

void onNewWorkspace(PHLWORKSPACEREF pWorkspace) {
    int contextNum = pWorkspace->m_id / g_iOffsetMultiplier;
    int wsNum      = pWorkspace->m_id % g_iOffsetMultiplier;

    pWorkspace->m_name = "C" + std::to_string(contextNum) + ":" + std::to_string(wsNum);
}

void onActiveWorkspace(PHLWORKSPACE pWorkspace) {
    if (!pWorkspace)
        return;
    g_iCurrentContext = pWorkspace->m_id / g_iOffsetMultiplier;
    pluginLog("Active Workspace: " + std::to_string(pWorkspace->m_id) + " (Context: " + std::to_string(g_iCurrentContext) + ")");
}

void runOnSwitchHook() {
    auto pConfigVal = HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprcontexts:on_switch");
    if (!pConfigVal)
        return;

    std::string cmd = std::any_cast<Hyprlang::STRING>(pConfigVal->getValue());
    if (cmd.empty())
        return;

    size_t      pos        = 0;
    std::string contextStr = std::to_string(g_iCurrentContext);
    while ((pos = cmd.find("%C", pos)) != std::string::npos) {
        cmd.replace(pos, 2, contextStr);
        pos += contextStr.length();
    }

    g_pKeybindManager->m_dispatchers["exec"](cmd);
}

SDispatchResult cycleContext(std::string arg) {
    bool next = arg == "next";
    pluginLog("Cycle called with arg: " + arg);

    const auto pCurrentWorkspace = g_pCompositor->getMonitorFromCursor()->m_activeWorkspace;
    if (!pCurrentWorkspace) {
        pluginLog("Error: No active workspace found");
        return {false, false, "No active workspace found"};
    }

    int currentID = pCurrentWorkspace->m_id;
    int currentCtx = currentID / g_iOffsetMultiplier;

    // Build a sorted set of populated workspace IDs in current context
    std::set<int> populated;
    for (auto& w : g_pCompositor->m_windows) {
        if (w->m_workspace && !w->isHidden() && (int)(w->m_workspace->m_id / g_iOffsetMultiplier) == currentCtx) {
            populated.insert(w->m_workspace->m_id);
        }
    }

    std::string popStr = "";
    for (int id : populated) popStr += std::to_string(id) + " ";
    pluginLog("Populated: " + popStr + " Current: " + std::to_string(currentID));

    int target = -1;
    if (next) {
        auto it = populated.upper_bound(currentID);
        if (it != populated.end()) {
            target = *it;
        } else {
            target = currentID + 1;
        }
    } else { // prev
        auto it = populated.lower_bound(currentID);
        if (it != populated.begin()) {
            target = *std::prev(it);
        } else {
            target = currentID - 1;
        }
    }

    // Boundary check: ensure target stays within the same context
    if (target / g_iOffsetMultiplier != currentCtx || target <= 0) {
        pluginLog("Target " + std::to_string(target) + " out of context or invalid, cancelling.");
        target = -1;
    }

    if (target != -1 && target != currentID) {
        pluginLog("Switching to target: " + std::to_string(target));
        g_pKeybindManager->m_dispatchers["workspace"](std::to_string(target));
    } else {
        pluginLog("No target found or target same as current");
    }

    return {};
}

SDispatchResult switchContext(std::string arg) {
    int newContext;
    try {
        newContext = std::stoi(arg);
    } catch (...) {
        return {false, false, "Invalid context ID"};
    }

    const auto pWorkspace = g_pCompositor->getMonitorFromCursor()->m_activeWorkspace;
    if (!pWorkspace)
        return {false, false, "No active workspace found"};

    int currentWSNum = pWorkspace->m_id % g_iOffsetMultiplier;
    g_iCurrentContext = newContext;
    int targetWS = (g_iCurrentContext * g_iOffsetMultiplier) + currentWSNum;

    g_pKeybindManager->m_dispatchers["workspace"](std::to_string(targetWS));

    runOnSwitchHook();

    HyprlandAPI::addNotification(PHANDLE, "[HyprContexts] Switched to context " + std::to_string(g_iCurrentContext), CHyprColor(0.0f, 1.0f, 0.0f, 1.0f), 2000);

    return {};
}

SDispatchResult contextWorkspace(std::string arg) {
    int wsNum;
    try {
        wsNum = std::stoi(arg);
    } catch (...) {
        return {false, false, "Invalid workspace number"};
    }

    const auto pWorkspace = g_pCompositor->getMonitorFromCursor()->m_activeWorkspace;
    if (!pWorkspace)
        return {false, false, "No active workspace found"};

    int currentContext = pWorkspace->m_id / g_iOffsetMultiplier;
    int target = (currentContext * g_iOffsetMultiplier) + wsNum;
    g_pKeybindManager->m_dispatchers["workspace"](std::to_string(target));

    return {};
}

SDispatchResult contextMoveToWorkspace(std::string arg) {
    int wsNum;
    try {
        wsNum = std::stoi(arg);
    } catch (...) {
        return {false, false, "Invalid workspace number"};
    }

    const auto pWorkspace = g_pCompositor->getMonitorFromCursor()->m_activeWorkspace;
    if (!pWorkspace)
        return {false, false, "No active workspace found"};

    int currentContext = pWorkspace->m_id / g_iOffsetMultiplier;
    int target = (currentContext * g_iOffsetMultiplier) + wsNum;
    g_pKeybindManager->m_dispatchers["movetoworkspace"](std::to_string(target));

    return {};
}

void renameExistingWorkspaces() {
    for (auto& w : g_pCompositor->getWorkspaces()) {
        int contextNum = w->m_id / g_iOffsetMultiplier;
        int wsNum      = w->m_id % g_iOffsetMultiplier;
        w->m_name      = "C" + std::to_string(contextNum) + ":" + std::to_string(wsNum);
    }
}

void onConfigReload() {
    g_iOffsetMultiplier = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprcontexts:offset")->getValue());
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprcontexts:offset", Hyprlang::INT{100});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprcontexts:on_switch", Hyprlang::STRING{""});
    HyprlandAPI::reloadConfig();

    g_iOffsetMultiplier = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprcontexts:offset")->getValue());

    renameExistingWorkspaces();

    Event::bus()->m_events.config.reloaded.listenStatic(onConfigReload);
    Event::bus()->m_events.workspace.created.listenStatic(onNewWorkspace);
    Event::bus()->m_events.workspace.active.listenStatic(onActiveWorkspace);

    HyprlandAPI::addDispatcherV2(PHANDLE, "hyprcontexts:switch", switchContext);
    HyprlandAPI::addDispatcherV2(PHANDLE, "hyprcontexts:workspace", contextWorkspace);
    HyprlandAPI::addDispatcherV2(PHANDLE, "hyprcontexts:movetoworkspace", contextMoveToWorkspace);
    HyprlandAPI::addDispatcherV2(PHANDLE, "hyprcontexts:cycle", cycleContext);

    pluginLog("Plugin Initialized!");
    HyprlandAPI::addNotification(PHANDLE, "[HyprContexts] Initialized!", CHyprColor(0.0f, 1.0f, 0.0f, 1.0f), 5000);

    return {"HyprContexts", "Meta-layer workspace manager", "YourName", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    // Cleanup
}
