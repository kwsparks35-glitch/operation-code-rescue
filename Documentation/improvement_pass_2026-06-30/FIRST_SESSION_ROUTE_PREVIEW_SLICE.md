# First Session Route Preview Slice

This pass tightens the launch language start screen so the player sees what the first selected-language deployment means before active gameplay begins. It extends the P0 `selected language terminal flow` inclusion with a text-first `FIRST-SESSION ROUTE PREVIEW` that explains route shape, save slot behavior, and resume continuity.

## Player-Facing Changes

- The launch-only start screen now includes `FirstSessionRoutePreviewText` before the language rows.
- The preview appears before gameplay and names:
  - the current/default track-only profile,
  - the language-specific save slot,
  - whether a resume save exists,
  - the first city route,
  - the protected terminal -> survivor marker -> extraction flow,
  - the terminal, curriculum stage, focus, landmark, survivor contact, and tuning band.
- The helper copy still says that this screen appears every time the game starts.
- The helper copy clarifies that the preview uses the current/default profile and that each `NEW` or `RESUME` row deploys only the clicked language.
- The same preview block now appends `LANGUAGE SAVE ROSTER`, listing Java, C, C+, C++, Python, and MATLAB as `RESUME AVAILABLE` or `NEW RUN READY` before active play.
- The 3D launch fallback scene now mirrors the route preview phrase so the packaged start space remains readable if the widget path is unavailable.

## Implementation

Runtime work is split between `UCodeRescueGameInstance`, `UCodeRescueMainMenuWidget`, and `ACodeRescueGameMode`.

- `UCodeRescueGameInstance::GetFirstSessionRoutePreviewSummary(ECodingLanguage Language)` builds the reusable summary.
- The summary uses `MakeLanguageSaveSlotName`, `DoesLanguageSaveExist`, and `GetLanguageSaveSummary` so save/resume copy is tied to the requested language profile.
- The summary reads the first campaign mission from `FCodeRescueCampaign::GetMissions()` and reports first city, terminal, curriculum stage/focus, landmark, survivor contact, and beginner/normal/challenge tuning band.
- `UCodeRescueGameInstance::GetLaunchLanguageSaveRosterSummary()` builds the all-language save roster from the same language-specific save existence checks used by the resume buttons.
- `UCodeRescueMainMenuWidget` creates `FirstSessionRoutePreviewText` only in launch-language mode and refreshes it alongside the language selection copy.
- `ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene()` adds a fallback world prompt with `FIRST-SESSION ROUTE PREVIEW` and `protected terminal -> survivor marker -> extraction`.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/first_session_route_preview_manifest.tsv`.
- Updated selected-language terminal flow, creative inclusion, accessibility, onboarding, visual regression, human QA, and implementation-ledger records.
- Wired `Scripts/verify_first_session_route_preview_slice_pass.py` into full QA and local CI readiness.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_first_session_route_preview_slice_pass.py`
- `python3 Scripts/verify_first_session_route_preview_slice_pass.py`
- `python3 Scripts/verify_launch_language_start_screen_save_pass.py`
- `python3 Scripts/verify_selected_language_terminal_flow_slice_pass.py`
- `python3 Scripts/verify_creative_development_implementation_ledger.py`
- module recompile
- scoped `git diff --check`

## Human QA Notes

Launch the game cold, inspect the start screen before selecting a language, then create and resume at least two different language saves. Confirm `FIRST-SESSION ROUTE PREVIEW` appears before gameplay, the route reads as protected terminal -> survivor marker -> extraction, all language rows remain selectable, and Resume returns only to the selected language save.
