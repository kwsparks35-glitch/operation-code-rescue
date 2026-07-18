#!/usr/bin/env python3
"""Static verifier for the language-safe save slots backup UX slice."""

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


save_h = read(SRC / "CodeRescueSaveSlotsWidget.h")
save_cpp = read(SRC / "CodeRescueSaveSlotsWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
visual_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
onboarding = read(PROJECT_ROOT / "Content/CodeRescueData/first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SAVE_SLOTS_LANGUAGE_BACKUP_UX_SLICE.md")

construct_body = function_body(save_cpp, "void UCodeRescueSaveSlotsWidget::NativeConstruct")
refresh_body = function_body(save_cpp, "void UCodeRescueSaveSlotsWidget::Refresh")
save_body = function_body(save_cpp, "void UCodeRescueSaveSlotsWidget::DoSave")
load_body = function_body(save_cpp, "void UCodeRescueSaveSlotsWidget::DoLoad")
delete_body = function_body(save_cpp, "void UCodeRescueSaveSlotsWidget::DoDelete")
feedback_body = function_body(save_cpp, "void UCodeRescueSaveSlotsWidget::SetFeedback")

check_all(
    save_h,
    [
        "class UBorder",
        "UTextBlock* SummaryText",
        "UTextBlock* FeedbackText",
        "UBorder* PanelFrame",
        "FLinearColor FeedbackColor",
        "SetFeedback(const FString& Message, const FLinearColor& Color)",
        "currently selected coding-language run",
        "start screen remains the",
    ],
    "save slots header must describe and store the language-backup UI contract",
)
check_all(
    save_cpp,
    [
        "#include \"CodeRescueSaveGame.h\"",
        "#include \"CodeRescueUITheme.h\"",
        "#include \"Components/BackgroundBlur.h\"",
        "#include \"Components/Border.h\"",
        "MirrorSaveSlotsThemeFromSettings",
        "DescribeBackupSlot",
        "MakeSaveSlotsActionButton",
        "OperationCodeRescue_Slot%d",
        "SaveSlotsLanguageName",
        "CodeRescueUI::Theme().bHighContrast",
        "GI->bReducedMotion",
        "GI->GetUITextScale()",
    ],
    "save slots implementation must use shared theme, backup summaries, and compatibility slot names",
)
check_all(
    construct_body,
    [
        "LANGUAGE SAVE BACKUPS",
        "PanelFrame = WidgetTree->ConstructWidget<UBorder>",
        "SaveSlotsBlur",
        "SummaryText = MakeSaveSlotsLabel",
        "FeedbackText = MakeSaveSlotsLabel",
        "LanguageSaveBackupScroll",
        "SetFeedback(",
        "start-screen language resume save remains authoritative",
    ],
    "construct path must create the themed backup panel and initial contract feedback",
)
check_all(
    refresh_body,
    [
        "Active language run: %s | Start-screen resume slot: %s",
        "Manual backups copy this run; loading a backup writes it back into that language resume slot.",
        "DescribeBackupSlot(SlotIdx)",
        "Save Backup",
        "Load Backup",
        "Delete",
        "bExists",
        "FSlateChildSize(ESlateSizeRule::Fill)",
    ],
    "refresh must show active language slot, backup states, and explicit backup actions",
)
check_all(
    save_body,
    [
        "const FString BackupSlot = MakeSlotName(SlotIndex)",
        "const FString LanguageSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "GI->SaveSlotName = BackupSlot",
        "const bool bBackupSaved = GI->SavePersistentRun()",
        "GI->SaveSlotName = LanguageSlot",
        "const bool bLanguageSaved = bBackupSaved && GI->SavePersistentRun()",
        "refreshed the %s start-screen resume save",
    ],
    "save action must write backup then restore and refresh the selected language resume slot",
)
check_all(
    load_body,
    [
        "UCodeRescueSaveGame* BackupSave",
        "GI->SaveSlotName = BackupSlot",
        "const bool bLoaded = GI->LoadPersistentRun()",
        "const FString LanguageSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "GI->SaveSlotName = LanguageSlot",
        "UGameplayStatics::SaveGameToSlot(BackupSave, LanguageSlot, 0)",
        "GI->LastSaveWallSeconds",
        "Loaded Backup %d into the %s start-screen resume save",
    ],
    "load action must load a backup and promote it into that backup language's resume slot",
)
check_all(
    delete_body,
    [
        "UGameplayStatics::DeleteGameInSlot",
        "GI->SaveSlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "active language resume save untouched",
    ],
    "delete action must remove only the manual backup and restore the active language slot",
)
check_all(
    feedback_body,
    [
        "FeedbackColor = Color",
        "FeedbackText->SetText",
        "CodeRescueUI::StyleText(FeedbackText",
    ],
    "feedback helper must persist semantic feedback color across refresh",
)
check_all(
    access_manifest,
    [
        "SaveSlotsLanguageBackupAccessibility",
        "Theme-styles language backup rows",
        "Save/Load/Delete backup labels",
    ],
    "accessibility manifest must document save slot backup accessibility",
)
check_all(
    visual_manifest,
    [
        "SaveSlotsLanguageBackups",
        "Active language save summary",
        "start-screen language resume contract",
    ],
    "visual regression targets must include the save backup overlay",
)
check_all(
    onboarding,
    [
        "save a language backup",
        "without losing the start-screen language resume slot",
        "Save Backup refreshes the active language run",
    ],
    "first ten minutes onboarding must document language-safe backups",
)
check("verify_save_slots_language_backup_ux_slice_pass.py" in full_qa,
      "full QA must run the save slots language backup verifier")
check("verify_save_slots_language_backup_ux_slice_pass.py" in local_ci,
      "local CI must run the save slots language backup verifier")
check("Save slots language backup UX slice" in progress,
      "progress log must document the save slots language backup UX slice")
check_all(
    slice_doc,
    [
        "Save Slots Language Backup UX Slice",
        "OperationCodeRescue_Slot0",
        "OperationCodeRescue_Language_<Track>",
        "LANGUAGE SAVE BACKUPS",
        "Save Backup",
        "Load Backup",
        "start screen can resume it later",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_save_slots_language_backup_ux_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_save_slots_language_backup_ux_slice_pass] PASS: language-safe save backup UX verified")
