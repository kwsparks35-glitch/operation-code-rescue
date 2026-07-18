# Stealth Avoidance Slice

This pass implements Top 50 recommendation 40, the stealth/avoidance option. Before this slice, ordinary zombies could commit to chase from proximity alone, so the player had limited room to avoid pressure through careful movement. The new behavior gives visibility and player-created noise a real role while preserving protected coding spaces and the selected-language save loop.

## Runtime Changes

- Added player-side stealth noise state to `ACodeRescueCharacter`.
- Movement, sprinting, weapon fire, melee, reloads, throwables, flashlight use, and radio scanner use now raise a short-lived `StealthNoiseLevel` and noise radius.
- Added `GetStealthNoiseRadius()` and `GetStealthStateSummary()` so AI and HUD can read the same gameplay state.
- Updated zombie AI to detect by sight through `AISightTrace` and by short-lived player noise, using last-known location investigation before chase.
- Reduced direct proximity-only chase: a nearby zombie now needs sight or noise to commit, while protected learning zones still suppress detection.
- Added HUD tactical readout text for `Stealth QUIET`, `LOW NOISE`, `AUDIBLE`, `NOISY`, or `EXPOSED`.
- Tagged participating actors with `Top50Recommendation40StealthAvoidance`, `StealthAvoidanceRuntime`, `StealthAvoidanceNoiseEmitter`, `StealthAvoidanceNoiseListener`, and related review tags.

## Player-Facing Result

The player can now move carefully around combat pressure instead of always being pulled into a direct fight. Sprinting, firing, scanning, using the flashlight, or throwing gear advertises the player's position, while slower movement and broken line of sight create an avoidance route. This keeps combat readable without letting stealth override the core rescue loop: coding spaces remain protected, route objectives remain intact, and selected-language progression is untouched.

## Files

- `Source/CodeRescueUnreal/CodeRescueCharacter.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueAIController.h`
- `Source/CodeRescueUnreal/CodeRescueAIController.cpp`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Content/CodeRescueData/stealth_avoidance_manifest.tsv`
- `Scripts/verify_stealth_avoidance_slice_pass.py`

## Validation

Static verifier:

```bash
python3 Scripts/verify_stealth_avoidance_slice_pass.py
```

Adjacent verifier coverage should include standard direct pursuit, protected learning zones, threat compass, tactical pickups, combat juice, and selected-language terminal flow.

## Manual QA

Walk a city route without sprinting and keep solid cover between the player and a standard zombie; the HUD should report quiet or low-noise stealth and the zombie should not immediately chase from proximity alone. Sprint, fire a weapon, use the scanner, toggle the flashlight, and throw a utility item near zombies; the stealth readout should rise and nearby zombies should investigate or chase depending on sight and noise. Enter a protected coding space while noisy and confirm terminal safety remains intact.
