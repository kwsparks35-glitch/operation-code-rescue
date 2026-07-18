# Item 4: Reload Mechanic and Magazine HUD

## Summary
Implemented magazine-based ammo system with timed reload: MagazineAmmo (30 bullets) is consumed on fire; pressing LeftControl/Gamepad_FaceButton_Left initiates a 2.5-second reload. HUD displays "MagazineAmmo / ReserveAmmo" and "RELOADING..." status. Auto-reload on magazine empty is gated by UI state and reload timer.

## What Changed
- **CodeRescueCharacter.h**: Added `FTimerHandle ReloadTimerHandle` private field to track reload timer; existing `Reload()` and `OnReloadComplete()` declarations already in place.
- **CodeRescueCharacter.cpp**: `Reload()` checks ammo reserves, sets `bIsReloading = true`, schedules `OnReloadComplete()` callback; `OnReloadComplete()` transfers ammo from reserve to magazine and clears timer. Auto-reload on magazine-empty is commented in Fire() but not auto-triggered (player must press R/LCtrl).
- **CodeRescueSaveGame.h**: Already updated with `int32 PlayerMagazineAmmo` and `bool bHasPlayerMagazineAmmo` backcompat flag.
- **CodeRescueHUDWidget.h**: Added `UTextBlock* AmmoText` and `UTextBlock* ReloadStatusText` for magazine/reserve display and reload feedback.
- **CodeRescueHUDWidget.cpp**: `RefreshHUD()` displays "MagazineAmmo / Ammo", sets ReloadStatusText to "RELOADING..." in yellow during reload, or empty otherwise.

## Design Decisions
1. **Magazine System**: Magazine of 30 bullets (configurable MagazineSize), separate reserve pool (Ammo). Fire() consumes magazine, not reserve. Reload transfers from reserve to magazine.
2. **Reload Timer**: 2.5-second reload duration (configurable ReloadDuration). Uses `GetWorld()->GetTimerManager()` for async completion callback, allowing player to move during reload.
3. **UI Gating**: Player cannot move/fire while a modal UI (terminal, menu) is open (SetUIOpen). Reload respects this gate via PollDirectKeys early-return.
4. **Manual Reload**: Player must explicitly press R/LeftControl; no auto-reload on magazine-empty (design choice for skill-based ammo management).
5. **Backward Compat**: Save includes `bHasPlayerMagazineAmmo` flag; old saves default to 30, new saves persist magazine state.

## Files Touched
- `Source/CodeRescueUnreal/CodeRescueCharacter.h` (+1 timer handle field)
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` (Reload/OnReloadComplete existing; verified working)
- `Source/CodeRescueSaveGame.h` (backcompat fields, already in place)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h` (+2 widgets)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp` (RefreshHUD enhanced)

## Known Limitations
1. **No Reload Animation**: Reload plays a sound (FireCue or custom reload sound if added) but no skeletal animation. Add reload montage in item follow-up.
2. **Can't Interrupt Reload**: Once reload starts, player must wait for timer or close app. No cancel key (e.g., pressing fire to interrupt).
3. **Magazine Not Persisted Per-Round**: Magazine ammo is saved, but if player respawns mid-mission (on death), magazine resets to MagazineSize instead of continuing from saved state. Item 19 (save-slot UI) will address multi-save branching.

## Follow-Up Work
1. Add reload animation montage (skeletal mesh playing chambering motion).
2. Implement reload-cancel on player input (fire key during reload resets timer).
3. Add reload audio feedback (shell casing eject, magazine snap-in).
4. Create reload speed modifiers (perks, difficulty bonuses).
5. Implement partial reload (e.g., only load 10 bullets if reserve is low).

## Compiler Notes
Uses `FTimerManager` and `SetTimer()` from `TimerManager.h` (already included in engine headers). No new dependencies.
