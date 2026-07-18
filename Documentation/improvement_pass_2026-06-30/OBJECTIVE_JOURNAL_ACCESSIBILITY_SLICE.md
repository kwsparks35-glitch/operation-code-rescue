# Objective Journal Accessibility Slice

Date: 2026-06-30

## Source Guidance

- `UX_OVERHAUL_GUIDE.md`: lists `CodeRescueObjectiveJournalWidget` as part of the theme rollout and calls for clearer, more consistent in-play UI.
- `TOP_50_RECOMMENDATIONS.pdf`: emphasizes objective clarity, text readability, accessibility settings, and reduced player confusion during early play.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: asks for reviewable, package-safe improvements with clear verification evidence.

## Implementation

`UCodeRescueObjectiveJournalWidget` now builds a theme-aware journal panel instead of a bare scroll list. The widget mirrors the saved accessibility state from `UCodeRescueGameInstance`, including:

- High Contrast HUD
- Reduced Motion
- Subtitle/Text scale

The journal now includes:

- a styled `OPERATION ROUTE JOURNAL` title
- an active route summary
- selected language and accessibility summary text
- scalable wrapped mission rows
- explicit text-first state labels: `DONE`, `ACTIVE`, `OPEN`, and `LOCKED`

## Playability Impact

The journal no longer requires the player to decode compact symbols such as `[X]` or `[>]`. Each row now says the route state directly and keeps color as a supporting cue, which makes the 465-mission campaign list more usable during first-session play and later saved-language progression.

The active summary uses saved terminal and survivor progress to report the current phase:

- secure the active coding terminal
- rescue the survivor
- extract and debrief
- campaign complete

## Accessibility

The journal uses `CodeRescueUI` theme tokens for panel styling, text color, type scale, shadows, and high-contrast colors. Reduced Motion lowers the blur strength, while text scale enlarges both the summary and rows through the existing subtitle-scale save field.

## Data / Audit Updates

Updated:

- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `progress.md`

Added:

- `Scripts/verify_objective_journal_accessibility_slice_pass.py`

## Verification

The verifier checks:

- theme mirroring from saved high-contrast, reduced-motion, and text-scale settings
- styled panel/title/summary construction
- active route summary with selected language and accessibility text
- text-first row state labels
- high-contrast row coloring
- scalable wrapped row text
- manifest, onboarding, documentation, progress, and QA script wiring

## Human QA Notes

Open the game in a saved language profile, press `J`, and confirm the journal shows the selected language, current active route phase, readable mission rows, and explicit route-state labels. Toggle High Contrast HUD, Reduced Motion, and UI Text Size in Settings, then reopen the journal to confirm color, blur, and text scale changes are visible.
