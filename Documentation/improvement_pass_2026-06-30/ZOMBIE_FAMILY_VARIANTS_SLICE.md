# Zombie Family Variants Slice

## Goal

Continue the June 25 character-animation, game-physics, world-development, and top-50 recommendation guidance by making zombie families readable in live play instead of only existing as asset-table rows.

## Runtime Implementation

- Added `ApplyZombieFamilyVariant` and `ApplyCityZombieFamilyVariant` to `ACodeRescueGameMode`.
- Centralized variant initialization, family tags, and save-record behavior for regular city waves, physics ambushes, encounter-director enemies, bosses, elite mini-bosses, dog dens, language breach patrols, and terminal-solve hordes.
- Kept regular city enemies, authored set-piece threats, language breach patrols, dog dens, and elite mini-bosses save-backed through `RecordZombieVariant`.
- Kept terminal-solve horde waves, boomer death adds, and boss phase adds transient so temporary combat pressure does not bloat save data.
- Added family audit tags such as `ZombieFamily_DogZombie`, `ZombieFamily_UrbanZombie`, `ZombieFamily_Bloated`, `ZombieFamily_Nurse`, `ZombieFamily_EliteCharger`, and `ZombieFamily_Default`.
- Updated visible markers so players can read the family label in normal waves, horde waves, physics ambushes, encounter-director roles, language breach patrols, dog dens, bosses, and mini-boss sentinels.
- Added local family tags to boomer death adds and boss phase adds, which spawn outside the GameMode helper path.

## Data And Documentation

- Added `Content/CodeRescueData/zombie_family_variants_manifest.tsv`.
- Updated:
  - `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
  - `Content/CodeRescueData/enemy_readability_manifest.tsv`
  - `Content/CodeRescueData/animation_coverage_manifest.tsv`
  - `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
  - `Content/CodeRescueData/visual_regression_targets.tsv`
  - `Run_Full_QA_Audit.command`
  - `Run_Local_CI_Readiness.command`
  - `progress.md`

## Validation

Added `Scripts/verify_zombie_family_variants_slice_pass.py`.

The verifier checks:

- shared helper declarations and implementations
- weighted city selection and fixed authored variant paths
- persistence boundaries for saved versus transient threats
- family audit tags for default, dog, urban, business, bloated, nurse, and elite variants
- marker label/color helpers and marker tag coverage
- normal wave, physics lane, encounter director, boss, mini-boss, dog den, language breach, horde, boomer add, and boss phase add coverage
- manifest, creative plan, enemy readability, animation coverage, visual target, human QA, progress, and QA script wiring

## Future Art Pass

This slice makes the current assets and fallback markers playable and auditable. Future asset work should replace fallback silhouettes with fully authored skeletal meshes, per-family locomotion and attack montages, family-specific hit reactions, tuned physics assets, unique growl/attack/death cues, and city-biome material variants.
