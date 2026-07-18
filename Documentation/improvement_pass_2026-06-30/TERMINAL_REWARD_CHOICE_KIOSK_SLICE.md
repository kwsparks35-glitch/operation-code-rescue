# Terminal Reward Choice Kiosk Slice

This pass implements the campaign flow-plan promise for a `reward choice kiosk`. The terminal now gives the player a one-time, save-backed reward selection after a live selected-language solve, while keeping practice runs and bypass-kit assists from unlocking the reward kiosk.

## Player-Facing Changes

- Added a terminal reward row with three explicit choices:
  - `REWARD: RESEARCH +2 RP`
  - `REWARD: FIELD KIT`
  - `REWARD: CRAFTING CACHE`
- Live `VALIDATE CODE` success unlocks the `REWARD CHOICE KIOSK` for that terminal.
- Practice runs do not unlock the reward kiosk.
- Bypass-kit solves do not unlock the reward kiosk.
- Reopening a solved terminal shows whether the kiosk is pending, already claimed, or unavailable.
- Reward choice state persists in the selected-language save and survives start-screen Resume.

## Reward Choices

- `Research Boost`: grants `+2 ResearchPoints`.
- `Field Kit`: grants ammo, one medkit, and one armor plate.
- `Crafting Cache`: grants scrap and one bypass kit.

Each terminal can claim exactly one reward choice. After the choice is claimed, the terminal disables all reward buttons and saves the claim to the active language profile.

## Implementation

Runtime work is split between `UCodeTerminalWidget`, `UCodeRescueGameInstance`, and `UCodeRescueSaveGame`.

- `UCodeTerminalWidget` creates `RewardChoiceResearchButton`, `RewardChoiceFieldKitButton`, and `RewardChoiceCraftingButton`.
- `UCodeTerminalWidget::RunValidation(false)` calls `MarkTerminalRewardChoiceEligible()` after a live successful validation.
- `UCodeTerminalWidget::RunValidation(true)` does not mark reward eligibility.
- `OnBypassClicked()` does not mark reward eligibility.
- `UCodeRescueGameInstance::ClaimTerminalRewardChoice()` grants the selected reward, records the claimed terminal ID, writes `LastTerminalRewardChoiceSummary`, and saves the language profile.
- `UCodeRescueGameInstance::GetTerminalRewardChoiceSummary()` provides pending, claimed, or unavailable text for solved-terminal reopen/resume states.
- `UCodeRescueSaveGame` persists `RewardChoiceEligibleTerminalIds`, `ClaimedTerminalRewardChoiceIds`, and last reward choice summary fields.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/terminal_reward_choice_kiosk_manifest.tsv`.
- Updated curriculum feedback, selected-language terminal flow, accessibility, visual regression, human QA, first-ten-minutes, creative inclusion, and implementation-ledger records.
- Wired `Scripts/verify_terminal_reward_choice_kiosk_slice_pass.py` into full QA and local CI readiness.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_terminal_reward_choice_kiosk_slice_pass.py`
- `python3 Scripts/verify_terminal_reward_choice_kiosk_slice_pass.py`
- `python3 Scripts/verify_terminal_practice_run_slice_pass.py`
- `python3 Scripts/verify_terminal_post_solve_debrief_slice_pass.py`
- `python3 Scripts/verify_selected_language_terminal_flow_slice_pass.py`
- `python3 Scripts/verify_launch_language_start_screen_save_pass.py`
- `python3 Scripts/verify_creative_development_implementation_ledger.py`
- module recompile
- scoped `git diff --check`

## Human QA Notes

Live-solve a terminal, close the terminal before claiming, save/quit, resume from the start screen, reopen the solved terminal, and claim one reward. Confirm the choice persists, the reward cannot be claimed twice, practice run output never unlocks the kiosk, and bypass-kit solves keep the kiosk unavailable.
