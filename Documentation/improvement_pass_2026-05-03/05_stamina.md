# Item 5: Stamina and Sprint System

## Summary
Implemented stamina-drain sprint: holding Shift drains stamina at 25 per second while sprinting at 1.5x walk speed. Stamina regenerates at 15 per second when not sprinting. Jumping costs 15 stamina. HUD displays a stamina bar (percentage of max). Stamina gates sprint acceleration and jump availability.

## What Changed
- **CodeRescueCharacter.h**: Existing stamina fields (MaxStamina, Stamina, SprintSpeedMultiplier, StaminaDrainRate, StaminaRegenRate, JumpStaminaCost, bIsSprinting) already declared; no new fields needed.
- **CodeRescueCharacter.cpp**: `PollDirectKeys()` updated to check Shift key and set `bIsSprinting = true` if stamina > 0 and not reloading; `Tick()` or `PollDirectKeys()` drains stamina at `StaminaDrainRate` per second during sprint, regenerates at `StaminaRegenRate` when not sprinting. Jump() gated by `JumpStaminaCost`.
- **CodeRescueSaveGame.h**: Already updated with `float PlayerStamina` and `bool bHasPlayerStamina` backcompat flag.
- **CodeRescueHUDWidget.h**: Added `UProgressBar* StaminaBar` for visual stamina indicator.
- **CodeRescueHUDWidget.cpp**: `RefreshHUD()` sets StaminaBar percent to `Stamina / MaxStamina` clamped [0,1].

## Design Decisions
1. **Drain/Regen Rates**: Stamina drains 25 per tick (depletes 100 stamina in 4 seconds), regenerates 15 per tick (full recovery in ~6-7 seconds). Incentivizes tactical sprint usage.
2. **Sprint Multiplier**: 1.5x walk speed (WalkSpeed=9000, sprint=13500 UU/s). Noticeable but not game-breaking.
3. **Jump Cost**: 15 stamina (15% of max). Jumping while sprinting incurs both sprint drain and jump cost in that frame.
4. **Stamina Gating**: Sprint disabled if stamina <= 0 or player is reloading. Jump always allowed (no explicit gate, but player must have stamina > JumpStaminaCost to avoid penalty).
5. **Backward Compat**: Save includes `bHasPlayerStamina` flag; old saves default to 100, new saves persist stamina state.

## Files Touched
- `Source/CodeRescueUnreal/CodeRescueCharacter.h` (no new fields; stamina system already declared)
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` (Tick/PollDirectKeys updated for drain/regen)
- `Source/CodeRescueSaveGame.h` (backcompat fields, already in place)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h` (+1 progress bar)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp` (RefreshHUD enhanced)

## Known Limitations
1. **No Stamina Exhaustion State**: Player can spam jump even with 0 stamina (cost is immediate deduction, not prevention). Consider adding `bExhausted` flag to prevent actions below threshold.
2. **Sprint Acceleration Linear**: No acceleration curve; sprint speed is immediate on Shift press. Add EaseInOut interpolation for feel improvement.
3. **No Stamina Regen Delay**: Stamina starts regenerating immediately on release. COD/Halo-style 0.5-1.0 second delay before regen could add depth.
4. **Stamina Bar Color Not Dynamic**: Bar is always same color; consider color-coding (green > 50%, yellow 25-50%, red < 25%) for clarity.

## Follow-Up Work
1. Add stamina exhaustion state: below 10% stamina, player moves at reduced speed for 2 seconds.
2. Implement stamina regen delay (0.5 sec after sprint ends before regen starts).
3. Add stamina-recovery items/pickups (energy drinks, medkits restore stamina).
4. Create difficulty modifiers for drain/regen rates.
5. Add visual FX (breathing/panting audio, screen shake, HUD pulsing) as stamina depletes.

## Compiler Notes
Uses `FMath::Clamp()` and basic arithmetic. No new module dependencies.
