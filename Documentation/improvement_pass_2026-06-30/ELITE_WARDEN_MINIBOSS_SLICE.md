# Elite Warden Mini-Boss Slice

This pass implements the P1 enemy request for elite warden and mini-boss staging as a save-aware combat layer around the existing city boss. The project already had `ABossZombieActor`, boss reveal presentation, and boss phase telegraphs; this slice adds the missing milestone bridge between terminal intel and optional boss pressure.

## Runtime Implementation

- Added `SpawnEliteWardenMiniBossStagingLayer` and call it immediately after `SpawnBossForCity` for non-sandbox campaign cities.
- The layer reads `SolvedTerminalIds.Contains(Mission.TerminalId)` and `NeutralizedZombieIds.Contains(BossId)` to present three states:
  - Dormant until terminal intel
  - Active after terminal intel
  - Warden defeated
- It creates three text-first runway anchors:
  - Intel Lock Gate
  - Mini-Boss Sentinel Lane
  - Warden Phase Gate
- Before intel, the layer displays preview silhouettes and warning labels only.
- After intel, it spawns save-aware charger, spitter, and boomer mini-boss sentinels with stable IDs, variant recording, encounter-director roles, and dispatch subtitle feedback.

## Gameplay Behavior

The sentinels use existing elite variants:

- `EliteCharger` as the pressure mini-boss
- `EliteSpitter` as the ranged sentinel mini-boss
- `EliteBoomer` as the anchor/area-denial mini-boss

Each mini-boss is registered as a streamed actor, uses `ConfigureEncounterDirective`, records its variant through `RecordZombieVariant`, and respects `NeutralizedZombieIds` on reload. The staging geometry is nonblocking and tagged for review, so it does not interfere with the existing boss reveal or phase telegraph actors.

## Tags and Review Hooks

The layer uses:

- `EliteWardenMiniBossStaging`
- `EliteWardenPressureGate`
- `MiniBossAfterIntelMilestone`
- `TextFirstEnemyReadability`
- `NoAccessBlocker`
- `CharacterAnimationDeepDive`
- `Top50Recommendations`

Active sentinels also receive `EliteWardenMiniBoss`, `WardenRunwaySentinel`, and role-specific mini-boss tags. Runtime smoke logs include `[CodeRescueEliteWardenMiniBoss]` with the state, intel flag, boss-defeated flag, spawned mini-boss count, and terminal ID.

## Data and QA Updates

- Added `Content/CodeRescueData/elite_warden_miniboss_manifest.tsv`.
- Updated the creative inclusion plan, enemy readability manifest, visual-regression targets, human QA checklist, and accessibility manifest.
- Added `Scripts/verify_elite_warden_miniboss_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.

## Validation

Required validation for this slice:

- `python3 -m py_compile Scripts/verify_elite_warden_miniboss_slice_pass.py`
- `python3 Scripts/verify_elite_warden_miniboss_slice_pass.py`
- `python3 Scripts/verify_boss_reveal_presentation_slice_pass.py`
- `python3 Scripts/verify_boss_phase_telegraph_slice_pass.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`
- Runtime log confirmation of `[CodeRescueEliteWardenMiniBoss]`
- `git diff --check`
- Touched-file trailing-whitespace scan
