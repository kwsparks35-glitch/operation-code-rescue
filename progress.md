Original prompt: "Hi Codex! Please complete all work on this game development that you have not yet completed. :)"

# Progress

## 2026-06-30 - Language profile recap slice

- Continued the June 25 flow-plan work by making the promised profile stats,
  stage recap, review recommendation, and save-slot preview playable in the
  objective journal.
- Added `UCodeRescueGameInstance::GetLanguageProfileRecapSummary()`, deriving
  `LANGUAGE PROFILE RECAP` from existing selected-language save fields instead
  of adding a new save schema.
- The recap now shows selected language, mastery title, difficulty, language
  solves/attempts/success rate, no-hint and perfect solves, streaks, Research
  Points, coding score, rescue/combat/run stats, active curriculum stage,
  current city/tier, review recommendation, and start-screen Resume save-slot
  state.
- Added `LanguageProfileRecapText` to
  `UCodeRescueObjectiveJournalWidget`, directly beneath language save
  continuity and before the learning debrief/replay blocks.
- Added `Content/CodeRescueData/language_profile_recap_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/LANGUAGE_PROFILE_RECAP_SLICE.md`,
  updated curriculum/journal/accessibility/onboarding/visual/human-QA/creative
  inclusion records, and wired
  `Scripts/verify_language_profile_recap_slice_pass.py` into local CI and full
  QA.

## 2026-06-30 - Challenge replay journal slice

- Continued the June 25 curriculum-feedback and replay-hook work by adding a
  save-backed `CHALLENGE REPLAY BRIEF` to the objective journal.
- Added `UCodeRescueGameInstance::GetChallengeReplayJournalSummary()`, derived
  from the existing selected-language learning debrief save fields, so the
  replay brief survives terminal close, save/quit, and start-screen language
  Resume without adding a new save schema.
- The replay brief now distinguishes clean solves from bypass-kit-assisted
  route opens, then presents the saved language track, challenge, concept,
  score, save slot, visible validation goal, hidden-test replay note, and one
  concrete next practice action.
- Added `ChallengeReplayBriefText` to
  `UCodeRescueObjectiveJournalWidget`, positioned after `LAST LEARNING
  DEBRIEF` and before route/inventory/survivor intel, with text-first
  auto-wrapped high-contrast styling.
- Added `Content/CodeRescueData/challenge_replay_journal_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/CHALLENGE_REPLAY_JOURNAL_SLICE.md`,
  updated curriculum/journal/accessibility/onboarding/visual/human-QA/creative
  inclusion records, and wired
  `Scripts/verify_challenge_replay_journal_slice_pass.py` into local CI and
  full QA.

## 2026-06-30 - Survivor intel archive slice

- Continued the June 25 survivor-intel and language-save work by adding a
  save-backed `SURVIVOR INTEL ARCHIVE` beneath the live objective-journal
  dossier.
- Added selected-language save fields for the latest uploaded survivor intel:
  terminal ID, survivor name, city route label, coding language, archive
  status, compact summary, validation score, and state flag.
- Added `UCodeRescueGameInstance::RecordSurvivorIntelDossier()` and
  `GetSurvivorIntelArchiveSummary()`; fresh language runs clear the archive,
  and resumed language slots restore it from the active language save.
- Clean terminal solves and Ctrl+B bypass-kit solves now upload survivor route
  intel into the active selected-language profile, including contact role,
  field need, rescue value, location, lesson payoff, score, and clean/assisted
  validation wording.
- Survivor rescue updates the matching archive from `ROUTE OPEN` to `RESCUED`
  before the existing rescue checkpoint save, so the journal keeps the
  extraction/debrief update after relaunch.
- Added `Content/CodeRescueData/survivor_intel_archive_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/SURVIVOR_INTEL_ARCHIVE_SLICE.md`,
  updated curriculum/journal/accessibility/onboarding/visual/human-QA/creative
  inclusion records, and wired
  `Scripts/verify_survivor_intel_archive_slice_pass.py` into local CI and full
  QA.
- Full QA initially exposed generated audio/animation coverage-manifest drift;
  repaired `Scripts/export_production_track_manifests.py` so regenerated audio
  and animation manifests retain the runtime accessibility, retarget,
  foot-grounding, Maya-cleanup, and zombie-family coverage tokens without
  changing their required row counts.
- Final validation passed on 2026-06-30: focused survivor archive/dossier,
  journal, terminal, learning-debrief, language-save, audio, and animation
  verifiers; `./Recompile_Module.command < /dev/null`; and
  `./Run_Full_QA_Audit.command`, including Unreal commandlets, headless runtime
  smoke, warning scan, and runtime log contracts. The retained smoke warnings
  were the known allowed navigation dirty-area and crowd-manager messages.
- Post-audit package validation also passed: `./Package_Mac_App.command`,
  packaged null smoke, and packaged render smoke all completed against
  `PackagedMac/Mac/CodeRescueUnreal.app`; retained warnings were limited to the
  known allowed navigation/crowd messages plus the render-mode CoreAudio
  sample-rate warning.

## 2026-06-30 - Persistent learning debrief slice

- Continued the June 25 curriculum-feedback and release-readiness work by
  making terminal learning takeaways persist beyond the terminal output.
- Added selected-language save fields for the latest learning debrief:
  challenge ID, concept, language label, score, compact summary, and state
  flag.
- Added `UCodeRescueGameInstance::RecordLearningDebrief()` and
  `GetLearningDebriefJournalSummary()`; fresh language runs clear the debrief,
  and resumed language slots restore it with the rest of the profile.
- Clean terminal solves now save the generated `POST-SOLVE DEBRIEF`, while
  bypass-kit solves save an `ASSISTED LEARNING DEBRIEF` explaining that route
  progress is saved but clean-solve rewards are disabled.
- Added a `LAST LEARNING DEBRIEF` journal section beside the language-save
  summary so players can review the last concept proof, language transfer,
  score, save slot, and next-practice note after closing the terminal or
  relaunching through the start-screen language Resume action.
- Added `Content/CodeRescueData/persistent_learning_debrief_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/PERSISTENT_LEARNING_DEBRIEF_SLICE.md`,
  updated curriculum/journal/accessibility/onboarding/visual/human-QA/creative
  inclusion records, and wired
  `Scripts/verify_persistent_learning_debrief_slice_pass.py` into local CI and
  full QA.
- Repaired related audio and animation coverage-manifest drift exposed by the
  full audit, preserving the 465-row audio manifest and 13-row animation
  manifest while restoring mono/visualized cue, retarget/control-rig,
  foot-grounding, Maya cleanup, and zombie-family coverage tokens.
- Final validation passed on 2026-06-30: focused persistent-debrief and
  language-save verifiers, full QA audit with headless runtime smoke, Mac
  package build, packaged null smoke, and packaged render smoke. The only
  retained smoke warnings were the known allowed immediate-exit navigation and
  crowd-manager messages plus the known render-mode CoreAudio sample-rate
  warning.
- Post-package release evidence was refreshed with
  `Scripts/generate_release_manifest.py`,
  `Scripts/verify_package_integrity_pass.py`, and
  `Scripts/verify_nonhuman_release_readiness_pass.py`; local package integrity
  is ready, while external distribution still correctly reports the existing
  signing/notarization credential boundary.

## 2026-06-30 - Creative implementation QA closeout

- Completed the follow-up audit pass for the June 25 creative-development
  implementation ledger and stealth/avoidance slice.
- Fixed three stale QA contracts found by `./Run_Full_QA_Audit.command`: the
  city realization verifier now recognizes the runtime post-process grade
  helper, the HUD again exposes the expected weapon slot and keyboard/wheel
  cycle wording, and the settings widget reports the exact color-vision live
  refresh feedback expected by the accessibility verifier.
- Re-ran the full QA audit successfully after those fixes, including the
  static verifier suite, editor rebuild, Unreal commandlet validations,
  headless runtime smoke, warning scan, and runtime log contract checks.
- Built a fresh Mac package at `PackagedMac/Mac/CodeRescueUnreal.app` after
  the final code changes and re-ran packaged null and render smoke tests.
  Both passed; the only retained warnings were the known allowed immediate-exit
  navigation/crowd messages and the known render-mode CoreAudio sample-rate
  warning.
- Current status: the June 25 implementation plan is represented by
  documented runtime slices and verifier coverage, while subjective human QA
  for visual/audio/animation feel remains tracked in the existing signoff and
  visual-regression manifests.

## 2026-06-30 - Creative development implementation ledger

- Added a consolidated implementation ledger for the June 25 creative
  development guidance set, covering `CHARACTER_ANIMATION_DEEPDIVE`,
  `GAME_PHYSICS_DEEPDIVE`, `OPERATION_CODE_RESCUE_RELEASE_DOSSIER`,
  `TOP_50_RECOMMENDATIONS`, and `WORLD_DEVELOPMENT_DEEPDIVE`.
- Mapped all 51 rows in `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
  to their primary slice documentation and verifier surface so future review
  does not depend on searching scattered notes by hand.
- Added `Scripts/verify_creative_development_implementation_ledger.py` and
  wired it into both local CI readiness and full QA audit so missing docs,
  missing verifier scripts, or dropped plan rows fail validation.

## 2026-06-30 - Stealth avoidance slice

- Continued the June 25 creative-development backlog by implementing Top 50
  recommendation 40, giving players a real stealth/avoidance option instead
  of forcing proximity-only zombie chase.
- Added player-side stealth noise state to `ACodeRescueCharacter`: quiet
  movement, sprinting, weapon fire, melee, reloads, throwables, flashlight
  exposure, and radio scanner use now feed a visible noise percentage and
  state label.
- Updated `ACodeRescueAIController` so zombies detect by sightline or
  short-lived player noise, investigate last-known locations, and lose contact
  when the player breaks sight and quiets down.
- Updated the HUD tactical readout with `Stealth QUIET`, `LOW NOISE`,
  `AUDIBLE`, `NOISY`, and `EXPOSED` states, while preserving protected coding
  spaces and selected-language save/resume behavior.
- Added `Content/CodeRescueData/stealth_avoidance_manifest.tsv`, documented
  the slice in
  `Documentation/improvement_pass_2026-06-30/STEALTH_AVOIDANCE_SLICE.md`, and
  wired `Scripts/verify_stealth_avoidance_slice_pass.py` into local CI and
  full QA.
- Validation passed: Python verifier compilation,
  `Scripts/verify_stealth_avoidance_slice_pass.py`, adjacent standard-pursuit,
  protected-learning-zone, threat-compass, selected-language terminal-flow,
  tactical-gear, and combat-juice verifiers, module recompile, Mac packaging,
  packaged null smoke, and packaged render smoke. Null smoke retained the
  known allowed navigation/crowd warnings; render smoke retained the known
  allowed CoreAudio sample-rate, navigation, and crowd warnings.

## 2026-06-30 - World bible lore slice

- Continued `WORLD_DEVELOPMENT_DEEPDIVE` section 7.2 and Top 50
  recommendation 36 by adding a compact runtime lore contract for the
  game's premise, pillars, factions/forces, technology rules, and per-city
  lore data.
- Added `SpawnWorldBibleLoreLayer` after environmental storytelling so every
  campaign city now surfaces a nonblocking field world-bible deck beside the
  world-story cues.
- Tagged the layer with `WorldBibleLoreLayer`, `CanonicalLoreContract`,
  `WorldBibleAndLoreGuidance`, `CodingAsEmpowermentPillar`,
  `SurvivingEngineersNetwork`, `AutomationAntagonistForce`,
  `InfectedPressureForce`, `TechnologyRulesReadable`, `PerCityLoreData`,
  `Top50Recommendation36`, `WorldDevelopmentDeepDive`, and runtime
  storytelling Data Layer stand-ins.
- Added `Content/CodeRescueData/world_bible_lore_manifest.tsv`, documented
  the slice in
  `Documentation/improvement_pass_2026-06-30/WORLD_BIBLE_LORE_SLICE.md`,
  added Signal Concord / Orphaned Automata entries to the character-world
  manifest, and updated creative inclusion, performance budget,
  visual-regression, human-QA, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation,
  `Scripts/verify_world_bible_lore_slice_pass.py`, adjacent environmental
  storytelling, coding-to-rescue world-response, survivor-archetype, runtime
  Data Layer, case-file, and world-promotion verifiers, module recompile, Mac
  packaging, packaged null smoke, and packaged render smoke. Null smoke
  retained the known allowed navigation/crowd warnings; render smoke retained
  the known allowed CoreAudio sample-rate, navigation, and crowd warnings.

## 2026-06-30 - Environmental storytelling slice

- Continued `WORLD_DEVELOPMENT_DEEPDIVE` sections 6.4 and 7 plus Top 50
  recommendation 35 by adding a runtime environmental storytelling layer that
  makes the coding-rescues-people premise visible in the city.
- Added `SpawnEnvironmentalStorytellingLayer` after collectible case files so
  every campaign city now has a nonblocking story deck for automation failure,
  safehouse engineer network, survivor stake, code cause/effect, and city
  chapter navigation.
- Fed the layer from existing mission fields including `MissionBrief`,
  `TerminalTitle`, `RadioBriefing`, `SurvivorName`, `CharacterStoryPlan`,
  `LandmarkName`, `RegionName`, `DistrictStyle`, and `NovelGameplayDetail`,
  and tagged it with `EnvironmentalStorytellingLayer`,
  `CodingRescuesPeoplePremise`, `WorldBiblePillar`,
  `Top50Recommendation35`, `WorldDevelopmentDeepDive`, and runtime
  storytelling Data Layer stand-ins.
- Added `Content/CodeRescueData/environmental_storytelling_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/ENVIRONMENTAL_STORYTELLING_SLICE.md`,
  and updated creative inclusion, performance budget, visual-regression,
  human-QA, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation,
  `Scripts/verify_environmental_storytelling_slice_pass.py`, adjacent case-file,
  coding-to-rescue world-response, city ambient-zone audio, runtime Data Layer,
  world-promotion, and survivor-archetype verifiers, module recompile, Mac
  packaging, packaged null smoke, and packaged render smoke. Null smoke
  retained the known allowed navigation/crowd warnings; render smoke retained
  the known allowed CoreAudio sample-rate, navigation, and crowd warnings.

## 2026-06-30 - Runtime Data Layer migration slice

- Continued `WORLD_DEVELOPMENT_DEEPDIVE` section 2.3 and Top 50
  recommendation 31 by adding a runtime bridge for future World
  Partition/Data Layer migration while preserving the current generated city
  fallback.
- Added `ApplyRuntimeDataLayerTags` and `SpawnRuntimeDataLayerMigrationLayer`
  so streamed city actors, protected learning spaces, encounter-director
  combat areas, solved rescue-route actors, and weather/lighting cues expose
  stable state/time/mode tags such as `RuntimeDataLayer_State_SafeBeat`,
  `RuntimeDataLayer_State_Overrun`,
  `RuntimeDataLayer_State_RescueRouteOpen`, and
  `RuntimeDataLayer_Time_DayNightCycle`.
- Extended `RegisterStreamedActor` with `RuntimeWorldPartitionStreamCell`,
  `CurrentCppWorldPartitionFallback`, `OneFilePerActorMigrationReady`, and
  `WorldPartitionReady` tags so the C++ streaming stand-in is auditable before
  authored World Partition maps and OFPA actors replace it.
- Added
  `Content/CodeRescueData/runtime_data_layer_migration_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/RUNTIME_DATA_LAYER_MIGRATION_SLICE.md`,
  and updated creative inclusion, world promotion, performance budget,
  visual-regression, human-QA, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation,
  `Scripts/verify_runtime_data_layer_migration_slice_pass.py`, adjacent world
  promotion, weather/lighting, protected-learning-zone AI, adaptive encounter
  director, and coding-to-rescue world-response verifiers, module recompile,
  Mac packaging, packaged null smoke, packaged render smoke, scoped
  `git diff --check`, and touched-file trailing-whitespace scan. Null smoke
  retained the known allowed navigation/crowd warnings; render smoke retained
  the known allowed CoreAudio sample-rate, navigation, and crowd warnings.

## 2026-06-30 - Fixed timestep physics contract slice

- Continued `GAME_PHYSICS_DEEPDIVE` and Top 50 recommendation 25 by making the
  project's fixed-step/substepping posture explicit in config and shared runtime
  code.
- Added `CodeRescuePhysicsStability` to apply damping floors, custom sleep and
  stabilization multipliers, max depenetration velocity, optional CCD, mass
  overrides, and QA tags to runtime physics bodies.
- Wired the contract into throwables, throwable pulse targets, barricades,
  barricade debris, zombie physical hit reactions, zombie ragdolls, primitive
  corpse fallback parts, the Jeep fallback body, and GameMode-spawned
  training/systems/stress physics props.
- Added
  `Content/CodeRescueData/fixed_timestep_physics_contract_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/FIXED_TIMESTEP_PHYSICS_CONTRACT_SLICE.md`,
  and updated creative inclusion, physics promotion, visual-regression,
  human-QA, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation,
  `Scripts/verify_fixed_timestep_physics_contract_slice_pass.py`, adjacent
  collision-channel, creative-physics, physics-promotion, throwable
  surface-impact, destructible-cover, zombie-death, zombie physical-animation,
  and Jeep surface-physics verifiers, module recompile, Mac packaging, packaged
  null smoke, packaged render smoke, scoped `git diff --check`, and
  touched-file trailing-whitespace scan. `verify_runtime_step_smoke_contracts.py`
  was confirmed as an Unreal commandlet script rather than a plain-Python check.
  Packaging initially caught constructor-time physics-body access from the new
  helper; the stabilizer was moved to runtime `BeginPlay`/activation paths and
  given an `RF_ClassDefaultObject` guard before the successful package rerun.
  Null smoke retained the known allowed navigation/crowd warnings; render smoke
  retained the known allowed CoreAudio sample-rate, navigation, and crowd
  warnings.

## 2026-06-30 - Collision channel gameplay contract slice

- Continued `GAME_PHYSICS_DEEPDIVE` and Top 50 recommendation 21 by moving
  weapon fire, AI sight, and interaction targeting onto named project collision
  channels instead of generic visibility traces.
- Added `CodeRescueCollisionChannels.h` as the shared source for
  `PlayerPawnObject`, `ZombiePawnObject`, `CoverObject`, `PickupObject`,
  `WeaponTrace`, `AISightTrace`, and `InteractionTrace`.
- Updated player weapon fire, elite spitter acid, AI visibility, zombie
  barricade checks, and player interaction targeting to use the new helper.
- Marked player, zombie, cover, pickup, case-file, terminal, survivor, friendly
  NPC, helipad, and Jeep runtime components with explicit channel responses and
  QA tags where appropriate.
- Added
  `Content/CodeRescueData/collision_channel_gameplay_contract_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/COLLISION_CHANNEL_GAMEPLAY_CONTRACT_SLICE.md`,
  and updated creative inclusion, physics promotion, visual-regression,
  human-QA, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation,
  `Scripts/verify_collision_channel_gameplay_contract_slice_pass.py`, adjacent
  creative-physics, physics-promotion, protected-learning-zone AI,
  selected-language terminal, tactical-gear pickup, case-file collectible,
  direct-pursuit zombie, destructible-cover physics, and surface-impact
  verifiers, module recompile, Mac packaging, packaged null smoke, packaged
  render smoke, scoped `git diff --check`, and touched-file
  trailing-whitespace scan. Null smoke retained the known allowed
  navigation/crowd warnings; render smoke retained the known allowed CoreAudio
  sample-rate, navigation, and crowd warnings. Two adjacent verifier/data drift
  issues were corrected while validating: the selected-language terminal
  verifier now follows the internal save helper, and onboarding copy now names
  the case-file pickup save check.

## 2026-06-30 - IK foot grounding review slice

- Continued the `CHARACTER_ANIMATION_DEEPDIVE` IK Rig, IK Retargeter, Control
  Rig, and foot-to-ground contact guidance by adding a full-body grounding
  promotion contract.
- Extended `CodeRescueRetargetRig` with `ApplyFootGroundingReview`, called
  from `ApplyRuntimeRetargetRigSlots`, so every full-body skeletal profile
  receives `FootGroundingRuntimeContract`, `FootIKGroundingReview`,
  `FootPlantTraceReady`, `PelvisOffsetReview`, `ControlRigFootContactReady`,
  and profile-specific foot-plant tags.
- Explicitly excluded `FirstPersonArms` from full-body foot grounding with
  `FootGroundingExcluded_FirstPersonArms`, preserving the separate weapon/hand
  IK path.
- Added a visible `FOOT IK` station to the runtime Maya/Houdini DCC review bay
  so the packaged world communicates foot planting, pelvis offset, and Control
  Rig foot-contact review before final IK assets replace fallbacks.
- Added `Content/CodeRescueData/ik_foot_grounding_review_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/IK_FOOT_GROUNDING_REVIEW_SLICE.md`,
  and updated retarget, animation coverage, character promotion, Maya cleanup,
  creative inclusion, visual-regression, human-QA, full-QA, and local-CI review
  surfaces.
- Validation passed: Python verifier compilation,
  `Scripts/verify_ik_foot_grounding_review_slice_pass.py`, adjacent
  retarget/animation-budget/Maya/player first-person/companion/survivor/zombie/
  boss/character-promotion/Unreal-systems character verifiers, module recompile,
  Mac packaging, packaged null smoke, packaged render smoke, scoped
  `git diff --check`, and touched-file trailing-whitespace scan. The camera
  roster and character-world asset verifiers were confirmed as Unreal commandlet
  scripts rather than plain-Python checks. Null smoke retained the known allowed
  navigation/crowd warnings; render smoke retained the known allowed CoreAudio
  sample-rate, navigation, and crowd warnings.

## 2026-06-30 - Objective route toast clarity slice

- Continued `TOP_50_RECOMMENDATIONS` objective-clarity guidance by adding a
  text-first HUD route acknowledgement layer for terminal solves, survivor
  rescues, extraction readiness, and checkpoint saves.
- Added `ObjectiveRouteToastText` plus `TriggerObjectiveRouteToast` and
  `RefreshObjectiveRouteToast` to `UCodeRescueHUDWidget`, observing solved
  terminals, rescued survivors, coding score, and selected-language save
  timestamps without replaying old toasts on resumed saves.
- Added terminal-solve, survivor-rescue, and `CHECKPOINT SAVED` messaging so
  players can see when the survivor route opened, extraction is ready, and the
  active coding-language run can resume from the start screen.
- Preserved accessibility contracts with high-contrast color variants,
  Reduced Motion stable alpha, shared UI text styling, and an optional
  visualized-sound-cue brightness handoff.
- Added
  `Content/CodeRescueData/objective_route_toast_clarity_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/OBJECTIVE_ROUTE_TOAST_CLARITY_SLICE.md`,
  and updated creative inclusion, visual-regression, human-QA, onboarding,
  curriculum feedback, accessibility, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation,
  `Scripts/verify_objective_route_toast_clarity_slice_pass.py`, adjacent
  objective-focus/minimap/HUD/journal/terminal/survivor/save verifiers, module
  recompile, Mac packaging, packaged null smoke, packaged render smoke, scoped
  `git diff --check`, and touched-file trailing-whitespace scan. Null smoke
  retained the known allowed navigation/crowd warnings; render smoke retained
  the known allowed CoreAudio sample-rate, navigation, and crowd warnings.

## 2026-06-30 - UI text scale settings slice

- Continued `TOP_50_RECOMMENDATIONS` accessibility/readability guidance by
  splitting general UI text size away from subtitle size.
- Added saved `UITextScale` state, `UCodeRescueGameInstance::GetUITextScale`,
  and `GetUITextScaleSummary`, while keeping `SubtitleScale` scoped to the
  subtitle overlay.
- Added a `UI Text Size` slider to Settings with queued readouts, Apply
  persistence, reset-default support, selected-language save continuity, and
  Apply feedback.
- Updated HUD, pause, tutorial, terminal, objective journal, minimap, skill
  tree, save slots, fast travel, damage feedback, victory, and death screens to
  read `CodeRescueUI::Theme().TextScale` from `GetUITextScale()`.
- Added `Content/CodeRescueData/ui_text_scale_settings_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/UI_TEXT_SCALE_SETTINGS_SLICE.md`,
  and updated accessibility, expanded-accessibility, creative inclusion,
  visual-regression, human-QA, full-QA, local-CI, and adjacent verifier
  contracts.
- Validation passed: `Scripts/verify_ui_text_scale_settings_slice_pass.py`
  compilation, UI text-scale verifier, adjacent expanded/settings/subtitle/HUD/
  damage/onboarding/pause/minimap/journal/fast-travel/skill-tree/save-slot/
  end-state/save-compatibility verifiers, mono/visualized-sound regressions,
  module recompile, Mac packaging, packaged null smoke, and packaged render
  smoke. Null smoke retained the known allowed navigation/crowd warnings;
  render smoke retained the known allowed CoreAudio sample-rate, navigation, and
  crowd warnings.

## 2026-06-30 - Mono audio accessibility slice

- Continued `TOP_50_RECOMMENDATIONS` item 44 by adding a saved `Mono Audio`
  accessibility mode for project-owned runtime cue playback.
- Added `bMonoAudio` to the selected-language save path and
  `UCodeRescueGameInstance::GetMonoAudioSummary`, with audio/accessibility
  summaries now reporting mono on/off state.
- Extended Settings with a `Mono Audio` checkbox, queued readouts, Apply
  persistence, and a live `ACodeRescueGameMode::RefreshMonoAudioSpatialization`
  pass for active ambient sounds and zombie growl components.
- Routed weapon, zombie, and survivor positional cues through
  `GetMonoSafeSoundLocation` so mono mode centers these project-owned sounds at
  the player instead of relying on stereo placement.
- Forced `bVisualizeSoundCues` on when mono mode is active, preserving readable
  threat/ambient/caption state for players using mono listening.
- Added
  `Content/CodeRescueData/mono_audio_accessibility_manifest.tsv`, documented
  the slice in
  `Documentation/improvement_pass_2026-06-30/MONO_AUDIO_ACCESSIBILITY_SLICE.md`,
  and updated creative inclusion, audio coverage, visual-regression, human-QA,
  accessibility, full-QA, and local-CI review surfaces.
- Validation passed: mono verifier compilation, mono audio verifier, adjacent
  visualized-sound/settings/ambient/reactive verifiers, module recompile, Mac
  packaging, packaged null smoke, packaged render smoke, scoped `git diff
  --check`, and touched-file trailing-whitespace scan. Null smoke retained the
  known allowed navigation/crowd warnings; render smoke retained the known
  allowed CoreAudio sample-rate, navigation, and crowd warnings.

## 2026-06-30 - Visualized sound cues accessibility slice

- Continued the June 25 `TOP_50_RECOMMENDATIONS` mix-accessibility guidance by
  adding a saved `Visualize Sound Cues` setting that mirrors important audio
  state through the HUD instead of relying on hearing alone.
- Added `bVisualizeSoundCues` to the selected-language save path and
  `UCodeRescueGameInstance::GetVisualizedSoundCueSummary` so the active
  threat, ambient, caption, and mix state is Blueprint-callable and reviewable.
- Extended the Settings menu with a `Visualize Sound Cues` checkbox, queued
  accessibility readout preview, reset-default behavior, and Apply persistence.
- Added `UCodeRescueHUDWidget` `SoundCueText` under the threat compass so calm,
  pursuit, protected safehouse, survivor-route, and extraction states expose
  threat music intensity, city ambient-zone intensity, and caption state as
  readable HUD text.
- Added
  `Content/CodeRescueData/visualized_sound_cues_accessibility_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/VISUALIZED_SOUND_CUES_ACCESSIBILITY_SLICE.md`,
  and updated creative inclusion, audio coverage, visual-regression, human-QA,
  accessibility, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation, visualized sound cues
  verifier, adjacent settings/audio/ambient/reactive verifiers, module
  recompile, Mac packaging, packaged null smoke, packaged render smoke, scoped
  `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - City ambient zone audio slice

- Continued the June 25 `WORLD_DEVELOPMENT_DEEPDIVE` `ZoneAmbientCues`
  guidance plus the `TOP_50_RECOMMENDATIONS` spatial-audio and mix-accessibility
  guidance by adding a package-safe ambient-zone director for city spaces.
- Added `UCodeRescueGameInstance` city ambient zone state, update, and summary
  helpers so the active zone label, future bed key, and intended intensity are
  visible beside the saved audio mix.
- Added `ACodeRescueCharacter::UpdateCityAmbientZoneAudio` with a low-frequency
  sampler that classifies entry approach, protected coding lab, safehouse
  exterior, street grid, transit corridor, civic block, survivor route, survivor
  camp, and extraction pad contexts from mission-relative anchors.
- Wired survivor and extraction ambience labels to selected-language progress by
  reading solved terminal and rescued survivor save state without changing the
  start-screen language selection or save/resume flow.
- Added sparse `[Ambient]` captions and QA tags so future authored ambience,
  attenuation, occlusion, mono mix, or visualize-sound work can bind to stable
  zone bed keys.
- Added `Content/CodeRescueData/city_ambient_zone_audio_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/CITY_AMBIENT_ZONE_AUDIO_SLICE.md`,
  and updated creative inclusion, audio coverage, visual-regression, human-QA,
  accessibility, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation, city ambient zone audio
  verifier, adjacent reactive-threat/audio-accessibility/city-radio verifiers,
  module recompile, Mac packaging, packaged null smoke, packaged render smoke,
  scoped `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Reactive threat audio music slice

- Continued the June 25 `TOP_50_RECOMMENDATIONS` spatial/reactive-audio
  guidance and `WORLD_DEVELOPMENT_DEEPDIVE` district-threat guidance by adding
  a package-safe reactive music bridge around nearby zombie pressure.
- Added `UCodeRescueGameInstance` reactive threat music state, scalar,
  summary, update, and refresh helpers so the existing music component responds
  to threat intensity while still respecting the saved `MusicVolume` setting.
- Added `ACodeRescueCharacter::UpdateReactiveThreatAudio` with range,
  critical-range, and cadence tuning; the sampler scans living zombies, smooths
  distance-weighted pressure, labels calm/watch/pursuit/critical states, and
  dampens to `safehouse muted` inside protected learning zones.
- Added sparse `[Audio]` state captions and QA tags so reactive music remains
  text-backed and reviewable without final authored stems.
- Added
  `Content/CodeRescueData/reactive_threat_audio_music_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/REACTIVE_THREAT_AUDIO_MUSIC_SLICE.md`,
  and updated creative inclusion, audio coverage, visual-regression, human-QA,
  accessibility, full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation, reactive threat audio
  verifier, adjacent threat-caption/audio-accessibility/city-radio verifiers,
  module recompile, Mac packaging, packaged null smoke, packaged render smoke,
  scoped `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Combat juice weapon feel slice

- Continued the June 25 `TOP_50_RECOMMENDATIONS` combat juice and weapon feel
  guidance plus the `CHARACTER_ANIMATION_DEEPDIVE` fire/reload montage guidance
  by adding a reduced-motion-aware runtime feedback bridge to
  `ACodeRescueCharacter`.
- Added combat-juice tuning for fire pitch/yaw kick, hit-stop-style duration,
  hit-confirm kick, damage direction kick, and reload settle kick.
- Added runtime helpers for fire, hit confirm, reload stage, incoming damage,
  reduced-motion scaling, and short QA-visible combat-juice windows.
- Wired the helpers into ammo fire, dry fire, no-ammo melee fallback, direct
  zombie hits, barricade hits, area weapon hits, aim-assist hits, reload
  start/finish, and player damage while preserving magazine, reserve ammo,
  damage mitigation, save, HUD, subtitle, and start-screen behavior.
- Added
  `Content/CodeRescueData/combat_juice_weapon_feel_manifest.tsv`, documented
  the slice in
  `Documentation/improvement_pass_2026-06-30/COMBAT_JUICE_WEAPON_FEEL_SLICE.md`,
  and updated creative inclusion, visual-regression, human-QA, accessibility,
  full-QA, and local-CI review surfaces.
- Validation passed: Python verifier compilation, combat-juice verifier,
  adjacent distinct weapon presentation/player first-person animation/damage
  feedback verifiers, module recompile, Mac packaging, packaged null smoke,
  packaged render smoke, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - Zombie physical-animation hit reaction slice

- Continued the June 25 `GAME_PHYSICS_DEEPDIVE` Phase 2 guidance by adding a
  living-zombie `UPhysicalAnimationComponent` bridge for nonfatal skeletal hit
  reactions while preserving existing motion, montage, glow, and ragdoll paths.
- Added `ZombiePhysicalHitReaction` to `ACodeZombieActor`, plus tuning for
  physical hit reaction enablement, root bone, blend weight, duration, and
  impulse strength.
- Added runtime helpers to bind promoted skeletal zombies with PhysicsAssets,
  resolve common spine/head/limb bone names with a safe whole-body fallback,
  trigger hit-zone impulses, fade `SetStrengthMultiplyer` and physics blend
  weights back to zero, and restore living skeletal no-collision after the cue.
- Kept fallback and save safety intact: primitive/unpromoted zombies still use
  launch/glow/hit-pose feedback and `HitReactMontage`, while death resets any
  active physical hit reaction before ragdoll or primitive corpse physics saves
  and owns the neutralized body.
- Added
  `Content/CodeRescueData/zombie_physical_animation_hit_reaction_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/ZOMBIE_PHYSICAL_ANIMATION_HIT_REACTION_SLICE.md`,
  and updated creative inclusion, enemy readability, physics promotion,
  human-QA, and visual-regression review surfaces.
- Validation passed: Python verifier compilation, zombie physical-animation
  hit reaction verifier, adjacent zombie death/motion/standard pursuit/physics
  promotion verifiers, module recompile, Mac packaging, packaged null smoke,
  packaged render smoke, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - Companion gesture readability slice

- Continued the June 25 `CHARACTER_ANIMATION_DEEPDIVE` and squad-readability
  guidance by adding a visual-only gesture layer to `ACompanionActor`.
- Added `CompanionRoleSignalLight`, role-color refresh, gesture tuning fields,
  base-pose caching for the companion skeletal mesh and role signal light, plus
  runtime tags including `CompanionGestureReadabilityRuntime`.
- Added `UpdateCompanionGesture` for idle follow/hold motion, support-fire
  recoil, medic-pulse lift, order acknowledgment, damage flinch, and role-light
  pulses without owning actor root movement, capsule collision, or formation
  logic.
- Wired gesture triggers into support fire, automatic/manual medic pulses,
  regroup, hold/follow/order barks, and companion damage while preserving the
  existing squad HUD, subtitles, Y/U/O/N controls, and selected-language save
  behavior.
- Added `Content/CodeRescueData/companion_gesture_readability_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/COMPANION_GESTURE_READABILITY_SLICE.md`,
  updated creative inclusion, squad personality, onboarding, human-QA, and
  visual-regression review surfaces, and wired
  `Scripts/verify_companion_gesture_readability_slice_pass.py` into full QA/local
  CI.
- Validation passed: Python verifier compilation, companion gesture
  readability verifier, adjacent squad personality/squad command/formation
  verifiers, module recompile, Mac packaging, packaged null smoke, packaged
  render smoke, scoped `git diff --check`, and touched-file trailing-whitespace
  scan.

## 2026-06-30 - Friendly NPC gesture readability slice

- Continued the June 25 `CHARACTER_ANIMATION_DEEPDIVE` safehouse-support
  guidance by adding a lightweight visual gesture layer to `AFriendlyNPCActor`.
- Enabled friendly NPC ticking for gesture-only updates, then added tuning
  fields, base-pose caching for the professional skeletal body, primitive
  fallback head, role badge, role prop, role icons, and service light, plus
  runtime tags including `FriendlyNPCGestureReadabilityRuntime`.
- Added `UpdateServiceGesture` for idle support motion, cooldown dimming,
  successful-service acknowledgment pulses, and refusal gestures without moving
  the actor root or primitive collision body.
- Added `TriggerServiceGrantGesture` after successful service use and
  `TriggerServiceDeniedGesture` for cooldown, full-health Medic, and
  insufficient-scrap Trader interactions while preserving the selected-language
  cooldown save path.
- Added
  `Content/CodeRescueData/friendly_npc_gesture_readability_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/FRIENDLY_NPC_GESTURE_READABILITY_SLICE.md`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  and wired `Scripts/verify_friendly_npc_gesture_readability_slice_pass.py`
  into full QA/local CI.
- Validation passed: Python verifier compilation, friendly NPC gesture
  readability verifier, adjacent friendly service/survivor gesture verifiers,
  module recompile, Mac packaging, packaged null smoke, packaged render smoke,
  scoped `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Survivor gesture readability slice

- Continued the June 25 `CHARACTER_ANIMATION_DEEPDIVE` survivor-readability
  guidance by adding a lightweight visual gesture layer to `ASurvivorActor`.
- Enabled survivor ticking for gesture-only updates, then added tuning fields,
  base-pose caching for the professional skeletal body, primitive fallback head,
  and rescue light, plus runtime tags including
  `SurvivorGestureReadabilityRuntime`.
- Added `UpdateSurvivorGesture` for idle life motion, locked-route refusal,
  rescue confirmation lift, and rescue-light pulse without moving the root
  collision body.
- Added `TriggerLockedGesture` when a survivor refuses rescue because the
  selected-language terminal is still unsolved, and `TriggerRescueGesture` when
  rescue succeeds.
- Replaced the instant successful-rescue hide with `ScheduleRescueFadeOut` so
  collision still disables immediately but the player sees a short rescue pose
  before the actor disappears and the existing extraction/save/companion flow
  continues.
- Added `Content/CodeRescueData/survivor_gesture_readability_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/SURVIVOR_GESTURE_READABILITY_SLICE.md`,
  updated creative inclusion, survivor roster, human-QA, and visual-regression
  review surfaces, and wired
  `Scripts/verify_survivor_gesture_readability_slice_pass.py` into full QA/local
  CI.
- Validation passed: Python verifier compilation, survivor gesture readability
  verifier, adjacent survivor roster/dialogue verifiers, module recompile, Mac
  packaging, packaged null smoke, packaged render smoke, scoped `git diff
  --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Zombie motion readability slice

