# HyprContexts Design Specification

## Overview
HyprContexts is a Hyprland plugin that provides a meta-layer for workspace management. It allows users to group workspaces into "contexts" using numeric offsets and dynamic renaming.

## Core Logic
### Offset Mathematics
- `Target Workspace ID = (currentContext * offsetMultiplier) + Workspace Number`
- Default `offsetMultiplier`: 100.
- `currentContext`: Integer (0-N).

## Components

### 1. Global State
- `int g_iCurrentContext`: Tracks the active context.
- `int g_iOffsetMultiplier`: Configurable multiplier (default 100).

### 2. Configuration
- `plugin:hyprcontexts:offset`: Configures the multiplier.

### 3. Custom Dispatchers
- `plugin:context:switch [ID]`: 
  1. Updates `g_iCurrentContext`.
  2. Calculates new workspace: `(newID * offset) + (currentWorkspaceID % offset)`.
  3. Moves user to new workspace.
- `plugin:context:workspace [num]`: 
  1. Calculates `target = (g_iCurrentContext * offset) + num`.
  2. Calls native `workspace` dispatcher with `target`.
- `plugin:context:movetoworkspace [num]`: 
  1. Calculates `target = (g_iCurrentContext * offset) + num`.
  2. Calls native `movetoworkspace` dispatcher with `target`.

### 4. Dynamic Naming Hook
- **Hook:** `createWorkspace`.
- **Logic:**
  - Extract `ID`.
  - Calculate `contextNum = ID / offset`.
  - Calculate `wsNum = ID % offset`.
  - Set `workspace->m_szName = "C" + contextNum + ":" + wsNum`.

## Error Handling
- Validate dispatcher arguments as valid integers.
- Handle cases where current workspace ID might be outside the expected range (e.g., special workspaces).

## Testing Strategy
1. **Unit tests for math:** Verify offset calculations for various inputs.
2. **Integration tests:** Mock Hyprland API calls for dispatchers and naming hooks.
3. **Manual verification:** Load plugin in Hyprland and test context switching/workspace navigation.
