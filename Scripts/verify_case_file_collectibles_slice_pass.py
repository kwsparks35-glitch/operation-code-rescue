#!/usr/bin/env python3
"""Static verifier for the collectible case-file slice."""

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


case_h = read(SRC / "CaseFilePickupActor.h")
case_cpp = read(SRC / "CaseFilePickupActor.cpp")
save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
game_mode_h = read(SRC / "CodeRescueGameMode.h")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")
case_manifest = read(DATA / "case_file_collectibles_manifest.tsv")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
character_manifest = read(DATA / "novel_character_world_design_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "CASE_FILE_COLLECTIBLES_SLICE.md")

collect_body = function_body(case_cpp, "bool ACaseFilePickupActor::Collect")
spawn_body = function_body(game_mode_cpp, "void ACodeRescueGameMode::SpawnCollectibleCaseFilesForCity")
apply_objective_body = function_body(gi_cpp, "void UCodeRescueGameInstance::ApplyObjectiveStateToLevel")

check_all(
    case_h,
    [
        "ACaseFilePickupActor",
        "CaseFileId",
        "CaseFileTitle",
        "CaseFileBody",
        "CaseFileTint",
        "bCollected",
        "Collect(ACodeRescueCharacter* Character)",
    ],
    "case-file actor header must expose collectible data and interaction",
)
check_all(
    case_cpp,
    [
        "BuildCaseFileSubtitle",
        "CollectibleCaseFile",
        "CaseFilePickup",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
        "ReleaseDossier",
        "SetCollectedVisualState",
        "CaseFileGroundSnapped",
    ],
    "case-file actor must provide readable visuals, tags, subtitle copy, and save-hide state",
)
check_all(
    collect_body,
    [
        "GI->RecordCaseFileCollected(CaseFileId, CaseFileTitle)",
        "GI->SavePersistentRun()",
        "UCodeRescueSubtitlesWidget::Push",
        "BuildCaseFileSubtitle(CaseFileTitle, CaseFileBody, GI)",
        "Destroy()",
    ],
    "collect flow must persist to selected-language save and inform the player",
)
check_all(
    save_h + gi_h + gi_cpp,
    [
        "CollectedCaseFileIds",
        "LastCollectedCaseFileTitle",
        "HasCollectedCaseFile",
        "RecordCaseFileCollected",
        "GetCaseFileCollectionSummary",
        "CollectedCaseFileIds.Reset()",
        "Save->CollectedCaseFileIds = CollectedCaseFileIds",
        "CollectedCaseFileIds = Save->CollectedCaseFileIds",
    ],
    "save game and game instance must persist case-file collection state",
)
check_all(
    apply_objective_body,
    [
        "TActorIterator<ACaseFilePickupActor>",
        "CollectedCaseFileIds.Contains(CaseFile->CaseFileId)",
        "CaseFile->bCollected = true",
        "CaseFile->SetActorHiddenInGame(true)",
        "CaseFile->SetActorEnableCollision(false)",
    ],
    "loaded objective state must hide already-collected case files",
)
check_all(
    character_cpp,
    [
        "CaseFilePickupActor.h",
        "Cast<ACaseFilePickupActor>(Actor)",
        "TActorIterator<ACaseFilePickupActor>",
        "CaseFile->Collect(this)",
    ],
    "player interaction scan must include case-file pickups",
)
check_all(
    game_mode_h + game_mode_cpp,
    [
        "CaseFilePickupActor.h",
        "SpawnCollectibleCaseFilesForCity",
        "SpawnCollectibleCaseFilesForCity(Mission, CityIndex, Origin, CityLabel)",
    ],
    "game mode must declare and call the collectible case-file spawn layer",
)
check_all(
    spawn_body,
    [
        "terminal_evidence",
        "survivor_note",
        "route_brief",
        "Mission.TerminalTitle",
        "Mission.CurriculumFocus",
        "Mission.LanguageTrackText",
        "Mission.CharacterStoryPlan",
        "Mission.NovelGameplayDetail",
        "Mission.ProgressionPlan",
        "Mission.AccessibilityPolishPlan",
        "NarrativeCaseFileCollectible",
        "CaseFileLanguageRunSave",
        "CollectibleCaseFilesGuidance",
        "[CodeRescueCaseFiles]",
    ],
    "spawn layer must create three mission-aware case files from June 25 guidance fields",
)
check_all(
    journal_cpp,
    [
        "GI->GetCaseFileCollectionSummary()",
        "Case files: profile unavailable",
        "ActiveLine + TEXT(\"\\n\") + LanguageLine + TEXT(\"\\n\") + CaseFileLine",
    ],
    "objective journal must surface case-file collection summary",
)
check_all(
    case_manifest,
    [
        "TerminalEvidence",
        "SurvivorNote",
        "RouteBrief",
        "ACaseFilePickupActor",
        "CollectedCaseFileIds",
        "LastCollectedCaseFileTitle",
        "selected-language save",
    ],
    "case-file manifest must document the three collectible families and save contract",
)
check_all(
    curriculum_manifest,
    [
        "CaseFileCollectibles",
        "ACaseFilePickupActor + journal case-file summary",
        "terminal concept, survivor note, and route brief",
    ],
    "curriculum feedback manifest must document case-file learning context",
)
check_all(
    onboarding,
    [
        "nearby case file can be collected",
        "journal case-file summary",
    ],
    "first-ten-minutes onboarding must include case-file pickup expectations",
)
check_all(
    visual_manifest,
    [
        "CaseFileCollectibles",
        "Glowing case-file slab, CASE FILE prompt, and journal case-file summary",
    ],
    "visual regression targets must include case-file review surface",
)
check("CaseFileCollectibles" in human_qa and "relaunch keeps collected files hidden" in human_qa,
      "human QA checklist must include collect/relaunch persistence check")
check_all(
    creative_plan,
    [
        "collectible case files",
        "verify_case_file_collectibles_slice_pass.py plus manual QA checklist plus packaged smoke",
    ],
    "creative development plan must mark collectible case files as implemented and verifiable",
)
check_all(
    accessibility_manifest,
    [
        "case-file collection summary",
        "survivor intel dossier",
    ],
    "accessibility manifest must include case-file summary in the journal contract",
)
check_all(
    character_manifest,
    [
        "collectible paper case files",
        "SpawnCollectibleCaseFilesForCity",
    ],
    "character/world design manifest must tie Glass Ward case files to runtime spawn layer",
)
check("verify_case_file_collectibles_slice_pass.py" in full_qa,
      "full QA must run the case-file verifier")
check("verify_case_file_collectibles_slice_pass.py" in local_ci,
      "local CI must run the case-file verifier")
check("Case file collectibles slice" in progress,
      "progress log must document the case-file slice")
check_all(
    slice_doc,
    [
        "Case File Collectibles Slice",
        "ACaseFilePickupActor",
        "SpawnCollectibleCaseFilesForCity",
        "CollectedCaseFileIds",
        "LastCollectedCaseFileTitle",
        "Objective Journal",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "Validation",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, source guidance, validation, and QA",
)

if errors:
    print("Case-file collectibles slice verification FAILED:")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("Case-file collectibles slice verification passed.")