- Continued the June 25 `CHARACTER_ANIMATION_DEEPDIVE` enemy-animation guidance
  by adding an additive runtime pose layer to `ACodeZombieActor`.
- Added `bEnableMotionReadability`, motion sway tuning, hit-reaction and attack
  lunge pose durations, base-pose caching for skeletal/primitive/glow
  components, and reset logic that avoids fighting ragdoll or primitive corpse
  physics.
- Added `UpdateMotionReadability` so chase sway, forward lean, attack windup,
  attack lunge, hit recoil, and protected learning-zone hold all produce visible
  non-collision component motion even when final authored montages are absent.
- Wired `TriggerAttackMotionCue` into player attacks and destructible-cover
  strikes, and wired `TriggerHitReactionMotionCue` into nonfatal zombie damage
  alongside existing hit impulses, optional hit-react montages, and death
  physics.
- Added `Content/CodeRescueData/zombie_motion_readability_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/ZOMBIE_MOTION_READABILITY_SLICE.md`,
  updated creative inclusion, enemy-readability, human-QA, and visual-regression
  review surfaces, and wired
  `Scripts/verify_zombie_motion_readability_slice_pass.py` into full QA/local
  CI.
- Validation passed: Python verifier compilation, zombie motion readability
  verifier, adjacent standard-pursuit/zombie-family/animation-budget verifiers,
  module recompile, Mac packaging, packaged null smoke, packaged render smoke,
  scoped `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - City street grid and storefront shell slice

- Continued the June 25 P0 world guidance by making the generated city route
  read as an authored street grid with ground-floor storefront shells instead
  of only skyline blocks and objective pads.
- Extended `SpawnMajorCityUrbanIdentityLayer` with `CityStreetGridStorefrontShell`
  and `ReadableCityStreetGrid` tags across roads, sidewalks, lane paint,
  facades, district signs, and review signage.
- Added readable crosswalk stripes and labels tagged with
  `StreetGridCrosswalkReadable`, `HumanScaleCurbCrossing`, and
  `RouteClearStreetShell`.
- Added storefront door recesses, ground windows, sign bands, awnings, and
  street-level shop-role labels; storefront actors carry
  `StorefrontShellGroundFloor`, `ModularStorefrontShell`,
  `ParallaxStorefrontReady`, and `ImportedWorldAssetPromotionTarget`.
- Added `Content/CodeRescueData/city_street_grid_storefront_shell_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/CITY_STREET_GRID_STOREFRONT_SHELL_SLICE.md`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  and wired `Scripts/verify_city_street_grid_storefront_shell_slice_pass.py`
  into full QA/local CI.
- Validation passed: Python verifier compilation, the city street/storefront
  verifier, adjacent safe-learning/world verifiers, Unreal world-assets
  commandlet, module recompile, Mac packaging, packaged render smoke, scoped
  `git diff --check`, and scoped trailing whitespace scan.

## 2026-06-30 - Standard direct-pursuit zombie slice

- Continued the June 25 P0 enemy guidance by making ordinary zombie pressure
  explicit, readable, and reviewable outside protected coding spaces.
- Added `ACodeZombieActor::ApplyStandardDirectPursuitProfile`, standard pursuit
  tuning fields, runtime tags, state summaries, pursuit captions, protected
  learning-zone hold tags, and fair attack-windup timing tied to the same
  cooldown that gates melee damage.
- Updated `ACodeRescueAIController` to tag direct chase and attack-hold behavior
  for profiled zombies, and updated regular city, physics ambush, encounter
  director, language breach, boss horde, and boomer-add spawn paths to apply the
  profile and tag marker actors with `StandardDirectPursuitZombie`.
- Updated the HUD threat compass, tactical readout, and combat alert line so
  standard pursuit state and `PURSUIT PRESSURE` are visible alongside distance,
  direction, role, and zombie family.
- Added `Content/CodeRescueData/standard_direct_pursuit_zombie_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/STANDARD_DIRECT_PURSUIT_ZOMBIE_SLICE.md`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  and wired `Scripts/verify_standard_direct_pursuit_zombie_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, standard pursuit verifier,
  protected learning-zone verifier, May 27 safe-learning controls verifier,
  adjacent threat-compass verifier, Unreal commandlet
  `verify_runtime_step_smoke_contracts.py`, module recompile, Mac packaging,
  packaged null/render smokes, scoped `git diff --check`, and scoped trailing
  whitespace scan.

## 2026-06-30 - Survivor archetype roster slice

- Continued the June 25 P0 survivor-archetype guidance by making rescued
  survivor teams read as role-specific roster members instead of generic
  objective markers.
- Added `FCodeRescueSurvivorArchetypeProfile` and campaign-level lesson-family
  mapping for Power-Grid Apprentice, Systems Mechanic, Radio-Code Cleanup
  Specialist, Archive Integrity Analyst, Drone Timing Coordinator, Data Medic,
  Network Engineer, and Supply-Cache Analyst identities.
- Added survivor runtime archetype fields, summary/prompt helpers, role tags,
  role-colored rescue lights/materials, and mission-derived configuration on
  spawn.
- Updated the survivor HUD prompt, relief-camp profile signage, rescue
  confirmation, rescue subtitles, dispatch line, companion handoff, and journal
  survivor dossier so role, icon, field need, rescue value, and selected-language
  save continuity are visible.
- Added `Content/CodeRescueData/survivor_archetype_roster_manifest.tsv`,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/SURVIVOR_ARCHETYPE_ROSTER_SLICE.md`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  and wired `Scripts/verify_survivor_archetype_roster_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, survivor archetype roster
  verifier, adjacent survivor intel dossier verifier, adjacent survivor rescue
  dialogue handoff verifier, Unreal commandlet `verify_character_world_assets.py`,
  module recompile, Mac packaging, packaged null/render smokes, scoped
  `git diff --check`, and scoped trailing whitespace scan.

## 2026-06-30 - Playable operator identity slice

- Continued the June 25 P0 playable-rescue-operator guidance by making the
  runtime pawn read as a named, save-backed operator rather than only a camera
  and mannequin body.
- Added selected-language operator identity save fields for callsign, role,
  profile note, and a back-compat flag, plus GameInstance helpers for
  language-derived initialization and identity summaries.
- Mapped Java, C, C+, C++, Python, and MATLAB runs to stable operator profiles
  while preserving the start screen as a language-selection screen.
- Updated language save summaries, HUD status text, live player tags, and the
  cast-promotion stage so the active operator callsign and role are visible
  before and during resumed gameplay.
- Added `Content/CodeRescueData/playable_operator_identity_manifest.tsv`,
  updated save-compatibility coverage, creative inclusion, human-QA, and
  visual-regression review surfaces, documented the slice in
  `Documentation/improvement_pass_2026-06-30/PLAYABLE_OPERATOR_IDENTITY_SLICE.md`,
  and wired `Scripts/verify_playable_operator_identity_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the playable operator identity
  verifier, save compatibility verifier, retarget/control-rig verifier, Unreal
  camera/roster commandlet, module recompile, Mac packaging, packaged
  null/render smokes, scoped `git diff --check`, and scoped trailing whitespace
  scan.

## 2026-06-30 - Friendly safehouse NPC service slice

- Continued the June 25 P0 character guidance by turning the Civilian Support
  Hub into a clearer save-backed service loop for Engineer, Medic, Scientist,
  and Trader NPCs.
- Added selected-language save fields for used safehouse NPC service IDs, plus
  GameInstance helpers to mark, clear, reset, summarize, save, load, and apply
  those service cooldowns.
- Added `AFriendlyNPCActor` service IDs, role display names, service summaries,
  saved cooldown restore, and benefit/cooldown prompts, then made successful
  services save immediately into the active language profile.
- Updated HUD prompts and support-hub signage so players can read each role
  benefit, per-language save behavior, and day-night reset rule before using a
  service.
- Added `Content/CodeRescueData/friendly_safehouse_npc_service_manifest.tsv`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/FRIENDLY_SAFEHOUSE_NPC_SERVICE_SLICE.md`,
  and wired `Scripts/verify_friendly_safehouse_npc_service_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new friendly safehouse
  NPC service verifier, save compatibility verifier,
  `verify_may27_unreal_systems_character_world_pass.py`,
  `verify_maya_character_cleanup_slice_pass.py`, Unreal commandlet
  `verify_character_world_assets.py`, module recompile, Mac packaging,
  packaged null/render smokes, scoped `git diff --check`, and scoped trailing
  whitespace scan.

## 2026-06-30 - Protected learning zone AI slice

- Continued the June 25 P0 protected-safehouse and zombie-exclusion guidance by
  turning the safehouse/terminal tags into a shared runtime protection rule.
- Added `ACodeRescueGameMode::IsLocationInsideProtectedLearningZone` so enemy
  AI, zombie attack code, player damage intake, and QA checks all use the same
  tagged protected-learning bounds instead of separate hard-coded assumptions.
- Tagged spawned terminals as protected learning anchors, including future bonus
  terminals and annex placements.
- Updated zombie AI so protected players are not visible, chased, or attacked,
  and updated zombie actor tick/elite/boomer/player-damage paths so zombie
  damage cannot leak into terminal or safehouse spaces.
- Added `Content/CodeRescueData/protected_learning_zone_ai_manifest.tsv`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/PROTECTED_LEARNING_ZONE_AI_SLICE.md`,
  and wired `Scripts/verify_protected_learning_zone_ai_slice_pass.py` into full
  QA/local CI.
- Validation passed: `verify_protected_learning_zone_ai_slice_pass.py`,
  `verify_may27_safe_learning_city_controls_pass.py`, Unreal commandlet
  `verify_runtime_step_smoke_contracts.py`, module recompile, Mac packaging,
  packaged null/render smokes, scoped `git diff --check`, and scoped trailing
  whitespace scan.

## 2026-06-30 - Weapon quick-slot armory slice

- Continued the June 25 P0 weapon/armory guidance by making the immediate
  arsenal readable in both the world and HUD instead of relying on hidden
  number-key bindings.
- Added selected-language save fields for active weapon, per-slot magazines,
  per-slot reserve ammo, and a `bHasWeaponQuickSlotState` back-compat flag.
- Threaded quick-slot state through GameInstance capture/save/load/apply, added
  `ACodeRescueCharacter::RestoreWeaponQuickSlotState`, and added
  `GetWeaponQuickSlotSummary` for a compact 1-0 HUD strip.
- Updated the tactical armory with a `QUICK SLOT BOARD`, key labels for slots
  1-0, wheel labels for the extended arsenal, and language-profile save
  wording that matches the HUD.
- Added `Content/CodeRescueData/weapon_quick_slot_armory_manifest.tsv`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/WEAPON_QUICK_SLOT_ARMORY_SLICE.md`,
  and wired `Scripts/verify_weapon_quick_slot_armory_slice_pass.py` into full
  QA/local CI.
- Validation passed: `verify_weapon_quick_slot_armory_slice_pass.py`,
  `verify_may27_tactical_arsenal_mcp_runtime.py`,
  `verify_save_compatibility_pass.py`, module recompile, Mac packaging,
  packaged null/render smokes, scoped `git diff --check`, and scoped trailing
  whitespace scan.

## 2026-06-30 - Death replay save-and-quit flow slice

- Continued the June 25 P0 graceful-failure guidance by turning death into a
  selected-language replay checkpoint rather than a risk of saving a
  zero-health pawn.
- Added `UCodeRescueGameInstance::SaveDeathRecoveryCheckpoint` and an internal
  cached-save path so death count, progress, selected language, and run state
  persist while resume starts from the current city entry pad with playable
  health and minimum recovery resources.
- Updated the death branch to increment death progress before showing the
  overlay, and updated `UCodeRescueDeathWidget` with a `DeathActionStatus`
  line plus explicit resume, fresh restart, save-and-quit, and quit logging.
- Added `Content/CodeRescueData/death_replay_save_quit_manifest.tsv`, updated
  creative inclusion, human-QA, and visual-regression review surfaces,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/DEATH_REPLAY_SAVE_QUIT_FLOW_SLICE.md`,
  and wired `Scripts/verify_death_replay_save_quit_flow_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new death replay
  save-and-quit verifier, adjacent end-state/health/save verifiers, module
  recompile, Mac packaging, packaged null smoke, packaged render smoke,
  packaged runtime log contracts, and scoped whitespace/diff checks.

## 2026-06-30 - Selected language terminal flow slice

- Continued the June 25 P0 launch and core-loop guidance by consolidating the
  selected-language start, resume, terminal, validation, and save behavior into
  one auditable flow.
- Updated terminal validation output with a `Language Run Lock` block that
  names the active language, language-specific save profile, and start-screen
  resume continuity after each terminal attempt.
- Hardened the in-engine curriculum validator so C reverse-string solutions
  that write into an output buffer pass the selected-language terminal path
  alongside Java, C+, C++, Python, and MATLAB solutions.
- Added `Content/CodeRescueData/selected_language_terminal_flow_manifest.tsv`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/SELECTED_LANGUAGE_TERMINAL_FLOW_SLICE.md`,
  and wired `Scripts/verify_selected_language_terminal_flow_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, selected-language terminal
  flow verifier, launch-language start-screen/save verifier, terminal
  language-track UX verifier, terminal post-solve debrief verifier, Unreal
  curriculum validator commandlet, module recompile, Mac packaging, packaged
  null smoke, packaged render smoke, packaged runtime log contracts, and scoped
  whitespace/diff checks.

## 2026-06-30 - Health damage survivability slice

- Continued the June 25 P0 combat-readability guidance by making the existing
  non-instant damage protections visible to the player through HUD mitigation
  language instead of leaving the health gauge as the only feedback surface.
- Added `ACodeRescueCharacter::LastDamageMitigationText` and HUD damage-alert
  suffixes for mercy-window, per-hit cap, armor plate, survival-lock, and
  emergency-medkit protections.
- Added `Content/CodeRescueData/health_damage_survivability_manifest.tsv`,
  updated the creative inclusion plan, human-QA checklist, and visual
  regression targets, documented the slice in
  `Documentation/improvement_pass_2026-06-30/HEALTH_DAMAGE_SURVIVABILITY_SLICE.md`,
  and wired `Scripts/verify_health_damage_survivability_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new health damage
  survivability verifier, adjacent rescue survivability/HUD vitals/damage
  feedback verifiers, module recompile, Mac packaging, packaged null smoke,
  packaged render smoke, packaged runtime log contracts, and scoped
  whitespace/diff checks.

## 2026-06-30 - Adaptive encounter director pressure slice

- Continued the June 25 AI, physics, world, and release-readiness guidance by
  making the authored encounter director react to objective state and resumed
  player resources instead of only using static role tuning.
- Updated `ACodeRescueGameMode::SpawnEncounterDirectorLayer` so it reads
  solved-terminal state, selected difficulty, and low-health/ammo/utility save
  state before placing the survivor-route pressure pocket.
- Added adaptive route-state/resource-state tags, a visible `DIRECTOR STATE`
  board, low-resource relief pickups, and adaptive health, damage, speed, and
  activation tuning for directed zombies.
- Added `Content/CodeRescueData/encounter_director_adaptive_pressure_manifest.tsv`,
  updated creative inclusion, human-QA, and visual-regression review surfaces,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/ADAPTIVE_ENCOUNTER_DIRECTOR_PRESSURE_SLICE.md`,
  and wired `Scripts/verify_adaptive_encounter_director_pressure_slice_pass.py`
  into full QA/local CI.
- Validation passed: Python verifier compilation, adaptive encounter director
  verifier, adjacent encounter director/destructible cover/physics-lane
  verifiers, module recompile, Mac packaging, packaged null smoke, packaged
  render smoke, packaged runtime log contracts, and scoped whitespace/diff
  checks.

## 2026-06-30 - Human-scale route access slice

- Continued the June 25 world and physics guidance by making the existing
  route-access and architecture-clarity systems explicit validation surfaces
  for the P0 human-scale doors, windows, stairs, and cover row.
- Added `Content/CodeRescueData/human_scale_route_access_manifest.tsv`
  covering entry access clearance, purpose-coded route labels, critical-path
  nonblocking cleanup, objective route beacon handoff, world promotion gates,
  and runtime log contracts.
- Updated the creative inclusion plan, visual-regression targets, and human-QA
  checklist so reviewers walk entry, armory, safehouse, language marker,
  terminal, survivor, and helipad routes for human-scale passability.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/HUMAN_SCALE_ROUTE_ACCESS_SLICE.md`
  and wired `Scripts/verify_human_scale_route_access_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new human-scale route
  access verifier, playability/readability verifier, world-promotion contract
  verifier, demo-readiness verifier, packaged null smoke, packaged render
  smoke, packaged runtime log contracts, and scoped whitespace/diff checks.

## 2026-06-30 - External demo preflight slice

- Continued the June 25 release-pipeline guidance by converting the P2 signed
  external demo row into explicit local-ready and credential-bound gates.
- Added `Content/CodeRescueData/external_demo_preflight_manifest.tsv` covering
  package integrity, bundle identity, local codesign, Gatekeeper/notarization,
  release manifest, support bundle, packaged smokes, and human playtest.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/EXTERNAL_DEMO_PREFLIGHT_SLICE.md`,
  updated the creative inclusion plan, non-human release gates, and release QA
  checklist, and wired `Scripts/verify_external_demo_preflight_slice_pass.py`
  into full QA, local CI, and non-human release readiness.
- Current package evidence: `Scripts/verify_package_integrity_pass.py` reported
  local package ready for bundle ID `com.operationcoderescue.CodeRescueUnreal`
  at 2077.2 MB, with external distribution still blocked by
  Gatekeeper/notarization credentials.
- Validation passed: Python verifier compilation, the new external demo
  preflight verifier, package-integrity preflight, release manifest generation,
  non-human release readiness verifier, current package evidence refresh, and
  scoped whitespace/diff checks.

## 2026-06-30 - Expanded accessibility options slice

- Continued the June 25 accessibility guidance by turning the existing settings
  controls into one auditable expanded-options contract instead of leaving the
  P2 accessibility row as a manual-only checklist item.
- Added `Content/CodeRescueData/expanded_accessibility_options_manifest.tsv`
  covering subtitles, subtitle size, high contrast, color vision, reduced
  motion, simplified hints, aim assist, reset defaults, settings readouts, and
  control profile export.
- Updated the creative inclusion plan, accessibility settings manifest,
  first-ten-minutes onboarding, visual-regression target, and human-QA checklist
  so settings persistence, live refresh, reset behavior, and export behavior
  are all reviewable.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/EXPANDED_ACCESSIBILITY_OPTIONS_SLICE.md`
  and wired `Scripts/verify_expanded_accessibility_options_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new expanded
  accessibility options verifier, settings color-vision live-refresh verifier,
  HUD vitals accessibility verifier, minimap route readability verifier,
  onboarding input glyph verifier, control remap profile export, adjacent
  settings/subtitle/damage/journal accessibility verifiers, and scoped
  whitespace/diff checks.

## 2026-06-30 - Sequencer intro boss reveal slice

- Continued the June 25 character-animation and Top 50 cinematic guidance by
  adding a package-safe blocking layer for intro, selected-language, terminal,
  survivor, extraction, and boss reveal beats.
- Added a visible `SEQUENCER BLOCKING REEL` to the cinematic street-life layer
  with camera rails, tripod markers, lens markers, beat labels, connector paths,
  and shared `SequencerIntroBossRevealBlocking`,
  `CinematicCameraBlockingReady`, `SequencerReadyFallback`, and
  `ControlRigReadyFallback` evidence tags.
- Extended `ABossRevealPresentationActor` so spawned reveal actors and reveal
  components participate in the same Sequencer/Control Rig fallback contract as
  the blocking reel.
- Added `Content/CodeRescueData/cinematic_sequence_blocking_manifest.tsv`,
  updated creative inclusion, visual-regression, and human-QA manifests,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/SEQUENCER_INTRO_BOSS_REVEAL_SLICE.md`,
  and wired `Scripts/verify_sequencer_intro_boss_reveal_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new Sequencer
  intro/boss reveal verifier, adjacent boss reveal presentation verifier,
  demo-readiness verifier, module recompile, Mac packaging, packaged null
  smoke, packaged render smoke, packaged runtime log contracts, and scoped
  whitespace/diff checks.

## 2026-06-30 - Maya character cleanup slice

- Continued the June 25 character-animation and production-pipeline guidance by
  turning Maya cleanup into a runtime-visible contract for every live skeletal
  presentation path.
- Extended `CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots` so player
  body, first-person arms, survivor, friendly NPC, companion, zombie, and boss
  meshes carry `MayaCharacterCleanupRuntimeContract` plus bind-pose, skeleton
  naming, socket, animation-take, LOD/material, FBX export, and physics-asset
  review tags.
- Added a visible Maya cleanup lane to the DCC review bay with stations for
  bind pose, skeleton names, sockets, animation takes, LOD/material budget,
  physics asset, FBX export, and promotion evidence.
- Added `Content/CodeRescueData/maya_character_cleanup_manifest.tsv`, expanded
  animation coverage and character-promotion contracts, updated creative
  inclusion, visual-regression, and human-QA manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/MAYA_CHARACTER_CLEANUP_SLICE.md`,
  and wired `Scripts/verify_maya_character_cleanup_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new Maya character cleanup
  verifier, retarget/control-rig verifier, editor Data Validation contract
  verifier, character-promotion contract verifier, animation-budget verifier,
  player first-person animation verifier, UnrealEditor-Cmd character promotion
  smoke, UnrealEditor-Cmd camera/character roster smoke, module recompile, Mac
  packaging, packaged null smoke, packaged render smoke, scoped
  `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Houdini modular city output slice

- Continued the June 25 world-development and production-pipeline guidance by
  extending the runtime Houdini/PCG review bay into a visible generated-output
  target lane.
- Added a deterministic `HOUDINI OUTPUT RECIPE` board that names the city seed,
  art kit, district, facade cells, rubble variation, collision proxy, route
  spline, and streaming-budget review expectations.
- Added package-safe runtime review modules for facade kit output, safehouse
  cells, rubble variation sets, collision proxies, streaming budget cells, and
  route spline knots.
- Tagged the lane with `HoudiniModularCityOutput`,
  `PCGDeterministicCitySeed`, `HoudiniCityKitRecipe`,
  `PCGRubbleVariationSet`, `PCGCollisionProxyReady`,
  `PCGStreamingBudgetCell`, `PCGRouteSplineReady`, and
  `PCGWorldPartitionBakeReview` so future Houdini/PCG exports have clear
  validation targets before replacing fallback city geometry.
- Moved the Chaos physics review props farther down the bay to keep the new
  generated-output lane readable and walkable.
- Added `Content/CodeRescueData/houdini_modular_city_output_manifest.tsv`,
  updated creative inclusion, visual-regression, human-QA, and performance
  budget manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/HOUDINI_MODULAR_CITY_OUTPUT_SLICE.md`,
  and wired `Scripts/verify_houdini_modular_city_output_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new Houdini modular city
  output verifier, world-promotion contract verifier, Mac asset import budget
  gate verifier, regional city kit identity verifier, UnrealEditor-Cmd world
  promotion smoke, module recompile, Mac packaging, packaged null smoke,
  packaged render smoke, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - Zombie family variants slice

- Continued the June 25 enemy/readability guidance by turning zombie family
  variants into a shared runtime contract instead of isolated spawn snippets.
- Added `ApplyZombieFamilyVariant` and `ApplyCityZombieFamilyVariant` to
  centralize variant initialization, family audit tags, and optional
  save-record behavior for weighted city threats and authored set pieces.
- Wired the contract into regular city waves, physics lane ambushes,
  encounter-director enemies, boss wardens, elite mini-boss sentinels, dog-den
  set pieces, language breach patrols, and terminal-solve horde waves.
- Added family tags to boomer death adds and boss phase adds, which spawn
  outside the GameMode helper path.
- Updated runtime markers so dog, urban, business, bloated, nurse, elite, and
  fallback families are readable through text labels, marker color, and audit
  tags while preserving save-backed assignments for persistent enemies and
  transient behavior for horde/add pressure.
- Added `Content/CodeRescueData/zombie_family_variants_manifest.tsv`, updated
  creative inclusion, enemy readability, animation coverage, visual-regression,
  and human-QA manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/ZOMBIE_FAMILY_VARIANTS_SLICE.md`,
  and wired `Scripts/verify_zombie_family_variants_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new zombie family
  variants verifier, character-promotion contract verifier, adjacent
  elite-warden, encounter-director, physics-lane, language-breach,
  zombie-death, retarget, and animation-budget verifiers, UnrealEditor-Cmd
  character promotion smoke, module recompile, Mac packaging, packaged null
  smoke, and packaged render smoke.

## 2026-06-30 - Retarget Control Rig slots slice

- Continued the June 25 character-animation guidance by adding a runtime
  `CodeRescueRetargetRig` helper that marks live skeletal actors with IK
  retarget and Control Rig integration slots.
- Added profile coverage for the player operator body, first-person arms,
  survivor heroes, friendly NPCs, companions, zombie crowds, and boss wardens.
- Wired the runtime slots into `ACodeRescueCharacter`, `ASurvivorActor`,
  `AFriendlyNPCActor`, `ACompanionActor`, `ACodeZombieActor`, and
  `ABossZombieActor` without replacing the stable fallback meshes or AnimBPs.
- Added `Content/CodeRescueData/retarget_control_rig_slots_manifest.tsv`,
  expanded animation coverage, character-promotion, creative inclusion,
  visual-regression, and human-QA manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/RETARGET_CONTROL_RIG_SLOTS_SLICE.md`,
  and wired `Scripts/verify_retarget_control_rig_slots_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new retarget/control-rig
  verifier, animation-budget verifier, camera/character roster verifier through
  UnrealEditor-Cmd, module recompile, Mac packaging, packaged null smoke,
  packaged render smoke, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - City radio bark cadence slice

- Continued the June 25 audio/readability guidance by making city radio and
  survivor bark subtitles reliable during early world-load timing.
- Added `UCodeRescueSubtitlesWidget::PendingQueue` so subtitle pushes made
  before the HUD subtitle overlay exists are buffered and then flushed only when
  subtitles are enabled.
- Added `BuildRadioRouteCadenceLine` and queued a `[Radio Relay]` line after
  each mission radio briefing, naming city/state, selected language, route
  phase, terminal, survivor, landmark, and next step.
- Preserved the existing cooked radio cue and macOS system voice fallback flow
  while ensuring text-first radio coverage is the guaranteed baseline.
- Added `Content/CodeRescueData/city_radio_bark_cadence_manifest.tsv`, updated
  creative inclusion, audio coverage, visual-regression, human-QA,
  accessibility, and onboarding manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/CITY_RADIO_BARK_CADENCE_SLICE.md`,
  and wired `Scripts/verify_city_radio_bark_cadence_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new city radio bark
  cadence verifier, survivor rescue handoff verifier, module recompile, Mac
  packaging, packaged null smoke, packaged render smoke, scoped
  `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Distinct weapon presentation slice

- Continued the June 25 `CHARACTER_ANIMATION_DEEPDIVE` and
  `TOP_50_RECOMMENDATIONS` weapons/readability guidance by adding a
  package-safe first-person presentation layer for the full 17-weapon arsenal.
- Added `FirstPersonWeaponSilhouette` to `ACodeRescueCharacter` as an owner-only,
  no-collision fallback model attached to the first-person camera.
- Added named presentation profiles for every `EWeaponType`, giving pistols,
  shotguns, rifles, grenades, knife, launchers, and utility explosives distinct
  fallback silhouettes, tint, stance, recoil, reload roll, and bob behavior.
- Connected fire, no-ammo melee fallback, reload start, reload completion,
  weapon swap, camera perspective switching, and Reduced Motion to the visible
  weapon presentation update.
- Added `Content/CodeRescueData/distinct_weapon_presentation_manifest.tsv`,
  updated creative inclusion, visual-regression, human-QA, and accessibility
  manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/DISTINCT_WEAPON_PRESENTATION_SLICE.md`,
  and wired `Scripts/verify_distinct_weapon_presentation_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new distinct weapon
  presentation verifier, tactical arsenal verifier, player first-person
  animation verifier, module recompile, Mac packaging, packaged null smoke,
  packaged render smoke, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - Inventory map and journal polish slice

- Continued the June 25 UI/readability guidance by turning the objective
  journal into a single field-ops surface for selected-language save state,
  route/map status, inventory/loadout resources, survivor intel, and city
  progress.
- Added `LANGUAGE SAVE`, `ROUTE MAP`, and `FIELD INVENTORY` sections to
  `UCodeRescueObjectiveJournalWidget`, backed by live `UCodeRescueGameInstance`
  save data and the active `ACodeRescueCharacter` resource/loadout state.
- Kept the start screen resume contract visible inside the run by showing the
  active language-only save slot and whether that language save already exists.
- Expanded the journal panel height and preserved auto-wrap, high-contrast,
  reduced-motion, and text-scale styling through the shared UI theme.
- Added `Content/CodeRescueData/inventory_map_journal_manifest.tsv`, updated
  creative inclusion, visual-regression, human-QA, accessibility, and
  first-ten-minutes manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/INVENTORY_MAP_JOURNAL_POLISH_SLICE.md`,
  and wired `Scripts/verify_inventory_map_journal_polish_slice_pass.py` into
  full QA/local CI.
- Validation passed: Python verifier compilation, the new inventory/map/journal
  verifier, minimap verifier, objective journal verifier, module recompile, Mac
  packaging, packaged null smoke, packaged render smoke, scoped `git diff
  --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Elite warden mini-boss staging slice

- Continued the June 25 enemy/readability guidance by adding
  `SpawnEliteWardenMiniBossStagingLayer` immediately after the per-city boss
  spawn.
- Added a save-aware warden runway with `INTEL LOCK GATE`,
  `MINI-BOSS SENTINEL LANE`, and `WARDEN PHASE GATE` anchors.
- Tied the runway to `SolvedTerminalIds.Contains(Mission.TerminalId)` and
  `NeutralizedZombieIds.Contains(BossId)` so it presents dormant, active, or
  defeated states from the selected-language save run.
- Added post-intel `CHARGER MINI-BOSS`, `SPITTER MINI-BOSS`, and
  `BOOMER MINI-BOSS` sentinels with stable save IDs, elite variants,
  encounter-director roles, variant recording, and dispatch subtitle feedback.
- Tagged the layer with `EliteWardenMiniBossStaging`,
  `EliteWardenPressureGate`, `MiniBossAfterIntelMilestone`,
  `TextFirstEnemyReadability`, `NoAccessBlocker`,
  `CharacterAnimationDeepDive`, `Top50Recommendations`, and runtime marker
  `[CodeRescueEliteWardenMiniBoss]`.
- Added `Content/CodeRescueData/elite_warden_miniboss_manifest.tsv`, updated
  creative inclusion, enemy readability, visual-regression, human-QA, and
  accessibility manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/ELITE_WARDEN_MINIBOSS_SLICE.md`,
  and wired `Scripts/verify_elite_warden_miniboss_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new elite warden
  mini-boss verifier, boss reveal verifier, boss phase telegraph verifier,
  module recompile, Mac packaging, packaged null smoke, packaged render smoke,
  runtime log confirmation for `[CodeRescueEliteWardenMiniBoss]`, scoped
  `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Challenge room concept art slice

- Continued the June 25 learning/world guidance by adding
  `SpawnChallengeRoomConceptArtLayer` after the protected coding safehouse.
- Added five open-front, nonblocking concept rooms: `VARIABLES LAB`,
  `LOOP CONTROL ROOM`, `ARRAY INDEX HALL`, `FUNCTION RELAY ROOM`, and
  `DEBUGGER TEST BAY`.
- Connected each room to the active selected-language track and mission
  curriculum so the rooms teach the current challenge rather than showing a
  generic coding display.
- Added a briefing board that surfaces curriculum focus, visible and hidden
  tests, hint text, learning support, visual debugger plan, and progression plan.
- Added lesson-specific artifacts for lock/truth gates, reverse arrows,
  palindrome mirrors, FizzBuzz beacons, even-filter lanes, linked-list chains,
  binary-search bands, and default sum power cells.
- Tagged the layer with `ChallengeRoomConceptArt`,
  `ChallengeConceptRoomReady`, `CodeConceptPhysicalSpace`,
  `TextFirstLearningCue`, `ProtectedLearningSpace`, `SelectedLanguageOnly`,
  `LearningWithoutDeathRisk`, `NoAccessBlocker`, `WorldDevelopmentDeepDive`,
  `Top50Recommendations`, and the runtime marker
  `[CodeRescueChallengeRoomConceptArt]`.
- Added `Content/CodeRescueData/challenge_room_concept_art_manifest.tsv`,
  updated creative inclusion, curriculum feedback, onboarding,
  visual-regression, human-QA, and accessibility manifests, documented the slice
  in
  `Documentation/improvement_pass_2026-06-30/CHALLENGE_ROOM_CONCEPT_ART_SLICE.md`,
  and wired `Scripts/verify_challenge_room_concept_art_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new challenge room
  concept art verifier, Unreal commandlet curriculum validator,
  `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command < /dev/null`,
  packaged null smoke, packaged render smoke, packaged runtime log contracts,
  runtime log confirmation of `[CodeRescueChallengeRoomConceptArt]`, scoped
  `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Regional city kit identity slice

- Continued the June 25 world-development guidance by adding
  `SpawnRegionalCityKitIdentityLayer` after the existing city landmark, art-kit,
  urban identity, and U.S. city-specific identity layers.
- Added three path-adjacent, nonblocking regional-kit anchors:
  `REGIONAL KIT ENTRY GATE`, `LANDMARK VISTA KIT`, and `OBJECTIVE DISTRICT KIT`.
- Reused the mission art kit, region, district style, landmark, visual profile,
  terrain token, signature cue, and district cue so city-family identity is
  visible as data-driven play-space dressing rather than only a manifest row.
- Added modular kit swatches for trim sheets, facade modules, prop dressing, and
  destruction dressing, plus art-kit motif stand-ins for harbor, desert solar,
  mountain, industrial, civic, transit, and metro fallback families.
- Tagged the layer with `RegionalCityKitIdentity`, `MajorCityRegionalKit`,
  `RegionalKitReady`, `LandmarkWayfindingKit`, `DistrictLevelInstanceStandIn`,
  `KitBibleRuntimeCue`, `WorldDevelopmentDeepDive`, `Top50Recommendations`, and
  the runtime marker `[CodeRescueRegionalCityKits]`.
- Added `Content/CodeRescueData/regional_city_kit_identity_manifest.tsv`,
  updated creative inclusion, visual-regression, human-QA, and accessibility
  manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/REGIONAL_CITY_KIT_IDENTITY_SLICE.md`,
  and wired `Scripts/verify_regional_city_kit_identity_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new regional city kit
  identity verifier, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render
  smoke, packaged runtime log contracts, runtime log confirmation of
  `[CodeRescueRegionalCityKits]`, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - Weather and lighting identity slice

- Continued the June 25 world-development guidance by adding
  `SpawnWeatherLightingIdentityLayer` after the existing per-city weather hook.
- Reused the active city climate profile and realization params so visible weather cues
  match existing cloud token, grade token, fog density, and sun color behavior.
- Added district-level weather/light cues for `ENTRY WEATHER SHELTER`,
  `SAFEHOUSE LIGHT POOL`, and `RESCUE ROUTE SKY CUE`, each with ground reflections,
  weather streaks, text-first climate labels, and point-light signal pools.
- Added readable climate family labels including `OVERCAST SHELTER LIGHTING`,
  `MARINE FOG ROUTE LIGHTING`, `WARM HAZE HEAT MIRAGE`, `HUMID STORM GLOW`,
  `COLD SNOW SKY`, and `CLEAR SKY CONTRAST`.
- Tagged the layer with `WeatherLightingIdentity`, `DistrictWeatherCue`,
  `NonBlockingWeatherCue`, `WeatherLightingSignalLight`, `WorldDevelopmentDeepDive`,
  and `Top50Recommendations`, and added the runtime marker
  `[CodeRescueWeatherLightingIdentity]`.
- Added `Content/CodeRescueData/weather_lighting_identity_manifest.tsv`, updated creative
  inclusion, visual-regression, human-QA, and accessibility manifests, documented the
  slice in `Documentation/improvement_pass_2026-06-30/WEATHER_LIGHTING_IDENTITY_SLICE.md`,
  and wired `Scripts/verify_weather_lighting_identity_slice_pass.py` into full QA/local CI.
- Validation passed: Python verifier compilation, the new weather lighting identity
  verifier, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render
  smoke, packaged runtime log contracts, runtime log confirmation of
  `[CodeRescueWeatherLightingIdentity]`, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - Interior mission spaces slice

- Continued the June 25 world-development guidance by adding `SpawnInteriorMissionSpacesForCity`
  to every streamed campaign city after the enterable civic safehouse layer.
- Added five open-front, human-scale mission interiors: `HOSPITAL TRIAGE CLINIC`,
  `SCHOOL STUDY SHELTER`, `CORNER STORE CACHE`, `TRANSIT OPERATIONS HUB`, and
  `CIVIC RECORDS ANNEX`.
- Each room now has floor/wall/header geometry, readable door gaps, text-first labels,
  mission boards, point-light identity, and replacement tags for future imported
  interior kits.
- Added functional room pickups so interiors are playable spaces: medkit, radio scanner
  charge, ammo pouch, flashlight battery, and bypass kit.
- Mission boards connect the active city curriculum, survivor, landmark, and
  terminal -> survivor -> helipad route context.
- Added `Content/CodeRescueData/interior_mission_spaces_manifest.tsv`, updated creative
  inclusion, onboarding, visual-regression, human-QA, accessibility, and curriculum
  feedback manifests, documented the slice in
  `Documentation/improvement_pass_2026-06-30/INTERIOR_MISSION_SPACES_SLICE.md`, and wired
  `Scripts/verify_interior_mission_spaces_slice_pass.py` into full QA/local CI.
- Validation passed: Python verifier compilation, the new interior mission spaces
  verifier, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render
  smoke, packaged runtime log contracts, runtime log confirmation of
  `[CodeRescueInteriorMissionSpaces]`, scoped `git diff --check`, and touched-file
  trailing-whitespace scan.

## 2026-06-30 - Tactical gear pickups slice

- Continued the June 25 Top 50/gameplay guidance by expanding tactical pickups from
  partial utility resources into a full saved field kit: `RadioScanner`,
  `FlashlightBattery`, `AmmoPouch`, `BypassKit`, medkit, armor, throwables, and scrap.
- Added real player systems for scanner charges, flashlight batteries, bypass kits,
  and ammo pouch capacity. `L` toggles a real `FieldFlashlight` point light, `Z`
  consumes a scanner charge for route guidance, and ammo pouches expand/refill reserve
  capacity.
- Added a `USE BYPASS KIT [Ctrl+B]` terminal action that consumes a bypass kit, records
  the selected language track, opens the survivor route, and disables clean-solve rewards
  for that terminal.
- Extended selected-language save persistence with `PlayerRadioScannerCharges`,
  `PlayerFlashlightBatteries`, `PlayerBypassKits`, `PlayerAmmoPouchCapacityBonus`,
  throwables, scrap, armor plates, and `bHasPlayerTacticalGear`, restored through
  `RestorePlayerResourcesDetailed`.
- Placed the expanded gear in the tactical armory, creative gear hub, and city route;
  updated HUD status, second-line, weapon-strip, and tactical readouts to show the full
  field-kit state.
- Added `Content/CodeRescueData/tactical_gear_pickups_manifest.tsv`, updated creative
  inclusion, onboarding, visual-regression, human-QA, and accessibility manifests,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/TACTICAL_GEAR_PICKUPS_SLICE.md`, and wired
  `Scripts/verify_tactical_gear_pickups_slice_pass.py` into full QA/local CI.
