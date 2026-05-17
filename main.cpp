#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <string>

inline HANDLE PHANDLE = nullptr;

int g_iCurrentContext = 0;
int g_iOffsetMultiplier = 100;

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

    int target = (g_iCurrentContext * g_iOffsetMultiplier) + wsNum;
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

    int target = (g_iCurrentContext * g_iOffsetMultiplier) + wsNum;
    g_pKeybindManager->m_dispatchers["movetoworkspace"](std::to_string(target));

    return {};
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
    HyprlandAPI::reloadConfig();

    g_iOffsetMultiplier = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprcontexts:offset")->getValue());

    Event::bus()->m_events.config.reloaded.listenStatic(onConfigReload);

    HyprlandAPI::addDispatcherV2(PHANDLE, "switchcontext", switchContext);
    HyprlandAPI::addDispatcherV2(PHANDLE, "contextworkspace", contextWorkspace);
    HyprlandAPI::addDispatcherV2(PHANDLE, "contextmovetoworkspace", contextMoveToWorkspace);

    HyprlandAPI::addNotification(PHANDLE, "[HyprContexts] Initialized!", CHyprColor(0.0f, 1.0f, 0.0f, 1.0f), 5000);

    return {"HyprContexts", "Meta-layer workspace manager", "YourName", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    // Cleanup
}
