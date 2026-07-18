#!/bin/zsh
# run_v4_integrated_test.sh — SINGLE integrated test run for the 2026-07-11
# pass-4 work (hero body, doors, wind, ground unification) layered on pass 3
# (CharactersV3 horde, WeaponsV4, authored-body ragdoll).
# Runs the game's own first-level integrated acceptance harness.
PROJ="/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
exec /Users/Shared/UE_5.7/Engine/Binaries/Mac/UnrealEditor \
  "$PROJ/CodeRescueUnreal.uproject" \
  -game -windowed -ResX=1280 -ResY=720 \
  -NoRadioVoice -NoSound -Unattended -NoSplash \
  -VisualReviewStart -FirstLevelIntegratedAcceptanceAudit \
  -ABSLOG="$PROJ/Saved/Logs/IntegratedV4_2026_07_11.log"
