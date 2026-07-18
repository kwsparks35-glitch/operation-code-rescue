# Onboarding Input Glyph Slice - 2026-06-30

## Purpose

This slice continues the June 25 creative-development pass by tightening the player's first playable minutes after the language-selection start screen. It maps directly to `TOP_50_RECOMMENDATIONS` items 6 and 7, the `OPERATION_CODE_RESCUE_RELEASE_DOSSIER` playability/readability guidance, and the existing `Content/CodeRescueData/first_ten_minutes_onboarding.tsv` route.

## Implemented Work

- Completed the language start-screen contract by adding the missing `C+` track to `UCodeRescueMainMenuWidget` with both `NEW C+ RUN` and `RESUME C+ SAVE` actions.
- Updated the fallback in-world launch language scene so Java, Python, C, C+, C++, and MATLAB all receive visible `TRACK ONLY` pedestals.
- Expanded `UCodeRescueTutorialWidget` from plain text into a themed onboarding overlay with:
  - a phase strip for the first-ten-minutes path,
  - an active language-specific save line,
  - page-specific input glyph cards,
  - simplified-hints support,
  - shared UI theme styling for contrast, text scale, and button treatment.
- Preserved the start-screen behavior: the launch language screen still appears before active gameplay, and the tutorial only runs after deployment or from the replay option.

## Gameplay Effect

Players can now explicitly choose or resume all six supported coding platforms before play begins, including `C+`. Once in-game, the onboarding tutorial reinforces the selected language save profile and presents the exact keys needed for movement, interaction, validation, combat, saving, route recovery, and mastery without relying only on paragraph text.

## Accessibility And Save Behavior

The tutorial mirrors `bHighContrastHUD`, `bReducedMotion`, `UITextScale`, and `bSimplifiedInputHints` from `UCodeRescueGameInstance`. The active language line uses `GetLanguageName()`, `MakeLanguageSaveSlotName()`, and `DoesLanguageSaveExist()` so players can see whether the current deployment is a fresh language run or a resume-capable language save.
When simplified hints are enabled, the tutorial reduces each page's card row to the core prompts for that step.

## Verification

Added `Scripts/verify_onboarding_input_glyph_slice_pass.py` and tightened `Scripts/verify_launch_language_start_screen_save_pass.py` so both local CI and full QA check the six-track start screen, C+ save slot, tutorial glyph regions, simplified-hints behavior, and documentation trail.
