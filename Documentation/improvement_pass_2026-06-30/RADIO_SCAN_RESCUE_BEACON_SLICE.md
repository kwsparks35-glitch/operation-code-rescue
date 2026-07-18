# Radio Scan Rescue Beacon Slice

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: make objective routes readable through diegetic city signals instead of relying only on HUD text.
- `TOP_50_RECOMMENDATIONS.pdf`: add radio scan and rescue beacon effects that visibly connect coding success to rescue-state changes.
- `CHARACTER_ANIMATION_DEEPDIVE.pdf`: keep survivor identity readable during route transitions and future rescue presentation work.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keep new effects cook-safe, fallback-friendly, documented, and covered by repeatable QA.

## Runtime Implementation

- Extended `AObjectiveFocusBeaconActor` with procedural, cook-safe components: `RadioScanRing`, `RadioSweepArm`, `RescueBeaconHalo`, `RadioPingA`, `RadioPingB`, and `RadioScanLabel`.
- Added mission copy fields for `TerminalTitle`, `MissionConcept`, and `LandmarkName` so the beacon can name the selected-language terminal route, survivor concept, and extraction landmark.
- Updated the phase label flow from generic objective text into explicit `RADIO SCAN`, `SURVIVOR PING`, and `RESCUE BEACON` states.
- Added `BuildRadioScanLine()` to produce text-first labels using the selected language, terminal title, survivor name, curriculum concept, and landmark.
- Kept all effect pieces nonblocking and built from engine primitives, so the slice does not depend on external Niagara or art assets.

## Player Experience

- Before terminal solve, the beacon reads as a radio scan for the active selected-language terminal route.
- After terminal solve, the same beacon becomes a survivor ping and shows the named survivor plus mission concept.
- After survivor rescue, the extraction phase shows a rescue beacon halo and labels the extraction as live at the city landmark.
- The beacon still tracks the same objective phases and save-backed terminal/survivor state as the existing objective-focus system.

## Accessibility

- Reduced Motion slows the scan sweep, ring motion, ping cadence, and drone-style rotation already controlled by `bReducedMotion`.
- The feature is text-first: color and motion reinforce the state, while `RADIO SCAN`, `SURVIVOR PING`, and `RESCUE BEACON` remain readable labels.
- The components remain nonblocking, so the effect does not create traversal or combat collision hazards.

## Data And QA Records

- Added `Content/CodeRescueData/radio_scan_rescue_beacon_manifest.tsv` to document phase triggers, visual components, label copy, and accessibility behavior.
- Updated curriculum feedback, onboarding, visual regression, human QA, creative inclusion, and accessibility manifests.
- Added `Scripts/verify_radio_scan_rescue_beacon_slice_pass.py` and wired it into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.

## Validation

- Static verifier: `python3 Scripts/verify_radio_scan_rescue_beacon_slice_pass.py`
- Regression verifier: `python3 Scripts/verify_objective_focus_beacon_slice_pass.py`
- Compile gate: `./Recompile_Module.command < /dev/null`
- Package gate: `./Package_Mac_App.command < /dev/null`
- Runtime smoke gates: `./Smoke_Test_Packaged_App.command null` and `./Smoke_Test_Packaged_App.command render`
- Runtime log gate: `python3 Scripts/verify_runtime_log_contracts.py Saved/Logs/PackagedSmoke_null.log` and render log equivalent.

## Human QA Notes

- Start a selected-language run and confirm the first objective beacon reads as `RADIO SCAN` for the active terminal route.
- Solve the terminal and confirm the beacon changes to `SURVIVOR PING`, naming the survivor and lesson concept.
- Rescue the survivor and confirm the extraction phase shows `RESCUE BEACON`, the halo appears, and the landmark/extraction copy is readable.
- Enable Reduced Motion and confirm the phase labels remain clear while scan/sweep motion is calmer.
