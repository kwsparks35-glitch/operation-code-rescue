# Item 16 — Settings menu widget

## What changed
New `UCodeRescueSettingsWidget` — a modal settings panel intended to be
opened from the pause menu. Provides:

| Control | Range | Persistence |
|---|---|---|
| Master volume | 0.0 – 1.0 | `FAudioDevice::SetTransientPrimaryVolume` |
| SFX volume | 0.0 – 1.0 | (cached only — needs Sound Mix asset) |
| Music volume | 0.0 – 1.0 | (cached only — needs Sound Mix asset) |
| Mouse sensitivity | 0.25 – 3.0 | `Player->DirectKeyboardTurnRate/LookRate` |
| FOV | 60 – 110 | `UCameraComponent::SetFieldOfView` |
| Fullscreen toggle | bool | `UGameUserSettings::SetFullscreenMode` |
| VSync toggle | bool | `UGameUserSettings::SetVSyncEnabled` |

Apply button writes the GameUserSettings + character + camera in one shot.
Close button returns input to game.

## Files touched
- `Source/CodeRescueUnreal/CodeRescueSettingsWidget.h/.cpp` (new)

## Wiring required (one-line in pause widget)
The pause widget needs a "Settings" button that does:
```cpp
if (UCodeRescueSettingsWidget* W = CreateWidget<UCodeRescueSettingsWidget>(PC, UCodeRescueSettingsWidget::StaticClass()))
{
    W->AddToViewport(150);
}
```
This is intentionally not auto-wired — the pause widget design wasn't
modified to keep the diff small. Add the button in your next pause-menu
edit.

## Design decisions
- Sliders cache values, Apply commits. Lets the player dial without
  every frame triggering a window resize.
- Mouse sensitivity rescales the existing `DirectKeyboardTurnRate` /
  `DirectKeyboardLookRate` rather than a true mouse-axis multiplier
  because this codebase uses polled-key input, not bound mouse axes.

## Known limitations
- SFX and Music sliders cache values but don't actually route to a Sound
  Mix Modifier — that asset doesn't exist yet. Master volume works via
  the audio device's transient primary volume.
- Settings aren't restored on next launch automatically — that's tied to
  `UGameUserSettings::SaveSettings()` which the user can wire as part of
  the editor save flow.

## Follow-up work
- Author a `SoundMix_Master` asset with three sound classes (SFX, Music,
  UI) and route each slider through `SetSoundMixClassOverride`.
- Wire the pause-menu Settings button.
- Add controller deadzone / inversion if controllers come in.
