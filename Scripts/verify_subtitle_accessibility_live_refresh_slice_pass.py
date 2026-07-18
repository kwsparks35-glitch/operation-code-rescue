#!/usr/bin/env python3
"""Static verifier for the subtitle accessibility live-refresh slice."""

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


sub_h = read(SRC / "CodeRescueSubtitlesWidget.h")
sub_cpp = read(SRC / "CodeRescueSubtitlesWidget.cpp")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SUBTITLE_ACCESSIBILITY_LIVE_REFRESH_SLICE.md")

construct_body = function_body(sub_cpp, "void UCodeRescueSubtitlesWidget::NativeConstruct")
tick_body = function_body(sub_cpp, "void UCodeRescueSubtitlesWidget::NativeTick")
push_body = function_body(sub_cpp, "void UCodeRescueSubtitlesWidget::Push")
refresh_body = function_body(sub_cpp, "void UCodeRescueSubtitlesWidget::RefreshAccessibilityState")
apply_body = function_body(sub_cpp, "void UCodeRescueSubtitlesWidget::ApplyAccessibilityStateFromSettings")
settings_apply_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnApplyClicked")

check_all(
    sub_h,
    [
        "#include \"Fonts/SlateFontInfo.h\"",
        "RefreshAccessibilityState",
        "BaseSubtitleFont",
        "bHasBaseSubtitleFont",
        "ApplyAccessibilityStateFromSettings",
    ],
    "subtitle header must expose live refresh and stable base font state",
)
check_all(
    construct_body,
    [
        "BaseSubtitleFont = LineText->GetFont()",
        "BaseSubtitleFont.Size = FMath::Max(22, BaseSubtitleFont.Size)",
        "bHasBaseSubtitleFont = true",
        "ApplyAccessibilityStateFromSettings(true)",
    ],
    "subtitle construction must capture a stable base font and apply saved settings",
)
check(
    "ApplyAccessibilityStateFromSettings(true)" in tick_body,
    "subtitle tick must keep the active overlay synchronized with saved accessibility state",
)
check_all(
    push_body,
    [
        "ActiveInstance->ApplyAccessibilityStateFromSettings(false)",
        "if (!GI->bSubtitlesEnabled) return",
        "Queue.Add",
    ],
    "subtitle push must re-read accessibility state and still no-op when subtitles are disabled",
)
check_all(
    refresh_body,
    [
        "ActiveInstance",
        "ApplyAccessibilityStateFromSettings(true)",
    ],
    "static refresh API must call into the active subtitle overlay",
)
check_all(
    apply_body,
    [
        "GI->SubtitleScale",
        "GI->bSubtitlesEnabled",
        "GI->bHighContrastHUD",
        "FMath::Clamp(GI->SubtitleScale, 0.75f, 1.75f)",
        "SubtitleFont = BaseSubtitleFont",
        "SubtitleFont.Size = FMath::RoundToInt(BaseSubtitleFont.Size * SubtitleScale)",
        "LineText->SetFont(SubtitleFont)",
        "LineText->SetColorAndOpacity",
        "Queue.Reset()",
        "LineText->SetText(FText::FromString(TEXT(\"\")))",
    ],
    "subtitle apply helper must re-read saved settings, re-scale font, recolor, and clear disabled subtitles",
)
check_all(
    settings_cpp,
    [
        "#include \"CodeRescueSubtitlesWidget.h\"",
        "UCodeRescueSubtitlesWidget::RefreshAccessibilityState()",
    ],
    "settings implementation must refresh active subtitles on Apply",
)
check_all(
    settings_apply_body,
    [
        "GI->bSubtitlesEnabled = bCachedSubtitles",
        "GI->SubtitleScale = FMath::Clamp(CachedSubtitleScale, 0.75f, 1.75f)",
        "CodeRescueUI::Theme().TextScale",
        "UCodeRescueSubtitlesWidget::RefreshAccessibilityState()",
    ],
    "Settings Apply must save subtitle settings, update theme scale, and refresh the active overlay",
)
check_all(
    manifest,
    [
        "SubtitleLiveRefresh",
        "Refreshes active subtitle font scale",
        "clears visible captions immediately",
    ],
    "accessibility manifest must record subtitle live-refresh behavior",
)
check("verify_subtitle_accessibility_live_refresh_slice_pass.py" in full_qa,
      "full QA must run the subtitle live-refresh verifier")
check("verify_subtitle_accessibility_live_refresh_slice_pass.py" in local_ci,
      "local CI must run the subtitle live-refresh verifier")
check("Subtitle accessibility live refresh slice" in progress,
      "progress log must document the subtitle live-refresh slice")
check_all(
    slice_doc,
    [
        "Subtitle Accessibility Live Refresh Slice",
        "RefreshAccessibilityState",
        "BaseSubtitleFont",
        "ApplyAccessibilityStateFromSettings",
        "clears the subtitle queue",
        "Remaining QA",
    ],
    "slice doc must explain implementation, player impact, verification, and remaining QA",
)

if errors:
    for error in errors:
        print(f"[verify_subtitle_accessibility_live_refresh_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_subtitle_accessibility_live_refresh_slice_pass] PASS: subtitle accessibility live refresh verified")
