#!/usr/bin/env python3
"""Static verifier for the selected-language breach encounter slice."""

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


gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "LANGUAGE_BREACH_ENCOUNTER_SLICE.md")

reveal_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::RevealSolvedTerminalRescueRoute")

check_all(
    reveal_body,
    [
        "LanguageBreachCheckpoint",
        "LanguageSpecificEncounter",
        "SelectedLanguageOnly",
        "RouteEncounterBeat",
        "LanguageTrack_",
        "LanguageBreachRewardCache",
        "LanguageBreachPatrol",
        "BREACH CHECKPOINT",
        "hold cover, then extract the survivor",
        "Use the checkpoint cover",
    ],
    "solved route must include a selected-language breach checkpoint and encounter beat",
)
check_all(
    reveal_body,
    [
        "ECodingLanguage::Java",
        "ECodingLanguage::C",
        "ECodingLanguage::Python",
        "ECodingLanguage::MATLAB",
        "ECodingLanguage::CPlus",
        "ECodingLanguage::Cpp",
    ],
    "all coding-language tracks must map to the breach encounter",
)
check_all(
    reveal_body,
    [
        "LanguageRewardKind",
        "LanguageRewardAmount",
        "LanguagePatrolVariant",
        "ApplyZombieFamilyVariant",
        "LanguageBreachZombieFamily",
        "TagLanguageBreach",
        "AddLanguageBreachTags",
    ],
    "breach checkpoint must vary rewards, patrol identity, and tags by language",
)
check("if (!bFromLoad && !bSandboxMode)" in reveal_body,
      "breach patrol must be immediate-only and must not respawn during save reconstruction or sandbox")
check("Actor->Tags.Contains(ResponseTag)" in reveal_body,
      "existing solved-route duplicate guard must still protect reconstructed actors")
check("SpawnRouteRewardPickup" in reveal_body and "APickupActor*" in reveal_body,
      "language breach reward should use the existing pickup actor path")
check("CodeRescueCodingWorldResponse" in reveal_body,
      "existing coding-world response log must remain available")
check("verify_language_breach_encounter_slice_pass.py" in full_qa,
      "full QA must run the language breach verifier")
check("verify_language_breach_encounter_slice_pass.py" in local_ci,
      "local CI must run the language breach verifier")
check("Language breach encounter" in progress,
      "progress log must document the language breach slice")
check("WORLD_DEVELOPMENT_DEEPDIVE" in slice_doc and "selected coding language" in slice_doc,
      "slice doc must map the work to world-development and language-selection guidance")

if errors:
    for error in errors:
        print(f"[verify_language_breach_encounter_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_language_breach_encounter_slice_pass] PASS: selected-language breach encounter verified")
