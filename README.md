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
make
```

### Loading the Plugin

To load the plugin automatically on startup, add the following to the **top** of your `hyprland.conf`:

```hyprlang
plugin = /path/to/hyprcontexts.so
```

*Note: Using `plugin =` is recommended over `exec-once` because it ensures the plugin dispatchers are registered before Hyprland validates your keybindings.*

Alternatively, you can load it manually:
```bash
hyprctl plugin load $(pwd)/hyprcontexts.so
```

## Configuration

Add the following to your `hyprland.conf`:

```hyprlang
plugin {
    hyprcontexts {
        offset = 100
        # Optional: Run a command on context switch. %C will be replaced with context ID.
        # on_switch = /path/to/script.sh %C
    }
}
```

### Isolating Waybar (Separate Environments)

If you want Waybar to only show workspaces for the current context, you can use the `on_switch` hook to update Waybar's configuration.

1.  **Create a script** (e.g., `~/.config/hypr/scripts/waybar-isolate.sh`):

    ```bash
    #!/bin/bash
    CONTEXT=$1
    # Update Waybar config to ignore other contexts.
    # This assumes your workspaces are named like C0:1, C1:1, etc.
    # We want to ignore everything EXCEPT the current context.
    sed -i "s/\"ignore-workspaces\": \[.*\]/\"ignore-workspaces\": [\"^C[^${CONTEXT}]:.*\"]/" ~/.config/waybar/config.jsonc
    # Signal Waybar to reload its config
    killall -SIGUSR2 waybar
    ```

2.  **Add to your `hyprland.conf`**:

    ```hyprlang
    plugin {
        hyprcontexts {
            on_switch = ~/.config/hypr/scripts/waybar-isolate.sh %C
        }
    }
    ```

### Keybindings

See `hyprland.conf.example` for recommended keybindings.

Available dispatchers:
- `hyprcontexts:switch [ID]`: Switch to a specific context (0, 1, 2, ...).
- `hyprcontexts:workspace [ID]`: Switch to a workspace within the current context.
- `hyprcontexts:movetoworkspace [ID]`: Move the active window to a workspace within the current context.
- `hyprcontexts:cycle [next/prev]`: Smart cycling within the current context.

### Smart Cycling Logic
The `hyprcontexts:cycle` dispatcher is designed to feel like native workspace swiping but isolated to your current context layer:
1. It automatically finds the next/previous workspace that has windows on it, even if those windows are hidden or unmapped.
2. If no populated workspaces exist in that direction, it falls back to gracefully moving one workspace forward/backward (e.g., from 1 to 2).
3. It strictly enforces context boundaries, preventing you from accidentally swiping into the next context (e.g., stopping at 99).

### Configuring Touchpad Swipes
To use 3-finger touchpad swipes to cycle through your context, add these native gesture binds to your Hyprland configuration (often in `input.conf` or `hyprland.conf` depending on your setup):

```hyprlang
gesture = 3, left, dispatcher, hyprcontexts:cycle, prev
gesture = 3, right, dispatcher, hyprcontexts:cycle, next
```

Example basic usage in `hyprland.conf`:
`bind = SUPER, 1, hyprcontexts:workspace, 1`
