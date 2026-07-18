# Health Damage Survivability Slice

Date: 2026-06-30

## Purpose

This slice closes the P0 `health gauge and non-instant zombie damage` row from the creative-development plan. It connects the existing HUD vitals, damage overlay, combat safety settings, armor, emergency medkit, and death replay UI into one reviewable survivability contract.

## What Changed

`ACodeRescueCharacter::ApplyDamage` now records `LastDamageMitigationText` for the latest hit. The text names active safety systems such as:

- `mercy window`
- `per-hit cap`
- `armor plate`
- `survival lock`
- `emergency medkit`

`UCodeRescueHUDWidget::RefreshHUD` appends that mitigation text to the existing `ATTACKED FROM` alert, so players can understand why an ordinary enemy hit reduced damage, why armor was consumed, why a lethal hit left them alive, or why an emergency medkit fired.

## Existing Runtime Contract

The survivability model now has explicit validation around:

- visible `PLAYER HEALTH` numeric and percentage readout;
- STABLE/LOW/CRITICAL health labels;
- high-contrast health/stamina colors;
- directional source/distance damage alert;
- mercy-window damage reduction;
- per-hit enemy damage cap;
- armor plate damage reduction and consumption;
- single-hit survival lock from healthy states;
- emergency medkit recovery and subtitle;
- death screen resume/save/quit choices.

## Review Hooks

New review files:

- `Content/CodeRescueData/health_damage_survivability_manifest.tsv`
- `Scripts/verify_health_damage_survivability_slice_pass.py`

Updated review surfaces:

- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

## Validation

Run:

```zsh
python3 Scripts/verify_health_damage_survivability_slice_pass.py
python3 Scripts/verify_june01_rescue_survivability_pass.py
python3 Scripts/verify_hud_vitals_theme_accessibility_slice_pass.py
python3 Scripts/verify_damage_feedback_accessibility_slice_pass.py
./Recompile_Module.command < /dev/null
./Package_Mac_App.command < /dev/null
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```
