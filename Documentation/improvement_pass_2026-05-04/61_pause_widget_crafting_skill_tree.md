# #61 — Pause widget Crafting + Skill Tree buttons

The pause menu (P / Esc) had stub `OpenCraftingWidget()` and
`OpenSkillTreeWidget()` helpers on `UCodeRescueGameInstance` from item
#54/#55 but no UI to actually invoke them. Added two new buttons,
following the existing `MakeMenuButton` pattern.

## Files
- `Source/CodeRescueUnreal/CodeRescuePauseWidget.h` — added
  `CraftingButton`, `SkillTreeButton`, `OnCraftingClicked()`,
  `OnSkillTreeClicked()`.
- `Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp` — constructed both
  buttons, bound `OnClicked` delegates, dispatched to GI helpers.

## What the buttons do
- **CRAFTING WORKBENCH** → `GI->OpenCraftingWidget()`. Currently surfaces
  recipe text via on-screen debug; the spend bindings (`CraftFlare`,
  `CraftStim`, `CraftGrenade`) remain BlueprintCallable for a future
  proper widget.
- **SKILL TREE** → `GI->OpenSkillTreeWidget()`. Lists all 8 nodes with
  their unlock status and shows current `ResearchPoints`. Spending
  happens via `TryUnlockSkill(int)`.

## Why
Closing the loop on items #54/#55. Both helpers already exist and are
unit-friendly; surfacing them on the pause menu means the player doesn't
have to know any console commands.
