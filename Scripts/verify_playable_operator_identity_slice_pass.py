#!/usr/bin/env python3
"""Static verifier for the playable operator identity slice."""

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
character_h = read(SRC / "CodeRescueCharacter.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
save_verifier = read(PROJECT_ROOT / "Scripts/verify_save_compatibility_pass.py")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/playable_operator_identity_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "PLAYABLE_OPERATOR_IDENTITY_SLICE.md")

resolve_identity = function_body(gi_cpp, "void ResolveOperatorIdentityForLanguage")
reset_run = function_body(gi_cpp, "void UCodeRescueGameInstance::ResetRun")
identity_init = function_body(gi_cpp, "void UCodeRescueGameInstance::InitializeOperatorIdentityForLanguage")
identity_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetOperatorIdentitySummary")
save_internal = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_run = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
save_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetLanguageSaveSummary")
fresh_run = function_body(gi_cpp, "bool UCodeRescueGameInstance::StartFreshLanguageRun")
resume_run = function_body(gi_cpp, "bool UCodeRescueGameInstance::ResumeLanguageRun")
character_begin = function_body(character_cpp, "void ACodeRescueCharacter::BeginPlay")
character_identity = function_body(character_cpp, "FString ACodeRescueCharacter::GetOperatorIdentitySummary")
hud_refresh = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
creative_layer = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCreativeRecommendationImplementationLayer")

check_all(
    save_h,
    [
        "FString OperatorCallsign",
        "FString OperatorRoleTitle",
        "FString OperatorProfileNote",
        "bool bHasOperatorIdentityState",
    ],
    "save game must persist operator identity with a back-compat flag",
)
check_all(
    gi_h,
    [
        "OperatorCallsign",
        "OperatorRoleTitle",
        "OperatorProfileNote",
        "bHasOperatorIdentityState",
        "GetOperatorIdentitySummary",
        "InitializeOperatorIdentityForLanguage",
    ],
    "game instance must expose operator identity fields and helpers",
)
check_all(
    resolve_identity,
    [
        "ECodingLanguage::Java",
        "Rhea Calder",
        "ECodingLanguage::C",
        "Ilan Cross",
        "ECodingLanguage::Python",
        "Noor Vance",
        "ECodingLanguage::MATLAB",
        "Mika Stone",
        "ECodingLanguage::CPlus",
        "Jules Ardent",
        "ECodingLanguage::Cpp",
        "Advanced Rescue Operator",
    ],
    "language-to-operator mapping must cover every launch language",
)
check_all(
    reset_run,
    ["InitializeOperatorIdentityForLanguage(SelectedLanguage)"],
    "fresh reset must seed an operator identity",
)
check_all(
    identity_init,
    [
        "ResolveOperatorIdentityForLanguage(Language, OperatorCallsign, OperatorRoleTitle, OperatorProfileNote)",
        "bHasOperatorIdentityState = true",
    ],
    "operator initializer must set mapped fields and compatibility flag",
)
check_all(
    identity_summary,
    [
        "OperatorCallsign.IsEmpty()",
        "OperatorRoleTitle.IsEmpty()",
        "OperatorProfileNote.IsEmpty()",
        "GetLanguageName()",
    ],
    "operator summary must have safe fallbacks and include language",
)
check_all(
    save_internal,
    [
        "Save->OperatorCallsign = OperatorCallsign",
        "Save->OperatorRoleTitle = OperatorRoleTitle",
        "Save->OperatorProfileNote = OperatorProfileNote",
        "Save->bHasOperatorIdentityState = bHasOperatorIdentityState",
    ],
    "save path must serialize operator identity",
)
check_all(
    load_run,
    [
        "Save->bHasOperatorIdentityState",
        "OperatorCallsign = Save->OperatorCallsign",
        "OperatorRoleTitle = Save->OperatorRoleTitle",
        "OperatorProfileNote = Save->OperatorProfileNote",
        "InitializeOperatorIdentityForLanguage(SelectedLanguage)",
    ],
    "load path must restore or initialize operator identity",
)
check_all(
    save_summary,
    [
        "OperatorCallsign",
        "OperatorRoleTitle",
        "%s %s",
    ],
    "language save summary must include operator callsign and role",
)
check_all(
    fresh_run,
    [
        "SelectedLanguage = Language",
        "InitializeOperatorIdentityForLanguage(Language)",
    ],
    "fresh language start must initialize the selected-language operator",
)
check_all(
    resume_run,
    [
        "InitializeOperatorIdentityForLanguage(Language)",
        "SelectedLanguage = Language",
    ],
    "missing-save resume path must initialize the selected-language operator",
)
check_all(
    character_h + character_cpp,
    ["GetOperatorIdentitySummary", "PlayableOperatorIdentityRuntime", "SelectedLanguageOperatorProfile", "PlayerOperator"],
    "player character must expose and tag runtime operator identity",
)
check_all(
    character_begin,
    ["!GI->bHasOperatorIdentityState", "InitializeOperatorIdentityForLanguage", "ApplySkillTreeToPlayer"],
    "player BeginPlay must initialize only missing older-save identity state",
)
check_all(
    character_identity,
    ["GI->GetOperatorIdentitySummary()", "Rhea Calder | Rescue Operator"],
    "character operator summary must delegate to GameInstance with fallback",
)
check_all(
    hud_refresh,
    [
        "OperatorCallsign",
        "OperatorRoleTitle",
        "Operator: %s (%s)",
    ],
    "HUD must display operator callsign and role",
)
check_all(
    creative_layer,
    [
        "ActiveOperatorSummary",
        "ACTIVE OPERATOR PROFILE",
        "GetOperatorIdentitySummary()",
        "PlayableOperatorIdentityRuntime",
        "SelectedLanguageOperatorProfile",
        "PlayableOperatorIdentitySave",
    ],
    "cast-promotion stage must mirror the active operator profile",
)
check_all(
    save_verifier,
    [
        "bHasOperatorIdentityState",
        "OperatorCallsign",
        "OperatorRoleTitle",
        "OperatorProfileNote",
        "GetOperatorIdentitySummary",
    ],
    "save compatibility verifier must cover operator identity",
)
check_all(
    manifest,
    [
        "Language-derived operator identity",
        "Selected-language operator save",
        "Start-screen resume summary",
        "HUD identity readout",
        "Cast-stage identity board",
    ],
    "manifest must describe operator identity surfaces",
)
for label, source, tokens in [
    ("creative inclusion plan", plan, ["playable rescue operator", "verify_playable_operator_identity_slice_pass.py"]),
    ("human QA checklist", qa, ["PlayableOperatorIdentityRuntime", "start-screen resume row"]),
    ("visual regression targets", visual, ["PlayableOperatorIdentityRuntime", "ACTIVE OPERATOR PROFILE"]),
    ("full QA command", full_qa, ["verify_playable_operator_identity_slice_pass.py"]),
    ("local CI command", local_ci, ["verify_playable_operator_identity_slice_pass.py"]),
    ("progress log", progress, ["playable_operator_identity_manifest.tsv", "verify_playable_operator_identity_slice_pass.py"]),
    ("documentation", doc, ["OperatorCallsign", "GetOperatorIdentitySummary", "Validation Plan"]),
]:
    check_all(source, tokens, f"{label} must reference the playable operator identity slice")

if errors:
    print("[verify_playable_operator_identity_slice_pass] FAILED")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_playable_operator_identity_slice_pass] OK")
