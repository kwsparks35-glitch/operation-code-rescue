#!/usr/bin/env python3
"""Umbrella static verifier for the next-20 demo-readiness implementation."""

from __future__ import annotations

import csv
from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOCS = PROJECT_ROOT / "Documentation"
CONFIG = PROJECT_ROOT / "Config"
FEMALE_RADIO_VOICES = {"Samantha", "Victoria", "Kyoko", "Tessa", "Karen"}
errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(msg)


def rows(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return []
    with path.open(encoding="utf-8", newline="") as fh:
        return list(csv.DictReader(fh, delimiter="\t"))


types = read(SRC / "CodeRescueTypes.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
settings_h = read(SRC / "CodeRescueSettingsWidget.h")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
subtitles_cpp = read(SRC / "CodeRescueSubtitlesWidget.cpp")
zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
companion_cpp = read(SRC / "CompanionActor.cpp")
runner_cpp = read(SRC / "CodeRunnerLibrary.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
default_game = read(CONFIG / "DefaultGame.ini")

# 1. Maple narration generation/import path.
maple_clone = read(PROJECT_ROOT / "Tools/MapleVoice/maple_voice_clone.py")
check("GenerationMixin" in maple_clone and "generation_config" in maple_clone,
      "Maple clone must include Transformers 4.50+ XTTS compatibility patch")
briefing_rows = rows(DATA / "radio_briefings.tsv")
female_briefings = [
    r for r in briefing_rows
    if r.get("voice", "").strip() in FEMALE_RADIO_VOICES
]
missing_maple_wavs = [
    r.get("slug", "") for r in female_briefings
    if not (PROJECT_ROOT / "Content/CodeRescueAssets/Audio/RadioSamples" / f"{r.get('slug', '')}_radio_briefing.wav").exists()
]
missing_maple_assets = [
    r.get("slug", "") for r in female_briefings
    if not (PROJECT_ROOT / "Content/CodeRescueAssets/Audio/RadioSamples" / f"{r.get('slug', '')}_radio_briefing.uasset").exists()
]
check(len(female_briefings) == 230,
      f"expected 230 female-voiced Maple briefing rows, found {len(female_briefings)}")
check(not missing_maple_wavs,
      "all expected Maple briefing WAVs must exist; missing: "
      + ", ".join(missing_maple_wavs[:10])
      + (" ..." if len(missing_maple_wavs) > 10 else ""))
check(not missing_maple_assets,
      "all expected Maple SoundWave assets must exist; missing: "
      + ", ".join(missing_maple_assets[:10])
      + (" ..." if len(missing_maple_assets) > 10 else ""))
check((PROJECT_ROOT / "Import_And_Wire_Maple_Narrations.command").exists(),
      "Maple import/wire commandlet wrapper must exist")
check("/Game/CodeRescueAssets/Blueprints/BP_CodeRescueGameMode" in read(PROJECT_ROOT / "Scripts/wire_radio_cues.py"),
      "Maple cue wiring must target the project GameMode Blueprint path")
check("StaticLoadObject" in read(SRC / "CodeRescueGameMode.cpp")
      and "/Game/CodeRescueAssets/Audio/RadioSamples" in read(SRC / "CodeRescueGameMode.cpp"),
      "native GameMode must fall back to slug-based Maple SoundWave loads")
check("/Game/CodeRescueAssets/Audio/RadioSamples" in default_game,
      "radio sample SoundWaves must be always-cooked for packaged Maple playback")

# 2. Clean commit/source-control guidance is preserved.
check((PROJECT_ROOT / ".gitattributes").exists(), "Git LFS attributes must exist")
check((DOCS / "SOURCE_CONTROL_HANDOFF_2026-06-18.md").exists(), "source-control handoff doc must exist")

# 3. Release manifest.
check((PROJECT_ROOT / "Scripts/generate_release_manifest.py").exists(),
      "release manifest generator must exist")

# 4. GameMode split continuation.
check((SRC / "CodeRescueGameModeSpawning.cpp").exists(), "GameMode spawning split file must exist")

# 5. Visual regression screenshots.
check((PROJECT_ROOT / "Scripts/generate_visual_regression_manifest.py").exists(),
      "visual regression manifest generator must exist")
check(len(rows(DATA / "visual_regression_targets.tsv")) >= 8,
      "visual regression targets must cover expected review surfaces")

# 6. Human QA checklist.
check(len(rows(DATA / "human_qa_signoff_checklist.tsv")) >= 10,
      "human QA signoff manifest must cover key review areas")
check("Story, Easy, Normal, Hard, Survival, Nightmare" in read(DOCS / "QA_PLAYTEST_CHECKLIST.md"),
      "QA checklist must mention the six difficulty presets")

# 7. Save compatibility tests.
check((PROJECT_ROOT / "Scripts/verify_save_compatibility_pass.py").exists(),
      "save compatibility verifier must exist")
check("0.8.0-demo-readiness" in read(SRC / "CodeRescueSaveGame.h"),
      "save schema must be versioned for demo readiness")

# 8. First-10-minutes onboarding.
check(len(rows(DATA / "first_ten_minutes_onboarding.tsv")) >= 10,
      "first ten minutes onboarding manifest must cover 10 minutes")

# 9. Remappable controls.
check(len(rows(DATA / "control_remap_manifest.tsv")) >= 18,
      "control remap manifest must cover gameplay controls")
check((PROJECT_ROOT / "Scripts/apply_control_remap_profile.py").exists(),
      "control profile export script must exist")

# 10. Difficulty presets.
for token in ("Story", "Easy", "Normal", "Hard", "Survival", "Nightmare"):
    check(token in types and token in gi_cpp, f"difficulty preset must be implemented: {token}")
check(len(rows(DATA / "difficulty_presets.tsv")) == 6, "difficulty preset manifest must have six rows")

# 11/12. Performance and asset budgets.
check(len(rows(DATA / "performance_city_layer_budget.tsv")) >= 8,
      "performance city layer budget must cover major layers")
check((PROJECT_ROOT / "Scripts/profile_city_layers_static.py").exists(),
      "static city-layer profiler must exist")
check((PROJECT_ROOT / "Scripts/verify_asset_budget_pass.py").exists(),
      "asset budget verifier must exist")
check(len(rows(DATA / "asset_budget_limits.tsv")) >= 7,
      "asset budget limits must cover common asset classes")

# 13. Enemy readability.
for token in ("AttackTelegraphRangeMultiplier", "AttackTelegraphLeadSeconds"):
    check(token in zombie_h and token in zombie_cpp, f"enemy readability hook missing: {token}")
check(len(rows(DATA / "enemy_readability_manifest.tsv")) >= 7,
      "enemy readability manifest must cover enemy feedback")

# 14. Squad personality.
check("DisplayName" in companion_cpp and "RoleLabel" in read(SRC / "CompanionActor.h"),
      "companion actors must expose name/role identity")
check("GetSupportFireBark" in companion_cpp and "TimeSinceRoleBark" in read(SRC / "CompanionActor.h"),
      "companion actors must have role-specific support-fire barks")
check(len(rows(DATA / "squad_personality_manifest.tsv")) >= 5,
      "squad personality manifest must cover five roles")

# 15. Curriculum UX.
for token in ("FailedChecks", "PassedTestCases", "TotalTestCases"):
    check(token in read(SRC / "CodeRescueTypes.h"), f"curriculum result field missing: {token}")
check(len(rows(DATA / "curriculum_feedback_manifest.tsv")) >= 6,
      "curriculum feedback manifest must cover learning UX")

# 16. Accessibility settings.
for token in (
    "SubtitleScale",
    "AimAssistScale",
):
    check(token in gi_h and token in settings_h and token in settings_cpp,
          f"accessibility setting missing UI/runtime support: {token}")
for runtime_token, widget_token in (
    ("bHighContrastHUD", "bCachedHighContrast"),
    ("bReducedMotion", "bCachedReducedMotion"),
    ("bSimplifiedInputHints", "bCachedSimplifiedHints"),
):
    check(runtime_token in gi_h and runtime_token in settings_cpp and widget_token in settings_h,
          f"accessibility setting missing UI/runtime support: {runtime_token}")
check("SubtitleFont.Size" in subtitles_cpp, "subtitle scale must affect subtitle rendering")
check("EffectiveAssistAngle" in character_cpp and "EffectiveAssistRadius" in character_cpp,
      "aim assist scale must affect assisted-hit behavior")
check("KnockbackScale" in character_cpp, "reduced motion must affect hit knockback")
check("bSimplifiedInputHints" in hud_cpp, "simplified input hints must affect HUD text")

# 17. Crash/log packaging.
check((PROJECT_ROOT / "Scripts/create_support_bundle.py").exists(),
      "support bundle script must exist")
check((PROJECT_ROOT / "Scripts/audit_maple_audio_assets.py").exists(),
      "Maple audio audit script must exist")
check((PROJECT_ROOT / "Scripts/verify_package_integrity_pass.py").exists(),
      "package integrity verifier must exist")
check((PROJECT_ROOT / "Scripts/verify_nonhuman_release_readiness_pass.py").exists(),
      "non-human release readiness verifier must exist")
check(len(rows(DATA / "nonhuman_release_readiness_gates.tsv")) >= 8,
      "non-human readiness gates manifest must cover automated release gates")

# 18. Mac signing/notarization.
check("SIGNING_NOTARIZATION_RUNBOOK" in read(DOCS / "DEMO_READINESS_ROADMAP_2026-06-18.md"),
      "demo readiness roadmap must point to signing runbook")
check((DOCS / "SIGNING_NOTARIZATION_RUNBOOK_2026-06-18.md").exists(),
      "signing/notarization runbook must exist")
check((DOCS / "NONHUMAN_RELEASE_READINESS_PASS_2026-06-18.md").exists(),
      "non-human release readiness doc must exist")

# 19. Local CI automation.
for token in (
    "verify_demo_readiness_pass.py",
    "verify_asset_budget_pass.py",
    "generate_release_manifest.py",
    "audit_maple_audio_assets.py",
    "verify_package_integrity_pass.py",
    "verify_nonhuman_release_readiness_pass.py",
    "create_support_bundle.py",
):
    check(token in local_ci, f"local CI command must run {token}")

# 20. Demo readiness roadmap.
check((DOCS / "DEMO_READINESS_ROADMAP_2026-06-18.md").exists(),
      "demo readiness roadmap must exist")
check("Demo Blockers" in read(DOCS / "DEMO_READINESS_ROADMAP_2026-06-18.md"),
      "demo readiness roadmap must include blocker/polish grouping")

for token in (
    "verify_save_compatibility_pass.py",
    "verify_asset_budget_pass.py",
    "verify_demo_readiness_pass.py",
):
    check(token in full_qa, f"full QA must register {token}")

if errors:
    for error in errors:
        print(f"[verify_demo_readiness_pass] FAIL: {error}")
    sys.exit(1)
print("[verify_demo_readiness_pass] PASS: next-20 demo-readiness contract intact")
