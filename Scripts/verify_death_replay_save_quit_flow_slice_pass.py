#!/usr/bin/env python3
"""Static verifier for the death replay save-and-quit flow slice."""

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


gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
death_h = read(SRC / "CodeRescueDeathWidget.h")
death_cpp = read(SRC / "CodeRescueDeathWidget.cpp")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/death_replay_save_quit_manifest.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "DEATH_REPLAY_SAVE_QUIT_FLOW_SLICE.md")

increment_death = function_body(gi_cpp, "void UCodeRescueGameInstance::IncrementDeathCount")
death_checkpoint = function_body(gi_cpp, "bool UCodeRescueGameInstance::SaveDeathRecoveryCheckpoint")
save_wrapper = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRun()")
save_internal = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
apply_damage = function_body(character_cpp, "void ACodeRescueCharacter::ApplyDamage")
death_construct = function_body(death_cpp, "void UCodeRescueDeathWidget::NativeConstruct")
death_status = function_body(death_cpp, "FString BuildDeathActionStatus")
death_resume = function_body(death_cpp, "void UCodeRescueDeathWidget::OnRestartFromSaveClicked")
death_save_quit = function_body(death_cpp, "void UCodeRescueDeathWidget::OnSaveAndQuitClicked")
death_quit = function_body(death_cpp, "void UCodeRescueDeathWidget::OnQuitClicked")

check_all(
    gi_h,
    [
        "bool SaveDeathRecoveryCheckpoint(bool bCountDeath)",
        "bool SavePersistentRunInternal(bool bCaptureWorldState)",
    ],
    "game instance header must expose death recovery save and private cached-save helper",
)
check('"CodeRescueCampaign.h"' in gi_cpp, "game instance must include campaign starts for recovery checkpoints")
check_all(
    increment_death,
    [
        "++DeathCount",
        "SaveDeathRecoveryCheckpoint(false)",
    ],
    "death count must persist through the recovery-checkpoint save path",
)
check("SavePersistentRun();" not in increment_death,
      "IncrementDeathCount must not use regular live-state save because it can serialize zero health")
check_all(
    death_checkpoint,
    [
        "if (bCountDeath)",
        "CaptureWorldStateFromLevel(GetWorld())",
        "FCodeRescueCampaign::GetPlayerStartLocation(RecoveryObjectiveIndex)",
        "LastPlayerRotation = FRotator(0.0f, 35.0f, 0.0f)",
        "LastPlayerHealth = FMath::Clamp",
        "LastPlayerHealth <= 0.0f",
        "LastPlayerMedkits = FMath::Max(LastPlayerMedkits, 1)",
        "SavePersistentRunInternal(false)",
        "CodeRescueDeathRecovery",
    ],
    "death recovery checkpoint must rewrite live death state into a playable selected-language save",
)
check("return SavePersistentRunInternal(true);" in save_wrapper,
      "regular SavePersistentRun must still capture live state through the internal helper")
check_all(
    save_internal,
    [
        "if (bCaptureWorldState)",
        "CaptureWorldStateFromLevel(GetWorld())",
        "SaveGameToSlot(Save, SaveSlotName, 0)",
    ],
    "internal save helper must optionally skip live capture and still serialize the selected slot",
)
check_all(
    apply_damage,
    [
        "GI->IncrementDeathCount();",
        "UCodeRescueDeathWidget::StaticClass()",
        "Mission failed: operative down.",
    ],
    "death branch must count death and show the recovery widget once health reaches zero",
)
check_all(
    death_h,
    [
        "UTextBlock* ActionStatusText",
        "void SetActionStatus",
        "OnSaveAndQuitClicked",
    ],
    "death widget header must include the recovery status surface and handlers",
)
check_all(
    death_cpp,
    [
        "BuildDeathActionStatus",
        "DeathActionStatus",
        "Playable recovery checkpoint",
        "Resume replays from the current city entry",
        "CodeRescueDeathFlow",
    ],
    "death widget must explain and log recovery checkpoint choices",
)
check_all(
    death_construct,
    [
        "ActionStatusText",
        "BuildDeathActionStatus(GI)",
        "SAVE THIS LANGUAGE RUN AND QUIT",
    ],
    "death construct path must surface the recovery status before the action buttons",
)
check_all(
    death_status,
    [
        "MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "GI->GetLanguageName()",
        "Save + Quit keeps this checkpoint for the start screen",
    ],
    "death status must identify selected language slot and start-screen resume contract",
)
check_all(
    death_resume,
    [
        "const ECodingLanguage Language = GI->SelectedLanguage",
        "GI->ResumeLanguageRun(Language)",
        "Loading %s recovery checkpoint",
        "OpenLevel",
    ],
    "death resume must explicitly load the selected-language save before reloading the level",
)
check_all(
    death_save_quit,
    [
        "GI->SaveDeathRecoveryCheckpoint(false)",
        "Saved playable %s recovery checkpoint",
        "The start screen can resume it",
        "QuitGame",
    ],
    "death save-and-quit must save a playable recovery checkpoint before closing",
)
check_all(
    death_quit,
    [
        "quit without additional save",
        "MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "QuitGame",
    ],
    "death quit path must be distinct from save-and-quit for QA logs",
)
check_all(
    manifest,
    [
        "Death recovery checkpoint",
        "Playable replay start",
        "Selected-language resume",
        "Save and quit",
        "Quit without extra save",
    ],
    "death replay manifest must document each player-facing recovery path",
)
check_all(
    plan,
    [
        "death replay save-and-quit flow",
        "verify_death_replay_save_quit_flow_slice_pass.py",
        "verify_end_state_language_run_continuity_slice_pass.py",
        "manual death-flow review",
    ],
    "creative inclusion plan must promote the death flow beyond generic runtime smoke",
)
check_all(
    qa,
    [
        "DeathReplaySaveQuitFlow",
        "save-and-quit",
        "city entry pad",
    ],
    "human QA checklist must require manual death replay/save review",
)
check_all(
    visual,
    [
        "DeathReplaySaveQuitFlow",
        "DeathActionStatus",
        "playable recovery checkpoint",
    ],
    "visual regression targets must include the updated death action status",
)
check("verify_death_replay_save_quit_flow_slice_pass.py" in full_qa,
      "full QA must run the death replay save-and-quit verifier")
check("verify_death_replay_save_quit_flow_slice_pass.py" in local_ci,
      "local CI must run the death replay save-and-quit verifier")
check("Death replay save-and-quit flow slice" in progress,
      "progress log must document the death replay save-and-quit slice")
check_all(
    doc,
    [
        "Death Replay Save-And-Quit Flow Slice",
        "SaveDeathRecoveryCheckpoint",
        "SavePersistentRunInternal",
        "RESUME FROM LANGUAGE SAVE",
        "SAVE THIS LANGUAGE RUN AND QUIT",
        "start screen",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, verification, and manual QA",
)

if errors:
    for error in errors:
        print(f"[verify_death_replay_save_quit_flow_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_death_replay_save_quit_flow_slice_pass] PASS: death replay save-and-quit flow is implemented and documented")
