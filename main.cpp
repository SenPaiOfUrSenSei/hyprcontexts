#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>

inline HANDLE PHANDLE = nullptr;

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    HyprlandAPI::addNotification(PHANDLE, "[HyprContexts] Initialized!", CHyprColor(0.0f, 1.0f, 0.0f, 1.0f), 5000);

    return {"HyprContexts", "Meta-layer workspace manager", "YourName", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    // Cleanup
}
