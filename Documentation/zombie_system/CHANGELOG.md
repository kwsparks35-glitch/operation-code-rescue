# Top-20 roadmap — session changelog

Session: 2026-04-29

This is the chronological story of the top-20 push, for anyone reading
the diffs and wondering what happened in what order. Each entry points
at the per-item file with full details.

## Wave 1 — variant system polish

C++ batch on top of the previous PIE-verified scaffolding.

- **#3 Death montage** — added `DeathMontage` soft ref to
  `FZombieVariantRow`. `ApplyRescueDamage` now sets `bIsDying`,
  immediately persists the kill, plays the montage, schedules destroy
  via `FTimerHandle DeathDestroyTimer` after `max(montage length,
  0.5s)`. (`03_death_montage.md`)
- **#4 Hit react flinch** — added `HitReactMontage` soft ref. Plays on
  any non-fatal hit. (`04_hit_react.md`)
- **#10 Audio cues** — added `GrowlCue`, `AttackCue`, `DeathCue` soft
  refs + persistent `UAudioComponent` for ambient growls scheduled at
  6–14 sec random intervals. Player audio: `FireCue`, `HitConfirmCue`,
  `DryFireCue`. (`10_audio.md`)
- **#18 Difficulty cap** — `InitializeFromVariant` clamps Health × at
  1.30, Damage × at 1.40 so Hard + tank variants don't compound to
  140 HP zombies. Speed left uncapped intentionally.
  (`18_difficulty_cap.md`)
- **#2 Velocity hookup (partial)** — `ACodeZombieActor::GetVelocity()`
  override returns per-tick computed velocity so locomotion AnimBPs
  read motion. Full `AActor → ACharacter` conversion deferred.
  (`02_character_locomotion.md`)

## Wave 2 — combat loop

- **#11 Player weapon** — `Fire()` now spawns `MuzzleFlashVFX`,
  `BulletImpactVFX`, plays `FireCue/HitConfirmCue/DryFireCue`, gates
  by `FireRefireDelay`. (`11_weapon_system.md`)
- **#12 Game-over screen** — new `UCodeRescueDeathWidget` modeled on
  `UCodeRescueVictoryWidget`. Three buttons: Restart-from-Save,
  Restart-Fresh, Quit. Spawns from `ACodeRescueCharacter::ApplyDamage`
  on `Health <= 0`. (`12_game_over.md`)

## Wave 3 — UX

- **#13 / #14 HUD + crosshair** — `UCodeRescueHUDWidget::RefreshHUD`
  does a 600-unit forward trace per frame, colors the crosshair by
  hit-actor type, and shows a contextual `[E] do thing` prompt below
  it. (`13_hud.md`, `14_crosshair.md`)

## Wave 4 — content

- **#15 More challenge types** — four new validator shapes (reverse,
  palindrome, fizzbuzz, filter) in `UCodeRunnerLibrary::ValidateInEngine`,
  plus four bonus terminals (`hospital_string_reverse`,
  `dock_palindrome_check`, `metro_fizzbuzz_signal`, `triage_even_filter`).
  Win threshold unchanged. (`15_challenge_types.md`)

## Wave 5 — progression

- **#17 Objective gating** — `ASurvivorActor::RequiredTerminalId`
  soft-gates 3 of 4 survivors behind their zone's main terminal.
  Refusal is on-screen yellow hint, NOT modal. (`17_objective_gating.md`)

## Wave 6 — engine prep

- **#1 / #5 AnimBP picker + tuning** — `Scripts/build_zombie_variants_table.py`
  rejects post-process AnimBPs, prefers locomotion-keyword matches,
  +5 bonus for skeleton-match. PACK_CONFIG zone weights rebalanced
  per the per-zone aesthetic; BaseMesh dropped to 0.05 (~1% spawn rate)
  since it has no anims. (`01_animbp_picker.md`, `05_variant_tuning.md`)
- **#6 / #7 NavMesh + AI controller stub** — `ANavMeshBoundsVolume`
  spawned in `SpawnWorld` with 800×320×40 actor scale (≈80,000×32,000×4,000
  units). DefaultEngine.ini gets `RuntimeGeneration=Dynamic` block so
  navmesh tiles auto-build at runtime. New `ACodeRescueAIController`
  stub class for future BT work. (`06_navmesh.md`, `07_ai_controller.md`)

## Documentation pass

- All 20 items now have a per-item `.md` file under this folder, plus
  `README.md` for the index.
- Memory at `~/Library/.../memory/project_code_rescue.md` updated
  with the fourth-pass summary.

## What was NOT done in-session

DEFERRED with detailed next-step recipes:

- **#8** Real environment art (Megascans/PCG kit-bash; needs human
  asset selection)
- **#9** Lighting pass (perceptual; needs human eye on real display)
- **#16** MetaHuman survivors + VO (needs Quixel Bridge + audio
  authoring)
- **#20** Real-student playtesting (needs real students)
- **#19** Actual `Package_Mac_App.command` Shipping cook — Dev build
  is clean which is a strong indicator, but a confirming Shipping run
  wasn't attempted in-session.

## Compile-cycle log

Each compile cycle is ~10–60 sec on the M4 Pro via
`Recompile_Module.command`. The four cycles in this session:

1. Initial batch (items 1–18, 20) — 1 error: `TSubclassOf<UUserWidget>`
   ↔ `UClass*` ternary mismatch in `ApplyDamage`. Fixed with explicit
   `UClass*` resolve.
2. Second cycle — 1 link error: `_Z_Construct_UClass_UCubeBuilder_NoRegister`
   undefined symbol (UCubeBuilder is editor-only). Fixed by switching
   the NavMeshBoundsVolume sizing to `SetActorScale3D` + dropping
   the `Builders/CubeBuilder.h` include.
3. Third cycle — clean. `[1/2] Compile [Apple] CodeRescueGameMode.cpp;
   [2/2] Link [Apple] UnrealEditor-CodeRescueUnreal.dylib; Result:
   Succeeded.`
4. Fourth cycle (after the audio/montage field additions) — clean,
   17/17 link succeeded.

## File-touch summary

New files this pass:

- `Source/CodeRescueUnreal/CodeRescueDeathWidget.h`
- `Source/CodeRescueUnreal/CodeRescueDeathWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueAIController.h`
- `Source/CodeRescueUnreal/CodeRescueAIController.cpp`
- `Documentation/zombie_system/` (this folder, 22 files)

Modified files:

- `Source/CodeRescueUnreal/CodeRescueTypes.h`
- `Source/CodeRescueUnreal/CodeZombieActor.h`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/SurvivorActor.h`
- `Source/CodeRescueUnreal/SurvivorActor.cpp`
- `Source/CodeRescueUnreal/CodeRunnerLibrary.cpp`
- `Scripts/build_zombie_variants_table.py`
- `Config/DefaultEngine.ini`
