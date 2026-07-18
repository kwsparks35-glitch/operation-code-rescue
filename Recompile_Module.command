#!/bin/zsh
# Builds the CodeRescueUnreal editor module via UBT. Invoke this when the
# editor is open and reports out-of-date C++ (or after Source/ edits) and
# you want a clean compile without the diagnostic noise of the older
# UE57_Source_Compile_Fix_And_Diagnose.command. Editor SHOULD be closed
# before running this script (UBT requires the editor's binaries not to be
# locked); the script will warn-and-continue if it isn't, since UE handles
# the in-process Live Coding case differently.

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

ENGINE_ROOT="$("$SCRIPT_DIR/Scripts/find_unreal_mac.sh")"
UPROJECT="$SCRIPT_DIR/CodeRescueUnreal.uproject"

if [[ ! -f "$UPROJECT" ]]; then
  echo "ERROR: CodeRescueUnreal.uproject not found at $UPROJECT"
  exit 1
fi

if pgrep -f "UnrealEditor" >/dev/null 2>&1; then
  echo "WARNING: Unreal Editor appears to be running."
  echo "          This build may fail if UBT can't write the editor dylib."
  echo "          If you hit a 'file is busy' error, quit the editor and rerun."
  echo ""
fi

BUILD="$ENGINE_ROOT/Build/BatchFiles/Mac/Build.sh"
if [[ ! -x "$BUILD" ]]; then
  echo "ERROR: UBT build script not found at $BUILD"
  exit 1
fi

echo "Engine: $ENGINE_ROOT"
echo "Target: CodeRescueUnrealEditor Mac Development"
echo "Project: $UPROJECT"
echo ""
echo "Compiling… (this may take 5–10 minutes on first run, faster on incremental)"
echo ""

"$BUILD" CodeRescueUnrealEditor Mac Development -Project="$UPROJECT" -WaitMutex

echo ""
echo "============================================================"
echo " BUILD SUCCEEDED. You can now reopen / refocus the editor"
echo " and rerun:"
echo "   exec(open(r\"$SCRIPT_DIR/Scripts/build_zombie_variants_table.py\").read())"
echo " in the editor's Output Log → Python console."
echo "============================================================"
if [[ -t 0 ]]; then
  read "?Press Return to close..."
fi