- Validation passed: Python verifier compilation, the new tactical gear pickups
  verifier, adjacent June 18 pickup/language/save verifiers, save compatibility verifier,
  `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command < /dev/null`,
  packaged null smoke, packaged render smoke, packaged runtime log contracts,
  scoped `git diff --check`, and touched-file trailing-whitespace scan.

## 2026-06-30 - Expanded extraction set-pieces slice

- Continued the June 25 world-development and Top 50 extraction guidance by adding
  `SpawnExpandedExtractionSetPieceForCity` to the campaign city spawn flow immediately
  after helipad creation.
- Added deterministic city variants for RooftopLift, ConvoyGate, HarborBoat, RailEvac,
  BridgeRun, and HelipadCommand so each city receives a readable authored extraction
  finale around the helipad.
- The staging uses existing cook-safe `SpawnBlock`, `SpawnRotatedBlock`, and
  `SpawnGuideText` helpers, keeps all decorative pieces nonblocking, and tags actors with
  `ExpandedExtractionSetPiece`, variant tags, `ExtractionSetPieceNonBlocking`,
  `HelipadClearancePreserved`, `WorldDevelopmentDeepDive`, `Top50Recommendations`, and
  `ReleaseDossier`.
- Extraction labels now name the set-piece variant, survivor, landmark, curriculum focus,
  and city so the end-of-city rescue area connects coding payoff, survivor rescue, and
  extraction route identity without changing the helipad fast-travel interaction.
- Added `Content/CodeRescueData/expanded_extraction_set_pieces_manifest.tsv`, updated
  curriculum feedback, creative inclusion, onboarding, visual-regression, human-QA, and accessibility manifests,
  documented the slice in
  `Documentation/improvement_pass_2026-06-30/EXPANDED_EXTRACTION_SET_PIECES_SLICE.md`,
  and wired `Scripts/verify_expanded_extraction_set_pieces_slice_pass.py` into full
  QA/local CI.
- Validation passed: Python verifier compilation, the new expanded extraction
  set-pieces verifier, helipad extraction-ready verifier, extraction debrief
  fast-travel verifier, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render
  smoke, packaged runtime log contracts, scoped `git diff --check`, touched-file
  trailing-whitespace scan, and runtime log confirmation of
  `[CodeRescueExtractionSetPiece]`.

## 2026-06-30 - Radio scan rescue beacon slice

- Continued the June 25 world/VFX/readability guidance by turning the existing objective beacon into a procedural radio-scan and rescue-beacon effect layer.
- Extended `AObjectiveFocusBeaconActor` with `RadioScanRing`, `RadioSweepArm`, `RescueBeaconHalo`, `RadioPingA`, `RadioPingB`, and `RadioScanLabel`, all built from cook-safe engine primitives and kept nonblocking.
- Added mission copy fields for terminal title, curriculum concept, and landmark so beacon labels can name the selected-language terminal route, survivor ping context, and extraction landmark.
- Updated phase copy to read `RADIO SCAN`, `SURVIVOR PING`, and `RESCUE BEACON` while preserving the existing terminal, survivor, and extraction objective-state logic.
- Reduced Motion now slows scan/sweep/ping/halo motion while text-first labels remain visible for accessibility review.
- Added `Content/CodeRescueData/radio_scan_rescue_beacon_manifest.tsv`, updated curriculum, onboarding, visual-regression, human-QA, accessibility, and creative inclusion manifests, documented the slice in `Documentation/improvement_pass_2026-06-30/RADIO_SCAN_RESCUE_BEACON_SLICE.md`, and added `Scripts/verify_radio_scan_rescue_beacon_slice_pass.py`, wired into full QA/local CI.
- Validation passed: Python verifier compilation, the new radio scan rescue beacon verifier, objective focus beacon regression verifier, `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render smoke, packaged runtime log contracts, scoped `git diff --check`, and trailing-whitespace scan for the new script/documentation/manifest and touched beacon source files.

## 2026-06-30 - Case file collectibles slice

- Continued the June 25 world/story/curriculum guidance by turning the planned collectible case files into playable, save-backed pickups instead of a manifest-only recommendation.
- Added `ACaseFilePickupActor` with a glowing ground-snapped slab, overlap/interact collection, `CASE FILE` subtitle feedback, collected-state hiding, and tags for the source deep-dive guidance.
- Added `CollectedCaseFileIds`, `LastCollectedCaseFileTitle`, `HasCollectedCaseFile()`, `RecordCaseFileCollected()`, and `GetCaseFileCollectionSummary()` to the selected-language save contract.
- Added `SpawnCollectibleCaseFilesForCity()` to spawn terminal evidence, survivor notes, and route briefs per mission using existing campaign fields for coding concept, language track, survivor story, landmark, progression, and accessibility context.
- Wired case files into nearest-interactable scanning, persistent level restoration, and the objective journal summary so collected files remain visible as save progress and hidden on relaunch.
- Added `Content/CodeRescueData/case_file_collectibles_manifest.tsv`, updated curriculum, onboarding, visual-regression, human-QA, accessibility, creative inclusion, and character/world manifests, documented the slice in `Documentation/improvement_pass_2026-06-30/CASE_FILE_COLLECTIBLES_SLICE.md`, and added `Scripts/verify_case_file_collectibles_slice_pass.py`, wired into full QA/local CI.
- Validation passed: Python verifier compilation, the new case-file collectibles verifier, survivor rescue dialogue handoff verifier, survivor intel dossier verifier, terminal post-solve debrief verifier, objective journal accessibility verifier, demo-readiness verifier, `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render smoke, packaged runtime log contracts, scoped `git diff --check`, and trailing-whitespace scan for the new source/script/documentation/manifest files.

## 2026-06-30 - Survivor rescue dialogue handoff slice

- Continued the June 25 character, world, curriculum payoff, and release-readiness guidance by making the survivor rescue moment explain the selected-language route payoff instead of using generic thank-you copy.
- Added mission-aware helper copy builders to `ASurvivorActor`: `BuildSurvivorLockedRouteLine()`, `BuildSurvivorRescueLine()`, `BuildExtractionDispatchLine()`, and `BuildCompanionHandoffLine()`.
- Blocked survivor interaction now names the terminal, concept, selected language, and landmark that still gate the rescue; successful rescue now names the selected-language fix, mission concept, city landmark, novel gameplay payoff, and survivor story.
- Dispatch now queues an extraction handoff after `GI->SavePersistentRun()`, confirming the survivor, city, live helipad route, selected-language save update, and `RESCUED` journal dossier state.
- Updated curriculum, onboarding, visual-regression, human-QA, creative inclusion, and character/world manifests, documented the slice in `Documentation/improvement_pass_2026-06-30/SURVIVOR_RESCUE_DIALOGUE_HANDOFF_SLICE.md`, and added `Scripts/verify_survivor_rescue_dialogue_handoff_slice_pass.py`, wired into full QA/local CI.
- Validation passed: Python verifier compilation, the new survivor rescue dialogue handoff verifier, rescue extraction presentation verifier, helipad extraction-ready verifier, survivor intel dossier verifier, terminal post-solve debrief verifier, demo-readiness verifier, `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render smoke, packaged runtime log contracts, and scoped whitespace checks.

## 2026-06-30 - Survivor intel dossier slice

- Continued the June 25 world/curriculum payoff guidance by making terminal success reviewable in the objective journal instead of only visible in terminal output.
- Added `IntelText`, `FindJournalMissionProgress()`, `SurvivorIntelStatusLabel()`, `SurvivorIntelNextStep()`, and `BuildSurvivorIntelDossier()` to `UCodeRescueObjectiveJournalWidget`.
- The journal now shows a `SURVIVOR INTEL DOSSIER` card with `LOCKED`, `ROUTE OPEN`, or `RESCUED` status, survivor contact, city rank, location, lesson payoff, validation attempts/best score, selected-language run, and next step.
- Updated curriculum, accessibility, onboarding, visual-regression, human-QA, and creative inclusion manifests, documented the slice in `Documentation/improvement_pass_2026-06-30/SURVIVOR_INTEL_DOSSIER_SLICE.md`, and added `Scripts/verify_survivor_intel_dossier_slice_pass.py`, wired into full QA/local CI.
- Validation passed: Python verifier compilation, the new survivor intel dossier verifier, objective journal accessibility verifier, terminal post-solve debrief verifier, demo-readiness verifier, `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render smoke, packaged runtime log contracts, and scoped whitespace checks.

## 2026-06-30 - Terminal post-solve debrief slice

- Continued the June 25 curriculum-feedback guidance by making terminal pass/fail output explain what the player learned instead of only reporting score and rewards.
- Added `GetConceptProofForChallenge()`, `GetLanguageTransferForChallenge()`, `GetNextPracticeRepForChallenge()`, `BuildPostSolveAfterActionDebrief()`, and `BuildRepairDebrief()` to `UCodeTerminalWidget`.
- Successful solves now append a `POST-SOLVE DEBRIEF` with concept proof, selected-language transfer, survivor-route follow-up, language save continuity, next practice, reward state, and language progress while preserving the existing survivor intel reward signal.
- Failed solves now append a `REPAIR DEBRIEF` with the active failed check, selected-language tactic, next validation move, and safehouse pause reminder.
- Updated curriculum, onboarding, visual-regression, human-QA, and creative inclusion manifests, documented the slice in `Documentation/improvement_pass_2026-06-30/TERMINAL_POST_SOLVE_DEBRIEF_SLICE.md`, and added `Scripts/verify_terminal_post_solve_debrief_slice_pass.py`, wired into full QA/local CI.
- Validation passed: Python verifier compilation, the new terminal post-solve debrief verifier, terminal diegetic restyle verifier, terminal language-track verifier, demo-readiness verifier, `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render smoke, packaged runtime log contracts, and scoped whitespace checks.

## 2026-06-30 - Terminal diegetic restyle slice

- Continued the June 25 coding-surface UX guidance by strengthening `UCodeTerminalWidget` as a
  readable safehouse terminal instead of a loose form overlay.
- Added persistent terminal chrome members for a status strip, framed code editor, diagnostics
  frame, diagnostics header, and themed panel/editor fills.
- The terminal now refreshes selected language, save profile, solved/active state, external
  toolchain versus in-engine fallback, high-contrast fills, reduced blur, and scalable monospace
  code styling from saved settings.
- Grouped Validate, Reset, Hint, MATLAB, and Close actions into two horizontal action rows with
  wrapped centered labels so the terminal is easier to scan at release-test resolutions.
- Updated `Content/CodeRescueData/curriculum_feedback_manifest.tsv`,
  `Content/CodeRescueData/accessibility_settings_manifest.tsv`, and
  `Content/CodeRescueData/visual_regression_targets.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/TERMINAL_DIEGETIC_RESTYLE_SLICE.md`, and added
  `Scripts/verify_terminal_diegetic_restyle_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Objective journal accessibility slice

- Continued the June 25 UX/accessibility rollout by rebuilding the objective journal from a
  symbol-heavy list into a theme-aware route-state panel.
- Added a styled journal panel, route summary, saved-language/accessibility summary, and scalable
  wrapped mission rows to `UCodeRescueObjectiveJournalWidget`.
- Replaced ambiguous `[X]`, `[>]`, `[ ]`, and `[L]` markers with text-first `DONE`, `ACTIVE`,
  `OPEN`, and `LOCKED` labels while keeping high-contrast colors as a secondary cue.
- The active mission summary now reports the current coding terminal, survivor rescue, or extraction
  phase using saved terminal/survivor progress for the selected language.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` and
  `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/OBJECTIVE_JOURNAL_ACCESSIBILITY_SLICE.md`, and added
  `Scripts/verify_objective_journal_accessibility_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Headshot feedback reduced-motion slice

- Continued the June 25 HUD rollout by replacing the one-size-fits-all headshot popup with
  accessibility-aware precision-hit feedback.
- Added `HeadshotFeedbackSlot` and `RefreshHeadshotFeedback()` to `UCodeRescueHUDWidget` so
  headshot feedback timing, text, motion, color, and size are handled in one auditable helper.
- Standard motion keeps the brief `HEADSHOT` pop/fade reward, while Reduced Motion shows a longer
  fixed `PRECISION HIT` readout with no slot movement.
- High Contrast HUD now uses bright amber precision-hit text, matching the damage and threat HUD
  accessibility language.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` and
  `Content/CodeRescueData/enemy_readability_manifest.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/HEADSHOT_FEEDBACK_REDUCED_MOTION_SLICE.md`, and added
  `Scripts/verify_headshot_feedback_reduced_motion_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Damage feedback accessibility slice

- Continued the June 25 combat readability and accessibility backlog by making the live damage
  overlay respond to saved high-contrast and reduced-motion settings.
- Added `UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState()`, active-instance tracking,
  and `ApplyAccessibilityStateFromSettings()` so Settings Apply can refresh the current overlay.
- Added high-contrast amber damage vignette/chevron colors, larger high-contrast directional
  chevrons, and a steady reduced-motion hit flash duration.
- Updated the HUD damage alert so the written direction/source/distance warning uses a high-contrast
  amber color when `bHighContrastHUD` is enabled.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` and
  `Content/CodeRescueData/enemy_readability_manifest.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/DAMAGE_FEEDBACK_ACCESSIBILITY_SLICE.md`, and added
  `Scripts/verify_damage_feedback_accessibility_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Subtitle accessibility live refresh slice

- Continued the June 25 accessibility and release-readiness backlog by making active subtitles
  refresh immediately when subtitle settings are applied.
- Added `UCodeRescueSubtitlesWidget::RefreshAccessibilityState()` and a stable
  `BaseSubtitleFont` so subtitle scale can be reapplied without compounding font sizes.
- Added `ApplyAccessibilityStateFromSettings()` to re-read saved subtitle toggle, subtitle scale,
  and high-contrast settings from `UCodeRescueGameInstance`.
- Clears the active subtitle queue and visible line immediately when subtitles are disabled, so
  threat captions and dispatch lines obey Settings Apply in live play.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/SUBTITLE_ACCESSIBILITY_LIVE_REFRESH_SLICE.md`, and
  added `Scripts/verify_subtitle_accessibility_live_refresh_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Threat compass HUD slice

- Continued the June 25 combat readability and accessibility backlog by adding a persistent visual
  threat compass to the active HUD.
- Added `ThreatCompassText` plus `FCodeRescueThreatHudInfo` / `GetNearestHudThreat()` to scan living
  bosses and zombies once per HUD refresh.
- Reports nearest hostile urgency, encounter-director role, variant label, player-relative direction,
  and distance, with boss and elite pressure promoted above ordinary tracking.
- Replaced the old tactical readout's plain `Nearest hostile` distance with a fuller threat line and
  hooked boss/elite urgency into the existing reload/low-health alert strip.
- Updated `Content/CodeRescueData/enemy_readability_manifest.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/THREAT_COMPASS_HUD_SLICE.md`, and added
  `Scripts/verify_threat_compass_hud_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Threat audio captions slice

- Continued the June 25 accessibility/readability backlog by adding subtitle-backed captions for
  nearby hostile audio and elite pressure events.
- Added `ACodeZombieActor::PushThreatCaption`, with per-zombie cooldown, proximity gating,
  player-relative direction labels, encounter-role prefixes, and variant labels.
- Routed captions through `UCodeRescueSubtitlesWidget::Push`, preserving the saved subtitles toggle
  and subtitle-scale behavior.
- Hooked captions to ambient growls, melee attacks, barricade strikes, death cues, spitter acid,
  charger dash, and boomer split-spawn events.
- Updated `Content/CodeRescueData/enemy_readability_manifest.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/THREAT_AUDIO_CAPTIONS_SLICE.md`, and added
  `Scripts/verify_threat_audio_captions_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Settings color vision live refresh slice

- Continued the June 25 accessibility/release-readiness backlog by making the color-vision settings
  selector affect active world grading immediately rather than only future city loads.
- Extracted per-zone post-process grading into `ACodeRescueGameMode::ConfigurePerZonePostProcessVolume`
  so city climate grades and color-vision corrections share one implementation.
- Tagged spawned post-process volumes with refresh metadata and added
  `RefreshActiveColorVisionPostProcess(EColorblindMode)` to update active streamed volumes from
  Settings Apply.
- Updated `UCodeRescueSettingsWidget::OnApplyClicked` to call the refresh after saving the selected
  color mode and to report the number of refreshed active world grades to the player.
- Updated the accessibility settings manifest, documented the slice in
  `Documentation/improvement_pass_2026-06-30/SETTINGS_COLOR_VISION_LIVE_REFRESH_SLICE.md`, and
  added `Scripts/verify_settings_color_vision_live_refresh_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Settings color vision slice

- Continued the release-facing accessibility work by exposing the existing saved
  `EColorblindMode` path in Settings, tied to `TOP_50_RECOMMENDATIONS` item 12.
- Added a `Color Vision Mode` cycle button to `UCodeRescueSettingsWidget` covering Standard,
  Deuteranope, Protanope, and Tritanope.
- Restored the saved color-vision mode on menu open, showed the queued mode in the Accessibility
  readout, reset it through `Reset Accessibility Defaults`, and wrote it back to
  `UCodeRescueGameInstance::ColorblindMode` on Apply.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/SETTINGS_COLOR_VISION_SLICE.md`, and added
  `Scripts/verify_settings_color_vision_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Settings audio accessibility slice

- Continued the June 25 creative-development backlog by tightening the settings menu's release-facing
  audio/accessibility contract, tied to `TOP_50_RECOMMENDATIONS`, the release dossier, and the
  `accessibility_settings_manifest` / `audio_coverage_manifest` data.
- Added persistent `MasterVolume`, `SfxVolume`, and `MusicVolume` fields to the save/game-instance
  layer, with live `GetAudioMixSummary`, SFX/music scalars, and `ApplyAudioMixSettings()`.
- Routed menu/city/boss music, city ambience, cooked radio cues, weapon sounds, zombie sounds, and
  survivor voice cues through the saved mix where runtime code owns the playback.
- Expanded `UCodeRescueSettingsWidget` with audio/gameplay/accessibility readouts that refresh as
  sliders and toggles change, plus a `Reset Accessibility Defaults` action that queues safe defaults
  and commits only when Apply is pressed.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv`, documented the slice in
  `Documentation/improvement_pass_2026-06-30/SETTINGS_AUDIO_ACCESSIBILITY_SLICE.md`, and added
  `Scripts/verify_settings_audio_accessibility_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Pause difficulty matrix slice

- Continued the June 25 creative-development backlog by making difficulty selection legible in
  active play, tied to `TOP_50_RECOMMENDATIONS`, `OPERATION_CODE_RESCUE_RELEASE_DOSSIER`, and the
  `difficulty_presets` / `first_ten_minutes_onboarding` data.
- Expanded `UCodeRescuePauseWidget` with a live difficulty matrix readout under the difficulty
  cycle button.
- Shows the full Story -> Easy -> Normal -> Hard -> Survival -> Nightmare preset order, zombie
  health/damage multipliers, player intent, and first-ten-minutes expectation for each preset.
- Grouped pause commands by purpose using visible labels for `RUN`, `SAVE`, `BALANCE`, `OPTIONS`,
  `LEARNING`, `LOADOUT`, and `EXIT`.
- Mirrored high contrast, reduced motion, and text scale into the pause UI before styling the menu.
- Preserved existing behavior: difficulty still cycles with one button, saves immediately, and
  applies to newly spawned enemies / restarted runs.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/PAUSE_DIFFICULTY_MATRIX_SLICE.md` and added
  `Scripts/verify_pause_difficulty_matrix_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Onboarding input glyph slice

- Continued the June 25 creative-development backlog by closing a first-launch usability gap tied
  to `TOP_50_RECOMMENDATIONS`, `OPERATION_CODE_RESCUE_RELEASE_DOSSIER`, and the
  `first_ten_minutes_onboarding` guidance.
- Added the missing `C+` launch-language option to `UCodeRescueMainMenuWidget`, including fresh-run
  and resume-save buttons, explicit handlers, and C+-specific save-slot verification.
- Updated the fallback launch language scene so all six supported tracks - Java, Python, C, C+,
  C++, and MATLAB - receive visible `TRACK ONLY` start-screen pedestals.
- Expanded `UCodeRescueTutorialWidget` with a phase strip, active language-save status line,
  page-specific input glyph cards, simplified-hints variants, and shared UI theme styling.
- Preserved the start-screen flow: the language selector still appears before gameplay, while the
  tutorial now reinforces the selected language save after deployment or from replay.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/ONBOARDING_INPUT_GLYPH_SLICE.md` and added
  `Scripts/verify_onboarding_input_glyph_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Objective focus beacon slice

- Continued the June 25 creative-development backlog by adding state-aware in-world objective
  clarity tied to `WORLD_DEVELOPMENT_DEEPDIVE`, `TOP_50_RECOMMENDATIONS`, and the release dossier
  guidance.
- Added `AObjectiveFocusBeaconActor`, a cook-safe runtime beacon with base ring, vertical column,
  rotating direction arrow, pulse core, phase progress nodes, text label, and objective light.
- Wired `SpawnPurposeClarityLayer()` so each city receives one registered beacon configured with
  canonical entry, terminal, survivor, and extraction coordinates.
- Made the beacon read the active language save state: unsolved terminal points to the language
  terminal, solved terminal points to the survivor team, and rescued survivor points to extraction
  while the player remains near the completed city.
- Added reduced-motion support plus `OnObjectiveBeaconPhaseChanged` as a Blueprint event for later
  authored Niagara, sound, camera, spline, or UI treatment.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/OBJECTIVE_FOCUS_BEACON_SLICE.md` and added
  `Scripts/verify_objective_focus_beacon_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Boss phase telegraph slice

- Continued the June 25 creative-development backlog by making boss phase changes readable during
  active combat, tied to `CHARACTER_ANIMATION_DEEPDIVE`, `GAME_PHYSICS_DEEPDIVE`,
  `TOP_50_RECOMMENDATIONS`, and the release dossier guidance.
- Extended `ABossZombieActor` with a cook-safe phase telegraph rig built from engine primitives:
  phase ring, overhead core, rotating warning sweep, phase 3 add-spawn beacons, and warning light.
- Wired `EnterPhase()` so phase 2 now presents an orange sprint/regen warning and phase 3 presents
  a stronger red add-spawn warning while preserving the existing speed, regeneration, and add-spawn
  behavior.
- Added `OnBossPhaseTelegraphStarted` as a Blueprint event so future authored animation, Niagara,
  sound, Control Rig, or camera work can hook into the same phase boundary.
- Added reduced-motion support by damping sweep, beacon, bobbing, and pulse motion while preserving
  readable warning color, light, and silhouette feedback.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/BOSS_PHASE_TELEGRAPH_SLICE.md` and added
  `Scripts/verify_boss_phase_telegraph_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Boss reveal presentation slice

- Continued the June 25 creative-development backlog by adding a proximity-triggered boss reveal
  presentation tied to `CHARACTER_ANIMATION_DEEPDIVE`, `TOP_50_RECOMMENDATIONS`, and the release
  dossier guidance.
- Added `ABossRevealPresentationActor`, a cook-safe runtime reveal beat with arena ring, threat
  gates, sweep bars, hovering boss crown, orbit beacons, and warning light.
- Exposed `OptionalSequencerRevealAsset` and `OnBossRevealStarted` so later authored Sequencer and
  Control Rig boss reveals can replace the C++ fallback without changing encounter spawning.
- Wired `SpawnBossForCity()` so each undefeated city boss receives a registered reveal layer that
  follows the boss, triggers when the player approaches, and self-cleans after the reveal duration.
- Added reduced-motion support by damping sweep, orbit, crown bob, and pulse movement while keeping
  warning color and light readability intact.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/BOSS_REVEAL_PRESENTATION_SLICE.md` and added
  `Scripts/verify_boss_reveal_presentation_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Extraction debrief fast-travel slice

- Continued the June 25 creative-development backlog by turning the extraction-ready helipad into
  a playable debrief and redeployment point tied to `WORLD_DEVELOPMENT_DEEPDIVE`,
  `TOP_50_RECOMMENDATIONS`, and the release dossier guidance.
- Extended `UCityFastTravelWidget` with helipad context fields for source city, survivor name,
  extraction-ready state, accent color, and next-city continuation.
- Updated `AHelipadActor::OpenFastTravelMenu()` so extraction-ready helipads pass their rescue
  state into the widget before it appears.
- Added an extraction debrief title and body that confirms the rescued survivor, source city, and
  active coding language save.
- Added a `Continue operation` action that uses `FCodeRescueCampaign::GetFirstIncompleteCityIndex`
  to send the player to the next incomplete city while retaining the existing solved-city fast
  travel list.
- Saved the active language run after every fast-travel teleport so post-extraction redeployment
  persists if the player closes the game.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/EXTRACTION_DEBRIEF_FAST_TRAVEL_SLICE.md` and added
  `Scripts/verify_extraction_debrief_fast_travel_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Helipad extraction-ready slice

- Continued the June 25 creative-development backlog by closing the survivor rescue loop with a
  world-visible extraction-ready helipad state tied to `WORLD_DEVELOPMENT_DEEPDIVE`,
  `TOP_50_RECOMMENDATIONS`, and the release dossier guidance.
- Extended `AHelipadActor` with a dormant extraction visual rig built from engine primitives:
  vertical column, sweep bars, hovering beacon, and rescue-colored light pulse.
- Wired `ASurvivorActor::Rescue()` so the current city's helipad becomes extraction-ready as soon
  as survivor rescue succeeds, while preserving the existing save and companion-spawn behavior.
- Updated `SpawnHelipadForCity()` so already rescued survivor progress restores the helipad's
  extraction-ready state after save/load or city respawn.
- Added reduced-motion support by damping sweep rotation, beacon bobbing, and light-pulse motion
  while preserving clear color and silhouette feedback.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/HELIPAD_EXTRACTION_READY_SLICE.md` and added
  `Scripts/verify_helipad_extraction_ready_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Animated route guidance drone slice

- Continued the June 25 creative-development backlog by adding active solved-route wayfinding tied
  to `WORLD_DEVELOPMENT_DEEPDIVE`, `CHARACTER_ANIMATION_DEEPDIVE`, and
  `TOP_50_RECOMMENDATIONS`.
- Added `ARescueRouteGuidanceDroneActor`, a cook-safe animated drone built from engine primitives
  with body, nose marker, rotors, signal panel, and route-guidance light.
- Wired drones into `RevealSolvedTerminalRescueRoute()` so every solved coding terminal now spawns
  moving guidance drones along each route segment from terminal to survivor/extraction path.
- Preserved existing solved-route contracts: terminal-specific duplicate tags, streamed cleanup,
  save/load route reconstruction, route reward pickups, and the language breach encounter remain
  intact.
- Added reduced-motion support by damping patrol speed, hover bobbing, rotor motion, and signal
  panel motion while keeping color and light guidance readable.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/ROUTE_GUIDANCE_DRONE_SLICE.md` and added
  `Scripts/verify_route_guidance_drone_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Rescue extraction presentation slice

- Continued the June 25 creative-development backlog by adding a survivor rescue/extraction beat
  tied to `CHARACTER_ANIMATION_DEEPDIVE`, `WORLD_DEVELOPMENT_DEEPDIVE`,
  `TOP_50_RECOMMENDATIONS`, and the release dossier's playable-polish guidance.
- Added `ARescueExtractionPresentationActor`, a cook-safe runtime presentation actor with landing
  disc, rescue beam, sweep arms, lift marker, orbit beacons, and key/fill lights.
- Wired the presentation into `ASurvivorActor::Rescue()` so successful rescues now spawn a clear
  extraction moment using the current city mission colors before the survivor is hidden.
- Preserved progression and save behavior: survivor gating, persistent rescue recording,
  companion spawning, `SavePersistentRun()`, and collision/visibility cleanup remain intact.
- Added reduced-motion damping plus future-facing `OptionalSequencerBeatAsset` and
  `OnRescuePresentationStarted` hooks for later Sequencer, Control Rig, camera, or animation
  replacement.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/RESCUE_EXTRACTION_PRESENTATION_SLICE.md` and added
  `Scripts/verify_rescue_extraction_presentation_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Procedural secondary-motion signal slice

- Continued the June 25 creative-development backlog by adding runtime secondary motion tied to
  `CHARACTER_ANIMATION_DEEPDIVE` cloth/gear guidance and `WORLD_DEVELOPMENT_DEEPDIVE` wayfinding
  guidance.
- Added `ASecondaryMotionSignalActor`, a cook-safe procedural mast/banner/cable actor with runtime
  wind flutter, tint configuration, and audit tags for cloth-ready replacement.
- Added `SpawnSecondaryMotionSignalLayer` to every generated campaign city with moving rescue
  signals at the coding safehouse, evac helipad, route staging point, and active survivor camp.
- Registered the signals with city streaming cleanup and survivor helper cleanup so they do not
  leave orphaned actors after city changes or survivor rescue.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/SECONDARY_MOTION_SIGNAL_SLICE.md` and added
  `Scripts/verify_secondary_motion_signal_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Runtime skeletal animation budget slice

- Continued the June 25 `CHARACTER_ANIMATION_DEEPDIVE` backlog by adding a shared runtime skeletal
  animation budget helper for player, first-person arms, hero NPCs, companions, and crowd zombies.
- Added `CodeRescueAnimationBudget::ApplySkeletalMeshBudget` with profiles for `PlayerBody`,
  `FirstPersonArms`, `HeroNPC`, and `CrowdZombie`.
- Applied visibility-based animation ticking, update-rate optimization, fixed skeletal bounds,
  bounds scaling, and a 30 Hz crowd-zombie tick cap where appropriate.
- Tagged budgeted actors/components with `AnimationBudget_Runtime`, `CharacterAnimationDeepDive`,
  and profile-specific tags so later profiling and review tools can find the runtime policy.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/ANIMATION_BUDGET_RUNTIME_SLICE.md` and added
  `Scripts/verify_animation_budget_runtime_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Player first-person animation slice

- Continued the June 25 creative-development backlog by addressing the
  `CHARACTER_ANIMATION_DEEPDIVE` Phase 1 gap that the player first-person camera had no visible
  body/arms presentation.
- Added an owner-only `FirstPersonArmsMesh` skeletal mesh component to `ACodeRescueCharacter`,
  attached to `FirstPersonCamera` with no collision, overlaps, or shadow participation.
- Wired the first-person mesh to the existing local Manny asset and animation class
  (`SKM_Manny` and `ABP_Manny`) so the view has a skeletal/AnimBP-backed runtime presentation
  instead of a floating camera.
- Updated camera-perspective behavior so the third-person body remains hidden in first-person while
  the new first-person arms mesh appears only in the owning player's first-person view.
- Added `UpdateFirstPersonArms` for lightweight procedural walk bob and input sway using movement
  speed, turn input, and look input, preserving the existing camera modes and controls.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/PLAYER_FIRST_PERSON_ANIMATION_SLICE.md` and added
  `Scripts/verify_player_first_person_animation_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Encounter director AI slice

- Continued the June 25 creative-development backlog by adding an authored AI encounter director
  layer tied to `WORLD_DEVELOPMENT_DEEPDIVE`, `CHARACTER_ANIMATION_DEEPDIVE`,
  `GAME_PHYSICS_DEEPDIVE`, `TOP_50_RECOMMENDATIONS`, and the release dossier guidance around
  readable, reviewable playable encounters.
- Added explicit zombie encounter roles (`Anchor`, `Flanker`, `Pressure`, and `Sentinel`) to
  `ACodeZombieActor`, plus configurable anchor, leash, flank-offset, and pressure tuning for
  authored encounters.
- Updated `ACodeRescueAIController` so directed zombies resolve role-specific movement targets
  during patrol, chase, and attack without changing default behavior for ordinary zombies.
- Added `SpawnEncounterDirectorLayer` to every unrescued survivor city: it places a directed
  encounter deck, role beacons, destructible cover, smoke/armor/ammo counterplay caches, and four
  save-aware role-directed zombies near the survivor route.
- Preserved sandbox and save contracts: sandbox mode skips the directed enemy wave, each directed
  zombie has a stable `CodeRescueHordeZombieIdBase + CityIndex * 1000 + 800 + i` ID, neutralized
  roles stay cleared after reload, and variants are recorded through the existing save system.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/ENCOUNTER_DIRECTOR_AI_SLICE.md` and added
  `Scripts/verify_encounter_director_ai_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Jeep surface-aware vehicle physics slice

- Continued the June 25 creative-development backlog by improving the jeep's physics feel from the
  `GAME_PHYSICS_DEEPDIVE` vehicle section while keeping the current non-skeletal jeep as a stable
  macOS fallback.
- Added ground-surface probing to `AJeepActor`, resolving physical material surface types first and
  falling back to authored tags such as `SurfaceConcrete`, `SurfaceMetal`, `SurfaceWood`, and
  `SurfaceDirt`.
- Added per-surface speed, acceleration, braking, turn-rate, and lateral-drift tuning so the jeep no
  longer feels like identical floating movement on every road surface.
- Added a small underbody surface cue light and tags `VehiclePhysicsFallback`,
  `ChaosVehicleReadyFallback`, `SurfaceAwareVehicle`, and `GamePhysicsDeepDive` for audit and
  future Chaos Vehicles migration.
- Added a concrete traction-training pad and updated staff-jeep label in `SpawnJeepForCity` so every
  spawned jeep has an authored surface-aware test context.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/JEEP_SURFACE_VEHICLE_PHYSICS_SLICE.md` and added
  `Scripts/verify_jeep_surface_vehicle_physics_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Destructible cover physics slice

- Continued the June 25 creative-development backlog by implementing playable destructible cover
  tied to `GAME_PHYSICS_DEEPDIVE`, `WORLD_DEVELOPMENT_DEEPDIVE`, and
  `CHARACTER_ANIMATION_DEEPDIVE` readability guidance.
- Upgraded `ABarricadeActor` from a timed blocking cube into health-based destructible cover with
  impact damage, `TakeDamage` support, healthy/cracked/critical visual states, and Chaos-simulated
  wood debris tagged `BarricadeDebris`, `DestructibleCoverDebris`, and `ChaosDestructionFallback`.
- Wired player firearms, area-effect weapons, melee swings, throwable direct impacts, and
  throwable utility pulses into `TakeBarricadeDamage` so the defensive cover loop is playable
  through existing controls.
- Added zombie route-blocking behavior: if a barricade is between the zombie and player and close
  enough to attack, the zombie plays its attack cue/montage and damages the barricade.
- Added three authored breakable barricades plus a `DESTRUCTIBLE COVER DRILL` sign to the physics
  ambush lane so players can test shooting, throwing, and kiting infected into cover.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/DESTRUCTIBLE_COVER_PHYSICS_SLICE.md` and added
  `Scripts/verify_destructible_cover_physics_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Authored physics-lane combat encounter

- Continued the June 25 creative-development backlog by turning the physics traversal yard into a
  small authored combat pocket tied to `GAME_PHYSICS_DEEPDIVE`, `WORLD_DEVELOPMENT_DEEPDIVE`, and
  `CHARACTER_ANIMATION_DEEPDIVE` readability guidance.
- Added a `PHYSICS AMBUSH DRILL` area inside `SpawnPhysicsTraversalYard` with encounter tags
  `PhysicsLaneCombatEncounter`, `AuthoredCombatEncounter`, and `UsesThrowablePhysicsLane`.
- Added five simulated `PhysicsLaneCombatProp` / `ThrowableImpactCoverProp` actors with metal/wood
  surface tags plus three concrete cover blocks so throwables, surface impacts, radial impulses,
  and cover choices are tested in the same playable pocket.
- Added a smoke cache, flare cache, and ammo cache tagged `PhysicsLaneCombatReward` to teach the
  intended utility loop before the player enters the ambush lane.
- Added three low-intensity, save-aware encounter zombies tagged `PhysicsLaneCombatZombie` and
  `ZombieDeathPhysicsReadabilityTarget`; sandbox mode skips the enemy wave, and each zombie uses a
  stable neutralization ID with variant recording.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/PHYSICS_LANE_COMBAT_ENCOUNTER_SLICE.md` and added
  `Scripts/verify_physics_lane_combat_encounter_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Surface-specific throwable impact feedback

- Continued the June 25 creative physics/world backlog by implementing the documented physical
  materials/surface-reaction slice from `GAME_PHYSICS_DEEPDIVE`.
- Added Concrete, Metal, Wood, Glass, Flesh, and Dirt physical surface entries to
  `Config/DefaultEngine.ini` so later `UPhysicalMaterial` assets can plug into a stable compact
  surface table.
- Extended `AThrowableActor` with surface-impact tuning, `OnComponentHit` subscription,
  `bReturnMaterialOnMove`, physical-material resolution, tag fallback resolution, per-surface glow
  colors, per-surface impulse scaling, impact cooldown, and runtime `CodeRescueSurfaceImpact` logs.
- Tagged the physics traversal yard floor, ramps, cover, and movable throwable targets with
  `SurfaceConcrete`, `SurfaceMetal`, or `SurfaceWood`, plus `SurfaceImpactTraining` /
  `PhysicalMaterialSurfaceReaction` tags for review tooling.
- Added a visible `SURFACE IMPACT RANGE` label to the throwable physics lane so the player can test
  concrete dust, metal spark, and wood chip reactions in the generated city environment.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/SURFACE_IMPACT_PHYSICS_SLICE.md` and added
  `Scripts/verify_surface_impact_physics_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Zombie death physics and hit readability

- Continued implementation of the June 25 creative development backlog by addressing a documented
  `GAME_PHYSICS_DEEPDIVE` / `CHARACTER_ANIMATION_DEEPDIVE` gap: zombies had hit/death animation
  hooks, but no ragdoll or physical corpse readability.
- Added tunable zombie combat/death settings for hit nudge strength, skeletal ragdoll enablement,
  primitive fallback corpse physics, ragdoll/primitive impulse strength, and corpse lifetime.
- Added nonfatal hit readability through `ApplyHitReadabilityImpulse`, preserving existing
  hit-react montages while adding a small direction-aware physical nudge and infection-glow pulse.
- Added a capped skeletal ragdoll death path that disables gameplay collision/movement, detaches the
  mesh, applies the `Ragdoll` collision profile, simulates all bodies, wakes rigid bodies, and adds a
  hit-zone-weighted impulse when a zombie mesh has a PhysicsAsset.
- Added a primitive fallback corpse path that detaches the cube body and sphere head, switches them
  to `PhysicsActor`, enables simulation, and applies linear/angular impulses so fallback enemies
  still produce visible physical death feedback.
- Preserved the existing save and encounter contract: objective markers are destroyed,
  `MarkZombieNeutralized` / `SavePersistentRun` still run immediately, boomer death behavior still
  fires before generic cleanup, and death montages remain the fallback when physics is unavailable.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/ZOMBIE_DEATH_PHYSICS_SLICE.md` and added
  `Scripts/verify_zombie_death_physics_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Language breach encounter

- Continued creative development by extending the solved-code rescue route into a selected-language
  encounter beat tied to `WORLD_DEVELOPMENT_DEEPDIVE` guidance around route clarity, readable
  cause/effect, and authored-feeling playable spaces.
- Expanded `ACodeRescueGameMode::RevealSolvedTerminalRescueRoute` so it now derives the active
  coding language from `UCodeRescueGameInstance::SelectedLanguage` and maps Java, C, Python,
  MATLAB, C+, and C++ to distinct route colors, breach cues, reward caches, and patrol variants.
- Added a persistent `LanguageBreachCheckpoint` on the opened route with collision cover, a lit
  header signal, language logic glyphs, readable checkpoint text, and `SelectedLanguageOnly` /
  `LanguageTrack_<Language>` tags for future editor review.
- Added a compact immediate-only `LanguageBreachPatrol` spawn that appears only after a fresh solve
  and is explicitly skipped during save reconstruction and sandbox mode, preventing repeated combat
  on resumed language saves.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/LANGUAGE_BREACH_ENCOUNTER_SLICE.md` and added
  `Scripts/verify_language_breach_encounter_slice_pass.py`, wired into full QA/local CI.

