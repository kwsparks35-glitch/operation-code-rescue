#!/bin/zsh
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
ENGINE_ROOT="$("$SCRIPT_DIR/Scripts/find_unreal_mac.sh")"
UPROJECT="$SCRIPT_DIR/CodeRescueUnreal.uproject"
ARCHIVE_DIR="$SCRIPT_DIR/PackagedMac"

if [[ ! -f "$UPROJECT" ]]; then
  echo "ERROR: CodeRescueUnreal.uproject was not found beside this launcher."
  exit 1
fi

rm -rf "$ARCHIVE_DIR"
mkdir -p "$ARCHIVE_DIR"
"$ENGINE_ROOT/Build/BatchFiles/RunUAT.sh" BuildCookRun \
  -project="$UPROJECT" \
  -platform=Mac \
  -clientconfig=Development \
  -build -cook -stage -pak -archive -nop4 \
  -archivedirectory="$ARCHIVE_DIR" \
  -utf8output

STAGED_APP="$SCRIPT_DIR/Saved/StagedBuilds/Mac/CodeRescueUnreal.app"
ARCHIVED_APP="$ARCHIVE_DIR/Mac/CodeRescueUnreal.app"
if [[ -d "$STAGED_APP/Contents/UE" ]]; then
  rm -rf "$ARCHIVED_APP"
  mkdir -p "$(dirname "$ARCHIVED_APP")"
  cp -R "$STAGED_APP" "$ARCHIVED_APP"
fi

APP_PATH="$(find "$ARCHIVE_DIR" -maxdepth 5 -type d -name '*.app' -print -quit)"
if [[ -n "$APP_PATH" ]]; then
  if [[ ! -d "$APP_PATH/Contents/UE" ]]; then
    echo "ERROR: Packaged app is missing cooked UE staged data at Contents/UE."
    exit 1
  fi
  echo "Packaged Mac app:"
  echo "$APP_PATH"
else
  echo "Packaging finished, but no .app bundle was found under $ARCHIVE_DIR."
  exit 1
fi
