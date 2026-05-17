# HyprContexts Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the HyprContexts Hyprland plugin to manage independent workspace contexts.

**Architecture:** A C++ plugin using Hyprland's Plugin API. It uses a global `g_iCurrentContext` and `g_iOffsetMultiplier` to calculate workspace IDs. It hooks into `createWorkspace` to rename workspaces with a "CX:Y" format and provides custom dispatchers for context switching and navigation.

**Tech Stack:** C++, Hyprland Plugin API, Makefile.

---

### Task 1: Basic Structure & Makefile

**Files:**
- Create: `main.cpp`
- Create: `Makefile`

- [ ] **Step 1: Create initial main.cpp with boilerplate**

```cpp
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>

inline HANDLE PHANDLE = nullptr;

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    HyprlandAPI::addNotification(PHANDLE, "[HyprContexts] Initialized!", CColor(0, 255, 0, 255), 5000);

    return {"HyprContexts", "Meta-layer workspace manager", "YourName", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    // Cleanup
}
```

- [ ] **Step 2: Create Makefile**

```makefile
PLUGIN_NAME=hyprcontexts

all:
	g++ -shared -fPIC --optimize=3 -I/usr/include/hyprland -std=c++23 main.cpp -o ${PLUGIN_NAME}.so `pkg-config --cflags pixman-1 libdrm hyprlang`

clean:
	rm -f ${PLUGIN_NAME}.so
```

- [ ] **Step 3: Verify compilation**

Run: `make`
Expected: `hyprcontexts.so` exists.

- [ ] **Step 4: Commit**

```bash
git add main.cpp Makefile
git commit -m "chore: initial plugin structure and makefile"
```

---

### Task 2: Global State & Config

**Files:**
- Modify: `main.cpp`

- [ ] **Step 1: Add global state and config registration**

```cpp
// Add to top of main.cpp after includes
int g_iCurrentContext = 0;
int g_iOffsetMultiplier = 100;

// Add to PLUGIN_INIT
HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprcontexts:offset", Hyprlang::INT{100});
HyprlandAPI::reloadConfig();
g_iOffsetMultiplier = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprcontexts:offset")->getValue());
```

- [ ] **Step 2: Update global variable on config reload**

```cpp
void onConfigReload(void* self, std::any data) {
    g_iOffsetMultiplier = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprcontexts:offset")->getValue());
}

// In PLUGIN_INIT
HyprlandAPI::registerCallbackDynamic(PHANDLE, "configReload", onConfigReload);
```

- [ ] **Step 3: Commit**

```bash
git add main.cpp
git commit -m "feat: add global state and configuration"
```

---

### Task 3: Custom Dispatchers

**Files:**
- Modify: `main.cpp`

- [ ] **Step 1: Implement context switch dispatcher**

```cpp
void switchContext(std::string arg) {
    try {
        int newContext = std::stoi(arg);
        if (newContext == g_iCurrentContext) return;

        CWorkspace* pWorkspace = g_pCompositor->m_pLastMonitor->activeWorkspace;
        if (!pWorkspace) return;

        int currentWSNum = pWorkspace->m_iID % g_iOffsetMultiplier;
        g_iCurrentContext = newContext;
        
        int targetWS = (g_iCurrentContext * g_iOffsetMultiplier) + currentWSNum;
        
        // Use native workspace dispatcher
        g_pKeybindManager->m_mDispatchers["workspace"](std::to_string(targetWS));
        
        HyprlandAPI::addNotification(PHANDLE, "[HyprContexts] Switched to context " + std::to_string(g_iCurrentContext), CColor(0, 255, 0, 255), 2000);
    } catch (...) {
        // Handle conversion error
    }
}
```

- [ ] **Step 2: Implement context navigation dispatchers**

```cpp
void contextWorkspace(std::string arg) {
    try {
        int wsNum = std::stoi(arg);
        int target = (g_iCurrentContext * g_iOffsetMultiplier) + wsNum;
        g_pKeybindManager->m_mDispatchers["workspace"](std::to_string(target));
    } catch (...) {}
}

void contextMoveToWorkspace(std::string arg) {
    try {
        int wsNum = std::stoi(arg);
        int target = (g_iCurrentContext * g_iOffsetMultiplier) + wsNum;
        g_pKeybindManager->m_mDispatchers["movetoworkspace"](std::to_string(target));
    } catch (...) {}
}
```

- [ ] **Step 3: Register dispatchers in PLUGIN_INIT**

```cpp
HyprlandAPI::addDispatcher(PHANDLE, "plugin:context:switch", switchContext);
HyprlandAPI::addDispatcher(PHANDLE, "plugin:context:workspace", contextWorkspace);
HyprlandAPI::addDispatcher(PHANDLE, "plugin:context:movetoworkspace", contextMoveToWorkspace);
```

- [ ] **Step 4: Commit**

```bash
git add main.cpp
git commit -m "feat: implement custom dispatchers"
```

---

### Task 4: Dynamic Naming Hook

**Files:**
- Modify: `main.cpp`

- [ ] **Step 1: Implement workspace creation callback**

```cpp
void onNewWorkspace(void* self, std::any data) {
    CWorkspace* pWorkspace = std::any_cast<CWorkspace*>(data);
    if (!pWorkspace || pWorkspace->m_iID <= 0) return;

    int contextNum = pWorkspace->m_iID / g_iOffsetMultiplier;
    int wsNum = pWorkspace->m_iID % g_iOffsetMultiplier;

    if (contextNum > 0 || pWorkspace->m_iID >= g_iOffsetMultiplier) {
        pWorkspace->m_szName = "C" + std::to_string(contextNum) + ":" + std::to_string(wsNum);
    }
}
```

- [ ] **Step 2: Register callback in PLUGIN_INIT**

```cpp
HyprlandAPI::registerCallbackDynamic(PHANDLE, "createWorkspace", onNewWorkspace);
```

- [ ] **Step 3: Commit**

```bash
git add main.cpp
git commit -m "feat: add dynamic workspace naming hook"
```

---

### Task 5: Documentation & Configuration

**Files:**
- Create: `hyprland.conf.example`
- Create: `README.md`

- [ ] **Step 1: Create configuration example**

```ini
plugin {
    hyprcontexts {
        offset = 100
    }
}

# Context switching
bind = SUPER CTRL, 1, plugin:context:switch, 0
bind = SUPER CTRL, 2, plugin:context:switch, 1
bind = SUPER CTRL, 3, plugin:context:switch, 2

# Workspace navigation
bind = SUPER, 1, plugin:context:workspace, 1
bind = SUPER, 2, plugin:context:workspace, 2
bind = SUPER, 3, plugin:context:workspace, 3

# Moving windows
bind = SUPER SHIFT, 1, plugin:context:movetoworkspace, 1
bind = SUPER SHIFT, 2, plugin:context:movetoworkspace, 2
bind = SUPER SHIFT, 3, plugin:context:movetoworkspace, 3
```

- [ ] **Step 2: Create README.md with build instructions**

```markdown
# HyprContexts

A Hyprland plugin for meta-layer workspace management.

## Build

```bash
make
```

## Install

```bash
hyprctl plugin load $(pwd)/hyprcontexts.so
```

## Configuration

See `hyprland.conf.example`.
```

- [ ] **Step 3: Commit**

```bash
git add hyprland.conf.example README.md
git commit -m "docs: add configuration example and readme"
```
