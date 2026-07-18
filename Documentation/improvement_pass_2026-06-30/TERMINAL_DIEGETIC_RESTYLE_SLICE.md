# Terminal Diegetic Restyle Slice

Date: 2026-06-30

## Source Guidance

- `UX_OVERHAUL_GUIDE.md`: calls out `CodeTerminalWidget` as the coding surface that should use phosphor/terminal styling, readable errors, and shared theme rules.
- `TOP_50_RECOMMENDATIONS.pdf`: recommends a diegetic terminal restyle, stronger onboarding clarity, and UI/accessibility consistency.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasizes package-safe, reviewable improvements that preserve playability.

## Implementation

`UCodeTerminalWidget` now stores and refreshes terminal chrome instead of building a one-pass vertical form. Added:

- `TerminalStatusText`
- `PanelFrame`
- `CodeEditorFrame`
- `OutputFrame`
- `DiagnosticsHeaderText`
- `SetDiagnosticsState()`

The coding surface now includes a status strip, framed code editor, framed diagnostics pane, and grouped action rows. The status strip reports the selected language track, solved/active state, combat-paused safehouse state, and external-toolchain versus in-engine validator mode.

## Accessibility

The terminal continues to mirror saved `UCodeRescueGameInstance` accessibility settings into `CodeRescueUI::Theme()`. This slice adds high-contrast-specific terminal/editor fills, reduced blur behavior already present in the overlay, and refreshed scalable monospace code styling.

## Playability Impact

The player now sees the coding interaction as an in-world safehouse tool:

- selected language and save profile remain visible
- compiler/fallback state is explicit
- code input and diagnostics are visually separated
- validation state changes to `READY`, `PASS`, `REPAIR`, `LIMIT`, `SOLVED`, or `HINTS`
- action buttons are grouped for faster scanning

## Data / Audit Updates

Updated:

- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `progress.md`

Added:

- `Scripts/verify_terminal_diegetic_restyle_slice_pass.py`

## Verification

The verifier checks:

- terminal chrome member storage
- horizontal action-row construction
- framed code editor and diagnostics panel construction
- high-contrast terminal/editor fills
- live toolchain state text
- diagnostics state changes on reset, empty code, length limit, validation pass/fail, hints, and MATLAB launch
- manifest, documentation, progress, and QA wiring

## Human QA Notes

Open a terminal from a selected language profile and confirm the panel shows the chosen track, save profile, compiler/fallback status, framed code editor, and diagnostics header. Validate an empty submission, a failing edit, and a passing solution; confirm diagnostics text and color change clearly in standard and High Contrast HUD modes.
