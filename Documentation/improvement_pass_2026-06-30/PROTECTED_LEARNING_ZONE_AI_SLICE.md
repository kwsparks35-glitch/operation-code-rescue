# Protected Learning Zone AI Slice

## Purpose

This slice completes the P0 protected coding safehouse and zombie safe-zone exclusion contract at runtime. The world already communicated that coding areas are protected, but the enemy systems still needed a shared enforceable rule so detection, chase, melee, elite abilities, and future direct zombie damage calls all respect the same learning-space boundary.

## Implementation

- Added `ACodeRescueGameMode::IsLocationInsideProtectedLearningZone`, a shared tagged-bounds query for protected learning spaces.
- The query recognizes existing world tags: `NoZombieLearningZone`, `ProtectedCodingChallengeZone`, `ProtectedLearningSpace`, `SafeTerminalLab`, and `BonusCodingChallengeSafeZone`.
- `SpawnTerminal` now tags each terminal actor as a protected coding anchor, so main and bonus terminals both participate in runtime protection.
- `ACodeRescueAIController` now stops movement and returns to patrol while the player is inside a protected learning zone, and visibility checks return false in that state.
- `ACodeZombieActor::Tick` now retreats or idles when the player is protected and exits before normal melee damage can fire.
- `ACodeZombieActor::TickEliteBehavior` and `OnBoomerDeath` now respect the same protection query.
- `ACodeRescueCharacter::ApplyDamage` now blocks zombie-sourced damage in protected learning zones as a final fail-safe.

## Player Impact

The player can enter the selected-language safehouse, terminal lab, or protected annex and focus on coding without hidden zombie damage leaking through the encounter systems. Combat still matters on the route outside the lab, but learning spaces now behave like actual protected classroom spaces instead of only being labeled that way.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescueAIController.h`
- `Source/CodeRescueUnreal/CodeRescueAIController.cpp`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Content/CodeRescueData/protected_learning_zone_ai_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Scripts/verify_protected_learning_zone_ai_slice_pass.py`
- `Scripts/verify_may27_safe_learning_city_controls_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- `python3 -m py_compile Scripts/verify_protected_learning_zone_ai_slice_pass.py Scripts/verify_may27_safe_learning_city_controls_pass.py`
- `python3 Scripts/verify_protected_learning_zone_ai_slice_pass.py`
- `python3 Scripts/verify_may27_safe_learning_city_controls_pass.py`
- Unreal commandlet: `Scripts/verify_runtime_step_smoke_contracts.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`

## Human QA Notes

Start a selected-language run, enter the protected safehouse, and lure or spawn a zombie near the terminal entrance. Confirm the zombie does not chase into the learning zone, does not apply melee or elite damage while the player is protected, and resumes normal pressure only after the player leaves for the city rescue route.
