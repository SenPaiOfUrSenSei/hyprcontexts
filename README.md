# HyprContexts

A Hyprland plugin for meta-layer workspace management. It allows you to have multiple "contexts" (sets of workspaces), effectively multiplying your available workspaces by switching between these contexts.

## How it works

HyprContexts uses a configurable `offset` to shift workspace IDs. For example, if your offset is 100:
- Context 0: Workspaces 1-99
- Context 1: Workspaces 101-199
- Context 2: Workspaces 201-299

## Build

```bash
make
```

## Install

```bash
hyprctl plugin load $(pwd)/hyprcontexts.so
```

## Configuration

Add the following to your `hyprland.conf`:

```hyprlang
plugin {
    hyprcontexts {
        offset = 100
    }
}
```

### Keybindings

See `hyprland.conf.example` for recommended keybindings.

Available dispatchers:
- `plugin:context:switch [ID]`: Switch to a specific context (0, 1, 2, ...).
- `plugin:context:workspace [ID]`: Switch to a workspace within the current context.
- `plugin:context:movetoworkspace [ID]`: Move the active window to a workspace within the current context.
