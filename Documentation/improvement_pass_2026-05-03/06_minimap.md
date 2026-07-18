# Item 6: Minimap Widget with POI Pings

## Summary
Built a minimap widget (CodeRescueMinimapWidget) that displays the player (white dot) and points of interest (POIs) with color-coded markers: terminals (yellow), survivors (cyan), stations (purple), pickups (green), zombies (red). Minimap refreshes every frame, showing player position and heading.

## What Changed
- **NEW: CodeRescueMinimapWidget.h**: UUserWidget subclass with NativeConstruct, NativeTick, RefreshMinimap functions; private canvas, zoom level, POI data structures.
- **NEW: CodeRescueMinimapWidget.cpp**: RefreshMinimap() queries world for terminals, survivors, stations, pickups, zombies via TActorIterator; draws on canvas with POI-specific colors; player at center of minimap.
- **CodeRescueCharacter.h**: No changes (minimap integrated into HUD widget hierarchy).
- **CodeRescueHUDWidget.h**: Added `CodeRescueMinimapWidget* MinimapWidget` child widget reference.
- **CodeRescueHUDWidget.cpp**: NativeConstruct creates MinimapWidget child; RefreshHUD calls MinimapWidget->RefreshMinimap() every frame.

## Design Decisions
1. **Color Coding**: Terminal=yellow, Survivor=cyan, Station=purple, Pickup=green, Zombie=red, Player=white. Distinct enough for at-a-glance recognition.
2. **Zoom**: Minimap shows ~5000 UU radius around player (configurable). Scales larger on harder difficulties.
3. **Canvas Rendering**: Direct FCanvas draw calls (no Slate complexity). Supports fast iterative updates.
4. **Real-Time Updates**: Refresh every Tick for smooth tracking. No culling; all POIs rendered (performance optimization deferred to item 20).
5. **Player Heading**: Player white dot + white line showing forward facing for orientation.

## Files Touched
- **NEW: Source/CodeRescueUnreal/CodeRescueMinimapWidget.h** (new file)
- **NEW: Source/CodeRescueUnreal/CodeRescueMinimapWidget.cpp** (new file)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h` (+1 minimap child)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp` (integrated minimap refresh)

## Known Limitations
1. **No Fog of War**: All POIs visible regardless of whether player has discovered them. Restrict visibility to explored zones (future work).
2. **No Dynamic Scaling**: Minimap zoom fixed; no pinch-to-zoom or scroll wheel support.
3. **Zombie Pings Update Slowly**: Zombie positions queried every Tick; high zombie count may cause framerate hitches (Item 20 performance pass will optimize).
4. **No Legend**: Minimap has no on-screen legend explaining color codes. Tooltip or settings screen needed.

## Follow-Up Work
1. Implement fog-of-war: only show POIs in discovered zones.
2. Add legend UI overlay (small HUD element explaining colors).
3. Add click-to-navigate: player can click minimap POI to set waypoint.
4. Create difficulty-based minimap blurriness (hard mode: blurry minimap).
5. Implement minimap toggle (M key to show/hide).

## Compiler Notes
Requires `Slate` and `SlateCore` modules for UCanvas rendering. Already included in standard Unreal builds.
