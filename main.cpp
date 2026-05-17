#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/event/EventBus.hpp>

inline HANDLE PHANDLE = nullptr;

int g_iCurrentContext = 0;
int g_iOffsetMultiplier = 100;

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

    HyprlandAPI::addNotification(PHANDLE, "[HyprContexts] Initialized!", CHyprColor(0.0f, 1.0f, 0.0f, 1.0f), 5000);

    return {"HyprContexts", "Meta-layer workspace manager", "YourName", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    // Cleanup
}
