# Expanded Extraction Set-Pieces Slice

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: make city objectives feel authored and climactic instead of ending on a generic marker.
- `TOP_50_RECOMMENDATIONS.pdf`: add rooftop, convoy, boat, rail, bridge, and helipad extraction endings as visible gameplay staging.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keep new features cook-safe, package-ready, clearly documented, and easy to validate.
- `GAME_PHYSICS_DEEPDIVE.pdf`: protect traversal and interaction readability by keeping decorative set-piece pieces nonblocking unless a later physics pass intentionally promotes them.

## Runtime Implementation

- Added `SpawnExpandedExtractionSetPieceForCity()` to `ACodeRescueGameMode`.
- `SpawnCampaignCity()` now calls the expanded extraction layer immediately after `SpawnHelipadForCity()`, so the set piece belongs to extraction without changing helipad, jeep, survivor, or fast-travel behavior.
- The set-piece variant is deterministic by `CityIndex % 6`, producing `RooftopLift`, `ConvoyGate`, `HarborBoat`, `RailEvac`, `BridgeRun`, and `HelipadCommand`.
- Every piece is built from existing cook-safe primitive helpers (`SpawnBlock`, `SpawnRotatedBlock`, and `SpawnGuideText`) and tagged with `ExpandedExtractionSetPiece`, its variant tag, `ExtractionSetPieceNonBlocking`, `HelipadClearancePreserved`, `WorldDevelopmentDeepDive`, `Top50Recommendations`, and `ReleaseDossier`.
- Labels use `Mission.SurvivorName`, `Mission.LandmarkName`, `Mission.CurriculumFocus`, and `Mission.CityName` so extraction staging connects the rescued survivor, lesson payoff, and city identity.

## Player Experience

- Reaching the helipad after rescue now reveals a visible authored extraction finale instead of only a single helipad marker.
- Cities rotate through six endings: a rooftop lift, convoy gate, harbor boat, rail evacuation, bridge run, or command-post extraction.
- The helipad remains the actual interaction point for extraction debrief and fast travel.
- The set pieces are spatially arranged around the helipad to frame the end-of-city route while preserving the existing EVAC HELIPAD prompt.

## Accessibility And Traversal

- The slice is text-first: each extraction area includes an `EXTRACTION SET PIECE` label plus a variant-specific label.
- Geometry is spawned with collision disabled and tagged `HelipadClearancePreserved`, avoiding accidental path blocks or fast-travel prompt interference.
- Color and shape reinforce the extraction fantasy, but the labels carry the state for players who cannot rely on color alone.

## Data And QA Records

- Added `Content/CodeRescueData/expanded_extraction_set_pieces_manifest.tsv` to document variants, runtime hook, visual contract, gameplay contract, and QA notes.
- Updated curriculum feedback, creative inclusion, onboarding, visual-regression, human-QA, and accessibility manifests.
- Added `Scripts/verify_expanded_extraction_set_pieces_slice_pass.py` and wired it into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.

## Validation

- Static verifier: `python3 Scripts/verify_expanded_extraction_set_pieces_slice_pass.py`
- Regression verifier: `python3 Scripts/verify_helipad_extraction_ready_slice_pass.py`
- Compile gate: `./Recompile_Module.command < /dev/null`
- Package gate: `./Package_Mac_App.command < /dev/null`
- Runtime smoke gates: `./Smoke_Test_Packaged_App.command null` and `./Smoke_Test_Packaged_App.command render`

## Human QA Notes

- Inspect city indices that cover each variant and confirm the extraction ending is readable from the helipad approach.
- Rescue a survivor, approach the helipad, and confirm the fast-travel/debrief menu still opens normally.
- Confirm the set-piece labels remain readable and do not replace the existing EVAC HELIPAD prompt.
- Confirm the jeep, survivor route, and extraction beacon remain visible and usable around the added staging.
