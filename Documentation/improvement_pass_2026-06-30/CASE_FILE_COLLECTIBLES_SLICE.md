# Case File Collectibles Slice

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: turn city routes into readable, authored spaces with optional environmental storytelling.
- `TOP_50_RECOMMENDATIONS.pdf`: add collectible case files that deepen story and coding context without blocking the core rescue loop.
- `CHARACTER_ANIMATION_DEEPDIVE.pdf`: make survivor identity and rescue stakes more specific before future character/animation promotion.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keep new features save-backed, auditable, and covered by repeatable QA.

## Runtime Implementation

- Added `ACaseFilePickupActor`, a glowing, ground-snapped collectible actor with mission title/body/tint data, overlap and interact collection, subtitle feedback, and hidden collected state.
- Added selected-language save persistence through `CollectedCaseFileIds` and `LastCollectedCaseFileTitle` on both `UCodeRescueGameInstance` and `UCodeRescueSaveGame`.
- Added `HasCollectedCaseFile()`, `RecordCaseFileCollected()`, and `GetCaseFileCollectionSummary()` so runtime actors, the journal, and future Blueprint/UI surfaces can share the same contract.
- Added `SpawnCollectibleCaseFilesForCity()` to `ACodeRescueGameMode`, spawning three case files per city: terminal evidence, survivor note, and route brief.
- Wired player interaction scanning so case files can be collected with the same nearest-interactable flow as terminals, pickups, NPCs, survivors, language stations, and helipads.

## Player Experience

- Terminal evidence connects the selected city's coding concept to the physical route and active language track.
- Survivor notes connect the mission survivor and radio briefing to the route unlocked by the protected coding terminal.
- Route briefs connect landmark identity, novel gameplay detail, progression intent, and accessibility polish into a world-facing note.
- Collection immediately saves to the active selected-language run, shows a `CASE FILE` subtitle, and destroys the pickup so the city does not duplicate it.
- The Objective Journal now includes a case-file collection summary beneath the active language and accessibility line.

## Save Contract

- `CollectedCaseFileIds` stores stable IDs in the form `{TerminalId}_case_{terminal_evidence|survivor_note|route_brief}`.
- `LastCollectedCaseFileTitle` gives the journal a readable recent-collection signal without needing to scan all actors.
- `ResetRun()` clears case-file state, while persistent save/load preserves it with existing terminal, survivor, zombie, settings, skill, and language-run data.
- `ApplyObjectiveStateToLevel()` hides already-collected case-file actors on reload, matching the survivor and zombie restoration pattern.

## Data And QA Records

- Added `Content/CodeRescueData/case_file_collectibles_manifest.tsv` as the traceability manifest for collectible type, source fields, runtime actor, save contract, and player feedback.
- Updated curriculum feedback, first-ten-minutes onboarding, human QA, visual regression, creative inclusion, accessibility, and character/world design manifests.
- Added `Scripts/verify_case_file_collectibles_slice_pass.py` and wired it into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.

## Validation

- Static verifier: `python3 Scripts/verify_case_file_collectibles_slice_pass.py`
- Compile gate: `./Recompile_Module.command < /dev/null`
- Package gate: `./Package_Mac_App.command < /dev/null`
- Runtime smoke gates: `./Smoke_Test_Packaged_App.command null` and `./Smoke_Test_Packaged_App.command render`
- Runtime log gate: `python3 Scripts/verify_runtime_log_contracts.py Saved/Logs/PackagedSmoke_null.log` and render log equivalent.

## Human QA Notes

- Start a selected-language run, enter the first city, collect a terminal evidence file, and confirm the `CASE FILE` subtitle names the save-backed language run.
- Collect the survivor note and route brief, then open the Objective Journal and confirm the count and last collected title update.
- Save, close, relaunch, select the same language resume slot from the start screen, and confirm collected case files remain hidden.
- Switch to a different language run and confirm that language can collect its own case files independently.