## 2026-06-30 - Coding-to-rescue world response

- Continued creative development from the June 25 deep-dive backlog by making solved code visibly
  change the rescue world, tying the work to `WORLD_DEVELOPMENT_DEEPDIVE` guidance around
  coding-as-rescue cause/effect, objective clarity, and environmental storytelling.
- Added `ACodeRescueGameMode::RevealSolvedTerminalRescueRoute`, which spawns a solved-code relay,
  pulsed route strips, mid-route beacons, a survivor extraction arch, a survivor-route light, and a
  small route reward cache after a terminal is solved.
- Tagged the response layer with `CodingToRescueWorldResponse`, `SolvedTerminalWorldResponse`,
  `WorldDevelopmentDeepDive`, `CodingCauseEffect`, `TerminalSolvedRouteVisible`, and a per-terminal
  `SolvedRoute_<TerminalId>` guard so repeated solve/save-restore paths do not duplicate actors.
- Wired terminal validation success to capture the terminal location before `MarkSolved()` and
  reveal the route immediately through GameMode.
- Wired solved-terminal save reconstruction through both `SpawnTerminal` and
  `UCodeRescueGameInstance::ApplyObjectiveStateToLevel`, so language-specific resumed saves rebuild
  the solved rescue route after the start screen without reopening solved terminals.
- Added `Documentation/improvement_pass_2026-06-30/CODING_TO_RESCUE_WORLD_RESPONSE_SLICE.md` and
  `Scripts/verify_coding_world_response_slice_pass.py`, then wired the verifier into full QA/local
  CI.

## 2026-06-30 - Creative throwable physics/world vertical slice

- Began the full creative implementation backlog from the June 25 deep-dive documents with a
  playable physics/world slice tied to `GAME_PHYSICS_DEEPDIVE C21-C23` and
  `WORLD_DEVELOPMENT_DEEPDIVE`.
- Reworked `ACodeRescueCharacter::ThrowActive` to use deferred throwable spawning so flare/smoke/stim
  kind is assigned before `BeginPlay`, fixing the prior default-flare setup race and consuming
  inventory only after a successful spawn.
- Expanded `AThrowableActor` with tunable launch impulse/upward boost, real physics launch,
  angular spin, delayed flare/smoke utility pulses, radial impulse against nearby simulated props,
  and small zombie damage/knockback through the existing `ApplyRescueDamage` path.
- Extended the physics traversal yard with a visible **THROWABLE PHYSICS LANE** and seven tagged,
  simulated `ThrowablePhysicsTarget` props using `PhysicsActor` collision so the new pulse mechanic
  can be tested in-world.
- Added a safehouse utility bench with flare/smoke/stim props and readable guidance explaining that
  `X SLOT` utility throwables now lure, stagger, and restore.
- Added project physics/collision foundations in `Config/DefaultEngine.ini`: Chaos substepping plus
  named Player/Zombie/Cover/Pickup/Weapon/AISight/Interaction channels for the broader C21 collision
  plan.
- Documented the slice in
  `Documentation/improvement_pass_2026-06-30/CREATIVE_PHYSICS_WORLD_VERTICAL_SLICE.md` and added
  `Scripts/verify_creative_physics_world_slice_pass.py`, wired into full QA/local CI.

## 2026-06-29 - Terminal language-track UX and Metal packaging recovery

- Confirmed the Xcode Metal Toolchain is now installed (`xcodebuild -find metal`) and reran the
  Mac package flow successfully; `PackagedMac/Mac/CodeRescueUnreal.app` was rebuilt, staged,
  signed locally, and archived without the previous Metal compiler failure.
- Ran the packaged launch-menu visual check successfully; the captured launch screen was nonblack
  and the log scanner only reported the allowed CoreAudio sample-rate warning.
- Ran the packaged NullRHI smoke test successfully; the runtime log contracts passed while the
  launch-menu bypass loaded active gameplay under the selected default Java track.
- Extended the June 25 UX guidance into `UCodeTerminalWidget`: the terminal now mirrors the shared
  `CodeRescueUITheme`, uses themed buttons/text/code-editor styling, labels hint reveal as
  `Ctrl+H`, and color-codes validation output with semantic success/error colors.
- Added a visible **LOCKED TRACK** terminal banner showing the selected language and exact
  language-specific save profile, with copy explaining that progress, hints, attempts, and solves
  remain in that language and can be resumed from the start screen on a future launch.
- Added `Scripts/verify_terminal_language_track_ux_pass.py` and wired it into full QA/local CI so
  the selected-language terminal cue, save-profile reminder, themed editor, and validation colors
  are protected alongside the start-screen save contract.

## 2026-06-29 - Launch coding-language start screen and language-specific save contract

- Reworked the launch-only start screen so Java, C, C++, Python, and MATLAB each have explicit
  **new run** and **resume save** actions before active gameplay begins.
- Added language-specific save profile helpers in `UCodeRescueGameInstance`
  (`OperationCodeRescue_Language_Java`, `_C`, `_Cpp`, `_Python`, `_MATLAB`) so autosaves and
  pause-menu **Save Now** preserve progression within the chosen language track.
- Changed fresh language starts to reset/delete only that language profile without writing
  launch-screen world state, and changed resume starts to load the selected language profile while
  re-locking the active language.
- Kept the start screen session gate intact so the language screen still appears on every future
  app launch, now with per-language resume options when saves exist.
- Added readable launch-stage fallback labels plus the UMG menu, and ensured spawned terminals and
  language stations are locked to the selected language before gameplay.
- Added `Scripts/verify_launch_language_start_screen_save_pass.py` and wired it into full QA/local
  CI; targeted verifiers, `git diff --check`, save compatibility, Python script syntax, editor
  module rebuild, and NullRHI launch smoke passed. Full Mac packaging reached cook but failed
  because Xcode is missing the Metal Toolchain (`xcodebuild -downloadComponent MetalToolchain`).

## 2026-06-25 - Review, top-50 recs, UX design-system overhaul, live Unreal bridge, release PDFs

Kenny asked for a full project review, top-50 recommendations with immediate implementation
(emphasis on radical UX/Design), a way for Claude to connect to and drive Unreal Engine, and a
comprehensive set of release-readiness PDFs (character animation, game physics, world development)
with table of contents, integrated hyperlinks, in-text citations, and references — as separate
files plus a master index. All delivered this pass.

- **UX/UI design-system overhaul (implemented C++).** Added
  `Source/CodeRescueUnreal/CodeRescueUITheme.{h,cpp}`: semantic color tokens, a single type scale
  (`EType`), a spacing scale, an accessibility `Theme()` singleton, and `Style*` helpers. Refactored
  MainMenu, Pause, Settings (now has a readable backdrop), Death, Victory, and DamageFeedback onto
  it. `CodeRescueHUDWidget::NativeConstruct` and the Settings apply/seed paths now sync `Theme()`
  from `UCodeRescueGameInstance` (bHighContrastHUD / bReducedMotion / SubtitleScale), so high
  contrast, reduced motion, and text scaling finally drive the themed UI (incl. the critical-HP
  vignette). Rollout plan for HUD/terminal/remaining screens in
  `Documentation/improvement_pass_2026-06-25/UX_OVERHAUL_GUIDE.md`. Static build-safety verified
  (includes present, braces balanced, no orphaned font blocks); compile on Mac via
  `Recompile_Module.command`.
- **Live Unreal editor bridge (implemented).** `Content/Python/claude_unreal_bridge.py` (+
  `init_unreal.py` auto-start) polls `Saved/ClaudeBridge/inbox/*.json` on the game thread and writes
  `outbox/` results + a heartbeat. Actions: ping, exec (arbitrary editor Python), console,
  screenshot (HighResShot), save_all, list_actors, data_validation. Launcher
  `Run_Claude_Unreal_Bridge.command`. Because the project folder is mounted into Claude's sandbox,
  this is a real async channel for Claude to inspect/script/screenshot the editor on the Mac across
  sessions. Documented honestly in `UNREAL_LIVE_BRIDGE.md`: the sandbox itself cannot host UE
  (ARM Linux, ~8 GB free, no GPU); use the bridge, computer-use, or the existing build `.command`s.
- **Documentation set** under `Documentation/improvement_pass_2026-06-25/`:
  `PROJECT_STATE_REVIEW_2026-06-25.md`, `TOP_50_RECOMMENDATIONS_2026-06-25.md`,
  `UX_OVERHAUL_GUIDE.md`, `UNREAL_LIVE_BRIDGE.md`, plus five PDFs with clickable TOC + reference
  hyperlinks (pandoc+xelatex): `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf` (master index),
  `TOP_50_RECOMMENDATIONS.pdf`, and the `CHARACTER_ANIMATION_DEEPDIVE`, `GAME_PHYSICS_DEEPDIVE`,
  `WORLD_DEVELOPMENT_DEEPDIVE` PDFs (with their source `.md`). Deep-dive research cited 26 / 30 / 25
  verified sources respectively, prioritizing official Epic documentation.
- **Confirmed gaps to release** (the substantive remaining work): character animation unrealized
  (primitive fallback meshes; unassigned skeletal/anim slots), physics rudimentary (no ragdoll,
  throwables without impulse, jeep on floating-pawn movement vs. Chaos Vehicles), world is
  runtime-procedural rather than authored, and `CodeRescueGameMode.cpp` is ~12k lines and should be
  split. macOS caveats: MetaHuman Animator not on Mac in 5.7, Groom strands unsupported on Metal
  (use hair cards), Motion Matching still experimental, Lumen software ray tracing on M1–M3.

## 2026-06-24 - No-human next 20 round-two evidence dashboard

- Implemented the next top twenty recommendations as a second no-human
  improvement pass focused on release-support evidence that can be generated
  without human playthrough, Apple credentials, or hosted CI.
- Added `Content/CodeRescueData/nohuman_next20_round2_recommendations.tsv`,
  `Scripts/generate_nohuman_next20_round2_evidence.py`,
  `Scripts/verify_next20_round2_nohuman_improvement_pass.py`, and
  `Run_NoHuman_Next20_Round2_Improvement.command`.
- The new evidence dashboard records release-manifest state, artifact
  freshness, input mapping conflicts, curriculum progression, localization
  source completeness, screenshot readability, data-manifest inventory,
  source-control slices, QA log warning summaries, support-bundle completeness,
  accessibility implementation mentions, save-schema fields, radio/content
  coverage, and asset-budget coverage.
- Wired the round-two pass into `Run_Full_QA_Audit.command`,
  `Run_Local_CI_Readiness.command`, `Scripts/generate_release_manifest.py`,
  and `Scripts/create_support_bundle.py`.
- Documented the pass in
  `Documentation/NOHUMAN_NEXT20_ROUND2_IMPROVEMENT_PASS_2026-06-24.md`.

## 2026-06-24 - No-human next 20 continued-improvement pass

- Implemented the safe, automatable portions of the next top twenty
  recommendations while preserving the explicit boundary around physical human
  playthrough, Apple Developer signing/notarization credentials, hosted CI, and
  newly authored art/audio approval.
- Added `Content/CodeRescueData/nohuman_next20_recommendations.tsv` as the
  machine-readable status ledger for all twenty recommendations.
- Added `Scripts/generate_nohuman_next20_evidence.py` and
  `Scripts/verify_next20_nohuman_improvement_pass.py`, then wired the new pass
  into `Run_NoHuman_Next20_Improvement.command`, `Run_Full_QA_Audit.command`,
  `Run_Local_CI_Readiness.command`, release manifest generation, and support
  bundle creation.
- Added persisted control-profile metadata plus an in-game Settings export
  button that writes `Saved/Config/ControlProfiles/runtime_controls_profile.json`
  for future remap/accessibility review without destabilizing live combat input.
- Expanded the external control profile JSON with direct-binding safety notes
  and upgraded the static city-layer profiler so it includes
  `performance_city_layer_budget.tsv` contracts in the generated profile.
- Documented the pass in
  `Documentation/NOHUMAN_NEXT20_IMPROVEMENT_PASS_2026-06-24.md`.

## 2026-06-23 - Review recommendation closure pass

- Implemented the non-human review recommendations while intentionally leaving
  human playthrough sign-off as the only excluded item.
- Fixed cooked radio cue priority so `bPreferCookedRadioBriefingCues` and
  `-UseCookedRadioVoice` run before macOS Samantha `/usr/bin/say` fallback.
- Replaced the placeholder Mac bundle identifier with
  `com.operationcoderescue.CodeRescueUnreal` in source config, visual-check
  tooling, and the current packaged app plist; re-signed the current package
  ad-hoc for local verification.
- Clarified current campaign scope as 465 total missions: 342 U.S. city stops
  followed by 123 global extension stops. Added source constants for both
  counts.
- Replaced the skill-tree debug stub with `UCodeRescueSkillTreeWidget`, a real
  pause-menu panel with eight unlock buttons, RP state, feedback text, save
  integration, and immediate current-pawn application.
- Made skill application run on player `BeginPlay` and use a transient applied
  mask so reload and magazine bonuses cannot stack from repeated apply calls.
- Improved trusted external validation: MATLAB PATH installs now work through
  `/usr/bin/env matlab`, C/C++ harnesses undef challenge/output macros, and
  C/C++/Python validators use generated sentinels instead of a fixed success
  string. Python user code is imported by a separate harness so early
  `sys.exit(0)` cannot spoof success.
- Added
  `Documentation/RECOMMENDATION_CLOSURE_PASS_2026-06-23.md` as the review index
  for this pass, and updated release, signing, source-control, Maple narration,
  and implementation notes.

## 2026-06-19 - Playability readability fix for launch language UI, posture, audio, HUD, and architecture

- Rechecked the packaged launch capture and found the remaining issue: Unreal's
  high-resolution screenshot could still see the 3D launch text from an
  unreadable side.
- Converted the startup chooser into a dedicated `bLaunchLanguageOnly`
  screen-space UI mode. On normal startup, the first actionable controls are
  Java, C, C++, Python, and MATLAB, and each button immediately launches a
  single-language active play session.
- Removed readable 3D chooser text from `SpawnLaunchLanguageSelectionScene`;
  the world layer is now a colored symbolic backdrop only, so there is no
  mirrored/back-side text for the player to approach or misread.
- Fixed the prone/flying player posture issue by keeping controller pitch off
  the character capsule while preserving camera pitch, then starting active
  play in readable third-person.
- Changed narration defaults so the game uses clear macOS Samantha speech at a
  moderate rate, with generated cooked radio cues gated behind
  `bPreferCookedRadioBriefingCues` or `-UseCookedRadioVoice` until human audio
  QA confirms intelligibility.
- Made core HUD affordances explicit: persistent navigation panel, weapon/ammo
  and item cycling strip, numeric health label, and health bar.
- Added `SpawnPurposeClarityLayer` to purpose-code Entry, Armory, Protected
  Coding Safehouse, Survivor, Extraction, and Optional Boss Risk architecture.
- Made route-adjacent decorative static architecture nonblocking while keeping
  arena lock walls intact.
- Added
  `Documentation/improvement_pass_2026-06-19/45_PLAYABILITY_READABILITY_FIX_PASS.md`
  and `Scripts/verify_june19_playability_readability_fix_pass.py`, then wired
  the verifier into `Run_Full_QA_Audit.command`.
- Final packaged-app validation passed on 2026-06-19 AKDT:
  `./Recompile_Module.command`, `./Run_Full_QA_Audit.command`,
  `./Package_Mac_App.command`, `./Run_Launch_Menu_Visual_Check.command`,
  `./Smoke_Test_Packaged_App.command null`,
  `./Smoke_Test_Packaged_App.command render`,
  `python3 Scripts/verify_june19_playability_readability_fix_pass.py`,
  `python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`,
  `python3 Scripts/verify_png_not_black.py Saved/Screenshots/LaunchMenu/launch_menu_20260619_201326.png`,
  and `git diff --check`.
- Fresh package:
  `PackagedMac/Mac/CodeRescueUnreal.app`, timestamp
  `2026-06-19 20:13:19 AKDT`.
- Launch visual proof:
  `Saved/Screenshots/LaunchMenu/launch_menu_20260619_201326.png`, metrics
  mean luminance `111.92`, max luminance `228.00`, visible ratio `0.7388`;
  log `Saved/Logs/LaunchMenuVisual.log`, timestamp
  `2026-06-19 20:13:35 AKDT`. The log proves the screen-space launch-only
  language chooser was created with Java, C, C++, Python, and MATLAB.
- Packaged active-play smoke logs:
  `Saved/Logs/PackagedSmoke_null.log`, timestamp
  `2026-06-19 20:13:46 AKDT`, and
  `Saved/Logs/PackagedSmoke_render.log`, timestamp
  `2026-06-19 20:13:53 AKDT`.
- Full QA smoke log:
  `Saved/Logs/HeadlessFullQASmoke.log`, timestamp
  `2026-06-19 20:11:10 AKDT`.

## 2026-06-19 - Black first-frame launch-language fix and visual proof

- Reviewed the user-provided launch screenshot and confirmed it was fully
  black with `Scripts/verify_png_not_black.py`: mean luminance `0.00`, max
  luminance `0.00`, visible ratio `0.0000`.
- Found a second implementation gap after the package refresh: startup loaded
  `/Engine/Maps/Entry` into `ACodeRescueGameMode`, while the language chooser
  did not yet have a visible in-world first frame/camera behind the UMG layer.
- Added a session-only launch-language gate in `UCodeRescueGameInstance` and
  `ACodeRescueGameMode` so first launch immediately prompts for Java, Python,
  C, C++, or MATLAB before active play begins.
- Removed the legacy user-facing `C+` launch button from the startup chooser so
  the visible launch choices are the supported Java, C, C++, Python, and MATLAB
  tracks. The older enum path remains in code only for save/data compatibility.
- Added a concrete `SpawnLaunchLanguageSelectionScene` first frame with a
  solid platform, readable prompt, language markers, lighting, backdrop, a
  dedicated camera, and a short view-target refresh, then kept active-play
  smoke coverage available through the
  `-CodeRescueBypassLaunchLanguageMenu` automation flag.
- Corrected main-menu campaign and sandbox travel defaults to `Entry` so
  selected-language play launches the packaged/generated gameplay environment
  instead of missing map names.
- Added `Run_Launch_Menu_Visual_Check.command` and
  `Scripts/verify_png_not_black.py` so packaged startup now has reusable visual
  proof against black-screen regressions.
- Rebuilt the package with `./Package_Mac_App.command`. Fresh package:
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app`,
  timestamp `2026-06-19 19:38:24 AKDT`.
- Verified the non-bypass packaged launch menu with
  `./Run_Launch_Menu_Visual_Check.command`; final capture:
  `Saved/Screenshots/LaunchMenu/launch_menu_20260619_193834.png`, visual
  metrics mean luminance `97.53`, max luminance `245.36`, visible ratio
  `0.6686`, log `Saved/Logs/LaunchMenuVisual.log`. The visual check also
  requires the `[CodeRescueLaunchLanguageMenu]` log marker because Unreal's
  high-resolution screenshot path can omit the UMG overlay while still showing
  the launch world.
- Verified active-play package paths with
  `./Smoke_Test_Packaged_App.command null` and
  `./Smoke_Test_Packaged_App.command render`; logs:
  `Saved/Logs/PackagedSmoke_null.log` and
  `Saved/Logs/PackagedSmoke_render.log`.
- Final validation passed:
  `./Package_Mac_App.command`, `./Run_Launch_Menu_Visual_Check.command`,
  `./Smoke_Test_Packaged_App.command null`,
  `./Smoke_Test_Packaged_App.command render`,
  `python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`,
  `python3 Scripts/verify_png_not_black.py Saved/Screenshots/LaunchMenu/launch_menu_20260619_193834.png`,
  `git diff --check`, and `./Run_Full_QA_Audit.command`. Full QA smoke log:
  `Saved/Logs/HeadlessFullQASmoke.log`, final runtime scan
  `2026-06-19 19:41:36 AKDT`.

## 2026-06-19 - Operational package refresh after implementation audit

- Re-audited the requested launch-language, single-language active-play,
  platform-grounding, symbolized-guide-text, and pickup-availability work after
  user review raised concern that the work was not visible in the game.
- Confirmed the implementation was present in source, including immediate
  language selection launch, active-play language locking, symbolized ambient
  guide markers, raised gameplay floors, pickup ground snapping, wider pickup
  triggers, and full armory/city pickup coverage.
- Found the likely disconnect: `PackagedMac/Mac/CodeRescueUnreal.app` was
  still the older runnable artifact from before the late 2026-06-18 source
  updates, so launching that bundle could show old behavior despite the source
  being implemented.
- Rebuilt the Mac package with `./Package_Mac_App.command`. Fresh package:
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app`,
  timestamp `2026-06-19 15:36:48 AKDT`, size `2.0G`.
- Verified the rebuilt package with `./Smoke_Test_Packaged_App.command null`;
  the packaged app started, mounted cooked content, loaded `/Engine/Maps/Entry`,
  emitted the selected Java safe-learning runtime marker, and exited cleanly.
  Log: `Saved/Logs/PackagedSmoke_null.log`, timestamp
  `2026-06-19 16:20:47 AKDT`.
- Verified the rendered packaged startup path with
  `./Smoke_Test_Packaged_App.command render`; it loaded the rendered runtime,
  mounted cooked content, loaded `/Engine/Maps/Entry`, emitted the selected Java
  safe-learning runtime marker, and exited cleanly. Log:
  `Saved/Logs/PackagedSmoke_render.log`, timestamp
  `2026-06-19 16:24:20 AKDT`.
- Final validation passed:
  `python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`,
  `git diff --check`, and `./Run_Full_QA_Audit.command`. Full QA smoke log:
  `Saved/Logs/HeadlessFullQASmoke.log`, timestamp
  `2026-06-19 16:23:17 AKDT`.

## 2026-06-18 - Launch language lock, platform grounding, symbols, and pickup availability

- Changed the main menu so selecting Java, C, C+, C++, Python, or MATLAB
  immediately starts a fresh campaign run on that selected language track.
- Removed active-play language switching by preventing procedural
  `ALanguageStationActor` spawns, making placed stations non-mutating, and
  updating terminal, HUD, academy, banner, mentor, and campaign text surfaces
  to show only the launch-selected track.
- Converted most world guide-text ribbons into compact symbolic markers while
  preserving readable text for keybinds, protected coding, and other
  non-symbolic instructions.
- Raised the large zone and city mission floors to meet the actor play plane,
  added pickup ground snapping, widened pickup triggers, and spawned every
  usable pickup kind in the armory and on the city route.
- Added
  `Content/CodeRescueData/launch_language_grounding_symbol_pickup_next20_manifest.tsv`,
  `Documentation/improvement_pass_2026-06-18/41_LANGUAGE_LAUNCH_GROUNDING_SYMBOL_PICKUP_PASS.md`,
  and `Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`, then
  wired the verifier into `Run_Full_QA_Audit.command`.
- Updated stale verifier expectations that still referenced language stations,
  cross-training, all-language HUD counters, or old menu labels.
- Validation passed: `python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`,
  `python3 Scripts/verify_may27_safe_learning_city_controls_pass.py`,
  `python3 Scripts/verify_june01_rescue_survivability_pass.py`,
  `python3 Scripts/verify_june18_public_hardening_pass.py`,
  `./Recompile_Module.command`, `git diff --check`, and
  `./Run_Full_QA_Audit.command`.

## 2026-06-18 - Non-human release readiness continuation

- Continued the release-readiness work outside physical human-testing.
- Added `Content/CodeRescueData/nonhuman_release_readiness_gates.tsv` to track
  automated gates, evidence paths, credential-only blockers, and the remaining
  physical human-test boundary.
- Added `Scripts/audit_maple_audio_assets.py` to verify all 230 expected Maple
  WAVs and SoundWave assets exist and have sane technical audio properties.
- Added `Scripts/verify_package_integrity_pass.py` to inspect the packaged app,
  required cooked files, Info.plist identity, local code-signature state,
  Gatekeeper assessment, package size, and external-distribution blockers.
- Added `Scripts/verify_nonhuman_release_readiness_pass.py` and
  `Run_NonHuman_Release_Readiness.command`.
- Updated release manifests, support bundles, local CI, the signing runbook,
  distribution guide, and demo-readiness roadmap to include the new gates.

## 2026-06-18 - Next 20 demo-readiness implementation pass

- Began implementing the next 20 continued-improvement recommendations as a
  demo-readiness layer across source, data manifests, release tooling, QA, and
  documentation.
- Unblocked the Maple XTTS narration generator for the current mash-ai
  environment by patching the project shim for Transformers 4.50+ generation
  behavior.
- Completed the full Maple female-voice narration batch for all 230
  female-voiced city missions, imported the cues into Unreal SoundWave assets,
  corrected the Blueprint wiring path, added an idempotent import wrapper,
  added a native slug-based cue load fallback in `CodeRescueGameMode.cpp`, and
  always-cooked `/Game/CodeRescueAssets/Audio/RadioSamples`.
- Added six difficulty presets while preserving old save enum values:
  Story, Easy, Normal, Hard, Survival, and Nightmare.
- Added persistent accessibility settings for subtitle size, high-contrast HUD,
  reduced motion, simplified input hints, and aim-assist scale; exposed them in
  the scrollable settings widget and wired them into subtitle rendering, HUD
  text, hit knockback, and assisted-hit behavior.
- Added manifests for control remapping, accessibility settings, first-ten-
  minutes onboarding, human QA sign-off, performance city-layer budgets, asset
  budgets, enemy readability, squad personality, curriculum feedback, difficulty
  presets, and visual regression targets.
- Added release and support tooling:
  `Scripts/generate_release_manifest.py`,
  `Scripts/generate_visual_regression_manifest.py`,
  `Scripts/create_support_bundle.py`, and
  `Scripts/apply_control_remap_profile.py`.
- Added static verifiers:
  `Scripts/verify_save_compatibility_pass.py`,
  `Scripts/verify_asset_budget_pass.py`, and
  `Scripts/verify_demo_readiness_pass.py`, then wired them into
  `Run_Full_QA_Audit.command`.
- Added `Run_Visual_Regression_Audit.command` and
  `Run_Local_CI_Readiness.command`.
- Completed `Run_Local_CI_Readiness.command` after the Maple import/wire pass.
  The run completed static verifiers, release manifest generation, visual
  regression manifest generation, module recompile, full QA, Mac package,
  packaged null smoke, packaged render smoke, runtime log-contract scans, and
  support-bundle creation.
- Latest package evidence:
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app`,
  release-manifest size `2.03 GB` (`du -sh`: `2.0G`), timestamp
  `Jun 18 17:36 AKDT 2026`, Maple coverage `230/230`, and support bundle
  output under `Saved/SupportBundles/`.
- Documentation added at
  `Documentation/NEXT_20_IMPLEMENTATION_PASS_2026-06-18.md`,
  `Documentation/DEMO_READINESS_ROADMAP_2026-06-18.md`,
  `Documentation/SIGNING_NOTARIZATION_RUNBOOK_2026-06-18.md`,
  `Documentation/VISUAL_REGRESSION_BASELINE_2026-06-18.md`, and
  `Documentation/SAVE_COMPATIBILITY_TEST_PLAN_2026-06-18.md`.

## 2026-06-18 - Recommendation implementation, public hardening, QA, and package refresh

- Implemented items 1-6 from the June 18 recommendation review.
- Added shared material utilities so tinted dynamic materials resolve back to
  authored/base parents instead of nesting `UMaterialInstanceDynamic` parents.
- Added runtime log contract verification and wired it into full QA plus
  packaged smoke tests, including checks for required public-demo markers, New
  York identity strings, Backspace/F8 recovery guidance, and absence of the
  material warning patterns fixed in this pass.
- Added `.gitattributes` Git LFS patterns for Unreal/media binaries, expanded
  `.gitignore` for Python/local environment noise, and documented source-control
  handoff expectations.
- Made Maple narration documentation honest for that pass: the pipeline was
  ready, generated cue coverage was `0/230` at the time, and fallback narration
  remained active until generation/import could run. This has since been
  superseded by the next-20 demo-readiness pass above, which completed Maple
  coverage at `230/230`.
- Disabled external compiler/interpreter validation by default through
  `CodeRescue.AllowExternalCodeValidation=0`, with trusted development/QA opt-in
  through `CodeRescue.AllowExternalCodeValidation=1` or
  `-AllowExternalCodeValidation`.
- Split spawning helpers from `CodeRescueGameMode.cpp` into
  `CodeRescueGameModeSpawning.cpp` and updated static verifiers for the new
  source layout.
- Added `Scripts/verify_june18_public_hardening_pass.py`.
- Validation passed: `python3 Scripts/verify_june18_public_hardening_pass.py`,
  `python3 Scripts/verify_maple_sinister_narration_pass.py`,
  `python3 Scripts/verify_june12_city_realization_pass.py`,
  `zsh -n Run_Full_QA_Audit.command`, `git diff --check`,
  `./Recompile_Module.command`, `./Run_Full_QA_Audit.command`,
  `./Package_Mac_App.command`, `./Smoke_Test_Packaged_App.command null`, and
  `./Smoke_Test_Packaged_App.command render`.
- Fresh full QA and packaged smoke logs passed runtime contracts and confirmed
  no `LogMaterial`, `not a valid parent for MaterialInstanceDynamic`, or
  `MID_MID_` warnings in the checked logs.
- Fresh Mac package rebuilt at
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app`
  with size `1.9G` and timestamp `Jun 18 14:49:01 AKDT 2026`.
- Documentation added at
  `Documentation/RECOMMENDATION_IMPLEMENTATION_PASS_2026-06-18.md`,
  `Documentation/SOURCE_CONTROL_HANDOFF_2026-06-18.md`, and
  `Documentation/MAPLE_NARRATION_STATUS_2026-06-18.md`.

## 2026-06-13 - City arena confinement and fall-recovery pass

- Added `SpawnGameplayArenaConfinementLayer` to every generated campaign city
  so the player cannot leave the active city environment.
- Spawned a blocking fall-recovery catch floor, four blocking perimeter lock
  walls, blocking corner rescue beacons, and non-blocking perimeter aesthetics
  including light strips, skyline-edge facades, entry crosswalk stripes, and
  in-world `Backspace/F8` recovery guidance.
- Tagged arena actors with `GameplayArenaConfinement`,
  `CityGameplayBoundary`, `FallRecoveryCatchFloor`, and `ArenaLockWall`, while
  keeping visual-only boundary dressing tagged `NoAccessBlocker`.
- Updated `EnsureEntryAccessCorridorClear` to skip
  `GameplayArenaConfinement` so access cleanup keeps the interior route clear
  without removing the outer collision lock.
- Added character-side arena safety monitoring. The player records the last
  grounded safe spot inside the city, automatically recovers if they drop below
  the playable deck or exit the outer arena margin, and can manually recover
  with `Backspace` or `F8`.
- Recovery stops movement, clears falling state, restores a survivable
  health/stamina floor, clears accidental UI/pause lock, saves the corrected
  location, and emits `[CodeRescueArenaRecovery]`.
- Added `Scripts/verify_june13_arena_confinement_pass.py` and wired it into
  `Run_Full_QA_Audit.command`.
- Documentation added at
  `Documentation/improvement_pass_2026-06-13/44_CITY_ARENA_CONFINEMENT_FALL_RECOVERY_PASS.md`.
- Validation passed: `python3 Scripts/verify_june13_arena_confinement_pass.py`,
  `python3 Scripts/verify_june12_us_city_identity_pass.py`,
  `git diff --check`, `./Recompile_Module.command`,
  `./Run_Full_QA_Audit.command`, `./Package_Mac_App.command`,
  `./Smoke_Test_Packaged_App.command null`, and
  `./Smoke_Test_Packaged_App.command render`.
- Full QA and both packaged smoke logs confirmed
  `[CodeRescueArenaConfinement]` for New York, plus
  `[CodeRescueUSCityIdentity]` and `[CodeRescueEntryAccess]`.
