#!/bin/zsh
set -e
setopt NULL_GLOB

if [[ -n "$UE_ENGINE_ROOT" && -d "$UE_ENGINE_ROOT" && -x "$UE_ENGINE_ROOT/Build/BatchFiles/RunUAT.sh" ]]; then
  echo "$UE_ENGINE_ROOT"
  exit 0
fi

candidates=(
  "/Users/Shared/UE_5.7/Engine"
  "/Users/Shared/UE_5.6/Engine"
  "/Users/Shared/UE_5.5/Engine"
  "/Users/Shared/UE_5.4/Engine"
  "/Users/Shared/Epic Games/UE_5.7/Engine"
  "/Applications/Epic Games/UE_5.7/Engine"
  "/Users/Shared/Epic Games"/UE_*/Engine
  "/Users/Shared"/UE_*/Engine
  "/Applications/Epic Games"/UE_*/Engine
  "$HOME/Applications/Epic Games"/UE_*/Engine
)

for engine in $candidates; do
  if [[ -x "$engine/Build/BatchFiles/RunUAT.sh" && -f "$engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll" ]]; then
    echo "$engine"
    exit 0
  fi
done

echo "ERROR: Unreal Engine not found. Set UE_ENGINE_ROOT to your Engine folder, e.g.:" >&2
echo "export UE_ENGINE_ROOT='/Users/Shared/UE_5.7/Engine'" >&2
exit 1
