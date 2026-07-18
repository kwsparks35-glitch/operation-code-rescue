#!/usr/bin/env python3
"""Static verifier for the hostile threat compass HUD slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing body for {signature}")
        return ""
    depth = 0
    for idx in range(brace, len(source)):
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                _cr_body = source[brace : idx + 1]  # 2026-07-04 BuildWidgetTreeNow migration
                if "::NativeConstruct" in signature and "BuildWidgetTreeNow();" in _cr_body:
                    return function_body(source, signature.replace("::NativeConstruct", "::BuildWidgetTreeNow"))
                return _cr_body
    errors.append(f"unterminated function {signature}")
    return ""


hud_h = read(SRC / "CodeRescueHUDWidget.h")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/enemy_readability_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "THREAT_COMPASS_HUD_SLICE.md")

construct_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")
refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
threat_body = function_body(hud_cpp, "FCodeRescueThreatHudInfo GetNearestHudThreat")
urgency_body = function_body(hud_cpp, "void FinalizeThreatHudInfo")
variant_body = function_body(hud_cpp, "FString ZombieVariantHudLabel")
role_body = function_body(hud_cpp, "FString ZombieRoleHudLabel")

check_all(
    hud_h,
    [
        "ThreatCompassText",
        "NavigationStripText",
        "TacticalReadoutText",
    ],
    "HUD header must declare the new threat compass text field",
)
check_all(
    hud_cpp,
    [
        "struct FCodeRescueThreatHudInfo",
        "bIsBoss",
        "bIsElite",
        "UrgencyLabel",
        "ZombieVariantHudLabel",
        "ZombieRoleHudLabel",
        "IsEliteZombieVariant",
        "GetNearestHudThreat",
    ],
    "HUD implementation must define structured nearest-threat metadata",
)
check_all(
    variant_body,
    [
        "DogZombie",
        "UrbanZombie4",
        "BusinessSuit",
        "BloatedFemale",
        "NurseFemale",
        "BaseMesh",
        "EliteSpitter",
        "EliteCharger",
        "EliteBoomer",
    ],
    "variant HUD label helper must cover imported and elite threats",
)
check_all(
    role_body,
    ["Anchor", "Flanker", "Pressure", "Sentinel"],
    "role HUD label helper must cover encounter director roles",
)
check_all(
    urgency_body,
    [
        "MELEE",
        "CLOSE",
        "BOSS",
        "ELITE",
        "PURSUIT",
        "TRACKING",
        "Info.Accent",
    ],
    "urgency helper must classify close, boss, elite, and normal threat pressure",
)
check_all(
    threat_body,
    [
        "TActorIterator<ABossZombieActor>",
        "TActorIterator<ACodeZombieActor>",
        "Cast<ABossZombieActor>",
        "DirectionLabelFromPlayer",
        "ZombieRoleHudLabel",
        "ZombieVariantHudLabel",
        "FinalizeThreatHudInfo(Result)",
    ],
    "nearest-threat scan must cover bosses and ordinary zombies without double-counting bosses",
)
check_all(
    construct_body,
    [
        "ThreatCompassText = WidgetTree->ConstructWidget<UTextBlock>",
        "THREAT COMPASS  clear",
        "ThreatCompassSlot",
        "SetPosition(FVector2D(0.0f, 286.0f))",
    ],
    "HUD construction must mount the visible threat compass strip below the objective",
)
check_all(
    refresh_body,
    [
        "const FCodeRescueThreatHudInfo ThreatInfo = GetNearestHudThreat(World, Character)",
        "ThreatCompassText",
        "THREAT COMPASS  %s | %s%s%s | %.0fm %s",
        "GI->bHighContrastHUD",
        "BOSS PRESSURE",
        "ELITE PRESSURE",
        "Threat %s: %s%s %.0fm %s%s%s",
    ],
    "HUD refresh must drive the compass, high-contrast coloring, and tactical threat readout",
)
check("Nearest hostile:" not in hud_cpp,
      "old plain nearest-hostile distance readout should be replaced")
check_all(
    manifest,
    [
        "ThreatCompassHUD",
        "Nearest hostile direction, type, role, and urgency",
        "HUD GetNearestHudThreat and ThreatCompassText",
    ],
    "enemy readability manifest must record the threat compass HUD slice",
)
check("verify_threat_compass_hud_slice_pass.py" in full_qa,
      "full QA must run the threat compass HUD verifier")
check("verify_threat_compass_hud_slice_pass.py" in local_ci,
      "local CI must run the threat compass HUD verifier")
check("Threat compass HUD slice" in progress,
      "progress log must document the threat compass HUD slice")
check_all(
    slice_doc,
    [
        "Threat Compass HUD Slice",
        "ThreatCompassText",
        "GetNearestHudThreat",
        "boss and elite pressure",
        "Player Impact",
        "Remaining QA",
    ],
    "slice doc must explain implementation, player impact, verification, and remaining QA",
)

if errors:
    for error in errors:
        print(f"[verify_threat_compass_hud_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_threat_compass_hud_slice_pass] PASS: threat compass HUD verified")
