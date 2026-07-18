# Item 17 — 60-second guided tutorial

## What changed
New `UCodeRescueTutorialWidget` — a 5-page modal overlay shown on first
launch. Pages cover:
1. Premise + how to advance
2. Movement (WASD, mouse, sprint, jump)
3. Combat (LMB fire, R reload, headshot bonus)
4. Interaction (E, T, J)
5. The rescue loop (city flow)

Player advances with **Space / Enter / [Next]**, skips with **Esc / [Skip]**.
Final page (or skip) writes `GI->bHasShownTutorial = true` and persists
via `SavePersistentRun()` so subsequent launches don't reshow.

`ACodeRescueGameMode::BeginPlay` checks the flag and spawns the widget
when false (z-order 2000 so it sits over the HUD).

## Files touched
- `Source/CodeRescueUnreal/CodeRescueTutorialWidget.h/.cpp` (new)
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h/.cpp` — `bHasShownTutorial`
  field + serialization.
- `Source/CodeRescueUnreal/CodeRescueSaveGame.h` — `bHasShownTutorial` field.
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` — spawn at BeginPlay.

## Design decisions
- Pages stored in a static `TArray<FString>` inside the .cpp so editing
  copy doesn't require touching the widget structure.
- `SetUIOpen(true)` ensures polled gameplay keys don't fire while the
  tutorial is up. UI input mode set in `NativeConstruct` so [Esc]/[Space]
  reach the widget instead of the player controller.
- "Skip" treated identically to "advance to end" — both end up at `Finish()`
  and set the flag.

## Known limitations
- Pure text overlay — no in-world arrows pointing at the terminal/etc.
  An interactive walkthrough is the natural follow-up.
- Doesn't gate gameplay; the player can dismiss and immediately start
  shooting. We rely on `SetUIOpen` to silence input only while open.

## Follow-up work
- Page-specific in-world arrows (Niagara billboards) pointing at the
  thing the page describes.
- Localized strings (move pages out of the .cpp into a string table).
- "Replay tutorial" button in pause menu for new playtesters.
