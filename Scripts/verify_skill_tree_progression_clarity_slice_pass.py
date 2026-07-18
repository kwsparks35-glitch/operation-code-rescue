#!/usr/bin/env python3
"""Static verifier for the skill tree progression clarity slice."""

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


skill_h = read(SRC / "CodeRescueSkillTreeWidget.h")
skill_cpp = read(SRC / "CodeRescueSkillTreeWidget.cpp")
curriculum_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/curriculum_feedback_manifest.tsv")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
visual_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SKILL_TREE_PROGRESSION_CLARITY_SLICE.md")

construct_body = function_body(skill_cpp, "void UCodeRescueSkillTreeWidget::NativeConstruct")
refresh_body = function_body(skill_cpp, "void UCodeRescueSkillTreeWidget::Refresh")
unlock_body = function_body(skill_cpp, "void UCodeRescueSkillTreeWidget::TryUnlockNode")
feedback_body = function_body(skill_cpp, "void UCodeRescueSkillTreeWidget::SetFeedback")

check_all(
    skill_h,
    [
        "class UBorder",
        "UTextBlock* SummaryText",
        "UBorder* PanelFrame",
        "FLinearColor FeedbackColor",
        "SetFeedback(const FString& Message, const FLinearColor& Color)",
    ],
    "skill tree header must persist progression clarity widgets and feedback color",
)
check_all(
    skill_cpp,
    [
        "#include \"CodeRescueUITheme.h\"",
        "constexpr int32 SkillTreeNodeCount = 8",
        "constexpr int32 SkillTreeNodeCost = 2",
        "MirrorSkillTreeThemeFromSettings",
        "CountUnlockedSkills",
        "SkillCategory",
        "SkillStatusLabel",
        "SkillNodeFill",
        "SkillTreeMilestoneLine",
        "CodeRescueUI::Theme().bHighContrast",
        "GI->bReducedMotion",
        "GI->GetUITextScale()",
    ],
    "skill tree implementation must use shared theme, accessibility settings, and progression helpers",
)
check_all(
    construct_body,
    [
        "PanelFrame = WidgetTree->ConstructWidget<UBorder>",
        "FIELD SKILL TREE",
        "SummaryText = MakeSkillTreeLabel",
        "FeedbackText = MakeSkillTreeLabel",
        "ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill))",
        "SetFeedback(TEXT(\"Select a READY node",
        "StyleSecondaryButton(CloseButton)",
    ],
    "construct path must create the themed panel, summary, feedback line, and scrollable node list",
)
check_all(
    refresh_body,
    [
        "Research Points %d | Skills %d/%d | Cost %d RP",
        "GI->GetLanguageName()",
        "GI->SaveSlotName",
        "GI->GetLearningProgressSummary()",
        "GI->GetLanguageProgressSummary()",
        "Save: unlocks persist immediately",
        "SkillTreeMilestoneLine(GI, UnlockedCount)",
        "SkillStatusLabel(bUnlocked, bCanAfford, RP)",
        "Outcome: %s",
        "Progression: applies now and remains saved to this language run.",
        "SetIsEnabled(!bUnlocked)",
    ],
    "refresh must surface language save, learning progress, RP cost, node state, and persistence copy",
)
check_all(
    unlock_body,
    [
        "Need %d more Research Point",
        "GI->TryUnlockSkill(NodeIndex)",
        "Saved to the %s run",
        "CodeRescueUI::Color::TerminalGreenBright()",
        "CodeRescueUI::Color::Warning()",
        "Refresh();",
    ],
    "unlock flow must explain short RP, save successful unlocks, and refresh state",
)
check_all(
    feedback_body,
    [
        "FeedbackText->SetText",
        "FeedbackColor = Color",
        "CodeRescueUI::StyleText(FeedbackText",
    ],
    "feedback helper must preserve semantic feedback color across refresh",
)
check_all(
    curriculum_manifest,
    [
        "SkillTreeProgressionClarity",
        "PointsText + SummaryText + SkillStatusLabel",
        "active language run",
    ],
    "curriculum feedback manifest must document skill tree progression clarity",
)
check_all(
    access_manifest,
    [
        "SkillTreeProgressionAccessibility",
        "high-contrast node states",
        "unlocked, ready, and locked skills",
    ],
    "accessibility manifest must document skill tree accessibility coverage",
)
check_all(
    visual_manifest,
    [
        "SkillTreeProgressionSurface",
        "ResearchPoints header",
        "unlocked/ready/locked node labels",
    ],
    "visual regression targets must include the skill tree progression surface",
)
check("verify_skill_tree_progression_clarity_slice_pass.py" in full_qa,
      "full QA must run the skill tree progression clarity verifier")
check("verify_skill_tree_progression_clarity_slice_pass.py" in local_ci,
      "local CI must run the skill tree progression clarity verifier")
check("Skill tree progression clarity slice" in progress,
      "progress log must document the skill tree progression clarity slice")
check_all(
    slice_doc,
    [
        "Skill Tree Progression Clarity Slice",
        "ResearchPoints",
        "active coding language",
        "UNLOCKED",
        "READY - spend 2 RP",
        "LOCKED - need N more RP",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_skill_tree_progression_clarity_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_skill_tree_progression_clarity_slice_pass] PASS: skill tree progression clarity verified")
