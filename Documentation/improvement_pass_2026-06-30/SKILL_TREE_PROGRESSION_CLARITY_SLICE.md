# Skill Tree Progression Clarity Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS_2026-06-25.md`, recommendation 46: make skill tree, crafting, and research legible and rewarding, with mastery and streaks prominent.
- `UX_OVERHAUL_GUIDE.md`: finish the shared UI theme rollout for `CodeRescueSkillTreeWidget`.

## Implementation

- Rebuilt the runtime-authored skill tree panel around the shared `CodeRescueUI` theme, including saved high-contrast, reduced-motion, and text-scale settings.
- Added a progression header that shows ResearchPoints, unlocked skill count, active coding language, language practice summary, learning mastery/streak summary, and active save slot.
- Converted skill rows from opaque buttons into state-labeled choices: `UNLOCKED`, `READY - spend 2 RP`, or `LOCKED - need N more RP`.
- Added category and outcome text for every node so the player can see the gameplay effect before spending RP.
- Kept unaffordable nodes clickable so the feedback line can explain exactly how many ResearchPoints are still needed.
- Preserved existing persistence behavior through `TryUnlockSkill()`, which applies the skill, saves the active language run, and refreshes the panel immediately.

## Player Impact

- The pause-menu skill tree now communicates what the player has earned, what they can buy, and how far they are from the next upgrade.
- Unlock decisions are tied directly to the selected coding language save, preventing confusion about cross-language progression.
- Mastery, streak, and language-track context remain visible at the moment the player spends ResearchPoints.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueSkillTreeWidget.h`
- `Source/CodeRescueUnreal/CodeRescueSkillTreeWidget.cpp`
- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Scripts/verify_skill_tree_progression_clarity_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_skill_tree_progression_clarity_slice_pass.py`
- Compile check: `./Recompile_Module.command < /dev/null`
- Packaging and packaged smoke should be re-run after this slice alongside the adjacent UI verifiers.

## Human QA Notes

- Open the game, select a language from the launch screen, pause, and open `LOADOUT: SKILL TREE`.
- Confirm the panel shows the active language run, ResearchPoints, skill count, mastery/streak line, and save slot.
- With fewer than 2 RP, click a locked node and confirm the feedback line states how many more RP are needed.
- With 2 or more RP, unlock a node and confirm the skill changes to `UNLOCKED`, RP drops by 2, and the language run saves.
- Toggle High Contrast HUD, Subtitle/Text size, and Reduced Motion in Settings, then reopen the skill tree and confirm the panel remains readable.
