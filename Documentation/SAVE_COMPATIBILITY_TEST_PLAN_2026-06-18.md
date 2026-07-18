# Save Compatibility Test Plan - 2026-06-18

## Automated Static Coverage

Run:

```bash
python3 Scripts/verify_save_compatibility_pass.py
```

The verifier checks:

- Explicit difficulty enum values preserve old Easy/Normal/Hard serialized
  values.
- Save back-compat flags are still present.
- The demo-readiness save schema is versioned as `0.8.0-demo-readiness`.
- New accessibility settings exist in SaveGame, GameInstance, and
  save/load serialization.

## Manual Runtime Coverage

1. Launch the packaged app.
2. Start a fresh New York run.
3. Change difficulty and accessibility settings.
4. Solve the New York terminal.
5. Rescue the survivor.
6. Neutralize at least one zombie.
7. Save through the pause menu.
8. Quit and relaunch.
9. Load the save.
10. Confirm difficulty, accessibility settings, terminal state, survivor state,
    zombie state, health, ammo, medkits, armor, score, and objective index.

## Back-Compat Expectations

- Older saves without new accessibility fields should load with default
  subtitle scale `1.0`, standard contrast, standard motion, full hints, and
  normal aim assist.
- Older saves with Easy/Normal/Hard difficulty values should keep the same
  meaning because those enum values remain `0`, `1`, and `2`.
