#!/usr/bin/env python3
"""Static verifier for the survivor archetype roster slice."""

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


campaign_h = read(SRC / "CodeRescueCampaign.h")
campaign_cpp = read(SRC / "CodeRescueCampaign.cpp")
survivor_h = read(SRC / "SurvivorActor.h")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/survivor_archetype_roster_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "SURVIVOR_ARCHETYPE_ROSTER_SLICE.md")

profile_fn = function_body(campaign_cpp, "FCodeRescueSurvivorArchetypeProfile FCodeRescueCampaign::GetSurvivorArchetypeProfile")
configure_fn = function_body(survivor_cpp, "void ASurvivorActor::ConfigureArchetypeFromMission")
summary_fn = function_body(survivor_cpp, "FString ASurvivorActor::GetSurvivorArchetypeSummary")
prompt_fn = function_body(survivor_cpp, "FString ASurvivorActor::GetInteractionPrompt")
begin_fn = function_body(survivor_cpp, "void ASurvivorActor::BeginPlay")
rescue_fn = function_body(survivor_cpp, "bool ASurvivorActor::Rescue")
hud_fn = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
interact_fn = function_body(character_cpp, "void ACodeRescueCharacter::Interact")
journal_fn = function_body(journal_cpp, "FString BuildSurvivorIntelDossier")
relief_fn = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnSurvivorReliefCamp")

check_all(
    campaign_h,
    [
        "FCodeRescueSurvivorArchetypeProfile",
        "Title",
        "IconLabel",
        "FieldNeed",
        "RescueSkill",
        "DossierHook",
        "AccentColor",
        "GetSurvivorArchetypeProfile",
    ],
    "campaign header must declare survivor archetype profile data",
)
check_all(
    profile_fn,
    [
        "ECampaignLessonKind::Sum",
        "Power-Grid Apprentice",
        "ECampaignLessonKind::Lock",
        "Systems Mechanic",
        "ECampaignLessonKind::Reverse",
        "Radio-Code Cleanup Specialist",
        "ECampaignLessonKind::Palindrome",
        "Archive Integrity Analyst",
        "ECampaignLessonKind::FizzBuzz",
        "Drone Timing Coordinator",
        "ECampaignLessonKind::EvenFilter",
        "Data Medic",
        "ECampaignLessonKind::LinkedListTraverse",
        "Network Engineer",
        "ECampaignLessonKind::BinarySearch",
        "Supply-Cache Analyst",
        "Mission.SecondaryAccentColor",
    ],
    "campaign profile resolver must cover every survivor archetype family",
)
check_all(
    survivor_h,
    [
        "ArchetypeTitle",
        "ArchetypeIconLabel",
        "ArchetypeFieldNeed",
        "ArchetypeRescueSkill",
        "ArchetypeDossierHook",
        "ArchetypeAccentColor",
        "GetSurvivorArchetypeSummary",
        "GetInteractionPrompt",
        "ConfigureArchetypeFromMission",
    ],
    "survivor actor must expose archetype fields and helpers",
)
check_all(
    configure_fn + begin_fn,
    [
        "GetSurvivorArchetypeProfile",
        "SurvivorArchetypeRosterRuntime",
        "SurvivorRoleReadableNameplate",
        "SelectedLanguageSurvivorHandoff",
        "SurvivorArchetype_",
        "SurvivorRoleIcon_",
        "RescueLight->SetLightColor(ArchetypeAccentColor)",
    ],
    "survivor runtime must configure tags and role-colored presentation",
)
check_all(
    summary_fn,
    ["ArchetypeTitle", "ArchetypeFieldNeed", "ArchetypeRescueSkill"],
    "survivor summary must include role, need, and field value",
)
check_all(
    prompt_fn,
    [
        "bRescued",
        "HasCompletedCityChallengeSet(GI, CityIndex)",  # 2026-07-11 refresh: gate is the full ten-station clearance now
        "[E] %s locked - coding clearance %d/%d",
        "[E] rescue %s - %s",
    ],
    "survivor prompt must show locked and rescue-ready archetype states",
)
check_all(
    rescue_fn,
    [
        "BuildSurvivorLockedRouteLine",
        "ArchetypeFieldNeed",
        "BuildSurvivorRescueLine",
        "ArchetypeRescueSkill",
        "ArchetypeDossierHook",
        "BuildExtractionDispatchLine(SurvivorName, ArchetypeTitle",
        "BuildCompanionHandoffLine(SurvivorName, ArchetypeTitle",
        "SavePersistentRun()",
    ],
    "rescue flow must use archetype handoff text and preserve save behavior",
)
check_all(
    hud_fn,
    [
        "ASurvivorActor* Survivor",
        "CrossColor = Survivor->ArchetypeAccentColor",
        "Prompt     = Survivor->GetInteractionPrompt()",
    ],
    "HUD hover prompt must use survivor archetype state",
)
check_all(
    interact_fn,
    [
        "Survivor->ArchetypeAccentColor.ToFColor(true)",
        "Survivor->GetSurvivorArchetypeSummary()",
    ],
    "interaction confirmation must name the survivor archetype",
)
check_all(
    journal_fn,
    [
        "GetSurvivorArchetypeProfile(*Mission)",
        "Role: %s [%s]",
        "Need: %s",
        "Rescue value: %s",
        "Dossier: %s",
    ],
    "journal dossier must surface survivor archetype details",
)
check_all(
    gamemode_cpp + relief_fn,
    [
        "Survivor->ConfigureArchetypeFromMission(Mission)",
        "Survivor Archetype Marker",
        "Survivor Archetype Halo",
        "SURVIVOR PROFILE",
        "Survivor->ArchetypeIconLabel",
        "Survivor->ArchetypeFieldNeed",
        "Survivor->ArchetypeRescueSkill",
        "Survivor->ArchetypeDossierHook",
        "SurvivorArchetypeRosterRuntime",
        "SelectedLanguageSurvivorHandoff",
    ],
    "game mode must spawn role-readable survivor markers and camp signage",
)
check_all(
    manifest,
    [
        "Campaign archetype resolver",
        "Runtime survivor actor",
        "World readability",
        "HUD prompt",
        "Journal dossier",
        "Rescue handoff",
    ],
    "manifest must document survivor roster surfaces",
)
check_all(
    plan,
    [
        "survivor archetype roster",
        "verify_survivor_archetype_roster_slice_pass.py",
        "verify_character_world_assets.py",
        "manual survivor archetype roster review",
    ],
    "creative inclusion plan must point P0 survivor row at this slice",
)
check_all(
    qa,
    ["SurvivorArchetypeRoster", "role marker", "journal dossier", "selected-language save"],
    "human QA checklist must include survivor archetype roster review",
)
check_all(
    visual,
    ["SurvivorArchetypeRoster", "Survivor role marker", "relief-camp profile card"],
    "visual regression targets must include survivor roster review",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_survivor_archetype_roster_slice_pass.py"],
    "local CI and full QA must run the survivor archetype verifier",
)
check_all(
    progress + doc,
    [
        "Survivor archetype roster slice",
        "FCodeRescueSurvivorArchetypeProfile",
        "SurvivorArchetypeRosterRuntime",
        "verify_survivor_archetype_roster_slice_pass.py",
    ],
    "progress and documentation must summarize the slice",
)

if errors:
    print("[verify_survivor_archetype_roster_slice_pass] FAIL", file=sys.stderr)
    for error in errors:
        print(f" - {error}", file=sys.stderr)
    sys.exit(1)

print("[verify_survivor_archetype_roster_slice_pass] OK")
