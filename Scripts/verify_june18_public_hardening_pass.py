#!/usr/bin/env python3
"""Static verifier for the June 18, 2026 public-hardening pass."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


material_h = read(SRC / "CodeRescueMaterialUtils.h")
material_cpp = read(SRC / "CodeRescueMaterialUtils.cpp")
game_mode = read(SRC / "CodeRescueGameMode.cpp")
spawning_cpp = read(SRC / "CodeRescueGameModeSpawning.cpp")
runner_h = read(SRC / "CodeRunnerLibrary.h")
runner_cpp = read(SRC / "CodeRunnerLibrary.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
packaged_smoke = read(PROJECT_ROOT / "Smoke_Test_Packaged_App.command")
runtime_contracts = read(PROJECT_ROOT / "Scripts/verify_runtime_log_contracts.py")
maple_readme = read(PROJECT_ROOT / "Tools/MapleVoice/README.md")
maple_doc = read(PROJECT_ROOT / "Documentation/MAPLE_NARRATION_STATUS_2026-06-18.md")
gitattributes = read(PROJECT_ROOT / ".gitattributes")
gitignore = read(PROJECT_ROOT / ".gitignore")
source_control_doc = read(PROJECT_ROOT / "Documentation/SOURCE_CONTROL_HANDOFF_2026-06-18.md")
distribution_doc = read(PROJECT_ROOT / "Documentation/DISTRIBUTION_GUIDE_MAC.md")

# 1. Dynamic material warning fix.
check("ResolveDynamicMaterialParent" in material_h and "ResolveDynamicMaterialParent" in material_cpp,
      "CodeRescueMaterialUtils must expose ResolveDynamicMaterialParent")
check("UMaterialInstanceDynamic" in material_cpp and "Dynamic->Parent" in material_cpp,
      "material utility must unwrap dynamic material parents before creating another MID")
check("CreateTintedDynamicMaterial" in material_cpp and "ApplyTintedMaterial" in material_cpp,
      "material utility must centralize tinted dynamic material creation/application")
check("CodeRescueMaterials::CreateTintedDynamicMaterial" in game_mode,
      "CodeRescueGameMode material overrides must use the shared material utility")
check("LogMaterial: Warning" in runtime_contracts and "MID_MID_" in runtime_contracts,
      "runtime log verifier must forbid dynamic-material parent warnings")

# 2. Source-control hygiene.
check("*.uasset filter=lfs diff=lfs merge=lfs -text" in gitattributes,
      ".gitattributes must route Unreal assets through Git LFS")
check("*.umap filter=lfs diff=lfs merge=lfs -text" in gitattributes,
      ".gitattributes must route Unreal maps through Git LFS")
check("__pycache__/" in gitignore and ".venv/" in gitignore,
      ".gitignore must cover Python caches and local venvs")
check("git lfs migrate info" in source_control_doc,
      "source-control handoff must document LFS migration/review guidance")

# 3. Runtime/visual assertions.
for marker in (
    "[CodeRescueArenaConfinement]",
    "[CodeRescueUSCityIdentity]",
    "[CodeRescueUnrealSystems]",
    "[CodeRescuePublicDemoQuality]",
    "[CodeRescueSafeLearning]",
    "[CodeRescueCreativeImplementation]",
    "[CodeRescueEntryAccess]",
    "signature='harbor statue silhouette and dense island skyline'",
    # 2026-07-17 pin migration: pass-6 made Backspace the single recovery key
    "Backspace recovery guidance",
):
    check(marker in runtime_contracts, f"runtime log verifier must assert {marker}")
check("verify_runtime_log_contracts.py" in full_qa,
      "full QA must run runtime log contracts after smoke log generation")
check("verify_runtime_log_contracts.py" in packaged_smoke,
      "packaged smoke must run runtime log contracts")

# 4. Maple status wording.
check("coverage is counted" in maple_readme.lower() and "live" in maple_readme.lower() and "fallback" in maple_readme.lower(),
      "Maple README must describe live cue coverage and fallback behavior")
check("coverage is intentionally counted live" in maple_doc.lower(),
      "Maple status doc must describe live coverage verification")

# 5. External code execution safety gate.
check("CodeRescue.AllowExternalCodeValidation" in runner_cpp,
      "CodeRunnerLibrary must define the external validator safety cvar")
check("AllowExternalCodeValidation" in runner_cpp and "FParse::Param" in runner_cpp,
      "CodeRunnerLibrary must support an explicit trusted-QA command-line opt-in")
check("AreExternalValidatorsAllowed" in runner_h and "AreExternalValidatorsAllowed" in runner_cpp,
      "CodeRunnerLibrary must expose AreExternalValidatorsAllowed")
check("ExternalValidationDisabledMessage" in runner_cpp,
      "CodeRunnerLibrary must provide a user-facing disabled-validation message")
check("return false;" in runner_cpp.split("bool UCodeRunnerLibrary::LaunchMATLABDesktop")[1],
      "LaunchMATLABDesktop must be blocked while external validators are disabled")
check("CodeRescue.AllowExternalCodeValidation=0" in distribution_doc,
      "distribution guide must document the default external-validation safety gate")
check("-AllowExternalCodeValidation" in distribution_doc and "-AllowExternalCodeValidation" in full_qa,
      "trusted local QA opt-in flag must be documented and used only by the full-QA validator commandlet")

# 6. GameMode split.
check("ACodeRescueGameMode::SpawnBlock" in spawning_cpp,
      "CodeRescueGameModeSpawning.cpp must own SpawnBlock")
check("ACodeRescueGameMode::SpawnDecorativeCivilian" in spawning_cpp,
      "CodeRescueGameModeSpawning.cpp must own decorative civilian assembly")
check("ACodeRescueGameMode::SpawnBlock" not in game_mode,
      "CodeRescueGameMode.cpp should not retain moved SpawnBlock implementation")

# Self-registration.
check("verify_june18_public_hardening_pass.py" in full_qa,
      "June 18 static verifier must be registered in Run_Full_QA_Audit.command")

if errors:
    for error in errors:
        print(f"[verify_june18_public_hardening_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_june18_public_hardening_pass] PASS: June 18 public-hardening contract intact")