- Fresh Mac package rebuilt at
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app`
  with size `1.9G` and timestamp `Jun 12 17:05:58 AKDT 2026`.
- No new blocking regressions were found. Remaining allowed diagnostics are
  the known immediate-quit navigation dirty-area warning, immediate-quit
  crowd-following RecastNavMesh warning, and unattended CoreAudio sample-rate
  query warning in render smoke.

## 2026-06-12 - U.S. city landscape and architecture identity pass

- Added a streamed U.S. city-specific visual profile layer to every U.S.
  campaign city through `SpawnUSCitySpecificIdentityLayer`.
- Added profile coverage for landscape, architecture, sky, roads, sidewalks,
  homes, vehicles, and local clothing cues.
- Covered all 342 U.S. campaign rows with regional/state baseline profiles and
  named overrides for high-signal cities including New York, Los Angeles,
  Chicago, Houston, Philadelphia, San Antonio, San Diego, Dallas, San Jose,
  Austin, San Francisco, Seattle, Denver, Washington, Las Vegas, Boston,
  Detroit, Miami, New Orleans, Urban Honolulu, Anchorage, and Salt Lake City.
- Spawned non-blocking terrain, skyline, road, sidewalk, home, vehicle,
  landscape, transit/freeway/civic/coastal/desert/mountain, and local-civilian
  clothing cues so existing access-point cleanup remains safe.
- Continued the city pass with non-blocking signature silhouettes for
  high-signal city and family identities, including harbor statue, hillside
  letters, suspension bridge, observation tower, civic obelisk, neon marquee,
  river bridge, mountain peak tower, tropical Deco, desert sun, tech campus,
  historic bell, music note, industrial motor, mission arch, naval harbor,
  volcanic surf, snow inlet, campus quad, and stockyard/freeway/water-tower
  markers.
- Continued the city pass with non-blocking district micro-scenes: waterfront
  and riverwalk boardwalks, transit shelters, historic/civic rows, warehouse
  docks, venue marquees, tech/campus labs, mountain trailheads, desert shade
  canopies, planned-neighborhood fronts, and local clothing accessory markers.
- Added runtime marker `[CodeRescueUSCityIdentity]` and editor tags
  `USCitySpecificIdentity`, `CitySpecificLandscape`,
  `CitySpecificArchitecture`, `CitySpecificSky`, `CitySpecificRoads`,
  `CitySpecificSidewalks`, `CitySpecificHomes`, `CitySpecificVehicles`, and
  `CitySpecificClothing`, plus `CitySpecificDistricts`.
- Added `Scripts/verify_june12_us_city_identity_pass.py` and wired it into
  `Run_Full_QA_Audit.command`; expanded it to lock the signature silhouettes
  and district micro-scenes.
- Latest source-side validation passed on 2026-06-13:
  `python3 Scripts/verify_june12_us_city_identity_pass.py`,
  `python3 Scripts/verify_june12_squad_command_status_pass.py`,
  `python3 Scripts/verify_may27_safe_learning_city_controls_pass.py`,
  `git diff --check`, `./Recompile_Module.command`, and
  `./Run_Full_QA_Audit.command`.
- Full QA smoke confirmed `[CodeRescueUSCityIdentity]` for New York and
  `[CodeRescueEntryAccess]`, including the New York
  `signature='harbor statue silhouette and dense island skyline'` field and
  `districts='waterfront or beach approach | transit stop and rail/bus corridor | historic core and stoop row'`;
  only the two known immediate-quit navigation and crowd-following warnings
  were allowed.
- Fresh Mac package rebuilt at
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app`
  with size `1.9G` and timestamp `Jun 12 16:45:31 AKDT 2026`, and passed both
  packaged smoke tests: `./Smoke_Test_Packaged_App.command null` and
  `./Smoke_Test_Packaged_App.command render`.
- Packaged smoke confirmed New York's signature and district fields, plus
  `[CodeRescueEntryAccess]` clearing 389 static blockers in both null and
  render modes. Remaining allowed package-smoke diagnostics are the known
  immediate-quit navigation dirty-area warning, immediate-quit crowd-following
  RecastNavMesh warning, and unattended CoreAudio sample-rate query warning in
  render smoke.
- Documentation added at
  `Documentation/improvement_pass_2026-06-12/43_US_CITY_LANDSCAPE_ARCHITECTURE_IDENTITY_PASS.md`.

## 2026-06-12 - Squad command/status roadmap and implementation slice

- Added a new 100-item continuation roadmap at
  `Documentation/improvement_pass_2026-06-12/42_NEXT_100_SQUAD_COMMAND_STATUS_ROADMAP.md`.
- Added compact squad health/status pips to the rescue-team HUD so active
  teammate roles and health percentages are visible during play.
- Added direct `N` medic call support. The player now receives a clear heal,
  cooldown, full-health, or no-medic message; successful calls can reference
  the recent attack direction.
- Added `O` squad hold/follow order. Hold stores each teammate's current
  position; Follow clears the order and returns the team to formation behavior.
- Made companions hold-aware while preserving support fire and medic utility.
- Updated HUD discoverability to advertise `Y/U/N/O squad`, `N MEDIC`, and
  `O HOLD`/`O FOLLOW` state.
- Added `Scripts/verify_june12_squad_command_status_pass.py` and wired it into
  `Run_Full_QA_Audit.command`.
- Updated the human QA checklist with squad health pips, medic call, and
  hold/follow playtest steps.
- Validation passed: `python3 Scripts/verify_june12_squad_command_status_pass.py`,
  `python3 Scripts/verify_june12_next100_improvement_pass.py`,
  `python3 Scripts/verify_june01_rescue_survivability_pass.py`,
  `./Recompile_Module.command`, and `./Run_Full_QA_Audit.command`.
- No new blocking regressions were found. Remaining allowed headless smoke
  diagnostics are the immediate-quit navigation dirty-area warning and the
  immediate-quit crowd-following RecastNavMesh warning.
- Package note: this was a code/QA slice, not a fresh packaged-app rebuild.

## 2026-06-12 - Next 100 roadmap, implementation slice, full QA, and package refresh

- Added a clearly prioritized 100-item improvement roadmap at
  `Documentation/improvement_pass_2026-06-12/40_NEXT_100_IMPROVEMENT_ROADMAP_AND_IMPLEMENTATION.md`.
- Began implementing the roadmap immediately with a playability/resilience
  slice: `U` cycles rescue-team formation spacing between Tight, Standard, and
  Wide; `Y` regroup now honors the active formation spacing.
- Added companion-side formation scaling so follow offsets, lateral offsets,
  personal-space radius, avoidance radius, and regroup placement update
  together instead of only changing HUD text.
- Added emergency auto-medkit resilience: if hostile damage leaves the player
  in the danger band and a medkit is available/off cooldown, the game spends
  one medkit, restores health, and reports the attack direction that caused it.
- Added critical-health callouts when emergency medkit support is unavailable,
  preserving the exact attack-direction feedback while recommending Q medkit
  use or regrouping with the medic.
- Updated HUD discoverability for `Y/U` squad controls, formation state, and
  emergency medkit readiness/cooldown.
- Added `Scripts/verify_june12_next100_improvement_pass.py` and wired it into
  `Run_Full_QA_Audit.command`.
- Updated the human QA checklist with formation cycling and emergency-medkit
  playtest steps.
- Rebuilt and packaged the fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app`; package size is `1.9G` and package
  timestamp is `Jun 12 15:14:58 AKDT 2026`.
- Validation passed: `python3 Scripts/verify_june12_next100_improvement_pass.py`,
  `python3 Scripts/verify_june01_rescue_survivability_pass.py`,
  `./Recompile_Module.command`, `./Run_Full_QA_Audit.command`,
  `./Package_Mac_App.command`, and
  `./Smoke_Test_Packaged_App.command null` plus
  `./Smoke_Test_Packaged_App.command render`.
- No new blocking regressions were found. Remaining allowed smoke diagnostics
  are the immediate-quit navigation dirty-area warning, the immediate-quit
  crowd-following RecastNavMesh warning, and the unattended macOS CoreAudio
  sample-rate query warning in render smoke.
- Documentation added at
  `Documentation/improvement_pass_2026-06-12/41_NEXT100_FORMATION_SURVIVABILITY_PACKAGE_PASS.md`.

## 2026-06-12 - Rescue team regroup control, full QA, and fresh Mac package pass

- Added a player-facing `Y` regroup command so active rescue-team companions
  can be pulled back into a staggered formation behind the player.
- Added companion-side regroup support that stops current movement before
  teleporting each operational teammate into formation, giving the player a
  recovery control when the squad falls behind or crowds an access point.
- Added subtitle/on-screen feedback with the exact number of teammates
  regrouped, plus a clear no-active-squad message when applicable.
- Updated the rescue-team HUD line to advertise `Y REGROUP` and briefly show
  `REGROUPED N` after the command.
- Expanded `Scripts/verify_june01_rescue_survivability_pass.py` so the
  regroup command, HUD feedback, movement stop, and teleport-regroup behavior
  are now covered by the full QA audit.
- Rebuilt and packaged the fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app`; package size is `1.9G` and package
  timestamp is `Jun 12 14:57:04 AKDT 2026`.
- Validation passed: `python3 Scripts/verify_june01_rescue_survivability_pass.py`,
  `./Recompile_Module.command`, `./Run_Full_QA_Audit.command`,
  `./Package_Mac_App.command`, and
  `./Smoke_Test_Packaged_App.command null` plus
  `./Smoke_Test_Packaged_App.command render`.
- No new blocking regressions were found. Remaining allowed smoke diagnostics
  are the immediate-quit navigation dirty-area warning, the immediate-quit
  crowd-following RecastNavMesh warning, and the unattended macOS CoreAudio
  sample-rate query warning in render smoke.
- Documentation added at
  `Documentation/improvement_pass_2026-06-12/39_RESCUE_TEAM_REGROUP_PACKAGE_PASS.md`.

## 2026-06-11 - Support squad non-blocking movement, HUD status, and QA verifier pass

- Made rescue-team companions player-safe by keeping world collision active
  while making their capsules ignore pawn and camera collision, preventing the
  five-person team from physically trapping or blocking the player.
- Enabled companion RVO avoidance and added a personal-space movement response
  so teammates step away when they drift too close to the player.
- Added a rescue-team HUD status line showing active squad count, medic
  nearby/away state, medic cooldown readiness, and support-fire availability.
- Added companion operational and medic-cooldown helpers for HUD/QA use.
- Added `Scripts/verify_june01_rescue_survivability_pass.py` and wired it into
  `Run_Full_QA_Audit.command` so the June survivability, attack-direction,
  rescue-squad, non-blocking companion, squad-HUD, access-cleanup, and doc
  contracts are checked in the full audit.
- Updated the older May 27 safe-learning verifier to expect the current
  `MaxEnemyDamagePerHitFraction = 0.16f` survivability balance.
- Validation passed: `python3 Scripts/verify_june01_rescue_survivability_pass.py`,
  `python3 Scripts/verify_may27_safe_learning_city_controls_pass.py`,
  `./Recompile_Module.command`, and `./Run_Full_QA_Audit.command`.
- Documentation added at
  `Documentation/improvement_pass_2026-06-11/38_SUPPORT_SQUAD_HUD_QA_POLISH.md`.

## 2026-06-01 - Rescue team, survivability, access cleanup, full QA, and fresh Mac package pass

- Made the player much more resilient with 250 max health, stronger armor
  mitigation, more medkit/armor capacity, a longer repeated-hit mercy window,
  reduced repeated-hit damage, and a single enemy-hit lethal guard so a healthy
  player does not die from one attack.
- Expanded the HUD with a larger player health label and a directional attack
  alert that reports where the player was hit, the source, the damage amount,
  and the source distance.
- Added a full five-member rescue support squad with medic, engineer,
  rifle-support, scout, and heavy-rescue roles. The team spawns near the entry
  rally point, follows in formation, assists in combat, and the medic can heal
  the player when nearby at low health.
- Confirmed the existing full arsenal cycling remains available through
  `1`-`0` quick slots, mouse wheel, bracket keys, and gamepad shoulder cycling.
- Expanded final city access cleanup so entry, armory, safehouse, language
  plaza, terminal, survivor, and helipad access points are cleared after all
  set dressing spawns. Simulated physics props are now frozen before collision
  is disabled to avoid Chaos physics warnings.
- Fixed two regressions found by the full QA pass: generated language-track
  text was missing the required `cross-training` token, and access cleanup was
  initially creating blocking `LogPhysics` warnings on simulated cover props.
- Rebuilt and packaged the fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app`; package size is `1.9G` and package
  timestamp is `May 31 19:59:46 AKDT / June 1 03:59:46 UTC 2026`.
- Validation passed: `./Run_Full_QA_Audit.command`,
  `./Package_Mac_App.command`, and
  `./Smoke_Test_Packaged_App.command null` plus
  `./Smoke_Test_Packaged_App.command render`. The final smoke scans allowed
  only the known unattended macOS CoreAudio sample-rate query warning in render
  mode plus the immediate-quit navigation dirty-area and crowd-following
  RecastNavMesh diagnostics.
- Documentation updated in
  `Documentation/improvement_pass_2026-06-01/37_RESCUE_TEAM_SURVIVABILITY_PACKAGE_PASS.md`,
  `Documentation/DISTRIBUTION_GUIDE_MAC.md`,
  `Documentation/QA_PLAYTEST_CHECKLIST.md`, and
  `Documentation/UNREAL_ACCOUNT_SAVE_HANDOFF_2026-05-20.md`.

## 2026-05-28 - Creative recommendations implementation, stress, and fresh Mac package pass

- Implemented the May 28 creative recommendation surfaces directly in gameplay
  with `SpawnCreativeRecommendationImplementationLayer` on every generated city:
  active asset-download intake bays, playable cast promotion slots,
  MetaHuman/Control Rig/IK/DCC tags, protected curriculum concept rooms,
  tactical pickup floors, city district-kit targets, AI director nodes, async
  physics props, and a visible comprehensive stress-test rig.
- Expanded tactical pickups beyond ammo/medkits with functional flares,
  smokes, stims, scrap, and armor plates. Armor plates now absorb enemy hits
  and reduce damage, and the HUD reports armor state alongside health, ammo,
  medkits, tactical gear, and language/campaign status.
- Added `Scripts/scan_may28_active_asset_downloads.py` plus
  `Content/CodeRescueData/active_download_asset_intake_2026_05_28.tsv` and
  `Saved/MCPFabUnreal/may28_active_asset_intake.json`; the scanner found 33
  active/local asset-intake rows, including Fab cache, MetaHuman packages,
  zombie/world candidates, Convai AI/NPC, ASYNC PHYSICS, and Quest Kit Pro.
- Added
  `Content/CodeRescueData/creative_recommendations_implementation_manifest.tsv`,
  `Scripts/verify_may28_creative_recommendations_pass.py`, and
  `Documentation/improvement_pass_2026-05-28/36_CREATIVE_RECOMMENDATIONS_IMPLEMENTATION_PASS.md`.
- Fixed two curriculum runtime-validator gaps uncovered during stress testing:
  MATLAB palindrome fallback now accepts `strcmp(s, fliplr(s))`, and MATLAB
  even-filter fallback now accepts vectorized logical indexing such as
  `values(mod(values, 2) == 0)`.
- Rebuilt and packaged the fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app`; package size is `1.9G` and package
  timestamp is `May 28 08:51:55 2026`.
- Validation passed: active asset scanner, May 28 creative verifier,
  safe-learning/city/controls verifier, gameplay/access verifier, tactical
  arsenal/MCP/runtime verifier, public-demo Fab/detail verifier, Unreal systems
  character/world verifier, Fab MCP porting verifier, Fab import/entry
  verifier, module rebuild, runtime-step smoke commandlet, camera/roster
  commandlet, character/world asset commandlet, curriculum validator commandlet
  across Java/C/Python/MATLAB/C+/C++, production-track commandlet, MCP Fab
  import commandlet, package build/cook/stage/archive, packaged null smoke,
  packaged render smoke, and `git diff --check`.
- Packaged smoke logs confirmed `[CodeRescueUnrealSystems]`,
  `[CodeRescuePublicDemoQuality]`, `[CodeRescueSafeLearning]`,
  `[CodeRescueCreativeImplementation]`, and `[CodeRescueEntryAccess]` markers
  before clean exit. The smoke harness allowed only the known unattended macOS
  CoreAudio sample-rate warning and immediate-quit navigation/crowd diagnostics.
- Honesty boundary: still-downloading/cache-only licensed Fab, MetaHuman,
  Maya, Houdini, AI, physics, and quest packages are staged, classified, and
  represented by promotion gates; they are not claimed as fully imported until
  Unreal materializes them locally and Mac validation confirms safe runtime use.

## 2026-05-28 - Creative development inclusion plan for active asset downloads

- Added a comprehensive creative development inclusion plan for the next
  imported-asset passes while Fab, MetaHuman, Maya, Houdini, and related Unreal
  assets are actively downloading.
- Documented recommended inclusions for the core loop, protected coding flow,
  playable characters, survivors, friendly NPCs, zombie families, bosses,
  city districts, major U.S. city identity, curriculum rooms, weapons, tactical
  gear, AI, physics, VFX, audio, UI, accessibility, QA, packaging, and the
  Unreal/Maya/Houdini/MetaHuman/Fab pipeline.
- Added a machine-readable intake tracker at
  `Content/CodeRescueData/creative_development_inclusion_plan.tsv` so incoming
  assets can be reviewed, categorized, imported, validated, and documented in a
  consistent way.
- Added
  `Documentation/improvement_pass_2026-05-28/35_CREATIVE_DEVELOPMENT_INCLUSION_PLAN.md`
  as the main review document for this planning pass.

## 2026-05-27 - Safe learning, selected language, city, controls, and health pass

- Moved campaign coding challenges into protected safehouse/annex spaces tagged
  as no-zombie learning zones, and moved the hidden binary-search bonus terminal
  out of the combat city so code entry is no longer mixed with zombie pressure.
- Opening a terminal now pauses combat, closing/destructing it unpauses combat,
  and successful validation rewards survivor-location intel before the player
  heads into the rescue route.
- Added main-menu deployment language selection and expanded the curriculum
  framework to six tracks: Java, C, Python, MATLAB, C+, and C++. Terminals now
  use the selected language instead of presenting all languages in every level.
- Added C+/C++ starter code, signature reminders, keyword coverage, manifest
  parsing, and a clang++ validator path for the existing mission shapes.
- Added a city identity/street-grid layer to each generated campaign city and
  moved regular zombie spawns into a combat district away from the protected
  learning safehouse.
- Reworked camera and gear controls so `F1`-`F6` select perspectives,
  `C`/`V` cycle perspective, and `1`-`0` quick-select weapon slots while
  wheel/brackets still cycle the wider arsenal.
- Added a HUD health gauge, capped enemy damage per hit to avoid single-contact
  deaths from healthy state, and added a death-screen save-and-quit option
  alongside replay choices.
- Added
  `Content/CodeRescueData/safe_learning_city_controls_manifest.tsv`,
  `Scripts/verify_may27_safe_learning_city_controls_pass.py`, and
  `Documentation/improvement_pass_2026-05-27/34_SAFE_LEARNING_CITY_CONTROLS_PASS.md`.
- Rebuilt and packaged the fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app`; package size is `1.9G` and package
  timestamp is `May 27 13:57:28 2026`.
- Validation passed: safe-learning/city/controls verifier, gameplay/access
  verifier, tactical arsenal/MCP/runtime verifier, public-demo Fab/detail
  verifier, Unreal systems character/world verifier, module rebuild,
  curriculum validator commandlet across Java/C/Python/MATLAB/C+/C++,
  runtime-step smoke commandlet, camera/roster commandlet, character/world
  asset commandlet, production-track commandlet, package build, packaged null
  smoke, packaged render smoke, and `git diff --check`.
- Packaged smoke logs confirmed `[CodeRescueSafeLearning]`,
  `[CodeRescueUnrealSystems]`, `[CodeRescuePublicDemoQuality]`, and
  `[CodeRescueEntryAccess]` markers before clean exit.

## 2026-05-27 - Public-demo Fab/detail polish and fresh package pass

- Added `SpawnPublicDemoFabDetailLayer` to every generated campaign city after
  the production-completion layer and before entry-corridor cleanup so the
  normal playable route now receives denser public-demo set dressing.
- Added wet-street route composition, parallax storefronts, glass windows, door
  frames, wall lamps, practical lighting, a ModernBridges hero overpass,
  tactical cover, mission-room polish, survivor-room polish, local Fab/design
  coverage panels, threat foreshadowing, and useful visible gear pickups.
- Advanced the local Fab/Unreal macOS MCP server to version 0.4.0 with
  `PUBLIC_DEMO_FAB_DETAIL_TRACKS`, the `public_demo_fab_detail_plan` tool, the
  `unreal://project/current/public-demo-fab-detail-plan` resource, and
  generated asset-plan/self-test coverage for the new public-demo detail plan.
- Added `Content/CodeRescueData/public_demo_fab_detail_manifest.tsv` and
  `Documentation/improvement_pass_2026-05-27/33_PUBLIC_DEMO_FAB_DETAIL_PASS.md`
  for future review of the latest local design/Fab inclusions.
- Rebuilt and packaged the fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app`; package size is `1.9G` and package
  timestamp is `May 27 12:48:45 2026`.
- Validation passed: public-demo Fab/detail verifier, Fab MCP porting verifier,
  Unreal systems character/world verifier, tactical arsenal/MCP/runtime
  verifier, MCP Python compile, MCP self-test, MCP audit/report generation,
  module rebuild, runtime-step smoke commandlet, character/world asset
  commandlet, camera/roster commandlet, production-track commandlet, curriculum
  validator commandlet, package build, packaged null smoke, packaged render
  smoke, and `git diff --check`.
- Packaged smoke logs confirmed `[CodeRescueUnrealSystems]`,
  `[CodeRescuePublicDemoQuality]`, and `[CodeRescueEntryAccess]` markers in the
  current app before clean exit.
- Caveat: this is a substantial local public-demo polish pass, not a claim of
  complete AAA commercial release readiness. External licensed Fab,
  Marketplace, MetaHuman, Maya, and Houdini assets still require user-owned
  downloads/exports, human playtesting, performance work, balancing, and final
  production QA.

## 2026-05-27 - Unreal systems character/world and fresh package pass

- Added a live Unreal systems development layer to every generated campaign
  city so MetaHuman-ready character slots, Maya/Houdini DCC intake,
  PCG/Houdini city review cells, Chaos/async-physics props, AI encounter
  director nodes, quest/mission kit boards, and Sequencer/ControlRig/IK/Groom
  hooks appear in the normal gameplay environment.
- Added novel character/world design targets for Rhea Calder, Mika Stone, Noor
  Vance, Jules Ardent, Ilan Cross, The Redline Warden, The Glass Ward, and The
  Broken Grid, with TSV manifests under `Content/CodeRescueData/` for future
  authored-asset replacement.
- Advanced the local Fab/Unreal macOS MCP server to version 0.3.0 with
  `UNREAL_CHARACTER_WORLD_DEVELOPMENT_TRACKS`, the
  `unreal_character_world_development_plan` tool, the
  `unreal://project/current/character-world-development-plan` resource, and
  generated asset-plan output that includes the new development tracks.
- Fixed the in-engine MATLAB-compatible reverse validator so shipped
  `fliplr(...)` solutions pass curriculum QA, then rebuilt the Unreal module.
- Rebuilt and packaged the fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app`; package size is `1.9G` and package
  timestamp is `May 27 12:27:16 2026`.
- Validation passed: Unreal systems character/world verifier, Fab MCP porting
  verifier, tactical arsenal/MCP/runtime verifier, gameplay/access verifier,
  Fab entry verifier, audit closure verifier, MCP Python compile, MCP
  self-test, MCP audit/report generation, module rebuild, curriculum validator
  commandlet, runtime-step smoke commandlet, camera/roster commandlet,
  character/world asset commandlet, production-track commandlet, package build,
  packaged null smoke, packaged render smoke, and `git diff --check`.
- Packaged smoke logs confirmed `[CodeRescueUnrealSystems]` and
  `[CodeRescueEntryAccess]` markers in the current app before clean exit.
- Documentation updated at
  `Documentation/improvement_pass_2026-05-27/32_UNREAL_SYSTEMS_CHARACTER_WORLD_PASS.md`,
  `Documentation/UNREAL_ACCOUNT_SAVE_HANDOFF_2026-05-20.md`, and
  `Documentation/DISTRIBUTION_GUIDE_MAC.md`.
- Caveat: licensed external MetaHuman, Fab, Maya, and Houdini assets are not
  downloaded automatically. This pass incorporates the local gameplay hooks,
  manifests, MCP access surface, and validation path for user-owned authored
  exports.

## 2026-05-27 - Gameplay access, camera, and fresh Mac package pass

- Rebuilt and packaged the current Operation Code Rescue Unreal work into a
  fresh Mac demo app at
  `PackagedMac/Mac/CodeRescueUnreal.app` after the May 20-21 character/world
  and later local improvement passes.
- Corrected the sideways-character issue by making zombies, boss-spawned adds,
  horde zombies, dog pups, and companions refresh orient-to-movement settings,
  face their current movement target, and move directly toward the player or
  follow target when navigation fallback movement is used.
- Removed the generated outside city wall/gate-barrier presentation from the
  playable entry flow. Levels now use low open-route beacons, route lights, and
  `AlwaysOpenLevelEntry` / `NoExteriorWallBarrier` tags instead of perimeter
  gate rails or global outside safety ground.
- Reduced procedural building footprint/height scaling for a tighter,
  human-scale survival-horror street feel while preserving the coding-rescue
  mission route, civilians, objectives, and authored set dressing.
- Hardened camera switching: `C`, `V`, and gamepad right shoulder cycle camera
  perspective; number keys `5` through `0` select FPS, TPS, tactical, top-down,
  isometric, and side-view cameras; weapons, throws, melee, and HUD crosshair
  traces now use the active gameplay camera.
- Updated demo/handoff documentation and the Mac distribution guide so review
  points at the current packaged app instead of the older package.
- Validation: May 27 gameplay/access verifier passed; Fab entry verifier
  passed; module rebuild succeeded; camera/roster, character/world assets, and
  runtime-step smoke commandlets passed with 0 errors and 0 warnings; package
  build succeeded; packaged null and rendered smoke tests both passed after log
  scanning.
- QA caveat: `Run_Full_QA_Audit.command` was started and passed its early
  build/static/campaign stages, but was stopped after
  `verify_curriculum_validator_shapes.py` hung without further progress. An
  isolated 180-second timed commandlet run reproduced the stall immediately
  after Unreal reported that it was starting that Python script.
- Documentation added at
  `Documentation/improvement_pass_2026-05-27/30_GAMEPLAY_ACCESS_CAMERA_PACKAGE_PASS.md`.

## 2026-05-24 - Audit implementation sprint

- Implemented the highest-impact immediate items from the 100-item
  comprehensive audit: terminal failure safety, legacy input warning cleanup,
  player-facing DEBUG label cleanup, authored prop mesh preference, object-level
  asset verification, runtime-step smoke coverage, warning-budget log scanning,
  closure verification, and a one-command full QA audit runner.
- Changed terminal widget failure behavior so coding objectives remain unsolved
  when the terminal UI cannot open. The user now receives a red retry/setup
  message instead of receiving unearned objective completion.
- Reworked the hero-city authored prop kit so it prefers imported/static-mesh
  props and keeps block fallbacks only for missing local assets.
- Updated character/world and camera/roster verifiers to load assets and compare
  exact object names, including the current `ZombieFemale_NurseOutfit` mesh.
- Added `Scripts/verify_runtime_step_smoke_contracts.py`,
  `Scripts/scan_audit_warnings.py`,
  `Scripts/verify_audit_implementation_closure.py`, and
  `Run_Full_QA_Audit.command`.
- Validation: rebuild succeeded; bespoke art/refinement static verifiers passed;
  audit closure verifier passed; graduated campaign, Next 100, curriculum
  validator, character/world assets, camera/roster, and runtime-step smoke
  commandlets all passed with 0 errors and 0 warnings; fresh headless runtime
  smoke exited with code 0; warning scanner passed with only the two known
  allowed NullRHI immediate-quit nav/crowd warnings; `git diff --check` and
  touched-file trailing-whitespace scans passed.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/26_AUDIT_IMPLEMENTATION_SPRINT.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Audit_Implementation_Sprint_2026-05-24.md`.

## 2026-05-24 - Comprehensive system audit and improvement findings

- Completed a full local audit of the current gameplay system, campaign data,
  curriculum validation, character roster, camera modes, world assets, bespoke
  art/UI passes, build state, runtime smoke, and recent log history.
- Verified 465 generated campaign/city missions through the graduated campaign
  and Next 100 commandlets, both with 0 errors and 0 warnings.
- Verified 32 completable coding challenge shapes across Java, C, Python, and
  MATLAB through the runtime validator commandlet, with 0 errors and 0
  warnings.
- Verified player/survivor/NPC/companion/zombie/boss roster spawning and six
  selectable camera perspectives through the camera/roster commandlet, with 0
  errors and 0 warnings.
- Verified character/world assets and both bespoke art/refinement static
  verifiers, all with 0 errors and 0 warnings.
- Rebuilt `CodeRescueUnrealEditor Mac Development`; result succeeded and the
  target was already up to date.
- Ran a fresh headless runtime smoke into
  `Saved/Logs/HeadlessComprehensiveAuditSmoke.log`; result exited with code 0.
  Current smoke-log scan found no missing-object warnings, linker warnings,
  load errors, stale `SM_postapo_bridge_001`, stale
  `SKM_ZombieFemaleClothingCasual01`, or stale `/Engine/EngineMeshes/Humanoid`
  warnings. The only current non-blocking warning patterns are immediate-quit
  NullRHI navigation/crowd-manager diagnostics.
- Documented 100 itemized improvement findings at
  `Documentation/improvement_pass_2026-05-24/25_COMPREHENSIVE_SYSTEM_AUDIT_AND_IMPROVEMENT_FINDINGS.md`
  and mirrored the report to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Comprehensive_System_Audit_Findings_2026-05-24.md`.

## 2026-05-24 - Bespoke authored-asset and animation refinement pass

- Added `SpawnBespokeAuthoredAssetRefinementLayer(...)` after the existing
  bespoke world layer so every generated city receives an additional imported
  mesh/material refinement pass.
- Replaced more blockout-only staging with imported props from StarterContent,
  ModernBridges, and Parallax Night Building: door frames, doors, windows,
  glass panes, wall lamps, pillar frames, statue crest, safe-room furniture,
  a post-apocalyptic overpass, and parallax backlot buildings.
- Added authored texture treatments using rough cobble, hewn/cut stone,
  walnut wood, burnished steel, rusted metal, glass, lamp, and tech-panel
  materials while preserving the city-generation and lesson-progression flow.
- Added a visible character animation clip stage with looped Quinn, Manny,
  ZombieFemale Nurse, and Dog Zombie skeletal animation clips near the coding
  route.
- Added zombie single-node animation fallbacks for professional zombie meshes
  when an AnimBP is missing, plus built-in AnimBP paths for compatible
  UrbanZombie4 and YI Modular variants.
- Added `Scripts/verify_bespoke_asset_animation_refinement.py` to verify the
  new world layer, imported mesh references, authored texture hooks,
  animation-clip hooks, documentation, progress notes, and launcher text.
- Validation: bespoke refinement verifier passed with 0 errors and 0 warnings;
  `CodeRescueUnrealEditor Mac Development` rebuild succeeded; headless runtime
  smoke exited with code 0; smoke-log scan found no errors, fatals, load
  errors, linker warnings, missing-object warnings, stale optional bridge
  references, stale nurse mesh-object references, or stale Humanoid skeleton
  dependency warnings; touched-file `git diff --check` and trailing-whitespace
  sweeps passed.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/24_BESPOKE_ASSET_ANIMATION_REFINEMENT_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Bespoke_Asset_Animation_Refinement_Report_2026-05-24.md`.

## 2026-05-24 - Bespoke survival-horror art and UI pass

- Added an original survival-horror coding-rescue presentation pass without
  copying direct franchise assets, characters, logos, layouts, names, or exact
  scenes.
- Added `SpawnBespokeSurvivalHorrorArtLayer(...)` and wired it into every
  generated campaign city after the Next 100 systems layer.
- New authored-feeling world pieces include a weathered courtyard facade,
  boarded windows, animated hanging lanterns, a rotating code reliquary table,
  over-the-shoulder terminal readability lane, safe-room tableau, threat gate,
  and bespoke NPC tableau placements.
- Polished the main menu, coding terminal, HUD, pause screen, victory screen,
  and death screen with darker grounded panels, warm brass/oxide accents,
  vignette treatments, and clearer survival-rescue hierarchy.
- Added `Scripts/verify_bespoke_survival_horror_art_ui.py` to verify the new
  world layer, animated prop hooks, UI polish tokens, documentation, and
  launcher text.
- Validation: bespoke art/UI verifier passed with 0 errors and 0 warnings;
  `CodeRescueUnrealEditor Mac Development` rebuild succeeded; headless runtime
  smoke exited with code 0; smoke-log scan found no errors, fatals, load
  errors, linker warnings, or stale UrbanZombie skeleton dependency warnings;
  touched-file `git diff --check` passed.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/23_BESPOKE_SURVIVAL_HORROR_ART_UI_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Bespoke_Survival_Horror_Art_UI_Report_2026-05-24.md`.

## 2026-05-24 - Next 100 implementation pass

- Implemented the 100-item roadmap as a systemic game-improvement layer across
  campaign data, terminal coaching, procedural world construction, character
  markers, progression kiosks, accessibility/polish prompts, and QA hooks.
- Added mission-level fields for language tracks, learning support, visual
  debugger plans, progression plans, character-story plans, flow plans,
  accessibility/polish plans, and QA verification plans across all generated
  campaign cities.
- Extended the curriculum audit API so automated tests can inspect the new
  learning/world/character/flow/accessibility/QA data for all 465 levels.
- Expanded the coding terminal with why-this-matters blurbs, prediction
  prompts, worked examples, code-trace prompts, visual debugger cues, mistake
  glossary text, and hidden-test debriefs.
- Added `SpawnNext100DevelopmentLayer` to every streamed city, including a
  Next 100 systems plaza, curriculum/world/character/flow/accessibility/QA
  boards, named language mentors, visual debugger props, street/transit
  details, reward-choice props, accessibility controls, and QA pedestals.
- Added `Scripts/verify_next100_implementation.py` to validate campaign data,
  terminal coaching hooks, world-spawn hooks, and documentation.
- Repaired the UrbanZombie4 skeleton reference by re-saving the asset through
  Unreal so the runtime smoke no longer reports the stale
  `/Engine/EngineMeshes/Humanoid` dependency.
- Added zombie AI navigation fallback so patrol and chase behavior uses direct
  movement input when Recast navigation data is unavailable.
- Validation: `CodeRescueUnrealEditor Mac Development` rebuild succeeded; Next
  100 implementation verification, graduated campaign/world verification,
  curriculum validator matrix, character/world asset verification, and
  camera-perspective/roster verification all passed with 0 errors and 0
  warnings. Headless runtime smoke exited with code 0, and touched-file
  `git diff --check` passed.
- Documented the pass in
  `Documentation/improvement_pass_2026-05-24/22_NEXT_100_IMPLEMENTATION_PASS.md`
  and mirrored the report to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Next_100_Implementation_Report_2026-05-24.md`.

## 2026-05-24 - Graduated curriculum and city identity pass

- Expanded the generated 465-city campaign so every playable city now carries
  a curriculum stage name, architecture signature, and novel gameplay detail.
- Reworked lesson selection into five graduated stages: Foundations, Control
  Flow, Collections and Strings, Data Structures, and Algorithmic Search.
- Added `LinkedListTraverse` and `BinarySearch` as advanced campaign lesson
  kinds and implemented Java, C, Python, and MATLAB starters and validator
  harnesses for both.
- Added `UCodeRescueCurriculumLibrary::GetCampaignAuditEntries()` plus
  `Scripts/verify_graduated_campaign_world.py` to audit every generated level
  for task progression, stage placement, unique city identity, architecture
  text, play-detail text, art-kit coverage, and complete visible/hidden tests.
- Added `Scripts/verify_curriculum_validator_shapes.py` to validate all eight
  lesson kinds across Java, C, Python, and MATLAB.
- Added `SpawnGraduatedCurriculumCityIdentityLayer` so each city has in-world
  READ/MODEL/CODE/TEST/RESCUE markers and lesson-specific visual details.
- Staged Manny/Quinn compatibility rig assets under
  `/Game/Characters/Mannequins/Rigs/...` and extended
  `Scripts/verify_character_world_assets.py` to require them.
- Updated `Run_Character_World_Demo.command` so playtesters see the new
  graduated curriculum and city-identity scope before launch.
- Validation: `CodeRescueUnrealEditor Mac Development` rebuild succeeded;
  the curriculum validator matrix, graduated campaign audit, character/world
  asset audit, and camera-perspective/roster audit all passed; explicit
  headless gameplay smoke exited with code 0.
- Documented the pass in
  `Documentation/improvement_pass_2026-05-24/20_GRADUATED_CURRICULUM_AND_CITY_IDENTITY_PASS.md`
  and mirrored the summary to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Graduated_Curriculum_City_Identity_Report_2026-05-24.md`.

## 2026-05-24 - Immediate game improvement pass

- Reviewed `/Users/labcomputer/UnrealEngine` reports and confirmed the active
  playable project remains
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix`.
- Completed the next 20 immediate game-improvement items:
  HUD stamina bar, tactical readout, closest-hostile distance, reload/low-health
  alerts, headshot feedback, expanded crosshair prompts, pickup/NPC/helipad/jeep
  interaction assist, mountable jeeps, checkpoint arches, route chevrons,
  safehouse ammo and medkit cache, operations board, curriculum wall panels,
  terminal cover, rescue extraction corridor, warden arena cover and floodlight,
  relief market, civic transit stop, distant horde silhouettes, skyline beacon,
  region-specific landmark dressing, and graduation podium.
