#!/usr/bin/env python3
"""Static verifier for the settings color-vision selector slice."""

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


settings_h = read(SRC / "CodeRescueSettingsWidget.h")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
types_h = read(SRC / "CodeRescueTypes.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SETTINGS_COLOR_VISION_SLICE.md")

construct_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::NativeConstruct")
cycle_body = function_body(settings_cpp, "EColorblindMode NextColorblindMode")
click_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnColorblindModeClicked")
readout_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::RefreshReadouts")
button_label_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::RefreshColorblindButtonLabel")
reset_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnResetAccessibilityClicked")
sync_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::SyncCachedControls")
apply_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnApplyClicked")
ppv_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPerZonePostProcessVolume")
configure_ppv_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::ConfigurePerZonePostProcessVolume")

check_all(
    types_h,
    [
        "enum class EColorblindMode",
        "Deuteranope",
        "Protanope",
        "Tritanope",
    ],
    "colorblind enum must remain available",
)
check_all(
    settings_h,
    [
        "CodeRescueTypes.h",
        "OnColorblindModeClicked",
        "ColorblindModeButton",
        "ColorblindModeButtonText",
        "CachedColorblindMode",
        "RefreshColorblindButtonLabel",
    ],
    "settings header must expose the color-vision selector state",
)
check_all(
    settings_cpp,
    [
        "ColorblindModeName",
        "NextColorblindMode",
        "Color Vision Mode",
        "Color Vision:",
    ],
    "settings implementation must name and expose the color-vision selector",
)
check_all(
    construct_body,
    [
        "CachedColorblindMode = GI->ColorblindMode",
        "ColorblindModeButton",
        "OnColorblindModeClicked",
        "Color Vision Mode",
        "RefreshColorblindButtonLabel()",
    ],
    "settings construct must restore, build, and initialize the color-vision selector",
)
check_all(
    cycle_body,
    [
        "EColorblindMode::None",
        "EColorblindMode::Deuteranope",
        "EColorblindMode::Protanope",
        "EColorblindMode::Tritanope",
    ],
    "color-vision cycle must include all four modes",
)
check_all(
    click_body + button_label_body + readout_body,
    [
        "CachedColorblindMode = NextColorblindMode(CachedColorblindMode)",
        "RefreshColorblindButtonLabel()",
        "ColorblindModeName(CachedColorblindMode)",
        "Accessibility | Subtitles",
        "high",
        "standard",
    ],
    "click/readout paths must update and show queued color-vision mode",
)
check_all(
    reset_body + sync_body,
    [
        "CachedColorblindMode = EColorblindMode::None",
        "RefreshColorblindButtonLabel()",
    ],
    "accessibility reset must restore Standard color vision and refresh the button",
)
check("GI->ColorblindMode = CachedColorblindMode" in apply_body,
      "Apply must write color-vision mode into the game instance for save/load")
check_all(
    gi_cpp,
    [
        "Save->ColorblindMode = ColorblindMode",
        "ColorblindMode = Save->ColorblindMode",
        "Deuteranope",
        "Protanope",
        "Tritanope",
    ],
    "game instance save/load/summary must carry color-vision mode",
)
check_all(
    ppv_body + configure_ppv_body,
    [
        "CBMode = GIcb->ColorblindMode",
        "ConfigurePerZonePostProcessVolume(PPV, CityIndex, GradeToken, CBMode)",
        "EColorblindMode::Deuteranope",
        "EColorblindMode::Protanope",
        "EColorblindMode::Tritanope",
        "S.ColorSaturation.X *= 0.6f",
        "S.ColorSaturation.Z *= 1.4f",
    ],
    "world grading must continue consuming the saved color-vision mode",
)
check_all(
    manifest,
    [
        "ColorSafeMarkers",
        "Color Vision Mode cycle button",
        "Standard",
        "Deuteranope",
        "Protanope",
        "Tritanope",
    ],
    "accessibility manifest must record the player-facing color-vision selector",
)
check("verify_settings_color_vision_slice_pass.py" in full_qa,
      "full QA must run the settings color-vision verifier")
check("verify_settings_color_vision_slice_pass.py" in local_ci,
      "local CI must run the settings color-vision verifier")
check("Settings color vision slice" in progress,
      "progress log must document the settings color-vision slice")
check_all(
    slice_doc,
    [
        "TOP_50_RECOMMENDATIONS",
        "colorblind validation",
        "Color Vision Mode",
        "Standard",
        "Deuteranope",
        "Protanope",
        "Tritanope",
        "hot-refresh",
    ],
    "slice doc must map the selector to recommendations and note remaining validation work",
)

if errors:
    for error in errors:
        print(f"[verify_settings_color_vision_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_settings_color_vision_slice_pass] PASS: settings color-vision selector verified")
