# Pause Difficulty Matrix Slice - 2026-06-30

## Purpose

This slice continues the June 25 implementation pass by making difficulty selection understandable in play instead of leaving it as a terse cycle button. It addresses `TOP_50_RECOMMENDATIONS` item 47, supports the `OPERATION_CODE_RESCUE_RELEASE_DOSSIER` first-session readiness goals, and turns `Content/CodeRescueData/difficulty_presets.tsv` plus `first_ten_minutes_onboarding.tsv` into visible player guidance.

## Implemented Work

- Expanded `UCodeRescuePauseWidget` with a `DifficultyDetailText` readout below the difficulty button.
- The pause menu now shows the full preset order: Story -> Easy -> Normal -> Hard -> Survival -> Nightmare.
- Each selected preset displays zombie health and damage multipliers, player intent, and first-ten-minutes expectation.
- The pause menu button labels now group commands by purpose: `RUN`, `SAVE`, `BALANCE`, `OPTIONS`, `LEARNING`, `LOADOUT`, and `EXIT`.
- The widget mirrors high contrast, reduced motion, and text scale before styling itself.
- Difficulty is saved immediately through `GI->SavePersistentRun()` and keeps the existing cycle behavior.

## Gameplay Effect

Players can now make an informed difficulty choice without leaving the game or reading external documentation. Story explicitly communicates its learning-first intent, Nightmare clearly warns repeat players about combat danger, and every preset explains how it should feel during the first route from orientation to terminal, survivor, save, and extraction.

## Verification

Added `Scripts/verify_pause_difficulty_matrix_slice_pass.py`, wired into full QA and local CI. The verifier checks the menu grouping labels, the six preset multiplier table, Story-to-Nightmare copy, save behavior, first-ten-minutes guidance, and this documentation trail.
