#!/bin/zsh
# run_v5_perspective_review.sh — pass-5 self-driving visual review:
# all camera perspectives, the ADS zoom ladder, the grenade arc + detonation,
# each screenshotted to Saved/Screenshots/FirstLevel/review_*.png, then exits.
PROJ="/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
exec /Users/Shared/UE_5.7/Engine/Binaries/Mac/UnrealEditor \
  "$PROJ/CodeRescueUnreal.uproject" \
  -game -windowed -ResX=1280 -ResY=720 \
  -NoRadioVoice -NoSound -Unattended -NoSplash \
  -VisualReviewStart -CodeRescuePerspectiveReview \
  -ABSLOG="$PROJ/Saved/Logs/PerspectiveReviewV5.log"
