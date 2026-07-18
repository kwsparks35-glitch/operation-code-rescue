# Fast Travel Evac Route Readability Slice

Date: 2026-06-30

## Source Guidance

- `UX_OVERHAUL_GUIDE.md`: identifies `CityFastTravelWidget` as one of the remaining UMG surfaces needing the shared visual language.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: calls for clearer authored rescue spaces and extraction destinations.
- `TOP_50_RECOMMENDATIONS.pdf`: emphasizes stronger post-objective feedback and clear next-step flow after major player actions.

## Implementation

- Rebuilt `UCityFastTravelWidget` around the shared `CodeRescueUI` theme with high-contrast, reduced-motion, and saved text-scale support.
- Added a centered `EvacRoutePanel` with reduced blur, themed title, extraction debrief text, and route summary.
- Added `BuildFastTravelSummary()` so the overlay shows the active coding language, language resume slot, cleared terminal count, rescue count, route mode, and post-teleport save behavior.
- Restyled the `Continue operation` action as a `NEXT OPERATION` button that names the next mission and keeps the selected language route explicit.
- Replaced terse destination labels with redeploy rows that explain city completion state, curriculum focus, and that arrival saves the active language run.
- Preserved the existing `ButtonToCityIndex` dispatch map, helipad context wiring, continue action, and post-teleport `SavePersistentRun()` behavior.

## Player Impact

- The helipad menu now reads as an evacuation debrief and route-control screen instead of a generic list of buttons.
- Players can see which language run will be saved before choosing the next operation or redeploying to a cleared city.
- Destination rows communicate why a city is available and whether its survivor has also been extracted.

## Files Changed

- `Source/CodeRescueUnreal/CityFastTravelWidget.h`
- `Source/CodeRescueUnreal/CityFastTravelWidget.cpp`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Scripts/verify_fast_travel_evac_route_readability_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_fast_travel_evac_route_readability_slice_pass.py`
- Adjacent verifier: `python3 Scripts/verify_extraction_debrief_fast_travel_slice_pass.py`
- Compile/package/smoke should be run because this changes a runtime UMG widget.

## Human QA Notes

- Rescue a survivor, interact with the extraction-ready helipad, and confirm the panel title reads `EVAC HELI - extraction debrief`.
- Confirm the route summary names the active language, start-screen resume slot, cleared terminals, and rescued teams.
- Confirm the `NEXT OPERATION` action names the next incomplete city.
- Confirm solved cities appear as `REDEPLOY` rows with completion state and curriculum focus.
- Toggle High Contrast HUD, Subtitle/Text size, and Reduced Motion, then reopen the helipad menu and confirm the overlay remains readable.
