# Item 7 — Helicopter fast-travel between solved cities

## What changed
- New `AHelipadActor`: 6m flat cylinder + blue point light + "Helipad" tag,
  spawned per city by `ACodeRescueGameMode::SpawnHelipadForCity` at offset
  `(2400, 2400, -10)` from city origin (post-50× scaling).
- New `UCityFastTravelWidget`: modal menu listing every city whose terminal
  is in `UCodeRescueGameInstance::SolvedTerminalIds`. Selecting a city
  teleports the player to that city's `FCodeRescueCampaign::GetPlayerStartLocation`
  and triggers a 0.4s camera fade.
- `ACodeRescueCharacter::Interact` now detects helipads via the "Helipad"
  tag (no header dependency) and calls `OpenFastTravelMenu` reflectively
  via `FindFunction` + `ProcessEvent`.

## Files touched
- `Source/CodeRescueUnreal/HelipadActor.h/.cpp` (new)
- `Source/CodeRescueUnreal/CityFastTravelWidget.h/.cpp` (new)
- `Source/CodeRescueUnreal/CodeRescueGameMode.h/.cpp` — `SpawnHelipadForCity`
  helper called from `SpawnCampaignCity`.
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` — interact-by-tag dispatch.

## Design decisions
- Helipad is a separate actor class rather than a tag-on-existing-mesh because
  it owns its widget reference + click handler.
- City list is filtered by saved-terminal IDs so the player can never warp
  to a city they haven't graduated. Empty solved list shows a yellow
  "no cities cleared yet" hint instead of an empty box.
- Click dispatch in `OnDestinationClicked` uses `IsHovered` to identify which
  button fired (UMG dynamic-delegates don't pass sender info). Falls back to
  the first button in the map if hover detection fails.
- Helipad placement is deterministic — same city always gets the helipad in
  the same spot. The author can override visuals by setting
  `FastTravelWidgetClass` on a Blueprint subclass.

## Known limitations
- No teleport sound effect.
- Dispatch via `FindFunction`+`ProcessEvent` works but loses compile-time
  type safety. If helipad interaction grows beyond one method, switch to a
  proper interface (UInterface + IHelipadInteract).
- Camera fade-in only; player still teleports while the screen is fading
  (UE camera fade doesn't naturally pause time). Acceptable for v1.

## Follow-up work
- Authored helipad mesh (Quixel pad asset).
- 2.0s teleport delay with a "boarding helicopter" sound + fade rather than
  instant teleport.
- Show a tiny preview thumbnail per destination city.
