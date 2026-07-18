#!/usr/bin/env python3
"""Static verifier for the color-vision live world-refresh slice."""

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


gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SETTINGS_COLOR_VISION_LIVE_REFRESH_SLICE.md")

configure_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::ConfigurePerZonePostProcessVolume")
spawn_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPerZonePostProcessVolume")
refresh_body = function_body(gamemode_cpp, "int32 ACodeRescueGameMode::RefreshActiveColorVisionPostProcess")
apply_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnApplyClicked")

check_all(
    gamemode_h,
    [
        "class APostProcessVolume",
        "RefreshActiveColorVisionPostProcess",
        "ConfigurePerZonePostProcessVolume",
        "Code Rescue|Accessibility",
    ],
    "game mode header must expose the live color-vision refresh API",
)
check_all(
    configure_body,
    [
        "FPostProcessSettings& S = PPV->Settings",
        "EffectiveGrade",
        "EColorblindMode::Deuteranope",
        "EColorblindMode::Protanope",
        "EColorblindMode::Tritanope",
        "S.ColorSaturation.X *= 0.6f",
        "S.ColorSaturation.Z *= 1.4f",
        "S.ColorSaturation.Z *= 0.6f",
        "S.ColorSaturation.X *= 1.4f",
    ],
    "configuration helper must preserve city grades and all color-vision corrections",
)
check_all(
    spawn_body,
    [
        "CBMode = GIcb->ColorblindMode",
        "CodeRescueZonePostProcess",
        "CodeRescueColorVisionRefresh",
        "CodeRescueCityIndex_",
        "CodeRescueGrade_",
        "ConfigurePerZonePostProcessVolume(PPV, CityIndex, GradeToken, CBMode)",
    ],
    "spawn path must tag volumes and use the shared color-vision helper",
)
check_all(
    refresh_body,
    [
        "StreamedCampaignActors",
        "TActorIterator<APostProcessVolume>",
        "ActorHasTag(ZoneTag)",
        "CodeRescueCityIndex_",
        "CodeRescueGrade_",
        "ConfigurePerZonePostProcessVolume(PPV, VolumeCityIndex, VolumeGradeToken, NewMode)",
        "CodeRescueColorVisionRefresh",
        "return RefreshedCount",
    ],
    "refresh API must reapply active city post-process grading and report count",
)
check_all(
    settings_cpp,
    [
        "#include \"CodeRescueGameMode.h\"",
        "RefreshedWorldColorVolumes",
    ],
    "settings implementation must include and track live color-refresh work",
)
check_all(
    apply_body,
    [
        "GI->ColorblindMode = CachedColorblindMode",
        "GM->RefreshActiveColorVisionPostProcess(GI->ColorblindMode)",
        "Color vision refreshed %d active world grade",
    ],
    "Apply must save the color mode, hot-refresh the active world, and report the result",
)
check_all(
    manifest,
    [
        "ColorSafeMarkers",
        "Color Vision Mode cycle button",
        "hot-refreshes active zone grading",
    ],
    "accessibility manifest must record active color-vision refresh behavior",
)
check("verify_settings_color_vision_live_refresh_slice_pass.py" in full_qa,
      "full QA must run the live color-vision refresh verifier")
check("verify_settings_color_vision_live_refresh_slice_pass.py" in local_ci,
      "local CI must run the live color-vision refresh verifier")
check("Settings color vision live refresh slice" in progress,
      "progress log must document the live color-vision refresh slice")
check_all(
    slice_doc,
    [
        "Settings Color Vision Live Refresh Slice",
        "ConfigurePerZonePostProcessVolume",
        "RefreshActiveColorVisionPostProcess",
        "CodeRescueZonePostProcess",
        "hot-refresh",
        "Human visual validation",
    ],
    "slice doc must explain implementation, player impact, verification, and remaining QA",
)

if errors:
    for error in errors:
        print(f"[verify_settings_color_vision_live_refresh_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_settings_color_vision_live_refresh_slice_pass] PASS: color-vision live refresh verified")