- Added `SpawnImmediateGameImprovementLayer` to every generated campaign city
  and tagged those actors with `ImmediateImprovementPass20` and
  `WorldDevelopment`.
- Fixed `Recompile_Module.command` so noninteractive successful builds no
  longer return a false failure from the final interactive prompt.
- Updated `Run_Character_World_Demo.command` so the local demo launcher names
  the immediate improvement layer and tactical HUD pass.
- Validation: `git diff --check` passed, `CodeRescueUnrealEditor Mac
  Development` rebuild succeeded, headless runtime smoke exited cleanly with
  code 0, and character/world asset verification passed with 0 errors.
- Documented the pass in
  `Documentation/improvement_pass_2026-05-24/12_IMMEDIATE_GAME_IMPROVEMENT_PASS.md`
  and mirrored the summary to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Immediate_Game_Improvement_Report_2026-05-24.md`.

## 2026-05-21 - World major city and 50-to-1 outbreak pass

- Expanded the campaign catalog from 342 U.S. major-city stops to 465 total
  major-city stops by adding 123 international major-city stops.
- Added global region classification and art-kit labels for East Asia, South
  Asia, Southeast Asia, Europe, Latin America, Africa, the Middle East,
  Oceania, and Canada.
- Added `SpawnWorldMajorCitySignatureLayer`, which gives every generated city a
  `WORLD MAJOR CITY ATLAS` district with financial core, transit spine, old
  city, relief market, quarantine edge, and regional set dressing.
- Added `ZombieToLivingPresenceRatio = 50`, `MaxActiveAIZombiesPerCity = 120`,
  and `BackgroundHordeClusterSize = 10`.
- Added living-presence estimation, target-zombie-presence computation, and
  background horde proxy spawning.
- Default active city now targets `14 * 50 = 700` zombie presences: up to 120
  active AI zombies plus 580 represented background zombie presences.
- Moved regular, boss, dog-pack, and horde-event zombie IDs into distinct large
  ID ranges to avoid collision under the denser population model.
- Updated the demo launcher and documented the pass in
  `Documentation/improvement_pass_2026-05-21/11_WORLD_MAJOR_CITY_50_TO_1_PASS.md`
  and `/Users/labcomputer/UnrealEngine/CodeRescue_World_Major_City_50to1_Report_2026-05-21.md`.
- Validation: `git diff --check` passed for touched source files, editor build
  succeeded, asset verification completed with 0 errors, and headless runtime
  smoke exited cleanly with code 0.

## 2026-05-21 - Full-game immersion pass

- Added a live HUD objective director that names the current campaign task,
  active city/state, distance to objective, and relative direction from the
  player.
- Added a cinematic street-life layer to every generated city: road
  centerlines, lane edges, crosswalks, streetlamps with point lights,
  wayfinding signs, abandoned vehicles, overhead utility cables, and a visible
  review marker.
- Tagged the new city-dressing actors as `CinematicStreetLife` and
  `WorldDevelopment` for editor filtering and future art passes.
- Updated the demo launcher and account handoff notes so the new pass is easy
  to find.
- Documented the pass in
  `Documentation/improvement_pass_2026-05-21/10_FULL_GAME_IMMERSION_PASS.md`
  and mirrored the summary to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Full_Game_Immersion_Report_2026-05-21.md`.
- Validation: `git diff --check` passed for touched source files, editor build
  succeeded, asset verification completed with 0 errors, and headless runtime
  smoke exited cleanly with code 0.

## 2026-05-04 — Designer wiring + novel content (improvement pass part 3, items 61–68)

