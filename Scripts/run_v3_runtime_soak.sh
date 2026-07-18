#!/bin/zsh
# run_v3_runtime_soak.sh — editor -game resume soak for the 2026-07-11 art+physics v3 pass.
# Resumes the real Cpp save (same harness as the .200 crash-fix verification) and
# writes an absolute log for marker auditing. Kill with: pkill -f CodeRescueAutoResume
PROJ="/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
exec /Users/Shared/UE_5.7/Engine/Binaries/Mac/UnrealEditor \
  "$PROJ/CodeRescueUnreal.uproject" \
  -game -windowed -ResX=1280 -ResY=720 \
  -CodeRescueAutoResumeLanguage=Cpp \
  -ABSLOG="$PROJ/Saved/Logs/ArtPhysicsV3Runtime_2026_07_11.log"
