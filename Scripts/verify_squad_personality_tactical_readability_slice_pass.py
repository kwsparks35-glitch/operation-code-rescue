#!/usr/bin/env python3
"""Static verifier for the squad personality tactical readability slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
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


companion_h = read(SRC / "CompanionActor.h")
companion_cpp = read(SRC / "CompanionActor.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "squad_personality_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SQUAD_PERSONALITY_TACTICAL_READABILITY_SLICE.md")

spawn_body = function_body(mode_cpp, "void ACodeRescueGameMode::SpawnRescueSupportTeamForCity")
regroup_body = function_body(character_cpp, "void ACodeRescueCharacter::RegroupRescueTeam")
formation_body = function_body(character_cpp, "void ACodeRescueCharacter::CycleSquadFormation")
hold_body = function_body(character_cpp, "void ACodeRescueCharacter::ToggleSquadHoldPosition")
refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
construct_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")

check_all(
    companion_h,
    [
        "MechanicalIdentity",
        "BarkStyle",
        "RoleAccentColor",
        "ConfigureSquadPersonality",
        "GetHudCallsign",
        "GetRoleStatusLabel",
        "GetOrderResponseBark",
        "PushRoleOrderBark",
    ],
    "companion header must expose runtime personality and readable HUD/order helpers",
)
check_all(
    companion_cpp,
    [
        "GetFirstName",
        "GetRoleCode",
        "GetRoleDutyShort",
        "GetRoleOrderResponse",
        "GetSupportFireBark(DisplayName, RoleLabel, BarkStyle)",
        "Moving with you. Call N if health drops.",
        "Route tools packed, following.",
        "Flank watch online.",
        "Wide rear guard set.",
        "Overwatch acknowledged.",
    ],
    "companion implementation must convert role identity into support barks and order responses",
)
check_all(
    spawn_body,
    [
        "Mira Hale",
        "Tomas Ives",
        "Ada Cross",
        "Noor Vance",
        "Briggs Vale",
        "Manual and automatic medic pulse",
        "Formation support and access reliability",
        "Steady support fire",
        "Wide formation and threat awareness",
        "Close formation anchor and pressure absorber",
        "ConfigureSquadPersonality",
        "Y/U/O/N orders now receive role-specific callouts",
    ],
    "support-team spawn must match the personality manifest and configure each role",
)
for old_name in ("Dex Romero", "Sora Park", "Lena Okafor", "Mateo Cruz"):
    check(old_name not in spawn_body, f"support-team spawn should not retain old placeholder name {old_name}")
check_all(
    regroup_body + formation_body + hold_body,
    [
        "RoleResponders",
        "PushRoleOrderBark",
        "REGROUP HOLD",
        "REGROUP FOLLOW",
        "GetSquadFormationLabel()",
        "GetSquadOrderLabel()",
    ],
    "squad orders must trigger concise role response barks",
)
check(regroup_body.count("PushRoleOrderBark") >= 1, "regroup order must push a role response")
check(formation_body.count("PushRoleOrderBark") >= 1, "formation order must push a role response")
check(hold_body.count("PushRoleOrderBark") >= 1, "hold/follow order must push a role response")
check_all(
    construct_body + refresh_body,
    [
        "SquadStatusText->SetAutoWrapText(true)",
        "GetHudCallsign",
        "GetRoleStatusLabel",
        "ROLES %s",
        "SUPPORT FIRE ONLINE",
    ],
    "HUD must show readable named role pips and role status readouts",
)
check_all(
    manifest,
    [
        "Mira Hale",
        "Tomas Ives",
        "Ada Cross",
        "Noor Vance",
        "Briggs Vale",
        "HUD shows named role pip",
        "Y/U/O/N",
    ],
    "squad personality manifest must document the runtime named-role evidence",
)
check("named role pips" in onboarding and "role-specific callouts" in onboarding,
      "first ten minutes onboarding must mention the named squad readability cue")
check("Named role pips" in visual_targets and "role-specific order feedback" in visual_targets,
      "visual regression targets must include the upgraded squad HUD signal")
check("Mira/Tomas/Ada/Noor/Briggs" in human_qa,
      "human QA checklist must call out the named squad roles")
check("verify_squad_personality_tactical_readability_slice_pass.py" in full_qa,
      "full QA must run the squad personality verifier")
check("verify_squad_personality_tactical_readability_slice_pass.py" in local_ci,
      "local CI must run the squad personality verifier")
check("Squad personality tactical readability slice" in progress,
      "progress log must document the squad personality tactical readability slice")
check_all(
    slice_doc,
    [
        "Squad Personality Tactical Readability Slice",
        "ConfigureSquadPersonality",
        "GetHudCallsign",
        "PushRoleOrderBark",
        "Mira Hale",
        "Player Impact",
        "Validation",
    ],
    "slice doc must explain the implementation, player impact, and validation path",
)

if errors:
    for error in errors:
        print(f"[verify_squad_personality_tactical_readability_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_squad_personality_tactical_readability_slice_pass] PASS: squad personality readability verified")
