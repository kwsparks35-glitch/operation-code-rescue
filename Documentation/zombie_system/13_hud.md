# Item 13 — HUD pass

**Status:** DONE — extended `UCodeRescueHUDWidget` with crosshair-hover
coloring and a centered interaction prompt below the crosshair. The
existing top-left status text (health/ammo/medkits/language/survivors/
zombies/score) is unchanged.

## What landed

`UCodeRescueHUDWidget` gained one new member:

```cpp
UTextBlock* InteractionPromptText;
```

Constructed below the existing crosshair, centered horizontally. The
`RefreshHUD()` per-frame body (called from `NativeTick`) now does a
600-unit forward line trace from the camera and:

- Colors the crosshair by what's hit:
  - **Red** for `ACodeZombieActor` (hostile)
  - **Yellow** for `ACodingTerminalActor` (interact)
  - **Cyan** for `ASurvivorActor` (friend)
  - **Light green** for `APickupActor`
  - **Purple** for `ALanguageStationActor`
  - **Default green** when nothing relevant is in range
- Sets the prompt text to a hint string per type, e.g.
  `"[E] open coding terminal"`, blank when nothing relevant.

The trace is single-line and runs once per frame — cheap.

## Why the crosshair re-color matters

In a busy scene with 30+ zombies, terminals, survivors, and pickups, the
player's primary signal for "what am I about to interact with" is the
crosshair color. Without this, players have to read the on-screen status
or gamble on pressing E.

## Files touched

- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h` — new field
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp` — extra construct
  block + trace logic in `RefreshHUD`; new includes for actor types
  and `Camera/CameraComponent.h`

## How to verify

PIE. Aim at:

- A zombie → crosshair red, no prompt.
- A terminal → crosshair yellow, prompt "[E] open coding terminal".
- A survivor → crosshair cyan, prompt "[E] rescue survivor".
- An ammo/medkit → crosshair light-green, prompt "[E] grab supplies".
- A language station → crosshair purple, prompt "[E] swap programming language".
- Empty space → crosshair default green, no prompt.

## Open polish

- The status text is still a single multi-line block. A future polish
  pass would split it into individual widgets (health bar instead of
  text, ammo with icon, language pill with color matching the station
  the player chose).
- `InteractionPromptText` could pulse opacity to draw the eye when an
  interactable first comes into focus. Easy add via
  `UWidgetAnimation` later.