- Closed the seven deferred wiring items from the prior 60-item pass:
  pause-menu Crafting + Skill Tree buttons (#61); per-city boss zombie
  + drivable jeep spawned at every helipad (#62); companion spawn on
  first survivor rescue gated by a new `bHasCompanion` GI flag (#63);
  mounted music hooks (`PlayMenuMusic` from main menu, `PlayCityMusic`
  from city game mode, `PlayHordeStinger` inside `TriggerBossHorde`,
  #64); `UCodeRescueModLoader::LoadAllMods()` invoked from
  `UCodeRescueGameInstance::Init` (#65); and Victory widget submits
  fastest-five-city / rescues / headshots / kill-streak scores to all
  four `ELeaderboardKind` lists (#66).
- Added two new content systems to make the world feel populated:
  themed city set-pieces rotated by `CityIndex % 5` (Lab Vault, Radio
  Tower, Dog-Pack Den with three fast `DogZombie`-variant zombies,
  Hospital Triage, Drone Wreckage; #67); and a brand-new actor class
  `AFriendlyNPCActor` with four roles (Engineer / Medic / Scientist /
  Trader) spawned around each city's language-station plaza, with
  daily-resetting Interact perks tied to the `bIsNight` day/night
  flip (#68).
- Extended `Scripts/build_zombie_variants_table.py` to also author
  rows for the three Elite variants (`EliteSpitter`, `EliteCharger`,
  `EliteBoomer`) so the C++ behavior already implemented in
  `ACodeZombieActor` has matching mesh/AnimBP entries in
  `DT_ZombieVariants`. Re-run the script in the editor's Python
  console to refresh the table.
- Documentation: per-item docs under
  `Documentation/improvement_pass_2026-05-04/` numbered 61–68, plus
  `00_OVERVIEW_PART3.md`.
- Mac validation gate: not yet re-run by Claude (Linux sandbox).
  Designer should run `./Recompile_Module.command 2>&1 | tail -80`
  then `./Package_Mac_App.command` and `./Smoke_Test_Packaged_App.command null`.

## 2026-05-04 — Unreal character/world bootstrap

- Built `CodeRescueUnrealEditor Mac Development` successfully after repairing the integration layer for the expanded character and world-generation work.
- Character systems now compile together: player weapons/reload/stamina/gamepad/throwables/barricades, zombie `ACharacter` locomotion and elite behaviors, survivor rescue persistence, crafting, and skill tree hooks.
- World-generation systems now compile together: expanded city spawn path, entry access, weather/day-night systems, secret terminals, helipads, authored props, sandbox mode, and save-state replay after generated content spawns.
- Documented the handoff in `Documentation/improvement_pass_2026-05-04/41_unreal_character_world_bootstrap.md`.
- Rechecked after UE 5.7 Engine Content was restored. `CodeRescueUnrealEditor Mac Development` builds successfully, commandlet startup initializes the editor and asset registry, and the GUI editor is now running against `CodeRescueUnreal.uproject`. The asset registry reports 9,766 uncontrolled assets available for authoring.

## 2026-05-02

- Started by inspecting the Unreal project, May 1 development notes, source files, and recent diagnostic logs.
- Confirmed the latest source already implements the major-city campaign win condition through `FCodeRescueCampaign::GetFirstIncompleteCityIndex`, so the older note about a 3-terminal/4-survivor victory requirement appears stale.
- Ran a fresh `CodeRescueUnrealEditor Mac Development` build through UE 5.7 UBT; result succeeded.
- Found a gameplay polish issue: terminal/survivor/zombie helper markers are separate actors and can remain visible after the objective actor is solved/rescued/neutralized during live play.
- Added helper tracking to coding terminals and survivors, and a live marker pointer for zombies, so objective markers/text are destroyed immediately when the matching objective is solved, rescued, or neutralized.
- Added save-count normalization so terminal/survivor/zombie counters are derived from saved ID arrays during load/save and cannot drift from old duplicate counter values.
- Made the 342-city journal scrollable and scroll to the active city row.
- Rebuilt `CodeRescueUnrealEditor Mac Development`; result succeeded.
- Ran `RunUAT BuildCookRun` for the Mac Development package; UAT completed successfully.
- Found that UAT's archive output copied the lean `Binaries/Mac` `.app` without cooked `Contents/UE` data, which caused a direct smoke launch to fail looking for the `.uproject`.
- Fixed `Package_Mac_App.command` so it replaces the archive app with the staged app when `Saved/StagedBuilds/Mac/CodeRescueUnreal.app/Contents/UE` exists, and fails if the packaged app is missing cooked UE data.
- Refreshed `PackagedMac/Mac/CodeRescueUnreal.app` from the staged app. The archived app is now 1.1 GB and includes `CodeRescueUnreal-Mac.pak/.ucas/.utoc` plus the global shader containers.
- Smoke-launched the archived app with `-NullRHI -nosound -NoRadioVoice -unattended -stdout -ExecCmds="Quit"`; it mounted the packaged containers, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.
- Completed the requested action-item closure pass for game development completion.
- Added `.gitignore` coverage for Unreal generated output, packaged builds, diagnostic logs, IDE metadata, and other local artifacts so source control stays focused on authored project files.
- Added `Smoke_Test_Packaged_App.command` with `render` and `null` modes. The script validates that the packaged app exists, confirms cooked package data is present, launches the app, and reports pass/fail from the process exit code.
- Added completion and release documentation:
  - `Documentation/GAME_DEVELOPMENT_COMPLETION_REPORT_2026-05-02.md`
  - `Documentation/QA_PLAYTEST_CHECKLIST.md`
  - `Documentation/RELEASE_CHECKLIST.md`
  - `Documentation/DISTRIBUTION_GUIDE_MAC.md`
  - `Documentation/ASSET_AUDIO_COMPLETION_GUIDE.md`
  - `Documentation/PLAYTEST_RESULTS_2026-05-02_AUTOMATED.md`
  - `Documentation/RELEASE_NOTES_2026-05-02.md`
- Ran a packaged app smoke test through the normal renderer/audio path with `-windowed -ResX=1280 -ResY=720 -NoRadioVoice -unattended -stdout -FullStdOutLogOutput -ExecCmds="Quit"`; it mounted packaged containers, initialized Metal RHI and CoreAudio, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.
- Ran `./Smoke_Test_Packaged_App.command null`; it passed and reported `Smoke test completed successfully.`
- Initialized a local git repository and committed the release-prep baseline as `a0d0bf5 Complete game release prep`.
- Continued game-parameter improvement pass:
  - Exposed player movement, interaction, weapon, healing, resource-cap, pickup-distribution, city-density, and encounter-balance values as named Unreal properties.
  - Added capped ammo/medkit helper functions and updated pickups to respect caps instead of silently overfilling resources.
  - Added interact collection for pickups while preserving overlap collection.
  - Reworked assisted fire to use an aim cone instead of nearest-target damage anywhere inside radius.
  - Added save/load persistence for player health, ammo, and medkit resources with a new `bHasPlayerResources` compatibility flag for older saves.
  - Updated HUD resource display to show current/max supplies and use the same interaction distance as gameplay.
  - Added `Documentation/GAMEPLAY_PARAMETER_TUNING_2026-05-02.md` and updated release/QA notes.
  - Rebuilt `CodeRescueUnrealEditor Mac Development`; result succeeded.
  - Re-ran `Package_Mac_App.command`; result succeeded and refreshed `PackagedMac/Mac/CodeRescueUnreal.app`.
  - Ran `./Smoke_Test_Packaged_App.command render`; result passed with Metal/CoreAudio startup, packaged container mounting, `/Engine/Maps/Entry` load, `CodeRescueGameMode` initialization, and clean exit.
  - Ran `./Smoke_Test_Packaged_App.command null`; result passed with packaged container mounting, `/Engine/Maps/Entry` load, `CodeRescueGameMode` initialization, and clean exit.
  - Updated automated playtest results to include the post-tuning repackage and smoke-test pass.
- Fixed city access blocking:
  - Found that the player starts outside the southwest city corner while full-collision perimeter rails surrounded the city.
  - Changed streamed city perimeter rails to visual, non-blocking guide rails.
  - Added a visible southwest entry pad/corridor leading from the player start toward the city interior.
  - Updated release notes and QA checklist to verify the city can be entered from the player start.
  - Rebuilt `CodeRescueUnrealEditor Mac Development`; result succeeded.
  - Re-ran `Package_Mac_App.command`; result succeeded and refreshed `PackagedMac/Mac/CodeRescueUnreal.app`.
  - Ran `./Smoke_Test_Packaged_App.command render`; result passed with Metal/CoreAudio startup, packaged container mounting, `/Engine/Maps/Entry` load, `CodeRescueGameMode` initialization, and clean exit.
  - Ran `./Smoke_Test_Packaged_App.command null`; result passed with packaged container mounting, `/Engine/Maps/Entry` load, `CodeRescueGameMode` initialization, and clean exit.
  - Updated automated playtest results to include the city-access repackage and smoke-test pass.
- Expanded each city to a 50x horizontal span:
  - Added shared `FCodeRescueCampaign` city-span helpers so city offsets, extents, origins, and player starts use the same scale.
  - Scaled the campaign city grid spacing by 50x to keep the 342-city layout separated for nearest-city and progression logic.
  - Scaled city floors, perimeter guide rails, entry pads, navigation bounds, guide text placement, skyline distribution, landmarks, art-kit geometry, language stations, terminals, survivors, pickups, and hostile spawn locations by 50x on the horizontal axes.
  - Raised player movement speed, braking, interaction reach, weapon range, and assisted-hit radius so the expanded cities remain playable.
  - Updated release notes, tuning documentation, automated playtest notes, and the manual QA checklist.
  - Rebuilt `CodeRescueUnrealEditor Mac Development`; result succeeded.
  - Re-ran `Package_Mac_App.command`; result succeeded and refreshed `PackagedMac/Mac/CodeRescueUnreal.app`.
  - Ran `./Smoke_Test_Packaged_App.command render`; result passed with Metal/CoreAudio startup, packaged container mounting, `/Engine/Maps/Entry` load, `CodeRescueGameMode` initialization, and clean exit.
  - Ran `./Smoke_Test_Packaged_App.command null`; result passed with packaged container mounting, `/Engine/Maps/Entry` load, `CodeRescueGameMode` initialization, and clean exit.

## Remaining Notes

- Source, editor build, packaged Mac app, automated smoke verification, release documentation, QA checklist, and source-control baseline are complete.
- The only remaining completion gate that cannot be automated here is subjective human playtest sign-off. The manual checklist is documented in `Documentation/QA_PLAYTEST_CHECKLIST.md`.
- Future asset polish remains optional: replace procedural block geometry with authored environment/objective assets, and import full WAV radio briefings if packaged voiceover should avoid macOS system speech.

## 2026-05-03 — 20-item improvement pass

Implemented the 20-item improvement roadmap. Per-item documentation lives
under `Documentation/improvement_pass_2026-05-03/`. See `00_OVERVIEW.md`
in that folder for the at-a-glance status table.

**Combat & AI tier (items 1–5)**
- 1. ACodeZombieActor refactored from `AActor` to `ACharacter`. Inherits
  CapsuleComponent + CharacterMovementComponent + Mesh. Removed the custom
  `GetVelocity()` override. AIControllerClass set; AutoPossessAI =
  `PlacedInWorldOrSpawned`.
- 2. `ACodeRescueAIController` got a 5-state machine
  (Patrol/Investigate/Chase/Attack/Stagger) with line-trace visibility
  checks. Build.cs now lists AIModule + NavigationSystem.
- 3. Hit-zone classifier on `ACodeRescueCharacter::Fire` (Head 2.0× / Torso
  1.0× / Limb 0.5×). HUD flashes "HEADSHOT!" near crosshair.
- 4. 12-round magazine + R-key reload. HUD shows MagazineAmmo / Reserve.
- 5. Stamina (0–100) + Shift-to-sprint (1.6× speed). Drain/regen + jump cost.

**World legibility tier (items 6–10)**
- 6. New `UCodeRescueMinimapWidget` mounted top-right, 220×220 px,
  40000-unit view radius. Color-coded POI dots.
- 7. New `AHelipadActor` per city + `UCityFastTravelWidget`. E-key opens
  fast-travel menu listing cleared cities. Player teleport with 0.4s fade.
- 8. `SpawnAuthoredPropsForCity` adds 8 placeholder prop instances in
  Anchorage (3 in other cities). Tagged "AuthoredProp" for later asset swap.
- 9. `SpawnPerZonePostProcessVolume` per city, 3-preset color grading cycle
  (Anchorage cool / Seattle overcast / Tokyo neon).
- 10. Zombie `GrowlAudio` set to `bAllowSpatialization=true`. New
  `ZoneAmbientCues` array on GameMode (assets pending import).

**Content tier (items 11–15)**
- 11. `ASurvivorActor` got `RescueVoCue` + `IdleBarkCue` soft refs and a
  20–30s idle-bark timer. Plays VO line on rescue.
- 12. `SpeakRadioBriefing` now prefers `CityRadioBriefingCues` cooked WAVs
  over macOS TTS. Removes `say` dependency from cooked builds once
  imported.
- 13. Two new challenge shapes (linked-list traversal, binary search) added
  to `UCodeRunnerLibrary::ValidateInEngine` with full pattern matching for
  Java/C/Python/MATLAB and anti-pattern guards.
- 14. `TriggerBossHorde(center, cityIndex)` spawns 8–12 boosted zombies in
  a ring around the just-solved terminal. Hooked from
  `UCodeTerminalWidget` after `RecordTerminalSolved`.
- 15. Run scoreboard: KillCount / RescueCount / TerminalSolveCount /
  RunSeconds / DeathCount / HeadshotCount on `UCodeRescueGameInstance`,
  serialized to SaveGame with backcompat. `GetScoreboardSummary()` formatter.

**Polish tier (items 16–20)**
- 16. `UCodeRescueSettingsWidget` — sliders (master vol, SFX, music,
  sensitivity, FOV) + toggles (fullscreen, vsync). Persists via
  `UGameUserSettings`.
- 17. `UCodeRescueTutorialWidget` — 5-page first-launch overlay covering
  movement / combat / interaction / rescue loop. `bHasShownTutorial` flag.
- 18. `UCodeRescueDamageFeedbackWidget` — full-screen blood vignette +
  4 directional hit-indicator chevrons.
- 19. `UCodeRescueSaveSlotsWidget` — 3 named slots, Save/Load/Delete per
  slot. `LastSaveWallSeconds` stamp on the GameInstance.
- 20. Distant-zombie tick throttle (>10000u → ~0.5 Hz) on
  `ACodeZombieActor::Tick`. HISM kit-bash + perf benchmark deferred.

**Files added (12)**
- `Source/CodeRescueUnreal/CodeRescueMinimapWidget.h/.cpp`
- `Source/CodeRescueUnreal/HelipadActor.h/.cpp`
- `Source/CodeRescueUnreal/CityFastTravelWidget.h/.cpp`
- `Source/CodeRescueUnreal/CodeRescueSettingsWidget.h/.cpp`
- `Source/CodeRescueUnreal/CodeRescueTutorialWidget.h/.cpp`
- `Source/CodeRescueUnreal/CodeRescueDamageFeedbackWidget.h/.cpp`
- `Source/CodeRescueUnreal/CodeRescueSaveSlotsWidget.h/.cpp`

**Mac validation (must run on the local machine)**
- `./Recompile_Module.command 2>&1 | tail -80` — fix any link errors. The
  most likely failure mode is a missing `#include` for one of the new
  widget headers; if so, add the include where the compiler points.
- `./Package_Mac_App.command 2>&1 | tail -40` if compile passes.
- `./Smoke_Test_Packaged_App.command null` — should exit cleanly.

**Pending wiring (deferred to keep this commit surgical)**
- Pause widget → Settings + Save Slots buttons (one CreateWidget call each).
- HUD widget → autosave pip (read `GI->LastSaveWallSeconds`).
- Character → DamageFeedback widget mount + NotifyDamageFromDirection call.
- Asset imports: MetaHuman survivor mesh, radio WAVs, ambient cues,
  Megascans props.

## 2026-05-04 — second 20-item improvement pass (items 21–40)

Closed all 4 deferred wiring items + landed 16 new gameplay/curriculum/polish
features. Per-item documentation under `Documentation/improvement_pass_2026-05-04/`;
see `00_OVERVIEW.md` for the at-a-glance table.

**Gap-close (items 21–25)**
- 21. Wired the 4 deferred items: pause widget Settings + Save Slots buttons,
  HUD autosave pip via `LastSaveWallSeconds`, Character mounts
  `UCodeRescueDamageFeedbackWidget` + zombie passes self as Instigator on
  attack so direction chevrons fire.
- 22. New `Scripts/wire_radio_cues.py` — UE Python console script that walks
  `Content/CodeRescueAssets/Audio/RadioSamples/` and bulk-assigns to
  `BP_CodeRescueGameMode::CityRadioBriefingCues` by slug match.
- 23. `Scripts/import_metahuman_survivor.py` — auto-wires a downloaded
  MetaHuman to `BP_SurvivorActor`'s ProfessionalSurvivorMesh + AnimClass.
- 24. `Scripts/import_zone_ambient_cues.py` — auto-wires 3 ambient WAVs.
- 25. Detailed Megascans recipe in `22_to_25_asset_import_recipes.md`.

**Combat depth (items 26–30)**
- 26. `EWeaponType` + `FWeaponDef` system. Pistol / Shotgun (5-pellet spread,
  8° cone) / Rifle / Grenade. Polled keys 1/2/3/4 swap. Per-weapon mag size,
  damage, refire delay, range, spread.
- 27. Melee fallback: when both magazine + reserve are empty, F triggers a
  200-unit cone-attack (80 damage, 0.6s cooldown, 90° forward arc).
- 28. `AThrowableActor` (flare/smoke/stim) + Q-key cycles slots, X throws.
  Flares register in a static `StaticActiveLures` array for AI consumption.
- 29. Three elite zombie variants in `EZombieVariant`: EliteSpitter (ranged
  acid trace, 3.5s CD), EliteCharger (sprint dash + knockback at <200u),
  EliteBoomer (AoE damage + spawns 3 small zombies on death).
- 30. `ABarricadeActor` placed via B-key. Costs 5 scrap (new resource on
  character). 30s lifetime, 200u doorway-blocker.

**Curriculum & player skill (items 31–34)**
- 31. Hint system on terminal widget: Ctrl+H reveals next pseudocode hint
  (-1 ResearchPoint). Per-shape hint ladders (sum, lock, reverse, palindrome,
  fizzbuzz, filter, linkedlist, binary search). Solving without hints earns
  +1 RP. ResearchPoints persisted in SaveGame (#31).
- 32. `GetAdaptiveDifficultyTier()` returns 0..3 based on `TerminalSolveCount`.
  Hook for adaptive challenge variants.
- 33. `ASandboxGameMode` subclass — flips `bSandboxMode` flag on parent
  GameMode so SpawnCampaignCity skips zombie spawns. Use as the GameMode for
  a `Maps/Sandbox.umap`.
- 34. `LogCodeAttempt()` writes NDJSON to `Saved/CodeAttempts/<id>.ndjson`
  on every validate (success or failure). Code is JSON-escaped + length-clamped.

**World life (items 35–37)**
- 35. Day/night cycle on GameMode Tick. Configurable period (default 240s),
  rotates `ADirectionalLight` pitch from -90 to +270, dims to 0.3 intensity
  at night. `bIsNight` flag scales SpawnCampaignCity zombie count by 1.4x.
- 36. `SpawnWeatherForCity` spawns per-zone Niagara emitter via soft refs
  in `ZoneWeatherSystems[]`. Cycles 0=snow / 1=rain / 2=fog by `CityIndex % 3`.
- 37. `SpawnSecretTerminalForCity` adds a hidden bonus terminal at deterministic
  off-the-beaten-path offset per city (binary-search challenge, 5x score reward).

**Polish (items 38–40)**
- 38. `UCodeRescueMainMenuWidget` (New Game / Continue / Sandbox / Settings /
  Credits / Quit) + `AMainMenuGameMode` for the splash map. Set as the launch
  target via `DefaultEngine.ini` `GlobalDefaultGameMode`.
- 39. `PollGamepad` complement to `PollDirectKeys`. Reads `Gamepad_LeftX/Y`
  for movement, `Gamepad_RightX/Y` for look, `RT` for fire, A=jump, X=interact,
  Y=reload, B=throw, DPad U/D = cycle throwable, Start=pause, Select=journal.
- 40. `UCodeRescueAchievementSystem` with 8 starter achievements (First Blood,
  Hello World, Lifeline, Polyglot, Speedrun, Frugal, Headhunter, Century).
  Bitmap stored in `UCodeRescueSaveGame::AchievementsUnlocked`.
  `EvaluateAll()` called from each scoreboard increment, fires toast via
  `AddOnScreenDebugMessage`.

**Files added (12)**
- `Source/CodeRescueUnreal/ThrowableActor.h/.cpp`
- `Source/CodeRescueUnreal/BarricadeActor.h/.cpp`
- `Source/CodeRescueUnreal/SandboxGameMode.h/.cpp`
- `Source/CodeRescueUnreal/CodeRescueMainMenuWidget.h/.cpp`
- `Source/CodeRescueUnreal/MainMenuGameMode.h/.cpp`
- `Source/CodeRescueUnreal/CodeRescueAchievementSystem.h/.cpp`
- `Scripts/wire_radio_cues.py`
- `Scripts/import_metahuman_survivor.py`
- `Scripts/import_zone_ambient_cues.py`

**Mac validation gate**
- `./Recompile_Module.command 2>&1 | tail -80`
- After compile clean, author `Maps/MainMenu.umap` and `Maps/Sandbox.umap`
  via the editor (5 min each). Set `MainMenuMap=/Game/Maps/MainMenu` and
  `GlobalDefaultGameMode=/Script/CodeRescueUnreal.MainMenuGameMode` in
  `DefaultEngine.ini` to make the splash the launch target.
- `./Package_Mac_App.command 2>&1 | tail -40`
- `./Smoke_Test_Packaged_App.command null`

## 2026-05-04 Character + world asset incorporation

- Runtime city generation now consumes local Parallax Night Building meshes for
  generated skyline towers and uses cube skyline blocks only as a fallback.
- Runtime authored city props now place ModernBridges access/harbor spans and
  Parallax building clusters. The entry access bridge is non-colliding and
  tagged `OpenCityAccess` so it improves world readability without blocking
  players from entering the city.
- Zombie spawns now have built-in fallback variant rows if
  `/Game/CodeRescueAssets/DT_ZombieVariants` is unavailable or empty. The
  fallback covers DogZombie, UrbanZombie4, YI Modular Zombie M04/F01,
  ZombieFemale Nurse, and the base Zombie mesh.
- Zombie visuals are now applied inside `InitializeFromVariant`, not only during
  `BeginPlay`, so runtime-spawned zombies visibly swap to their skeletal mesh
  even when Unreal dispatches `BeginPlay` before GameMode assigns variant data.
- Survivor teams now default to the local Quinn skeletal mesh/AnimBP when no
  Blueprint-specific survivor mesh is assigned.
- Added `Scripts/verify_character_world_assets.py` for repeatable Unreal
  commandlet verification of the character/world assets used by this pass.
- Validation completed: `CodeRescueUnrealEditor Mac Development` build
  succeeded, and `verify_character_world_assets.py` completed with 0 errors.
  The only warning is the optional `SM_postapo_bridge_001` registry miss; runtime
  bridge loading falls back to the registered modern bridge meshes.

## 2026-05-20 Character, world, and mission objective demo pass

- Friendly NPCs now default to real local Manny/Quinn mannequin meshes and
  matching AnimBPs by role. Engineer/Trader use Manny; Medic/Scientist use
  Quinn. The cube/sphere NPC silhouette remains only as a missing-asset
  fallback.
- Survivor Quinn fallback mesh placement was corrected so survivor teams sit
  at ground level inside the rescue marker rather than reading as offset props.
- Runtime city generation now adds a StarterContent landscape layer:
  textured terrain underlay, concrete/cobblestone roads, waterline material
  for coastal/lake/river cities, deterministic rocks, deterministic bushes,
  route lamps, and a civilian rest-stop prop.
- Runtime city generation now adds an in-world mission route:
  numbered objective pads/beacons for Start, Select Language, Solve Terminal,
  Rescue Team, and Optional Warden, plus route strips and an entry mission
  board.
- Added a second character/world clarity layer: role-specific NPC badges/props
  and a generated civilian support hub with workstations, canopies, light
  strips, and role signage around the Engineer/Medic/Scientist/Trader team.
- Added `Run_Character_World_Demo.command` to launch the playable demo in a
  1600x900 Unreal game window with the new people/world/objective pass active.
- Updated `Scripts/verify_character_world_assets.py` to check the new mannequin,
  AnimBP, StarterContent terrain, road, objective-pad, rock, and bush assets.
- Documentation saved at
  `Documentation/improvement_pass_2026-05-20/00_CHARACTER_WORLD_MISSION_DEMO.md`.
- Local validation completed: `CodeRescueUnrealEditor Mac Development` build
  succeeded, `Scripts/verify_character_world_assets.py` succeeded with 0
  errors, and a headless `-game -NullRHI` launch smoke exited cleanly.
- Unreal account handoff documentation saved at
  `Documentation/UNREAL_ACCOUNT_SAVE_HANDOFF_2026-05-20.md`.

## 2026-05-20 Signed-in Unreal/Fab continuation

- Confirmed the project remains linked for signed-in Epic/Unreal Launcher
  discovery at `/Users/labcomputer/Documents/Unreal Projects/CodeRescueUnreal`.
- Confirmed local Epic app support and Fab/Vault cache paths exist. The
  `/Users/labcomputer/UnrealEngine/MetaHuman_Downloads` staging folder is
  present but currently empty, so no MetaHuman character package was imported.
- Added `SpawnAccountLinkedAssetShowcase(...)`: every generated city now has a
  Fab/Vault content bay with a ModernBridges span, Parallax Night Building
  towers, intake crates, and a MetaHuman-ready marker. Showcase actors are
  tagged `FabShowcase` and `AccountLinkedAsset` for editor search.
- Added `SpawnSurvivorReliefCamp(...)`: unrescued survivors now have a relief
  camp with table, chairs, supply shelf, cot, medical cross, hazard rail, and a
  civilian profile sign tied to the required lesson.
- Added `Open_CodeRescue_In_Unreal_Editor.command` for opening the active
  project directly in the signed-in Unreal Editor session.
- Updated `Run_Character_World_Demo.command`,
  `Scripts/verify_character_world_assets.py`,
  `Documentation/UNREAL_ACCOUNT_SAVE_HANDOFF_2026-05-20.md`, and
  `Documentation/improvement_pass_2026-05-20/01_SIGNED_IN_ACCOUNT_WORLD_PASS.md`
  with the signed-in continuation work.
- Validation completed: `CodeRescueUnrealEditor Mac Development` build
  succeeded, `Scripts/verify_character_world_assets.py` succeeded with 0
  errors, and a headless `-game -NullRHI` launch smoke exited cleanly.

## 2026-05-20 Character recognition and world composition continuation

- Expanded `SpawnDecorativeCivilian(...)` so mannequin civilians now receive a
  colored chest badge, shoulder sash, presence halo, and optional in-world
  display label.
- Added `SpawnCharacterIdentityCourt(...)`: each generated city now has an
  entry-side Civilian Cast court with three named figures: Civic Guide, Signal
  Scout, and Rescue Liaison. Names rotate deterministically by city.
- Added `SpawnWorldCompositionLayer(...)`: each objective gets colored
  viewframe pillars, header beams, banner wash, suspended lamp, and compact
  route label for stronger player attention and wayfinding.
- Improved mission diorama framing: Field Classroom gets window-wall framing;
  Debug Field Lab gets observation glass; classroom/lab civilians now have
  direct labels (`Nova`, `Kai`, `Dr. Vale`).
- Updated `Scripts/verify_character_world_assets.py` and documented the work at
  `Documentation/improvement_pass_2026-05-20/03_CHARACTER_RECOGNITION_WORLD_COMPOSITION_PASS.md`.
- Validation completed: `CodeRescueUnrealEditor Mac Development` build
  succeeded, `Scripts/verify_character_world_assets.py` succeeded with 0
  errors, and a headless `-game -NullRHI` launch smoke exited cleanly.

## 2026-05-20 Mission diorama and civilian presence continuation

- Added `SpawnDecorativeCivilian(...)` to place non-interactive Manny/Quinn
  mannequin civilians as mission-space storytelling figures. These actors do
  not touch survivor rescue or save-game state.
- Added `SpawnMissionDioramas(...)` and wired it into every generated city
  after the objective route spawns.
- New dioramas:
  - Field Classroom at the language-selection objective, with lesson board,
    benches, and two civilian learners.
  - Debug Field Lab at the coding-terminal objective, with tech floor, table,
    server racks, glowing screens, cable runs, and an analyst civilian.
  - Quarantine Line at the optional warden objective, with metal floor, gate
    frame, watch towers, barricades, and warning lights.
- Updated `Scripts/verify_character_world_assets.py` with the extra diorama
  assets and documented the pass at
  `Documentation/improvement_pass_2026-05-20/02_MISSION_DIORAMA_CIVILIAN_PASS.md`.
- Validation completed: `CodeRescueUnrealEditor Mac Development` build
  succeeded, `Scripts/verify_character_world_assets.py` succeeded with 0
  errors, and a headless `-game -NullRHI` launch smoke exited cleanly.

## 2026-05-20 — Visibility & playability overhaul (verified Mac compile + live demo)

- Ran a real Mac compile of the full 68-item codebase via
  `Recompile_Module.command`. Result: `BUILD SUCCEEDED`. All 85 source
  files / ~15,700 lines compile and link cleanly — the prior passes' code
  is real and buildable.
- Launched the actual playable game (`Run_Character_World_Demo.command`).
  Confirmed it generates its world, renders, and responds to WASD + mouse.
- Found the game was punishingly dark and printed a "multiple directional
  lights competing" warning. Fixed it with a 10-item visibility pass:
  - W1: `SpawnWorld()` now reuses the single `SunLight` (also tagged as the
    SkyAtmosphere sun) instead of spawning a second directional light.
    Verified live — the competing-lights warning is gone.
  - W2: day/night night intensity `0.3 -> 3.2` with a cool-blue moonlight
    color; day stays warm white `7.0`.
  - W3: `SkyLight` intensity `1.6 -> 3.0`.
  - W4: post-process auto-exposure `MinBrightness 0.4 -> 0.03`,
    `Bias 0.5 -> 1.0`, `MaxBrightness 2.0 -> 2.6`.
  - W5: `TimeOfDay` default `0.25 -> 0.12` so a fresh run opens in daylight.
  - M1: each of the 5 objective stops spawns a colored beacon point light,
    registered with the city streaming system for cleanup.
  - C1: survivor rescue beacon `1500 -> 3000` intensity, `420 -> 750` reach.
  - C2: friendly-NPC role light `2400 -> 3200`, warm color, `800 -> 950`.
  - P1: HUD crosshair/status/prompt enlarged with stronger drop shadows.
- Files changed: `CodeRescueGameMode.cpp` + `.h`, `SurvivorActor.cpp`,
  `FriendlyNPCActor.cpp`, `CodeRescueHUDWidget.cpp`.
- Re-ran `Recompile_Module.command` after the edits: `Result: Succeeded` —
  all changes compile and link cleanly.
- Documented at
  `Documentation/improvement_pass_2026-05-20/04_VISIBILITY_PLAYABILITY_OVERHAUL.md`
  and produced a prioritized 50-item roadmap at
  `Documentation/improvement_pass_2026-05-20/05_TOP_50_DEVELOPMENT_ROADMAP.md`.
- Honest note: a clean daytime "after" screenshot could not be captured —
  the editor game window pauses when unfocused so the 240s day/night cycle
  did not advance to daytime during observation, and the player camera
  buried into geometry. W2-W5 are coded + compile-verified; their visual
  result is best confirmed by a fresh play session (it now opens in day).

## 2026-05-20 — Playtest fixes round 2 (four issues from Kenny's playtest)

Kenny playtested and reported: (1) game opens black for minutes, (2) only
movement keys work, (3) endless empty field with no objectives/characters,
(4) wants multiple camera perspectives. All four addressed:

- Fix 1 — interaction inputs. `PollDirectKeys` used
  `WasInputKeyJustPressed()` for every action key; it did not fire reliably
  so only `IsInputKeyDown`-driven movement worked. Replaced with a
  self-rolled press detector: snapshot all action keys via `IsInputKeyDown`
  each frame, diff vs last frame. Reload also bound to R.
- Fix 2 — empty field. `FCodeRescueCampaign::GetCitySpanScale()` was 50.0,
  scattering all content kilometres apart (player spawned ~320 m from the
  first objective). Changed to 2.0 — the whole city contracts uniformly to
  a compact ~130 m walkable area. `WalkSpeed` lowered 9000 → 900 to match.
- Fix 3 — dark open. `SpawnWorld()` enabled Lumen GI, which takes minutes
  to converge on Mac. Disabled Lumen GI + reflections + Virtual Shadow
  Maps; the scene is now lit instantly by direct sun + SkyLight.
- Fix 4 — camera. Added a spring-arm + third-person camera and a visible
  mannequin body. Press C in game to cycle first-person / third-person /
  far third-person.
- Files: `CodeRescueCharacter.{h,cpp}`, `CodeRescueCampaign.cpp`,
  `CodeRescueGameMode.cpp`.
- `Recompile_Module.command` re-run after the edits: `BUILD SUCCEEDED`.
- Live-verified: the game now opens BRIGHT and the player spawns right next
  to content (zombie, objective beacon, structures, objective text all
  visible at spawn). Input + camera are compile-verified; best confirmed by
  a normal play session.
- Documented at
  `Documentation/improvement_pass_2026-05-20/06_PLAYTEST_FIXES.md`.

## 2026-05-21 — Input system rebuild (playtest round 3)

Kenny playtested round 2: environment/lighting good, but no interaction
keys worked (E/T/J/P/R/C/Tab/Enter/1-4 — only movement), no way to reach a
menu/exit, and buildings are too small.

- Root cause found: `Config/DefaultInput.ini` puts the project on Enhanced
  Input (`DefaultInputComponentClass = EnhancedInputComponent`). Movement
  worked because the 4 movement axes are *bound* (`BindAxis`). Every action
  key was never bound — only polled in `PollDirectKeys`, and that polling
  does not work in this build. Round 2 swapped one polling method for
  another, so it still failed.
- Fix: `SetupPlayerInputComponent` now binds every action key with
  `BindKey` (E/Enter/Tab/G interact, Space/F/LMB fire, R/Ctrl reload, Q
  medkit, T objective, J journal, P/Escape pause, C camera, 1-4 weapons,
  X throw, B barricade, H/M help). Same event routing as the working
  movement axes. Removed the polled action-key block from `PollDirectKeys`
  so nothing double-fires. Added 4 no-arg weapon-swap wrappers.
- This also fixes "no way to exit": the pause menu already frees the mouse
  cursor when it opens; P/Escape now actually open it, so the player can
  click Resume/Quit.
- Files: `CodeRescueCharacter.{h,cpp}`.
- `Recompile_Module.command` → `BUILD SUCCEEDED` (23 compile/link steps).
- Buildings too small (#1): deliberately NOT rushed — structure size and
  city spacing are coupled to one scale helper, so enlarging buildings
  needs a careful decoupling pass to avoid overlaps. Next-round priority.
- Documented at
  `Documentation/improvement_pass_2026-05-20/07_INPUT_SYSTEM_REBUILD.md`.

## 2026-05-21 — Combat fix (playtest round 4)

Kenny playtested round 3: Space (Fire) now works and shoots a red tracer —
confirming the BindKey input fix works. But the shot did no damage to
zombies, and other keys still showed no visible effect.

- Space firing proves `BindKey` works, so every action key's handler IS
  being called. The remaining issues are the handlers' effects, not the
  bindings.
- Fixed `Fire()`'s fire-rate gate: it used `TimeSinceLastFire`, a counter
  incremented in the actor Tick. If Tick is ever impaired the counter
  freezes at 0 after the first shot, gating every subsequent shot — so the
  weapon fires once and never again. Replaced with a world-time gate
  (`GetWorld()->GetTimeSeconds()`), which always advances.
- Made the hit confirmation unmistakable: a direct hit now prints
  "HIT! Zombie HP remaining: N" so the player can see damage landing.
- Files: `CodeRescueCharacter.{h,cpp}`. `Recompile_Module.command` →
  `BUILD SUCCEEDED`.
- Still open: building proportions; camera-perspective switch needs
  confirmation; full combat verification needs a hands-on playtest.

## 2026-05-21 — Review and aesthetic integration pass

Reviewed both requested locations: `/Users/labcomputer/Desktop/Operation_Code_Rescue`
and `/Users/labcomputer/UnrealEngine`. The active game project remains
`code_rescue_unreal_ue57_rebuild_fix/CodeRescueUnreal.uproject`. The account
folder has the prior session report, Unreal trace logs, and MetaHuman
groom/template packages; it does not currently expose a complete imported
MetaHuman body asset, so runtime character work remains on the project-local
Manny/Quinn meshes with explicit role labels.

- Fixed the top open world issue from the round-3 docs: building proportions
  were too small after the 50x-to-2x compact-city fix. Added
  `CityArchitectureExtent` so building visual scale is no longer coupled to
  objective spacing.
- Updated generated skyline meshes/fallback blocks, Fab showcase towers, and
  authored Parallax building clusters to use architectural scale and correct
  ground placement.
- Added an enterable civic safehouse to every generated city with walk-in
  doorway, walls, route board, brick floor, props, ceiling lamp, and named
  decorative civilians (`Iris / Safehouse Lead`, `Noor / Route Scout`).
- Updated the character/world asset verifier to require the new safehouse floor
  and window-frame assets.
- Updated demo launcher text and account handoff documentation. Added detailed
  docs at
  `Documentation/improvement_pass_2026-05-21/08_REVIEW_AND_AESTHETIC_INTEGRATION.md`
  plus a mirrored report at
  `/Users/labcomputer/UnrealEngine/CodeRescue_Review_Report_2026-05-21.md`.
- Validation: `CodeRescueUnrealEditor Mac Development` build succeeded; asset
  verification passed with 0 errors; headless runtime smoke exited with code 0.

## 2026-05-21 — Character, combat, and physics-world pass

Continued character and world development with focus on player/NPC/enemy
readability and physically grounded traversal that still protects learning
momentum.

- Player/user character: added fall-speed tracking, hard-landing damage, and a
  training landing assist that prevents surprise lethal falls by holding the
  player at 1 health and draining stamina.
- Player/user character: enemy attacks now apply a small physical knockback
  impulse, and rapid stacked hits are softened by a short mercy window.
- Player/user character: fixed firing so magazine shots no longer add spent
  rounds back into reserve ammo.
- Zombies/attack characters: close-range zombie attacks now telegraph by
  turning the infection light bright red before impact.
- Zombies/attack characters: Charger dash now resets its cooldown when a dash
  starts, preventing repeated immediate dash restarts.
- Boss characters: boss phase thresholds and phase-two regeneration now use the
  actual spawned max health after tier and variant scaling.
- Boss characters: boss phase-three add pressure is now capped, tracked, and
  assigned unique IDs rather than spawning unlimited duplicate-ID adds.
- Virtual world: added a `PHYSICS YARD` to every generated city with
  collision-enabled ramps, cover, a raised platform, gravity gauge, soft landing
  assist pad, and training labels.
- Virtual world: added a reusable `SpawnRotatedBlock` helper so future ramps
  and physically readable set-pieces can be authored cleanly.
- Documentation added at
  `Documentation/improvement_pass_2026-05-21/09_CHARACTER_COMBAT_PHYSICS_WORLD_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Character_Combat_Physics_Report_2026-05-21.md`.
- Validation: `CodeRescueUnrealEditor Mac Development` build succeeded; asset
  verification passed with 0 errors; headless runtime smoke exited with code 0.

## 2026-05-24 — Coding learning gamification pass

Continued the game-improvement work with a curriculum-first pass so the
survival loop more clearly rewards learning Java, C, Python, and MATLAB.

- Added persistent learning mastery stats: validation attempts, success/fail
  totals, current and best streaks, no-hint solves, perfect solves, per-language
  attempts, and per-language no-hint solves.
- Added mastery titles and summary APIs for terminal, HUD, and world boards.
- Upgraded terminal validation output with attempt number, mastery grade,
  concept label, first failed check, concept-specific repair advice, and
  language-specific tips.
- Expanded rewards with coding-score bonuses and ResearchPoint milestones for
  independent solves, first-try perfect solves, and streaks.
- Extended code-attempt NDJSON logs with language, attempt count, hints used,
  first failed check, score, success state, and submitted code.
- Added a HUD learning readout showing mastery title, current streak, best
  streak, and perfect solves.
- Added a Coding Learning Gamification world layer with academy progress board,
  language mastery monuments, concept practice lanes, data-flow breadcrumbs,
  validation rubric board, test/hint crates, and learning streak tower.
- Updated the demo launcher notes to advertise the learning pass.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/13_CODING_LEARNING_GAMIFICATION_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Coding_Learning_Gamification_Report_2026-05-24.md`.
- Validation: `git diff --check` passed; `CodeRescueUnrealEditor Mac
  Development` build succeeded; headless runtime smoke exited with code 0; asset
  verification passed with 0 errors.

## 2026-05-24 — Continued 50-item improvement pass

Completed the requested next 50 immediate improvements while preserving the
core intention: gamified learning across Java, C, Python, and MATLAB.

- Terminal learning loop: added session best score, last score, consecutive
  failure count, last failed check, empty-code and 20,000-character guardrails,
  Ctrl+Enter validation, Ctrl+R reset starter, reset button, challenge checklist,
  language signature reminders, reward preview, validation test counts,
  cleaner stdout/stderr empty states, and a fuller solved-terminal summary.
- Persistence: mission progress now records attempts, best score, completion,
  completed mission ids, and concept success/failure counts through the game
  instance save path.
- HUD: expanded the bottom learning readout with selected language academy mode,
  total attempts, success rate, current/best streak, no-hint solves, perfect
  solves, and per-language solve distribution.
- World construction: added debug ladder, validator test bench, syntax sparring
  ring, algorithm mural, language relay path, compile tower, debugging/testing/
  refactoring labels, rescue-loop signage, terminal breadcrumbs, perfect-solve
  podium, no-hint mastery plaques, city curriculum banner, and active mission
  learning objective board.
- Runtime reliability: switched regular zombie, boss, dog-pack, horde, and
  split-spawn creation to collision-adjusted always-spawn parameters after the
  smoke test exposed a non-fatal zombie spawn collision warning.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/14_CONTINUED_50_ITEM_IMPROVEMENT_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Continued_50_Item_Improvement_Report_2026-05-24.md`.
- Validation: `git diff --check` passed; `CodeRescueUnrealEditor Mac
  Development` build succeeded; headless runtime smoke exited with code 0; asset
  verification passed with 0 errors.

## 2026-05-24 — Character and world realization pass

Continued aesthetic and gameplay-environment development so every generated
campaign city carries clearer human context, visible character roles, and
world-building that reinforces the coding-rescue premise.

- Added `SpawnCharacterWorldRealizationLayer(...)` and wired it into every
  generated campaign city after the learning gamification layer.
- New city dressing includes a character story concourse, six named civilian
  vignettes, role-specific props, survivor profile wall, go-bag, evacuation
  clipboard, safe market, clinic/repair/study/trade stalls, evacuation queue,
  threat-readability silhouettes, boss danger-zone markers, lived-in skyline
  window lights, and hanging Java/C/Python/MATLAB banners.
- Friendly NPC interaction text now includes role-specific world notes, making
  the Engineer, Medic, Scientist, and Trader read as active members of the
  rescue network rather than generic vendors.
- Survivor blocked/rescue subtitles now surface the survivor's story and make
  the unsolved coding lesson feel like the practical route unlock.
- Updated `Run_Character_World_Demo.command` so the launcher names the new
  character/world realization layer for future playtest review.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/15_CHARACTER_WORLD_REALIZATION_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Character_World_Realization_Report_2026-05-24.md`.
- Validation: `git diff --check` passed; `CodeRescueUnrealEditor Mac
  Development` build succeeded; headless runtime smoke exited with code 0; asset
  verification passed with 0 errors.
- Remaining warnings are existing optional/external-content or engine cvar
  warnings: missing optional mannequin foot IK rig, non-focusable tutorial
  overlay focus warning, missing UrbanZombie4 `/Engine/EngineMeshes/Humanoid`
  dependency, optional missing `SM_postapo_bridge_001`, and existing
  scalability cvar warnings.

## 2026-05-24 — 500-ledger orientation sprint

Established the requested next 500 personally recommended changes and completed
the first verified implementation batch: items 001-035.

- Added
  `Documentation/improvement_pass_2026-05-24/16_NEXT_500_PERSONAL_RECOMMENDATIONS_LEDGER.md`
  with 500 individually numbered recommendations and checkbox status tracking.
- Marked only items 001-035 complete after implementation and verification;
  items 036-500 remain pending.
- Added `SpawnFirstMinuteOrientationLayer(...)` and wired it into every
  generated campaign city.
- The new first-minute layer adds an orientation plaza, mission stack board,
  five-step route markers, color path strips, controls board, rescue promise
  board, beginner practice lanes, beacon comparison display, safe-zone warning,
  danger-threshold warning, lost-route prompt, and city-debrief sign.
- Updated the first-launch tutorial with clearer pages for the rescue loop,
  movement/safety, interaction keys, language choice, terminal validation,
  combat basics, and mastery rewards.
- Updated `Run_Character_World_Demo.command` so playtesters can identify the
  new 500-ledger orientation sprint.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/17_500_LEDGER_ORIENTATION_SPRINT.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_500_Ledger_Orientation_Sprint_Report_2026-05-24.md`.
- Validation: `git diff --check` passed; `CodeRescueUnrealEditor Mac
  Development` build succeeded; headless runtime smoke exited with code 0; asset
  verification passed with 0 errors.
- Remaining warnings are existing optional/external-content or engine cvar
  warnings: optional missing `SM_postapo_bridge_001` and existing scalability
  cvar warnings.

## 2026-05-24 — Playability warning cleanup and ledger continuation

Continued the requested 500-item improvement ledger and completed items 036-050,
focused on replayable onboarding, safer tutorial dismissal, state-aware HUD
guidance, return-to-route support, and warning-free asset verification.

- Updated
  `Documentation/improvement_pass_2026-05-24/16_NEXT_500_PERSONAL_RECOMMENDATIONS_LEDGER.md`
  to mark only items 036-050 complete; ledger progress is now 50 complete and
  450 pending.
- Tutorial UX now includes a second-click skip confirmation, valid focus/Escape
  handling, replay from pause, and replay from the main menu.
- HUD objective direction now reacts to language selection, failed validation,
  successful terminal solve, survivor rescue, extraction readiness, nearby boss
  risk, missing interactable targets, and prolonged idle time.
- World guidance now includes additional return-to-route markers near the
  first-minute orientation path while preserving the existing safe-zone label,
  danger-threshold marker, and city debrief board.
- Runtime/verifier warning cleanup removed the missing optional postapo bridge
  from runtime selection, removed the optional bridge verifier warning, and
  converted the UE 5.7 Lumen tracing enum CVar to a numeric value.
- Updated `Run_Character_World_Demo.command` so the launcher advertises the
  new playability continuation pass.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/18_PLAYABILITY_WARNING_AND_LEDGER_CONTINUATION.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Playability_Warning_Ledger_Continuation_Report_2026-05-24.md`.
- Validation: `git diff --check` passed; `CodeRescueUnrealEditor Mac
  Development` build succeeded; headless runtime smoke exited with code 0; asset
  verification passed with 0 errors and 0 warnings.

## 2026-05-24 — Camera and character roster pass

Added six selectable gameplay perspectives and verified the locally available
Unreal character roster used by the generated coding-rescue environment.

- Player camera perspectives now include First-Person, Third-Person, Tactical
  Third-Person, Top-Down, Isometric, and Side-View 2.5D.
- Controls now support `C` to cycle every perspective plus direct selection via
  `5` FPS, `6` TPS, `7` tactical, `8` top-down, `9` isometric, and `0`
  side-view.
- Movement and look handling now adapt to fixed-camera modes so top-down,
  isometric, and side-view play remain readable during active gameplay.
- Companion characters now use the Quinn mannequin mesh and animation at
  runtime, matching the existing Manny/Quinn-friendly roster direction.
- Zombie variant selection now supplements partial DataTable content with the
  built-in regular and elite fallback roster, including EliteSpitter,
  EliteCharger, and EliteBoomer.
- Added `Scripts/verify_camera_perspectives_and_character_roster.py` to load
  character assets, spawn core gameplay character classes, and actively cycle
  all six player perspectives across three verification passes. The verifier
  also checks roster interaction methods and skeletal visual components.
- Expanded `Scripts/verify_character_world_assets.py` to require Manny, Quinn,
  simple mannequin, legacy UE4 mannequin, and zombie character assets.
- Updated `Run_Character_World_Demo.command` so playtesters see the new camera
  controls and roster verification scope.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/19_CAMERA_AND_CHARACTER_ROSTER_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Camera_And_Character_Roster_Report_2026-05-24.md`.
- Validation: `CodeRescueUnrealEditor Mac Development` build succeeded; camera
  and roster verification passed with 0 errors and 0 warnings; asset
  verification passed with 0 errors and 0 warnings; headless runtime boot smoke
  exited with code 0.

## 2026-05-24 — Production track completion pass

Completed a second production-track review focused on unfinished animation,
all-city art, radio/audio, localization, visual review, release-certification,
and profiling support.

- Added radio briefing and voice fields to the campaign audit API so production
  manifests are generated from the same live mission data used by gameplay.
- Made radio subtitles always push before cue/TTS selection, preserving mission
  briefing readability in packaged, offline, headless, and `-NoRadioVoice`
  runs.
- Added an all-city production completion plaza with radio, localization,
  visual-QA, and profiling markers.
- Expanded the authored in-world animation showcase from 4 to 13 looped clips
  across survivor, engineer, dog, nurse, base zombie, business zombie, and
  bloated zombie coverage.
- Added production manifest export and verification scripts for complete
  radio, localization, audio, art, visual-review, performance, and animation
  coverage.
- Remapped unsafe UrbanZombie4/Elite Boomer runtime variant assets to locally
  loadable modular zombie meshes after the smoke log scanner caught a missing
  `/Engine/EngineMeshes/Humanoid` dependency in this engine install.
- Added dedicated launchers for performance profiling and visual review capture,
  and wired the production verifier into `Run_Full_QA_Audit.command`.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/27_PRODUCTION_TRACK_COMPLETION_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Production_Track_Completion_Report_2026-05-24.md`.
- Continued this pass with first-view production polish: added an authored
  arrival composition to every generated city, applied textured material
  treatments to systemic buildings and bespoke backlot structures, replaced
  remaining purple/magenta language/accessibility accents with amber/teal/coral
  tones, and added clean `-VisualReviewStart` capture behavior that hides only
  world-space helper labels during visual review.
- Updated `Run_Performance_Profile.command` so future performance runs also
  execute the shared warning scanner.
- Final validation: full QA audit passed; visual-review capture passed and
  saved
  `Saved/Screenshots/VisualReview/visual_review_20260524_210714.png`;
  performance profile passed with only allowed navigation/crowd warnings; Mac
  packaging passed; packaged null smoke passed against
  `PackagedMac/Mac/CodeRescueUnreal.app` with only allowed navigation/crowd
  warnings.

## 2026-05-24 — MacOS Fab/Unreal MCP asset-porting pass

- Reviewed the attached MCP/Fab/Unreal SOP and macOS forward-migration design
  PDFs and implemented a local stdio MCP server at
  `MCP_Server_Development/fab_unreal_macos_mcp/server.py`.
- Added MCP tools for root verification, local Fab/Vault cache scanning,
  Unreal project inspection, asset compatibility classification, dry-run-first
  porting plans, project manifest writing, explicit local-content staging, and
  Unreal commandlet planning.
- Added `Run_Fab_Unreal_MCP_Server.command` and
  `Run_Fab_Unreal_MCP_Audit.command` for repeatable operation on this Mac.
- Generated `Content/CodeRescueData/fab_unreal_mcp_asset_plan.json`,
  `Content/CodeRescueData/fab_unreal_mcp_world_generation_queue.tsv`, and
  `Saved/MCPFabUnreal/latest_asset_audit.json`.
- Added Unreal-side validation script
  `Scripts/mcp_fab_unreal_import_validate.py` and standalone verifier
  `Scripts/verify_fab_unreal_mcp_porting.py`.
- Audit result: 16 local Fab/Vault entries detected; 2 portable, 5
  portable-after-retarget, 9 manual-review-required. Seven entries are already
  represented in project content: Building Interior Cubemap Material Function,
  Dog Zombie, Modern Bridges, Urban Zombie 4, Zombie, Zombie - Bloated Female,
  and Zombie - Business Suit.
- Unreal-side MCP asset validation passed through `UnrealEditor-Cmd` with 0
  errors and 0 warnings; all seven already-present Fab-derived content roots
  were confirmed by the Asset Registry.
- Documented the implementation and handoff in
  `Documentation/improvement_pass_2026-05-24/28_MACOS_FAB_UNREAL_MCP_ASSET_PORTING.md`.

## 2026-05-24 — Fab import and universal entry access pass

- Extended the local macOS Fab/Unreal MCP server with
  `import_available_fab_assets`, an explicit import/status tool that accounts
  for all locally detected Fab/Vault entries, stages only user-owned
  materialized local `Content` folders, and refuses to bypass Fab/Epic
  authentication, licensing, or missing Mac plugin/source requirements.
- Ran the MCP audit and import tool. Result: 16 local Fab/Vault entries
  inspected; 7 already incorporated in the game environment; 0 new
  materialized content folders available to stage; 9 entries documented as
  requiring Fab Launcher/Fab Window materialization or source/plugin review.
- Wrote the import status ledger to
  `Content/CodeRescueData/fab_unreal_mcp_import_status.tsv` and the import
  execution report to `Saved/MCPFabUnreal/import_available_report.json`.
- Corrected the reported spawn-side wall blockade systemically: moved the
  campaign player start onto the universal entry pad, added an always-open
  entry ramp/gate layer to every generated city, and added a late collision
  clearance pass for the spawn-to-city corridor after all world/art layers
  spawn.
- Added `Scripts/verify_fab_import_and_entry_access.py` and wired it into
  `Run_Full_QA_Audit.command`; the verifier checks MCP import status,
  465-level campaign coverage through `SpawnCampaignCity`, and the source hooks
  for the no-blockade entry corridor.
- Validation: `python3 Scripts/verify_fab_import_and_entry_access.py` passed;
  `CodeRescueUnrealEditor Mac Development` rebuild succeeded; the full QA audit
  completed successfully, including all static verifiers, all Unreal
  commandlets, headless runtime smoke, and smoke-log scanning with only the two
  known allowed immediate-quit NullRHI navigation/crowd warnings.
- Documentation added at
  `Documentation/improvement_pass_2026-05-24/29_FAB_IMPORT_AND_UNIVERSAL_ENTRY_ACCESS_PASS.md`
  and mirrored to
  `/Users/labcomputer/UnrealEngine/CodeRescue_Fab_Import_Universal_Entry_Access_Report_2026-05-24.md`.

## 2026-06-30 — Skill tree progression clarity slice

- Continued the June 25 creative development pass by implementing the TOP 50
  recommendation for clearer meta-progression in the pause-menu skill tree.
- Restyled `CodeRescueSkillTreeWidget` with the shared `CodeRescueUI` theme and
  mirrored saved accessibility settings for high contrast, reduced motion, and
  text scale.
- Added a progression header showing ResearchPoints, unlocked skill count,
  active coding language, language practice summary, learning mastery/streak
  summary, and the active language save slot.
- Reworked skill rows to show explicit `UNLOCKED`, `READY - spend 2 RP`, and
  `LOCKED - need N more RP` states, with category, outcome, and persistence
  copy for each upgrade.
- Kept locked-but-unaffordable nodes selectable so the feedback line can tell
  the player exactly how many ResearchPoints are still needed.
- Documented the slice at
  `Documentation/improvement_pass_2026-06-30/SKILL_TREE_PROGRESSION_CLARITY_SLICE.md`,
  added curriculum/accessibility/visual manifest rows, and wired
  `Scripts/verify_skill_tree_progression_clarity_slice_pass.py` into full QA
  and local CI readiness.
- Validation passed: Python verifier compilation, the new skill-tree verifier,
  whitespace diff check, adjacent terminal/objective/settings/launch-language
  UI verifiers, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke. Smoke logs contained only the already-allowed immediate-exit
  navigation/crowd warnings plus the known render-mode CoreAudio sample-rate
  warning.

## 2026-06-30 — HUD vitals theme accessibility slice

- Continued the June 25 HUD theme rollout by replacing hardcoded health and
  stamina refresh colors with helper-driven, shared-theme vitals.
- Added `HudVitalStateLabel()`, `HudHealthFillColor()`,
  `HudHealthLabelColor()`, and `HudStaminaFillColor()` to make the green,
  amber, red, stamina-blue, and high-contrast states reviewable in one place.
- Updated the health readout to show explicit `STABLE`, `LOW`, and `CRITICAL`
  state text alongside numeric health and percent.
- Re-styled the health label through `CodeRescueUI::StyleText()` so saved text
  scale affects the combat HUD, and widened the label slot to preserve
  readability at larger sizes.
- Mirrored saved high-contrast, reduced-motion, and subtitle/text-scale
  settings into the shared HUD theme inside `RefreshHUD()` so live settings
  changes continue to affect the vitals surface.
- Updated accessibility, visual-regression, and safe-learning manifests;
  documented the work at
  `Documentation/improvement_pass_2026-06-30/HUD_VITALS_THEME_ACCESSIBILITY_SLICE.md`;
  and wired `Scripts/verify_hud_vitals_theme_accessibility_slice_pass.py` into
  full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new HUD vitals verifier,
  whitespace diff check, adjacent headshot/damage/threat/demo/readability
  verifiers, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke. Smoke logs contained only the already-allowed immediate-exit
  navigation/crowd warnings plus the known render-mode CoreAudio sample-rate
  warning.

## 2026-06-30 — Save slots language backup UX slice

- Continued the June 25 progression/readability pass by reframing pause-menu
  save slots as manual backups of the active coding-language run.
- Rebuilt `CodeRescueSaveSlotsWidget` with the shared `CodeRescueUI` theme,
  reduced-motion blur, high-contrast-aware colors, scalable text, an active
  language summary, and per-backup state descriptions.
- Preserved the existing compatibility backup files
  `OperationCodeRescue_Slot0`, `OperationCodeRescue_Slot1`, and
  `OperationCodeRescue_Slot2`.
- Updated Save Backup so it writes the manual backup and then refreshes the
  selected `OperationCodeRescue_Language_<Track>` start-screen resume save.
- Updated Load Backup so it loads the backup and promotes that backup into the
  loaded run's language resume slot, keeping the launch screen authoritative
  for future resumes.
- Updated Delete so it removes only the manual backup and restores
  `SaveSlotName` to the active language slot.
- Updated accessibility, visual-regression, and first-ten-minutes onboarding
  manifests; documented the work at
  `Documentation/improvement_pass_2026-06-30/SAVE_SLOTS_LANGUAGE_BACKUP_UX_SLICE.md`;
  and wired `Scripts/verify_save_slots_language_backup_ux_slice_pass.py` into
  full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new save-backup verifier,
  adjacent launch-language/save compatibility/onboarding/settings verifiers,
  whitespace diff check, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke. Smoke logs contained only the already-allowed immediate-exit
  navigation/crowd warnings plus the known render-mode CoreAudio sample-rate
  warning.

## 2026-06-30 — Fast travel evac route readability slice

- Continued the June 25 UX/world-development pass by restyling
  `CityFastTravelWidget`, which the June 25 UX guide identified as a remaining
  shared-theme rollout target.
- Rebuilt the helipad fast-travel overlay into a themed `EvacRoutePanel` with
  reduced-motion blur, scalable text, high-contrast-aware colors, extraction
  debrief text, and a route summary.
- Added route summary copy showing the active coding language, the
  start-screen language resume slot, cleared terminal count, rescued team
  count, route mode, and post-teleport save behavior.
- Restyled the extraction-ready continue action as a `NEXT OPERATION` button
  that names the next incomplete city and keeps the selected language route
  explicit.
- Replaced terse solved-city labels with `REDEPLOY` rows showing the mission
  label, completion state, curriculum focus, and that arrival saves the active
  language run.
- Preserved the existing helipad context wiring, `ButtonToCityIndex` dispatch
  map, continue action, cleared-city teleport behavior, and post-teleport
  `SavePersistentRun()` call.
- Updated accessibility, visual-regression, and first-ten-minutes onboarding
  manifests; documented the work at
  `Documentation/improvement_pass_2026-06-30/FAST_TRAVEL_EVAC_ROUTE_READABILITY_SLICE.md`;
  and wired `Scripts/verify_fast_travel_evac_route_readability_slice_pass.py`
  into full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new fast-travel evac
  route verifier, existing extraction debrief fast-travel verifier, adjacent
  helipad/route/save/journal verifiers, whitespace diff check,
  `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command <
  /dev/null`, packaged null smoke, and packaged render smoke. Smoke logs
  contained only the already-allowed immediate-exit navigation/crowd warnings
  plus the known render-mode CoreAudio sample-rate warning.

## 2026-06-30 — Minimap route readability slice

- Continued the June 25 HUD/readability pass by restyling
  `CodeRescueMinimapWidget`, another UMG surface named in the UX guide as a
  remaining shared-theme rollout target.
- Rebuilt the top-right minimap from a raw dot canvas into a themed `NAV MAP`
  instrument with a stable `MinimapThemedPanel`, inner radar plot, summary
  line, nearest-objective cue, and compact category legend.
- Mirrored saved high-contrast, reduced-motion, and subtitle/text-scale
  settings into the minimap refresh path so the HUD scanner follows the same
  accessibility contract as the journal, terminal, skill tree, vitals, and
  fast-travel surfaces.
- Added active coding-language context and visible terminal/survivor/language/
  threat counts to the minimap summary so the selected language route stays
  present during navigation.
- Added text-first nearest objective guidance via `MakeRouteCue()`, reporting
  the nearest code, rescue, or language objective with distance and cardinal
  direction.
- Added high-contrast category colors and size-coded dots so player, terminal,
  survivor, language marker, aid pickup, and threat signals are distinguishable
  by more than hue alone.
- Updated accessibility, visual-regression, and first-ten-minutes onboarding
  manifests; documented the work at
  `Documentation/improvement_pass_2026-06-30/MINIMAP_ROUTE_READABILITY_SLICE.md`;
  and wired `Scripts/verify_minimap_route_readability_slice_pass.py` into full
  QA and local CI readiness.
- Validation passed: Python verifier compilation, the new minimap route
  readability verifier, adjacent HUD vitals/objective journal/threat compass/
  route-guidance verifiers, whitespace diff check, `./Recompile_Module.command
  < /dev/null`, `./Package_Mac_App.command < /dev/null`, packaged null smoke,
  and packaged render smoke. Smoke logs contained only the already-allowed
  immediate-exit navigation/crowd warnings plus the known render-mode CoreAudio
  sample-rate warning.

## 2026-06-30 — End state language run continuity slice

- Continued the June 25 release-readiness/save-clarity pass by upgrading the
  death and victory overlays into language-aware end-state summaries.
- Mirrored saved high-contrast, reduced-motion, and subtitle/text-scale
  settings into `CodeRescueDeathWidget` and `CodeRescueVictoryWidget` before
  building their panels.
- Added scrollable end-state content so large accessibility text can still
  expose title, language summary, stats, and actions at 720p.
- Added death/victory language summaries showing the active or completed coding
  language, the `MakeLanguageSaveSlotName()` start-screen resume slot, and
  `GetLanguageProgressSummary()`.
- Expanded death and victory stats with research points, run time, death count,
  and headshots while preserving existing survivor, terminal, zombie, score,
  restart, quit, and leaderboard behaviors.
- Renamed end-state actions to make their language-run effects explicit:
  resume from language save, start fresh language run, save this language run
  and quit, and save completion and quit.
- Added victory `SavePersistentRun()` coverage during victory construction and
  before victory quit, so completed language progress remains available from
  the future start screen.
- Updated accessibility, visual-regression, safe-learning, human-QA, and
  first-ten-minutes onboarding manifests; documented the work at
  `Documentation/improvement_pass_2026-06-30/END_STATE_LANGUAGE_RUN_CONTINUITY_SLICE.md`;
  and wired `Scripts/verify_end_state_language_run_continuity_slice_pass.py`
  into full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new end-state language
  continuity verifier, adjacent launch-language/save/onboarding/death-flow
  verifiers, refreshed historical May 27 safe-learning verifier, whitespace
  diff check, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke. Smoke logs contained only the already-allowed immediate-exit
  navigation/crowd warnings plus the known render-mode CoreAudio sample-rate
  warning.

## 2026-06-30 — Mac rendering AA readiness slice

- Continued the June 25 Apple Silicon/Metal performance guidance by replacing
  the packaged default TSR anti-aliasing path with the lower-risk TAA baseline
  recommended for Mac readability and runtime cost control.
- Updated `Config/DefaultEngine.ini` so the renderer profile now records
  `r.AntiAliasingMethod=2` and `r.TemporalAA.Upsampling=False`, with comments
  explaining that Unreal's CVar values map `2` to TAA and `4` to TSR.
- Added `RendererProfile` to `performance_city_layer_budget.tsv`, tying the
  renderer default to the same performance review surface used for city-layer
  budgets.
- Updated the creative-development inclusion plan, human QA signoff checklist,
  and visual-regression targets so Mac renderer readiness is visible during
  review rather than hidden inside project settings.
- Documented the work at
  `Documentation/improvement_pass_2026-06-30/MAC_RENDERING_AA_READINESS_SLICE.md`
  and wired `Scripts/verify_mac_rendering_aa_readiness_slice_pass.py` into full
  QA and local CI readiness.
- Validation passed: Python verifier compilation, the new Mac rendering AA
  readiness verifier, adjacent animation budget/asset budget/demo readiness
  verifiers, whitespace diff check, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render
  smoke, and packaged-log confirmation that both launch paths applied
  `r.TemporalAA.Upsampling:0` and `r.AntiAliasingMethod:2`. Smoke logs
  contained only the already-allowed immediate-exit navigation/crowd warnings
  plus the known render-mode CoreAudio sample-rate warning.

## 2026-06-30 — Mac hair-card compatibility slice

- Continued the June 25 Apple Silicon character guidance by converting the
  groom/hair caveat into an in-game and automated compatibility contract.
- Updated the Unreal systems character stage so it presents `Mac hair-card
  fallback` as the runtime path while keeping strand grooms in review-only
  status for Apple GPUs.
- Updated Mika Stone's character slot and greeting so the medic role advertises
  the promoted MetaHuman body, Maya/Houdini, Control Rig, IK, and card/mesh hair
  path, with strand grooms clearly held back for Mac review.
- Updated the active download intake board to separate `Groom review` from
  `Hair cards`, and added `MacHairCardRuntimeReady` plus
  `GroomStrandReviewOnlyMac` tags to the relevant in-world actors.
- Added `Content/CodeRescueData/mac_hair_compatibility_manifest.tsv`, refreshed
  the active asset intake scanner/table wording, and updated the Unreal systems,
  novel character, creative inclusion, human QA, and visual-regression
  manifests.
- Documented the work at
  `Documentation/improvement_pass_2026-06-30/MAC_HAIR_CARD_COMPATIBILITY_SLICE.md`
  and wired `Scripts/verify_mac_hair_card_compatibility_slice_pass.py` into
  full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new Mac hair-card
  compatibility verifier, adjacent Unreal systems/active asset scanner/Mac
  rendering/May 28 creative/MCP porting verifiers, whitespace diff check,
  `./Recompile_Module.command < /dev/null`, `./Package_Mac_App.command <
  /dev/null`, packaged null smoke, and packaged render smoke. The verifier
  classified 213 local groom assets and 7 local `.mhpkg` sources as review-only
  inputs with a Mac card/mesh fallback path. Smoke logs contained only the
  already-allowed immediate-exit navigation/crowd warnings plus the known
  render-mode CoreAudio sample-rate warning.

## 2026-06-30 — Mac feature capability gate slice

- Continued the June 25 Apple Silicon/Metal guidance by making Nanite, SM6,
  Virtual Shadow Maps, Lumen hardware tracing, and large PCG/Fab imports an
  explicit Mac feature-capability gate instead of an implicit project setting.
- Updated the active download intake board so it now separates `Nanite SM6`
  from `Fallback LOD`, with the board stating that Nanite/SM6 promotion needs
  M2-or-newer class hardware, macOS 15+ review, and a non-Nanite fallback.
- Added `MacNaniteSM6ReviewGate` and `MacNonNaniteFallbackReady` tags to the
  intake panels so visual review, static verifiers, and future validators can
  distinguish high-end renderer candidates from fallback-ready content.
- Clarified `DefaultEngine.ini` around VSMs: capability remains available for
  reviewed Nanite/foliage content, while runtime play continues to disable VSMs
  on the current procedural/non-Nanite fallback geometry.
- Added `Content/CodeRescueData/mac_feature_capability_manifest.tsv`, updated
  performance/creative/human-QA/visual manifests, documented the work at
  `Documentation/improvement_pass_2026-06-30/MAC_FEATURE_CAPABILITY_GATE_SLICE.md`,
  and wired `Scripts/verify_mac_feature_capability_gate_slice_pass.py` into
  full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new Mac feature
  capability gate verifier, adjacent Mac rendering/hair compatibility/public
  hardening verifiers, whitespace diff check, `./Recompile_Module.command <
  /dev/null`, `./Package_Mac_App.command < /dev/null`, packaged null smoke,
  and packaged render smoke. Both packaged logs confirmed
  `Nanite/SM6 review gates` plus `r.Shadow.Virtual.Enable = "0"` at runtime.
  The smoke scanner now explicitly allows Unreal's non-playability-affecting
  backup-log rotation warning while keeping missing-object, linker, fatal, and
  stale-asset blockers strict.

## 2026-06-30 — Mac asset import budget gate slice

- Continued the June 25 performance guidance by turning imported LOD, texture,
  shader, HLOD, VFX, and physics budget requirements into a visible Mac runtime
  promotion gate.
- Updated the active download intake board with `LOD audit`, `Texture cap`, and
  `Shader trim` panels tagged as `MacLODBudgetReviewGate`,
  `MacTextureMemoryReviewGate`, and `MacShaderComplexityReviewGate`.
- Updated the creative implementation runtime breadcrumb so packaged smoke logs
  now prove that `Mac LOD/texture/shader asset budget gates` are present in the
  compiled game.
- Added `Content/CodeRescueData/mac_asset_import_budget_gate.tsv` to classify
  crowd zombies, hero/survivor characters, static city modules, Nanite/HLOD
  candidates, texture/material instances, shader/VFX/fog effects, and physics
  props by their required Mac promotion rule.
- Updated performance/creative/human-QA/visual manifests, documented the work
  at
  `Documentation/improvement_pass_2026-06-30/MAC_ASSET_IMPORT_BUDGET_GATE_SLICE.md`,
  and wired `Scripts/verify_mac_asset_import_budget_gate_slice_pass.py` into
  full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new Mac asset import
  budget gate verifier, adjacent Mac feature/hair/rendering/public hardening
  verifiers, whitespace diff check, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke. Both packaged logs confirmed `Nanite/SM6 review gates`,
  `Mac LOD/texture/shader asset budget gates`, and
  `r.Shadow.Virtual.Enable = "0"` at runtime, with only the already-allowed
  immediate-exit navigation/crowd warnings plus the render-mode CoreAudio
  sample-rate warning.

## 2026-06-30 — Editor Data Validation contract slice

- Continued the June 25 validation guidance by adding the first native
  editor-only Data Validation module for promoted Operation Code Rescue asset
  manifests.
- Added `CodeRescueUnrealEditor` as an editor module and implemented
  `UCodeRescueAssetManifestValidator`, a `UEditorValidatorBase` subclass using
  the UE 5.7 `CanValidateAsset_Implementation` and
  `ValidateLoadedAsset_Implementation` signatures.
- The validator now blocks promoted `UCodeRescueAssetManifest` assets when
  zombie/survivor skeletal meshes, city building meshes, barricade meshes, VFX,
  radio briefing audio, or zombie attack audio references are missing.
- Added `Scripts/verify_code_rescue_data_validation_unreal.py` to confirm the
  validator is registered in UnrealEditor-Cmd and to write
  `Saved/DataValidation/code_rescue_data_validation_contract.json`.
- Added `Content/CodeRescueData/editor_data_validation_contract.tsv`, updated
  creative/human-QA manifests, documented the work at
  `Documentation/improvement_pass_2026-06-30/EDITOR_DATA_VALIDATION_CONTRACT_SLICE.md`,
  and wired `Scripts/verify_editor_data_validation_contract_pass.py` plus the
  Unreal smoke script into QA.
- Validation passed: Python verifier compilation, the new editor Data
  Validation verifier, adjacent Mac asset/feature/hair/rendering verifiers,
  whitespace diff check, `./Recompile_Module.command < /dev/null`,
  UnrealEditor-Cmd Data Validation smoke, `./Package_Mac_App.command <
  /dev/null`, packaged null smoke, and packaged render smoke.
- Unreal smoke report: `Saved/DataValidation/code_rescue_data_validation_contract.json`
  confirmed `/Script/CodeRescueUnreal.CodeRescueAssetManifest` and
  `/Script/CodeRescueUnrealEditor.CodeRescueAssetManifestValidator` are
  registered, with zero current manifest assets requiring validation.

## 2026-06-30 — Character promotion validation slice

- Continued the June 25 character-animation Data Validation guidance by
  adding a native validator for the zombie variant DataTable that drives
  runtime enemy silhouettes.
- Added `UCodeRescueZombieVariantTableValidator`, a `UEditorValidatorBase`
  subclass that validates `UDataTable` assets using `FZombieVariantRow`.
- The validator requires promoted zombie rows to have a readable display name,
  loadable skeletal mesh, loadable locomotion AnimBP class, bounded health /
  damage / speed / mesh-scale multipliers, bounded zone weights, and no
  Mac-incompatible strand-groom path. `Default` and `BaseMesh` remain explicit
  fallback exceptions.
- Added `Scripts/verify_character_promotion_validation_unreal.py` to load
  `/Game/CodeRescueAssets/DT_ZombieVariants` in UnrealEditor-Cmd, mirror the
  promotion checks, and write
  `Saved/DataValidation/code_rescue_character_promotion_validation.json`.
- Added
  `Content/CodeRescueData/character_promotion_validation_contract.tsv`,
  updated editor Data Validation / creative / human-QA manifests, documented
  the work at
  `Documentation/improvement_pass_2026-06-30/CHARACTER_PROMOTION_VALIDATION_SLICE.md`,
  and wired the new static and Unreal verifiers into QA.
- Validation passed: Python verifier compilation, the new character promotion
  static verifier, adjacent editor Data Validation / Mac asset / feature / hair
  verifiers, whitespace diff check, `./Recompile_Module.command < /dev/null`,
  UnrealEditor-Cmd character promotion smoke, `./Package_Mac_App.command <
  /dev/null`, packaged null smoke, and packaged render smoke.
- Unreal smoke report:
  `Saved/DataValidation/code_rescue_character_promotion_validation.json`
  confirmed
  `/Script/CodeRescueUnrealEditor.CodeRescueZombieVariantTableValidator`,
  one promoted `EliteBoomer` row, zero validation errors, skeletal mesh
  `/Game/UrbanZombie4/Mesh/Separated/SK_UrbanZombie4_Body.SK_UrbanZombie4_Body`,
  AnimBP class `/Game/UrbanZombie4/Demo/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C`,
  and zone weights `0=0.2`, `1=0.3`, `2=0.55`.

## 2026-06-30 — Physics promotion validation slice

- Continued the June 25 game-physics Data Validation guidance by adding a
  native validator for promoted physics-bearing content.
- Added `UCodeRescuePhysicsPromotionValidator`, a `UEditorValidatorBase`
  subclass that validates Physics Assets for authored simple bodies and treats
  runtime zombie ragdoll candidates as stricter assets requiring body and
  constraint coverage.
- Added a Geometry Collection validation branch that surfaces the fixed seed,
  live-piece budget, cached set-piece, and sleep/disable review contract before
  future destruction assets are promoted.
- Upgraded `ABarricadeActor` debris so each readable break-apart chunk now
  schedules `DebrisSleepDisableDelay`, sleeps the body, zeroes velocity, stops
  simulation, switches to query-only collision, and tags the chunk as
  `ChaosDebrisSleepDisabled`.
- Added
  `Content/CodeRescueData/physics_promotion_validation_contract.tsv`, updated
  editor Data Validation / creative / Mac budget / performance / human-QA
  manifests, documented the work at
  `Documentation/improvement_pass_2026-06-30/PHYSICS_PROMOTION_VALIDATION_SLICE.md`,
  and wired the new static and Unreal verifiers into QA.
- Validation passed: Python verifier compilation, the new physics promotion
  static verifier, adjacent editor Data Validation / character promotion / Mac
  asset / destructible cover / zombie death physics / surface impact verifiers,
  whitespace diff check, `./Recompile_Module.command < /dev/null`,
  UnrealEditor-Cmd physics promotion smoke via
  `Scripts/verify_physics_promotion_validation_unreal.py`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke.
- Unreal smoke report:
  `Saved/DataValidation/code_rescue_physics_promotion_validation.json`
  confirmed
  `/Script/CodeRescueUnrealEditor.CodeRescuePhysicsPromotionValidator`, one
  promoted `EliteBoomer` physics row, skeletal mesh
  `/Game/UrbanZombie4/Mesh/Separated/SK_UrbanZombie4_Body.SK_UrbanZombie4_Body`,
  Physics Asset
  `/Game/UrbanZombie4/Mesh/Phy_UrbanZombie4_PhysicsAsset.Phy_UrbanZombie4_PhysicsAsset`,
  17 bodies, 16 constraints, `ragdoll_promotion_ready=true`, 23 project Physics
  Assets, 20 sampled zombie-related Physics Assets, zero Geometry Collections,
  and zero validation errors. Packaged smoke logs contained only the
  already-allowed immediate-exit navigation/crowd warnings plus the known
  render-mode CoreAudio sample-rate warning.

## 2026-06-30 — World promotion validation slice

- Continued the June 25 world-development Data Validation guidance by adding a
  native validator for promoted city/world content.
- Added `UCodeRescueWorldPromotionValidator`, a `UEditorValidatorBase`
  subclass that validates imported city-module `UStaticMesh` candidates and
  applies stricter rules to assets explicitly staged as runtime-promoted world
  modules.
- Runtime-promoted city modules now require renderable LOD data, material slots
  for trim/master-material review, simple collision for gameplay traces, and no
  complex-as-simple collision for walkable/blocking surfaces unless a manifest
  exception is documented.
- Added PCG / World Partition / Packed Level Actor / HLOD / Data Layer review
  coverage so future authored + procedural city assets must carry streaming,
  fallback, safe-beat, and Mac package evidence before replacing the current
  playable C++ city fallback.
- Added `Content/CodeRescueData/world_promotion_validation_contract.tsv`,
  updated editor Data Validation / creative / Mac budget / performance /
  human-QA / visual manifests, documented the work at
  `Documentation/improvement_pass_2026-06-30/WORLD_PROMOTION_VALIDATION_SLICE.md`,
  and wired `Scripts/verify_world_promotion_validation_contract_pass.py` plus
  `Scripts/verify_world_promotion_validation_unreal.py` into QA.
- Validation passed: Python verifier compilation, the new world promotion
  static verifier, adjacent editor Data Validation / character promotion /
  physics promotion / Mac asset / Mac feature verifiers, whitespace diff check,
  `./Recompile_Module.command < /dev/null`, UnrealEditor-Cmd world promotion
  smoke via `Scripts/verify_world_promotion_validation_unreal.py`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke.
- Unreal smoke report:
  `Saved/DataValidation/code_rescue_world_promotion_validation.json` confirmed
  `/Script/CodeRescueUnrealEditor.CodeRescueWorldPromotionValidator`, 22 current
  world candidate static meshes, 0 strict runtime-promoted static meshes, 32 map
  assets, 64 HLOD-like assets, 1 Data Layer-like asset, and zero validation
  errors. Packaged smoke logs contained only the already-allowed immediate-exit
  navigation/crowd warnings plus the known render-mode CoreAudio sample-rate
  warning.

## 2026-06-30 — Mac compatibility validation slice

- Continued the June 25 Apple Silicon / Metal compatibility guidance by adding
  a native validator for the remaining future-only editor Data Validation
  surface.
- Added `UCodeRescueMacCompatibilityValidator`, a `UEditorValidatorBase`
  subclass that treats Groom / HairStrands-like assets as Mac review-only
  inputs unless a card or mesh fallback is documented, validates LOD evidence
  for Mac-promoted skeletal and static meshes, and warns on shader/texture-heavy
  material candidates so their budget evidence stays attached to promotion.
- Added Nanite/SM6 fallback rules so runtime-promoted static assets cannot be
  treated as Nanite-only without `MacNaniteSM6ReviewGate` evidence plus a
  `MacNonNaniteFallbackReady` path.
- Added `Content/CodeRescueData/mac_compatibility_validation_contract.tsv`,
  updated editor Data Validation / creative / performance / human-QA / visual
  manifests, documented the work at
  `Documentation/improvement_pass_2026-06-30/MAC_COMPATIBILITY_VALIDATION_SLICE.md`,
  and wired `Scripts/verify_mac_compatibility_validation_contract_pass.py` plus
  `Scripts/verify_mac_compatibility_validation_unreal.py` into QA.
- Validation passed: Python verifier compilation, the new Mac compatibility
  static verifier, adjacent editor / Mac hair / Mac feature / Mac asset / Mac
  rendering / character / physics / world verifiers, whitespace diff check,
  `./Recompile_Module.command < /dev/null`, UnrealEditor-Cmd Mac compatibility
  smoke via `Scripts/verify_mac_compatibility_validation_unreal.py`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, and packaged
  render smoke.
- Unreal smoke report:
  `Saved/DataValidation/code_rescue_mac_compatibility_validation.json`
  confirmed
  `/Script/CodeRescueUnrealEditor.CodeRescueMacCompatibilityValidator`, 359
  groom-like assets, 55 skeletal mesh candidates, 22 static mesh candidates,
  143 material candidates, 0 strict runtime-promoted Mac assets, 213 groom
  `.uasset` files, 5 MetaHuman `.mhpkg` sources, 2 groom art-source `.mhpkg`
  sources, all source/manifest renderer gates passing, and zero validation
  errors. Packaged render smoke also confirmed `SF_METAL_SM6` startup and Metal
  library mmap evidence; packaged smoke logs contained only the already-allowed
  immediate-exit navigation/crowd warnings plus the known render-mode CoreAudio
  sample-rate warning.

## 2026-06-30 — Squad personality tactical readability slice

- Continued the June 25 release and character readability guidance by making
  the rescue support team read as named tactical teammates instead of generic
  role actors.
- Added `MechanicalIdentity`, `BarkStyle`, `RoleAccentColor`,
  `ConfigureSquadPersonality()`, `GetHudCallsign()`,
  `GetRoleStatusLabel()`, `GetOrderResponseBark()`, and
  `PushRoleOrderBark()` to `ACompanionActor`.
- Updated the city support-team roster to match
  `squad_personality_manifest.tsv`: Mira Hale as Medic, Tomas Ives as
  Engineer, Ada Cross as Rifle Support, Noor Vance as Scout, and Briggs Vale
  as Heavy Rescue.
- Regroup, formation, hold, and follow orders now push short role-specific
  subtitle confirmations from up to two teammates, so Y/U/O/N commands feel
  acknowledged without opening a new command menu.
- Updated the HUD squad status to wrap into two lines with named role pips,
  role status snippets, medic readiness, support-fire state, formation, order,
  and manual medic prompt.
- Updated squad, onboarding, visual-review, and human-QA manifests, documented
  the work at
  `Documentation/improvement_pass_2026-06-30/SQUAD_PERSONALITY_TACTICAL_READABILITY_SLICE.md`,
  and wired `Scripts/verify_squad_personality_tactical_readability_slice_pass.py`
  into full QA and local CI readiness.
- Validation passed: Python verifier compilation, the new squad personality
  tactical readability verifier, legacy rescue-team survivability verifier,
  demo-readiness verifier, `./Recompile_Module.command < /dev/null`,
  `./Package_Mac_App.command < /dev/null`, packaged null smoke, packaged render
  smoke, packaged runtime log contracts, and scoped whitespace diff check.

## 2026-06-30 — Fail-safe objective board slice

- Continued the June 25 playability/readability guidance by adding a save-backed
  `FAIL-SAFE OBJECTIVE BOARD` to the `J` objective journal.
- Added `UCodeRescueGameInstance::GetFailSafeObjectiveBoardSummary()` so the
  journal can derive selected language, language-only save slot, start-screen
  Resume availability, active route phase, terminal/survivor state, return
  markers, safe recovery controls, protected terminal safety, and the next
  action from existing save/campaign state.
- Added `FailSafeObjectiveBoardText` to `UCodeRescueObjectiveJournalWidget`,
  styled through the existing high-contrast/text-scale journal path, so the
  recovery guidance appears before the route map and inventory readouts.
- Added `Content/CodeRescueData/fail_safe_objective_board_manifest.tsv`,
  updated the inventory/journal, accessibility, visual regression, human QA,
  first-ten-minutes, creative inclusion, and implementation-ledger records, and
  documented the work at
  `Documentation/improvement_pass_2026-06-30/FAIL_SAFE_OBJECTIVE_BOARD_SLICE.md`.
- Wired `Scripts/verify_fail_safe_objective_board_slice_pass.py` into full QA
  and local CI readiness; focused validation and module recompile are queued for
  this pass.

## 2026-06-30 — Terminal practice run slice

- Continued the June 25 terminal flow-plan guidance by adding a practice-only
  rehearsal path before live selected-language validation.
- Added `PracticeRunButton`, `OnPracticeClicked()`, `PracticeRunCount`, and
  shared `RunValidation(bool bPracticeOnly)` handling to
  `UCodeTerminalWidget`.
- `PRACTICE RUN [Ctrl+P]` now validates the current code and reports
  `PRACTICE RUN - NO SAVE ADVANCE`, `PRACTICE-ONLY DEBRIEF`, and
  `Practice Run Lock` without marking the terminal solved, revealing the
  survivor route, saving attempts/solves, awarding ResearchPoints or coding
  score, changing streaks, or writing survivor intel archive data.
- Live `VALIDATE CODE` still commits the existing post-solve debrief, rewards,
  survivor-route reveal, language save update, and journal continuity.
- Added `Content/CodeRescueData/terminal_practice_run_manifest.tsv`, updated
  curriculum, selected-language terminal flow, accessibility, visual, human QA,
  onboarding, creative inclusion, and implementation-ledger records, documented
  the work at
  `Documentation/improvement_pass_2026-06-30/TERMINAL_PRACTICE_RUN_SLICE.md`,
  and wired `Scripts/verify_terminal_practice_run_slice_pass.py` into full QA
  and local CI readiness.

## 2026-06-30 — Terminal reward choice kiosk slice

- Continued the June 25 terminal flow-plan guidance by implementing the
  promised `reward choice kiosk` as a live terminal reward surface.
- Added save-backed `RewardChoiceEligibleTerminalIds`,
  `ClaimedTerminalRewardChoiceIds`, `LastTerminalRewardChoiceSummary`, and
  related helper methods to the selected-language profile so one reward choice
  can persist through start-screen Resume without duplicate claims.
- Added `REWARD: RESEARCH +2 RP`, `REWARD: FIELD KIT`, and
  `REWARD: CRAFTING CACHE` terminal buttons; live `VALIDATE CODE` success now
  unlocks the kiosk, while practice runs and bypass-kit solves leave it
  unavailable.
- Implemented `UCodeRescueGameInstance::ClaimTerminalRewardChoice()` to grant
  ResearchPoints, field supplies, or crafting resources, record the claim, and
  save the active language profile.
- Added `Content/CodeRescueData/terminal_reward_choice_kiosk_manifest.tsv`,
  updated curriculum, selected-language terminal flow, accessibility, visual,
  human QA, onboarding, creative inclusion, and implementation-ledger records,
  documented the work at
  `Documentation/improvement_pass_2026-06-30/TERMINAL_REWARD_CHOICE_KIOSK_SLICE.md`,
  and wired `Scripts/verify_terminal_reward_choice_kiosk_slice_pass.py` into
  full QA and local CI readiness.

## 2026-06-30 — First-session route preview slice

- Continued the June 25 selected-language start-screen guidance by adding a
  `FIRST-SESSION ROUTE PREVIEW` before active gameplay begins.
- Added `UCodeRescueGameInstance::GetFirstSessionRoutePreviewSummary()` so
  launch UI can show the current/default track-only profile, language save
  slot, resume state, first city, protected terminal -> survivor marker ->
  extraction route shape, terminal, curriculum stage/focus, landmark, survivor
  contact, and beginner/normal/challenge tuning band from campaign data.
- Added `UCodeRescueGameInstance::GetLaunchLanguageSaveRosterSummary()` so the
  same preview block lists Java, C, C+, C++, Python, and MATLAB as
  `RESUME AVAILABLE` or `NEW RUN READY` before the user selects a row.
- Added `FirstSessionRoutePreviewText` to `UCodeRescueMainMenuWidget` launch
  language mode and clarified that each `NEW` or `RESUME` row deploys only the
  clicked language.
- Added a fallback `FIRST-SESSION ROUTE PREVIEW` world prompt to the 3D launch
  selection scene so the route contract remains visible if the widget path is
  unavailable.
- Added `Content/CodeRescueData/first_session_route_preview_manifest.tsv`,
  updated selected-language terminal flow, accessibility, onboarding, visual,
  human QA, creative inclusion, and implementation-ledger records, documented
  the work at
  `Documentation/improvement_pass_2026-06-30/FIRST_SESSION_ROUTE_PREVIEW_SLICE.md`,
  and wired `Scripts/verify_first_session_route_preview_slice_pass.py` into
  full QA and local CI readiness.

## 2026-06-30 — Field checklist HUD slice

- Continued the June 25 onboarding/objective-clarity guidance by adding a live
  `FIRST TEN MINUTES FIELD CHECKLIST` to the HUD beside the minimap/navigation
  stack.
- Added `FieldChecklistText` / `FirstTenMinutesFieldChecklistText` to
  `UCodeRescueHUDWidget` so the player can see the active track-only save slot,
  start-screen Resume/autosave state, protected terminal -> survivor marker ->
  extraction route shape, current phase, and recovery keys during live play.
- The checklist changes phase from protected terminal to survivor marker to
  extraction based on `SolvedTerminalIds` and `RescuedSurvivorNames`, and it
  names `E`, `Ctrl+P`, `T`, `Backspace/F8`, `J`, and `P/Esc` where relevant.
- Added `Content/CodeRescueData/field_checklist_hud_manifest.tsv`, updated
  objective route, accessibility, onboarding, visual, human QA, creative
  inclusion, and implementation-ledger records, documented the work at
  `Documentation/improvement_pass_2026-06-30/FIELD_CHECKLIST_HUD_SLICE.md`,
  and wired `Scripts/verify_field_checklist_hud_slice_pass.py` into full QA
  and local CI readiness.

## 2026-06-30 - Independent review and curriculum-first gallery slice

- Reviewed `Operation_Code_Rescue_Review_2026-06-30.pdf` and cleared the
  watchdog's real-regression gate by updating stale verifier expectations for
  launch-language drift, AI pursuit wording, HUD weapon wording, armor
  mitigation wording, terminal validation helper movement, and survivor intel
  ledger counts.
- Updated `Run_Local_CI_Readiness.command` so the local CI verifier gate now
  starts with `Scripts/claude_oversight_watchdog.py`; retained a comment-only
  verifier coverage index for legacy self-checks while avoiding the old
  hand-executed curated list.
- Extended `SpawnChallengeRoomConceptArtLayer` with
  `CurriculumFirstReviewGallery`, an eight-station teaching-core review space
  covering sum return, lock boolean, reverse string, palindrome, FizzBuzz, even
  filter, linked list, and binary search.
- Each gallery station now contains a visible-test pylon, hidden-test pylon,
  common-mistake marker, mentor character proxy, survivor character proxy, and
  text-first label tagged with `VisibleHiddenTestGallery`,
  `ValidatorArchetypeProof`, `IntrinsicIntegrationReview`,
  `OperationReview20260630`, and `ThreeDReviewCandidate`.
- Added `Scripts/render_curriculum_first_review_gallery.py` to produce
  `Saved/VisualReview/curriculum_first_review_gallery_render.png` for owner
  review of the structures and character proxies without opening Unreal.
- Added `Content/CodeRescueData/curriculum_first_review_gallery_manifest.tsv`,
  documented the work at
  `Documentation/improvement_pass_2026-06-30/CURRICULUM_FIRST_REVIEW_GALLERY_SLICE.md`,
  and updated challenge-room, curriculum feedback, accessibility, onboarding,
  visual review, human QA, implementation-ledger, and verifier coverage.

## 2026-07-01 - Data-driven filter node runtime slice

- Continued the Claude/Codex curriculum-first directive by proving one concept
  node end-to-end before attempting the full curriculum graph expansion.
- Expanded `Content/CodeRescueData/curriculum_database.json` so the tier-4
  filter node now has five playable challenges for Java, C, C+, C++, Python,
  and MATLAB, each with micro-lessons, worked examples, prompts, visible and
  hidden tests, misconceptions, strategies, `world_effect`, post-solve copy,
  and language-specific starter skeletons.
- Wired `UCodeTerminalWidget` to select the data-driven filter node for
  existing even/filter terminals, show the teach/apply payload in the terminal
  checklist, replace compatible solved starters with exercise skeletons, append
  post-solve reinforcement, record telemetry, and reveal adaptive scaffold text
  after repeated repair attempts.
- Wired `ACodeRescueGameMode::RevealSolvedTerminalRescueRoute` to spawn
  `ACodeRescueSolveEffectActor` on successful validation and add a reviewable
  `DataDrivenFilterNode` 3D response with kept/rejected lanes, convoy output
  markers, mentor point-gesture proxy, survivor boarding pose proxies, and
  nonblocking physics-safe rails.
- Added `Content/CodeRescueData/data_driven_filter_node_manifest.tsv`, updated
  `Documentation/improvement_pass_2026-07-01/LEARNING_VERTICAL_SLICE.md` and
  `00_OVERVIEW.md`, and strengthened
  `Scripts/verify_learning_vertical_slice_pass.py` to enforce data coverage,
  runtime wiring, solved-route world response, and documentation traceability.

## 2026-07-01 - Data-driven boolean node runtime slice

- Continued the curriculum-first implementation by expanding the tier-2
  boolean/conditionals node as the second end-to-end playable concept.
- Updated `Content/CodeRescueData/curriculum_database.json` so the airlock
  access node now has five playable challenges for Java, C, C+, C++, Python,
  and MATLAB with validator-compatible `shouldUnlock` / `should_unlock`
  starter skeletons, visible tests, hidden truth-table tests, misconceptions,
  strategies, `world_effect`, and post-solve explanations.
- Generalized `UCodeTerminalWidget` data-driven selection so existing lock
  terminals now receive the same teach/apply payload, exercise starter,
  telemetry, post-solve reinforcement, and adaptive scaffold behavior as the
  filter node.
- Extended `ACodeRescueGameMode::RevealSolvedTerminalRescueRoute` with a
  `DataDrivenBooleanNode` world response: truth-table lamps, paired required
  switch pylons, an opening airlock slab, mentor truth-table pose proxy,
  survivor exit pose proxy, and nonblocking physics-safe rails.
- Added `Content/CodeRescueData/data_driven_boolean_node_manifest.tsv`, updated
  the July 1 learning docs and overview, and extended
  `Scripts/verify_learning_vertical_slice_pass.py` so future verification
  checks both the filter and boolean concept nodes.

## 2026-07-01 - Data-driven reverse node runtime slice

- Continued the curriculum-first implementation by expanding the tier-5
  strings/indexing reverse node as the third end-to-end playable concept.
- Updated `Content/CodeRescueData/curriculum_database.json` so the reverse
  code node now has five playable challenges for Java, C, C+, C++, Python, and
  MATLAB with validator-compatible `reverseString` / `reverse_string` starter
  skeletons, visible tests, hidden boundary tests, misconceptions, strategies,
  `world_effect`, and post-solve explanations.
- Generalized `UCodeTerminalWidget` data-driven selection so existing reverse
  terminals now receive the same teach/apply payload, exercise starter,
  telemetry, post-solve reinforcement, and adaptive scaffold behavior as the
  filter and boolean nodes.
- Extended `ACodeRescueGameMode::RevealSolvedTerminalRescueRoute` with a
  `DataDrivenStringNode` world response: input/output glyph tiles, last-to-first
  transfer beams, a rolling vault door slab, mentor last-to-first pose proxy,
  survivor unlock pose proxy, and nonblocking physics-safe rails.
- Added `Content/CodeRescueData/data_driven_reverse_node_manifest.tsv`, updated
  the July 1 learning docs and overview, and extended
  `Scripts/verify_learning_vertical_slice_pass.py` so future verification
  checks the filter, boolean, and reverse concept nodes together.

## 2026-07-01 - Revised recommendations validation bridge

- Reviewed `Operation_Code_Rescue_Revised_Recommendations_2026-07-01.pdf` and targeted its highest-priority
  gap: data-driven validation and generic terminal selection.
- Added curriculum `validator` metadata for the 15 currently wired boolean, filter, and reverse challenge
  rows, then exposed that field through `FCodeRescueChallenge` and the learning loader.
- Reworked `UCodeTerminalWidget` to match terminal challenges to curriculum rows through a normalized
  validator-key path and to synthesize the runtime `FChallengeSpec` from the selected curriculum row,
  including visible and hidden tests copied into `FChallengeSpec::TestCases`.
- Extended `UCodeRunnerLibrary` so Java, C, C+, C++, Python, and MATLAB external validators generate
  assertions from curriculum `TestCases` for `boolean_lock`, `even_filter`, and `reverse_string` nodes.
  The safe in-engine fallback still uses static analysis but now reports declared test counts.
- Added `Documentation/improvement_pass_2026-07-01/REVISED_RECOMMENDATIONS_RESPONSE.md`, updated the
  July 1 overview/learning docs, and strengthened `Scripts/verify_learning_vertical_slice_pass.py` to
  enforce the validator metadata and declarative-test bridge.

## 2026-07-09 - Production world, camera occlusion, art, and package pass

- Replaced the default all-layers presentation with a curated production world
  profile while retaining prototype galleries behind the explicit
  `-CodeRescueDevelopmentShowcase` flag.
- Removed production cloud plates, objective proxy towers, briefing walls,
  background horde display geometry, labels, and bounded arrival blockers;
  retained gameplay structures, HUD guidance, focus beacons, drone guidance,
  encounters, educational terminals, and the mountable Jeep.
- Rebuilt the arrival as an open intersection with a perpendicular cross
  street, connected sidewalks and crosswalks, 54 streetscape actors, 38
  CityKitV3 actors plus the replacement V4 storefront, six cross-street
  buildings, authored vehicle cover, and a SidewalkV3/BusStopV3 safehouse
  approach.
- Added a third-person bounds-based camera occlusion pass for visible
  no-collision architecture, preserved the always-on spring-arm probe, and
  retained contained top-down/isometric camera lengths below roof level.
- Promoted Manny/Quinn plus animation blueprints as production player,
  survivor, and support-team defaults while preserving prototype characters
  behind `-CodeRescueUsePrototypeCharacters`.
- Refined and regenerated 19 Blender CityKitV3 GLBs and five WeaponsV3 GLBs,
  reimported all 24 assets in place, and added reproducible import and review
  render scripts.
- Fixed the packaged launch selector lifecycle by resolving input through the
  GameMode-owned widget across Slate rebuilds; the final package held the start
  screen for more than 15 seconds with no auto-selection or recovery warning.
- Compiled, clean-cooked, staged, locally signed, and archived
  `PackagedMac/Mac/CodeRescueUnreal.app`; deep code-sign and local integrity
  checks passed, followed by an 85-second error-free rendered package soak.
- Added complete implementation, verification, package, and visual-review
  records under `Documentation/improvement_pass_2026-07-09/`.

## 2026-07-09 - First-level combat, armory, interaction, and V4 art pass

- Added a production two-arm aiming presentation layer that preserves Manny
  locomotion, turns the upper body toward the camera aim, and keeps the held
  weapon attached to the right hand.
- Restored stamina-aware Space/gamepad jump with deliberate vertical velocity
  and air-control tuning; Space no longer invokes weapon fire.
- Rebuilt the `P` pause surface around a clickable Field Armory with a live 3D
  weapon viewport, all 17 weapon entries, current/capacity ammunition, tactical
  descriptions, statistics, Previous/Next controls, and functional Equip.
- Reworked firearm resolution to combine world occlusion, weapon-channel and
  zombie-object traces, exact capsule geometry, narrow line-of-sight aim assist,
  and physically constrained piercing/explosive behavior.
- Added localized ballistic cavity/decal wounds to zombies and side-aware bite
  wounds to the player, while replacing launch-scale death reactions with
  clamped impact-direction impulses.
- Added a nine-second grounded zombie corpse hold followed by a 2.8-second
  gradual sink/scale fade and removal; fallen companions use an equivalent
  readable lifecycle.
- Authored nine Blender 5.1.2 V4 assets for first-level structures, cover,
  weapons, and wounds; imported all nine into Unreal and verified render
  triangles, LODs, materials, and bounds through a commandlet audit.
- Hard-gated the new storefront, armory, triage checkpoint, sandbag cover, and
  practical lighting to New York, replacing one V3 facade instead of
  overlapping it.
- Added deterministic editor and packaged audit paths for the 17-item armory
  cycle and full jump/bite/two-shot/wound/death/corpse/fade lifecycle. Both
  packaged audits reported COMPLETE PASS.
- Rebuilt `PackagedMac/Mac/CodeRescueUnreal.app` as bundle version
  `51494982.0.194`, verified both packaged smoke profiles, local integrity, and
  deep code signing, and recorded the implementation and test evidence in the
  July 9 documentation set.

## 2026-07-09 - First-level V5 access, target lock, sky, locality, and final package

- Added persistent auto target lock with living/collidable/range/cone/line-of-sight
  validation, smooth controller and body turning, two-arm Manny torso aim, a
  HUD lock marker, and physical shot redirection only while the lock is valid.
- Removed post-miss proximity damage and added a deterministic skyward-miss
  assertion proving that a nearby off-ray zombie receives zero damage.
- Authored and imported five Blender V5 assets: an accessible market, clinic,
  open cafe, point-star field, and detailed moon. The three structures have
  literal 230 cm entrances, explicit door-preserving collision, interiors,
  practical lighting, and functional pickups.
- Moved first-level roads, sidewalks, crosswalks, and structures onto one
  canonical ground plane, disabled the old New York transform rewrite, and
  removed the V4 checkpoint that overlapped the clinic entrance.
- Enabled GameMode ticking at construction time and implemented a continuous
  sun, sunset, night, sunrise, moon, stars, fog, and skylight cycle.
- Hardened zombie physical-animation startup by creating bodies lazily on
  impact and using the procedural pose fallback immediately for invalid rigs;
  no repeated invalid-body warnings remain in the clean editor integration run.
- Added exact first-level challenge audits for Java, C, C+, C++, Python, and
  MATLAB plus a 48/48 six-language validator-shape matrix.
- Added one packaged integrated acceptance run covering 3/3 entrances, 6/6
  language solutions, four sky phases, target lock, jump, bite wound, harmless
  miss, two physical hits, corpse hold/fade/removal, and 17/17 armory previews.
- Clean-cooked 1,815 packages and archived bundle version `51494982.0.196` at
  `PackagedMac/Mac/CodeRescueUnreal.app`. Null and Metal smoke tests, local
  package integrity, and deep code signing passed; Developer ID signing and
  notarization remain external credentialed distribution steps.
- Recorded implementation, exact solutions, logs, packaged screenshots, and
  package evidence under `Documentation/improvement_pass_2026-07-09/`.

## 2026-07-10 - Survivor access, ten challenges, supplies, pause mouse, and V6 package

- Unified first-level construction and player recovery around shared campaign
  arena bounds, expanded the catch floor/walls, ground-snapped recovery to the
  actual walkable surface, and moved the clinic plus two route blockers so the
  survivor, extraction point, all three interiors, and lower street plane remain
  reachable without an out-of-arena respawn.
- Added long-approach and doorway capsule sweeps; the final editor and packaged
  audits passed all three building entrances for clearance, street approach,
  arena containment, and level ground.
- Established a ten-station requirement for all 465 campaign cities, preserving
  each legacy stage-one ID while generating 4,650 globally unique challenge
  IDs. Updated survivor, HUD, journal, beacon, travel, director, boss, and
  fail-safe progression to enforce and display `x/10`.
- Built the first-level 5-by-2 protected coding concourse and validated the ten
  physical terminals across Java, C, C+, C++, Python, and MATLAB for 60/60
  accepted canonical submissions. The audit proves nine stations remain locked
  and the tenth unlocks the survivor route.
- Added three-item clean-solve supply caches and deterministic zombie-death
  supply drops, both ground-snapped and runtime-audited.
- Increased the full day/night cycle to 1,800 seconds and verified day, sunset,
  night, sunrise, stars, and moon.
- Reworked pause input to UI-only/no-capture operation, enabled all 18 actions
  for mouse/touch, added a geometry-based pointer fallback, retained the 17-item
  live armory, and added save-on-craft Flare, Stim, and Grenade recipes.
- Clean-cooked and archived bundle version `51494982.0.197` at
  `PackagedMac/Mac/CodeRescueUnreal.app`. NullRHI smoke, Metal smoke, a 20-second
  normal language-gate hold, the full packaged integrated audit, asset budgets,
  local package integrity, and strict deep code-sign verification passed.
- Added the implementation dossier, exact ten-station solution reference, and
  final evidence under `Documentation/improvement_pass_2026-07-10/`.

## 2026-07-11 - Narration, ground, terminal, population, validator, animation, reader, and V7 package

- Added single-owner radio narration with city-switch debounce, tracked cooked
  audio/system speech, replacement shutdown, and EndPlay cleanup. The packaged
  city-switch audit stopped the prior narrator and left no speech process.
- Replaced catch-floor-derived grounding with canonical city-plane
  normalization, moved the emergency catch floor to local Z=-620, and hardened
  recovery against below-world hits. The packaged Los Angeles audit recovered
  at relative Z=92 with zero elevated regions and no loop.
- Restored renewable zombie populations across resumed saves while preserving
  permanent boss/miniboss defeats. The real Java save ignored 130 historical
  renewable deaths and loaded 124 active enemies in each audited city.
- Rebuilt terminal editor state brushes for 18.89:1 contrast, suppressed HUD and
  transient debug overlap during modal use, and captured a clean Metal review.
- Fixed the reverse validator to accept prefix `--i`, added the exact reported
  solution as a regression case, and passed 60/60 external challenge checks plus
  the alternate solution at score 100.
- Removed solved-state `CODE ACCEPTED` reader replay, made the reader focusable
  and mouse-closable, and enforced reader/journal/pause exclusivity.
- Replaced Manny's undriven sample animation dependency with explicit idle,
  walk, run, jump, fall, and cooked-safe procedural land states while retaining
  physical two-arm aiming and target lock. Runtime animation and combat audits
  passed without additive-clip warnings.
- Extended pause pointer routing, action telemetry, and paused UI presentation;
  the Metal audit verified all 18 controls, crafting, and all 17 weapon previews.
- Clean-cooked 1,815 packages and archived locally signed bundle version
  `51494982.0.198` at `PackagedMac/Mac/CodeRescueUnreal.app`. Editor/package
  integration, real-save recovery, language-gate hold, warning scan, local
  integrity, and deep code signing passed. Full records are under
  `Documentation/improvement_pass_2026-07-11/`.
