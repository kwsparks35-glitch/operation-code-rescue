#!/usr/bin/env python3
"""Static verifier for the friendly safehouse NPC service slice."""

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


save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
npc_h = read(SRC / "FriendlyNPCActor.h")
npc_cpp = read(SRC / "FriendlyNPCActor.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/friendly_safehouse_npc_service_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "FRIENDLY_SAFEHOUSE_NPC_SERVICE_SLICE.md")

reset_run = function_body(gi_cpp, "void UCodeRescueGameInstance::ResetRun")
save_internal = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_run = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
mark_service = function_body(gi_cpp, "bool UCodeRescueGameInstance::MarkFriendlyNPCServiceUsed")
reset_services = function_body(gi_cpp, "void UCodeRescueGameInstance::ResetFriendlyNPCServiceCooldowns")
apply_state = function_body(gi_cpp, "void UCodeRescueGameInstance::ApplyObjectiveStateToLevel")
npc_begin = function_body(npc_cpp, "void AFriendlyNPCActor::BeginPlay")
npc_prompt = function_body(npc_cpp, "FString AFriendlyNPCActor::GetInteractionPrompt")
npc_apply = function_body(npc_cpp, "void AFriendlyNPCActor::ApplySavedServiceState")
npc_interact = function_body(npc_cpp, "bool AFriendlyNPCActor::Interact")
hud_update = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
gamemode_tick = function_body(gamemode_cpp, "void ACodeRescueGameMode::Tick")
support_hub = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnFriendlyNPCsForCity")

check_all(
    save_h,
    [
        "TArray<FString> UsedFriendlyNPCServiceIds",
        "bool bHasFriendlyNPCServiceState",
        "Saved per selected-language slot",
    ],
    "save game must persist friendly NPC service cooldowns",
)
check_all(
    gi_h,
    [
        "UsedFriendlyNPCServiceIds",
        "bHasFriendlyNPCServiceState",
        "HasFriendlyNPCServiceCooldown",
        "MarkFriendlyNPCServiceUsed",
        "ClearFriendlyNPCServiceCooldown",
        "ResetFriendlyNPCServiceCooldowns",
        "GetFriendlyNPCServiceSummary",
    ],
    "game instance must expose selected-language NPC service APIs",
)
check_all(
    reset_run,
    [
        "UsedFriendlyNPCServiceIds.Reset()",
        "bHasFriendlyNPCServiceState = false",
    ],
    "fresh language run must clear NPC service cooldowns",
)
check_all(
    save_internal,
    [
        "Save->UsedFriendlyNPCServiceIds = UsedFriendlyNPCServiceIds",
        "Save->bHasFriendlyNPCServiceState = bHasFriendlyNPCServiceState",
    ],
    "save path must serialize NPC service cooldown state",
)
check_all(
    load_run,
    [
        "UsedFriendlyNPCServiceIds = Save->UsedFriendlyNPCServiceIds",
        "bHasFriendlyNPCServiceState = Save->bHasFriendlyNPCServiceState",
        "UsedFriendlyNPCServiceIds.RemoveAll",
    ],
    "load path must restore and sanitize NPC service cooldown state",
)
check_all(
    mark_service,
    [
        "UsedFriendlyNPCServiceIds.AddUnique(ServiceId)",
        "bHasFriendlyNPCServiceState = true",
        "SavePersistentRun()",
    ],
    "marking a used service must save the selected-language profile",
)
check_all(
    reset_services,
    [
        "UsedFriendlyNPCServiceIds.Reset()",
        "bHasFriendlyNPCServiceState = true",
        "SavePersistentRun()",
    ],
    "day-night reset must clear saved NPC cooldowns",
)
check_all(
    apply_state,
    [
        "TActorIterator<AFriendlyNPCActor>",
        "NPC->ApplySavedServiceState()",
    ],
    "world-state apply must restore live NPC cooldowns",
)
check_all(
    npc_h,
    [
        "GetServiceId",
        "GetRoleDisplayName",
        "GetServiceSummary",
        "GetInteractionPrompt",
        "IsServiceOnCooldown",
        "ApplySavedServiceState",
    ],
    "friendly NPC header must expose service prompt/state helpers",
)
check_all(
    npc_begin,
    [
        "FriendlySafehouseNPCService",
        "SelectedLanguageSupportSave",
        "SafehouseNPCServiceLoop",
        "NPCService_",
        "ApplySavedServiceState()",
    ],
    "friendly NPC begin play must tag and restore service state",
)
check_all(
    npc_cpp,
    [
        "RoleServiceBenefit",
        "FriendlyNPC_City%03d_%s",
        "%s: %s once per day-cycle",
    ],
    "friendly NPC source must define stable IDs and role benefit summaries",
)
check_all(
    npc_prompt,
    [
        "IsServiceOnCooldown()",
        "[E] %s used - resets next day/night shift",
        "[E] %s",
        "GetServiceSummary()",
    ],
    "interaction prompt must show benefit or cooldown",
)
check_all(
    npc_apply,
    [
        "HasFriendlyNPCServiceCooldown(GetServiceId())",
        "bPerkUsedThisDay",
    ],
    "friendly NPC must apply saved cooldown state",
)
check_all(
    npc_interact,
    [
        "ApplySavedServiceState()",
        "day-night shift",
        "GetLanguageName()",
        "MarkFriendlyNPCServiceUsed(GetServiceId())",
        "Support saved to %s profile",
    ],
    "friendly NPC interaction must respect and save selected-language cooldowns",
)
check_all(
    hud_update,
    [
        "AFriendlyNPCActor* NPC",
        "Prompt     = NPC->GetInteractionPrompt()",
    ],
    "HUD prompt must show role-specific NPC service text",
)
check_all(
    gamemode_tick,
    [
        "ResetFriendlyNPCServiceCooldowns()",
        "It->ResetDailyPerk()",
    ],
    "day-night transition must clear saved and live NPC cooldowns",
)
check_all(
    support_hub,
    [
        "services save per language and reset at day/night shift",
        "saved per language",
        "FriendlySafehouseNPCService",
        "SelectedLanguageSupportSave",
        "SafehouseNPCServiceLoop",
        "NPC->GetServiceSummary()",
        "resets day/night",
    ],
    "support hub must teach the player the language-save service loop",
)
check_all(
    manifest,
    [
        "Service IDs",
        "Selected-language service save",
        "HUD service prompts",
        "Day-night reset",
        "Support hub signage",
    ],
    "manifest must describe friendly NPC service surfaces",
)
for label, source, tokens in [
    ("creative inclusion plan", plan, ["FriendlySafehouseNPCService", "verify_friendly_safehouse_npc_service_slice_pass.py"]),
    ("human QA checklist", qa, ["FriendlySafehouseNPCService", "Engineer, Medic, Scientist, and Trader"]),
    ("visual regression targets", visual, ["FriendlySafehouseNPCService", "HUD prompt showing benefit or cooldown"]),
    ("full QA command", full_qa, ["verify_friendly_safehouse_npc_service_slice_pass.py"]),
    ("local CI command", local_ci, ["verify_friendly_safehouse_npc_service_slice_pass.py"]),
    ("progress log", progress, ["friendly_safehouse_npc_service_manifest.tsv", "verify_friendly_safehouse_npc_service_slice_pass.py"]),
    ("documentation", doc, ["UsedFriendlyNPCServiceIds", "AFriendlyNPCActor", "Validation Plan"]),
]:
    check_all(
        source,
        tokens,
        f"{label} must reference the friendly safehouse NPC service slice",
    )

if errors:
    print("[verify_friendly_safehouse_npc_service_slice_pass] FAILED")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_friendly_safehouse_npc_service_slice_pass] OK")
