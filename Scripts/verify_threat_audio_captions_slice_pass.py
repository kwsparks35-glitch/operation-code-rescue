#!/usr/bin/env python3
"""Static verifier for the hostile threat audio captions slice."""

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


zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
subtitles_cpp = read(SRC / "CodeRescueSubtitlesWidget.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/enemy_readability_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "THREAT_AUDIO_CAPTIONS_SLICE.md")

caption_body = function_body(zombie_cpp, "void ACodeZombieActor::PushThreatCaption")
direction_body = function_body(zombie_cpp, "FString DirectionLabelFromZombieToPlayer")
variant_body = function_body(zombie_cpp, "FString ZombieThreatVariantLabel")
role_body = function_body(zombie_cpp, "FString ZombieThreatRolePrefix")
growl_body = function_body(zombie_cpp, "void ACodeZombieActor::ScheduleNextGrowl")
tick_body = function_body(zombie_cpp, "void ACodeZombieActor::Tick")
damage_body = function_body(zombie_cpp, "void ACodeZombieActor::ApplyRescueDamage")
elite_body = function_body(zombie_cpp, "bool ACodeZombieActor::TickEliteBehavior")
boomer_body = function_body(zombie_cpp, "void ACodeZombieActor::OnBoomerDeath")

check_all(
    zombie_h,
    [
        "LastThreatCaptionWorldTime",
        "PushThreatCaption",
        "CooldownSeconds",
        "RadiusUU",
    ],
    "zombie header must declare the throttled threat-caption helper",
)
check_all(
    zombie_cpp,
    [
        "#include \"CodeRescueSubtitlesWidget.h\"",
        "ZombieThreatVariantLabel",
        "ZombieThreatRolePrefix",
        "DirectionLabelFromZombieToPlayer",
    ],
    "zombie implementation must include subtitle routing and label helpers",
)
check_all(
    variant_body,
    [
        "DogZombie",
        "UrbanZombie4",
        "BusinessSuit",
        "BloatedFemale",
        "NurseFemale",
        "EliteSpitter",
        "EliteCharger",
        "EliteBoomer",
    ],
    "variant label helper must cover imported and elite threats",
)
check_all(
    role_body,
    ["Anchor", "Flanker", "Pressure", "Sentinel"],
    "role label helper must cover encounter director roles",
)
check_all(
    direction_body,
    [
        "ForwardDot",
        "RightDot",
        "ahead",
        "behind",
        "right",
        "left",
        "here",
    ],
    "direction helper must report player-relative threat direction",
)
check_all(
    caption_body,
    [
        "Now - LastThreatCaptionWorldTime",
        "CooldownSeconds",
        "DistanceUU",
        "RadiusUU",
        "LastThreatCaptionWorldTime = Now",
        "UCodeRescueSubtitlesWidget::Push",
        "[Threat %s]",
    ],
    "caption helper must throttle, proximity gate, and push subtitle text",
)
check_all(
    growl_body + tick_body + damage_body + elite_body + boomer_body,
    [
        "PushThreatCaption(TEXT(\"growl\")",
        "PushThreatCaption(TEXT(\"strikes cover\")",
        "PushThreatCaption(TEXT(\"attack\")",
        "PushThreatCaption(TEXT(\"down\")",
        "PushThreatCaption(TEXT(\"acid spit\")",
        "PushThreatCaption(TEXT(\"charging\")",
        "PushThreatCaption(TEXT(\"explosion releases small infected\")",
    ],
    "hostile audio/ability events must emit threat captions",
)
check_all(
    subtitles_cpp,
    [
        "if (!GI->bSubtitlesEnabled) return",
        "SubtitleScale",
    ],
    "subtitle widget must continue honoring saved subtitle accessibility settings",
)
check_all(
    manifest,
    [
        "ThreatAudioCaptions",
        "Subtitle-backed hostile audio captions",
        "PushThreatCaption from growl/attack/elite/death events",
    ],
    "enemy readability manifest must record threat audio captions",
)
check("verify_threat_audio_captions_slice_pass.py" in full_qa,
      "full QA must run the threat audio captions verifier")
check("verify_threat_audio_captions_slice_pass.py" in local_ci,
      "local CI must run the threat audio captions verifier")
check("Threat audio captions slice" in progress,
      "progress log must document the threat audio captions slice")
check_all(
    slice_doc,
    [
        "Threat Audio Captions Slice",
        "PushThreatCaption",
        "growl",
        "melee attack",
        "spitter acid",
        "charger dash",
        "boomer split-spawn",
        "Human playtest",
    ],
    "slice doc must explain implementation, impact, verification, and remaining QA",
)

if errors:
    for error in errors:
        print(f"[verify_threat_audio_captions_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_threat_audio_captions_slice_pass] PASS: threat audio captions verified")
