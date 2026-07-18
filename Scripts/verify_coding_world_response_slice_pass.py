#!/usr/bin/env python3
"""Static verifier for the coding-to-rescue world response slice."""

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
terminal_cpp = read(SRC / "CodeTerminalWidget.cpp")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "CODING_TO_RESCUE_WORLD_RESPONSE_SLICE.md")

reveal_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::RevealSolvedTerminalRescueRoute")
spawn_terminal_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnTerminal")
validate_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")
apply_objective_body = function_body(gi_cpp, "void UCodeRescueGameInstance::ApplyObjectiveStateToLevel")

check("RevealSolvedTerminalRescueRoute" in gamemode_h, "GameMode header must expose the world response API")
check_all(
    reveal_body,
    [
        "SolvedRouteCity_",  # 2026-07-11 refresh: tag gained city suffix
        "CodingToRescueWorldResponse",
        "SolvedTerminalWorldResponse",
        "WorldDevelopmentDeepDive",
        "CodingCauseEffect",
        "TerminalSolvedRouteVisible",
        "SpawnSolvedLight",
        "SpawnRouteRewardPickup",
        "SpawnPulseSegment",
        "TagSolvedRoute",  # 2026-07-11 refresh: blocking CODE ACCEPTED reader deliberately removed (morning regression pass)
        "bFromLoad",  # 2026-07-11 refresh: reveal keeps an explicit from-load path
        "SURVIVOR ROUTE UNLOCKED",
        "SolvedRouteRewardPickup",
        "CodeRescueCodingWorldResponse",
    ],
    "GameMode reveal must create tagged route, beacon, and reward response",
)
check("Actor->Tags.Contains(ResponseTag)" in reveal_body, "world response must guard against duplicate solved-route actors")
check("UCodeRescueSubtitlesWidget::Push" in reveal_body, "immediate solve should announce the route unlock")
check("RevealSolvedTerminalRescueRoute(Id, CityIndex, Location, true);" in spawn_terminal_body,
      "solved terminals must reconstruct the route during terminal spawn")
check_all(
    validate_body,
    [
        "const FVector SolvedTerminalLocation = TerminalActor->GetActorLocation()",
        "TerminalActor->MarkSolved()",
        "GameMode->RevealSolvedTerminalRescueRoute",
        "SolvedTerminalLocation",
        "false",
    ],
    "terminal validation success must capture location before hiding the actor and reveal the route immediately",
)
check("CodeRescueGameMode.h" in gi_cpp,
      "game instance must include GameMode for save-state route reconstruction")
check("RevealSolvedTerminalRescueRoute(Id, CityIndex, Location, true)" in gamemode_cpp,
      "save restore must rebuild solved terminal world response")  # 2026-07-11 refresh: reconstruction moved into GameMode terminal spawn
check("Terminal->MarkSolved();" in apply_objective_body,
      "save restore must still hide solved terminals")
check("verify_coding_world_response_slice_pass.py" in full_qa,
      "full QA must run the coding world response verifier")
check("verify_coding_world_response_slice_pass.py" in local_ci,
      "local CI must run the coding world response verifier")
check("Coding-to-rescue world response" in progress,
      "progress log must document the coding-to-rescue world response slice")
check("WORLD_DEVELOPMENT_DEEPDIVE" in slice_doc and "coding-as-rescue" in slice_doc,
      "slice doc must map the work to the world-development guidance")

if errors:
    for error in errors:
        print(f"[verify_coding_world_response_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_coding_world_response_slice_pass] PASS: coding-to-rescue world response verified")
