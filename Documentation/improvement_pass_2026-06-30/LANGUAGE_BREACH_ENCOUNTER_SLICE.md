# Language Breach Encounter Slice

## Purpose

This pass continues the creative development backlog by turning the solved-code rescue route into a small playable encounter beat. The previous slice made the environment visibly respond when a terminal was solved; this slice makes that response depend on the selected coding language and gives the player a readable cover-and-cache checkpoint before extracting the survivor.

## Guidance Used

- `WORLD_DEVELOPMENT_DEEPDIVE`: make coding-as-rescue legible through cause/effect, route clarity, environmental storytelling, and authored-feeling spaces.
- Launch-language requirement: the selected coding language should remain the active implementation language throughout gameplay, including world feedback after a solve.
- Previous route-unlock slice: solved terminals already reveal route pulses, beacons, rewards, and save-restored route state.

## Implementation

`ACodeRescueGameMode::RevealSolvedTerminalRescueRoute` now resolves the active language from `UCodeRescueGameInstance::SelectedLanguage` and maps it to:

- a visible language label and color,
- a language-specific breach cue,
- a language-specific route reward,
- a patrol zombie variant for the immediate solve encounter.

The unlocked route now includes a tagged `LanguageBreachCheckpoint` area with collision cover, a lit header signal, logic-glyph panels, route text, and a language reward cache. The checkpoint actors are also tagged `LanguageSpecificEncounter`, `SelectedLanguageOnly`, `RouteEncounterBeat`, and `LanguageTrack_<Language>`, allowing future QA, capture, or editor tooling to find the feature.

## Immediate Encounter Versus Save Restore

The checkpoint set dressing and reward cache are reconstructed with the solved route because they represent persistent world state. The patrol zombies are intentionally gated behind `!bFromLoad && !bSandboxMode`, so reloading a language save does not repeatedly respawn the same breach fight and sandbox practice remains combat-free.

## Language Mapping

- Java: flare cache, urban zombie patrol, object-contract breach cue.
- C: armor cache, business-suit patrol, pointer-route breach cue.
- Python: smoke cache, nurse patrol, readable-route breach cue.
- MATLAB: stim cache, bloated patrol, matrix-route breach cue.
- C+: scrap cache, base-mesh patrol, legacy-bridge breach cue.
- C++: ammo cache, charger patrol, template-route breach cue.

## Verification

Added `Scripts/verify_language_breach_encounter_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the selected-language mapping, checkpoint tags, language reward cache, immediate-only patrol guard, solved-route duplicate guard, documentation, and QA wiring.

## Remaining Work

This still uses procedural primitives so it can package reliably today. A later art pass should replace the checkpoint pieces with authored modular meshes, add audio/VFX per language, tune patrol placement through playtesting, and eventually connect the encounter to animation montages or survivor rescue cinematics.
