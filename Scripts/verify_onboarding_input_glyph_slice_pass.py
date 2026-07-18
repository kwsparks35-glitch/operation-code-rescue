#!/usr/bin/env python3
"""Static verifier for the onboarding input glyph and language-save tutorial slice."""

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


tutorial_h = read(SRC / "CodeRescueTutorialWidget.h")
tutorial_cpp = read(SRC / "CodeRescueTutorialWidget.cpp")
main_menu_h = read(SRC / "CodeRescueMainMenuWidget.h")
main_menu_cpp = read(SRC / "CodeRescueMainMenuWidget.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
launch_verifier = read(PROJECT_ROOT / "Scripts/verify_launch_language_start_screen_save_pass.py")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ONBOARDING_INPUT_GLYPH_SLICE.md")

construct_body = function_body(tutorial_cpp, "void UCodeRescueTutorialWidget::NativeConstruct")
show_page_body = function_body(tutorial_cpp, "void UCodeRescueTutorialWidget::ShowPage")
cards_body = function_body(tutorial_cpp, "TArray<FTutorialActionCard> GetTutorialActionCards")
refresh_body = function_body(tutorial_cpp, "void UCodeRescueTutorialWidget::RefreshActionCardsForPage")
add_card_body = function_body(tutorial_cpp, "void UCodeRescueTutorialWidget::AddActionCard")
save_line_body = function_body(tutorial_cpp, "FString UCodeRescueTutorialWidget::BuildLanguageSaveLine")
phase_body = function_body(tutorial_cpp, "FString UCodeRescueTutorialWidget::BuildPhaseStripLine")
launch_scene = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene")

check_all(
    tutorial_h,
    [
        "PhaseStripText",
        "LanguageSaveText",
        "InputHintModeText",
        "ActionCardBox",
        "RefreshActionCardsForPage",
        "AddActionCard",
        "BuildLanguageSaveLine",
        "BuildPhaseStripLine",
    ],
    "tutorial header must expose the onboarding glyph UI fields and helpers",
)
check_all(
    tutorial_cpp,
    [
        "CodeRescueUITheme.h",
        "Components/HorizontalBox.h",
        "Components/HorizontalBoxSlot.h",
        "Components/VerticalBox.h",
        "Components/VerticalBoxSlot.h",
        "struct FTutorialActionCard",
        "GetTutorialActionCards",
        "C / C+",
        "PY / MATLAB",
    ],
    "tutorial implementation must build themed action glyph cards",
)
check_all(
    construct_body,
    [
        "CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD",
        "CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion",
        "CodeRescueUI::Theme().TextScale = GI->GetUITextScale()",
        "TutorialPhaseStrip",
        "TutorialLanguageSave",
        "TutorialActionCards",
        "TutorialInputHintMode",
        "CodeRescueUI::StylePrimaryButton(NextButton)",
        "CodeRescueUI::StyleSecondaryButton(SkipButton)",
    ],
    "NativeConstruct must create themed phase, save, glyph, and hint-mode regions",
)
check_all(
    show_page_body,
    [
        "BuildPhaseStripLine(Index)",
        "BuildLanguageSaveLine()",
        "bSimplifiedInputHints",
        "SIMPLIFIED HINTS",
        "FULL INPUT GLYPHS",
        "RefreshActionCardsForPage(Index)",
    ],
    "ShowPage must refresh page-specific language-save and glyph state",
)
check_all(
    cards_body,
    [
        "bSimplified",
        "WASD",
        "MOUSE",
        "E",
        "P/ESC",
        "JAVA",
        "C / C+",
        "C++",
        "RUN",
        "LMB",
        "STREAK",
    ],
    "action card table must cover movement, interaction, language, validation, combat, and mastery prompts",
)
check_all(
    refresh_body,
    [
        "ActionCardBox->ClearChildren()",
        "GI && GI->bSimplifiedInputHints",
        "GetTutorialActionCards(Index, bSimplified)",
        "AddActionCard",
    ],
    "RefreshActionCardsForPage must rebuild cards based on simplified input settings",
)
check_all(
    add_card_body,
    [
        "CodeRescueUI::StylePanel",
        "TutorialActionGlyph",
        "TutorialActionLabel",
        "AddChildToHorizontalBox",
        "FSlateChildSize(ESlateSizeRule::Fill)",
    ],
    "AddActionCard must create equal-width themed glyph cards",
)
check_all(
    save_line_body,
    [
        "GI->GetLanguageName()",
        "UCodeRescueGameInstance::MakeLanguageSaveSlotName",
        "GI->DoesLanguageSaveExist",
        "ACTIVE LANGUAGE SAVE",
        "fresh language run ready",
        "resume data found",
    ],
    "language save line must read the active language-specific save contract",
)
check_all(
    phase_body,
    [
        "FIRST TEN MINUTES",
        "LANGUAGE TRACK",
        "VALIDATION",
        "MASTERY",
    ],
    "phase strip must map pages to first-ten-minutes guidance",
)
check_all(
    main_menu_h + main_menu_cpp + launch_verifier,
    [
        "CPlusLanguageBtn",
        "ResumeCPlusBtn",
        "OnCPlusLanguageClicked",
        "OnResumeCPlusClicked",
        "NewLanguageRunLabel(ECodingLanguage::CPlus)",
        "ResumeLanguageRunLabel(GI, ECodingLanguage::CPlus)",
        "StartLanguageRun(ECodingLanguage::CPlus)",
        "ResumeLanguageRun(ECodingLanguage::CPlus)",
        "OperationCodeRescue_Language_CPlus",
    ],
    "launch language start screen and verifier must include the C+ track",
)
check('TEXT("C+")' in launch_scene, "fallback launch scene must show a C+ pedestal")
check("verify_onboarding_input_glyph_slice_pass.py" in full_qa,
      "full QA must run the onboarding input glyph verifier")
check("verify_onboarding_input_glyph_slice_pass.py" in local_ci,
      "local CI must run the onboarding input glyph verifier")
check("Onboarding input glyph slice" in progress,
      "progress log must document the onboarding input glyph slice")
check_all(
    slice_doc,
    [
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "first_ten_minutes_onboarding",
        "input glyph",
        "language-specific save",
        "C+",
        "start screen",
        "simplified hints",
    ],
    "slice doc must map onboarding work to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_onboarding_input_glyph_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_onboarding_input_glyph_slice_pass] PASS: onboarding input glyph slice verified")
