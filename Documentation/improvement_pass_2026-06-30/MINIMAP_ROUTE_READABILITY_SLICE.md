# Minimap Route Readability Slice

Date: 2026-06-30

## Source Guidance

- `UX_OVERHAUL_GUIDE.md`: identifies `CodeRescueMinimapWidget` as a remaining HUD surface needing the shared visual language.
- `TOP_50_RECOMMENDATIONS.pdf`: calls for objective markers and route guidance tied to the minimap and memory work.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: emphasizes readable authored rescue spaces and clear player orientation across the city route.

## Implementation

- Rebuilt `UCodeRescueMinimapWidget` from a raw dot canvas into a themed `NAV MAP` instrument using the shared `CodeRescueUI` visual system.
- Added `MinimapThemedPanel`, `MinimapSummary`, `MinimapRouteCue`, and `MinimapLegend` so the minimap now exposes the active coding language, scan counts, nearest route objective, and category meaning.
- Added `MapAreaSizePx` so the radar plot remains stable inside the existing 220 x 220 HUD footprint while preserving room for the title, summary, cue, and legend.
- Mirrored saved high-contrast, reduced-motion, and subtitle/text-scale settings into the minimap each refresh.
- Added high-contrast category dot colors and size-coded dots so terminals, survivors, language markers, aid pickups, threats, and the player are distinguishable by more than color alone.
- Added a reduced-motion refresh cadence so the minimap updates at a calmer rate when reduced motion is enabled without changing the HUD mount point.
- Added `MakeMinimapSummary()` and `MakeRouteCue()` helpers so the player gets text-first route guidance such as `Nearest CODE: 87m NE`.

## Player Impact

- The minimap now reads as a route scanner instead of an unlabeled cluster of dots.
- Players can confirm which coding language run is active while navigating, reinforcing the start-screen language selection contract during play.
- The nearest code, rescue, or language objective is named with distance and cardinal direction, reducing reliance on color scanning alone.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueMinimapWidget.h`
- `Source/CodeRescueUnreal/CodeRescueMinimapWidget.cpp`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Scripts/verify_minimap_route_readability_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_minimap_route_readability_slice_pass.py`
- Adjacent verifier: `python3 Scripts/verify_hud_vitals_theme_accessibility_slice_pass.py`
- Compile/package/smoke should be run because this changes a runtime HUD widget.

## Human QA Notes

- Start a run and confirm the top-right HUD widget is titled `NAV MAP`.
- Confirm the summary names the active coding language and visible terminal/survivor/language/threat counts.
- Confirm the route cue names the nearest `CODE`, `RESCUE`, or `LANG` objective with distance and direction.
- Confirm the legend explains `P`, `T`, `S`, `L`, and `!` markers.
- Toggle High Contrast HUD, Subtitle/Text size, and Reduced Motion, then confirm the minimap remains readable and stable.
