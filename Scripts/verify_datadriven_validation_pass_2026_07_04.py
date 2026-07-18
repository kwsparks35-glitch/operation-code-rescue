#!/usr/bin/env python3
"""verify_datadriven_validation_pass_2026_07_04.py

Gate for the 2026-07-04 part-2 pass: R1 generic data-driven validation (every
curriculum entry's declared tests execute / structurally gate), R2 full concept-
graph terminal wiring with a tier ladder, ambient v2 zombies, colorblind-safe
beacons, and the verifier-suite migration.
"""
from __future__ import annotations
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
FAILURES: list[str] = []


def check(cond: bool, msg: str) -> None:
    print(f"[verify_datadriven_validation_pass_2026_07_04] {'PASS' if cond else 'FAIL'}: {msg}")
    if not cond:
        FAILURES.append(msg)


def has(path: Path, *needles: str) -> bool:
    try:
        t = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return False
    return all(n in t for n in needles)


runner = SRC / "CodeRunnerLibrary.cpp"
term = SRC / "CodeTerminalWidget.cpp"

# --- R1: generic execution harnesses -------------------------------------------
check(has(runner, "ExtractStarterFunctionName", "GenericDeclarativeSupported"),
      "R1: harness derives the target function from the challenge's own starter")
check(has(runner, "BuildGenericJavaHarness", "BuildGenericCMain", "BuildGenericCppMain",
          "BuildGenericPythonTestBlock", "BuildGenericMatlabRunner"),
      "R1: generic harness builders exist for all five toolchains")
for hook in ("BuildGenericJavaHarness(Challenge)", "BuildGenericCMain(Challenge, Sentinel)",
             "BuildGenericCppMain(Challenge, Sentinel)", "BuildGenericPythonTestBlock(Challenge, SentinelLiteral)",
             "BuildGenericMatlabRunner(Challenge, SandboxDir)"):
    check(has(runner, hook), f"R1: kind-based builder routes Unknown -> {hook.split('(')[0]}")
check(has(runner, "Kind == ECodeChallengeKind::Unknown && GenericDeclarativeSupported(Challenge, bScalarOnly)"),
      "R1: CanUseDeclarativeTests admits any test-bearing challenge generically")
check(has(runner, "Structural gate for data-driven challenge", "Keeps the required signature"),
      "R1: in-engine fallback structurally gates unknown-kind challenges w/ tests")

# --- R2: full concept graph + tier ladder ---------------------------------------
check(has(term, "return !ChallengeId.IsEmpty();"),
      "R2: every standard terminal is data-driven eligible")
check(has(term, "(Challenge.VisibleTests.Num() + Challenge.HiddenTests.Num()) > 0"),
      "R2: uncurated terminals accept any test-bearing curriculum entry")
check(has(term, "Compatible.Sort", "A.Tier < B.Tier", "Progress % Compatible.Num()"),
      "R2: tier ladder orders concept progression across cities")
check(has(term, "generic harness"),
      "R2: oracle text explains the generic execution mode")

# --- top-50 quick wins ------------------------------------------------------------
check(has(SRC / "CodeZombieActor.cpp", "ZombieShamblerV2", "ZombieBruteV2", "bUsingV2ZombieBody"),
      "item 42: procedural-fallback zombies now wear the authored v2 infected")
check(has(SRC / "CodeZombieActor.cpp", "ZombiePhysicalAnimationDeferredUntilImpact",
          "RecreatePhysicsState", "Bodies.Num() == 0"),
      "lazy physics guard: body state is created on impact and zero-body rigs fall back before binding")
check(has(SRC / "CodeRescueBeaconMarkerActor.cpp", "SnapToColorblindSafeAccent", "EColorblindMode::Tritanope"),
      "item 49: beacon accents snap to colorblind-safe anchors per mode")

# --- verifier-suite health ----------------------------------------------------------
check(has(ROOT / "Scripts" / "migrate_verifiers_buildwidgettree_2026_07_04.py", "BuildWidgetTreeNow"),
      "verifier migration script committed for reproducibility")
check(has(ROOT / "Scripts" / "verify_may27_safe_learning_city_controls_pass.py", "IsLocationInsideProtectedLearningZone"),
      "may27 verifier now asserts the protected-learning-zone contract")
check(has(ROOT / "Scripts" / "verify_june18_launch_grounding_symbol_pickup_pass.py",
          "SpawnLaunchLanguageSelectionScene"),
      "june18 verifier allows launch-scene stations, forbids active-play ones")

print()
if FAILURES:
    print(f"[verify_datadriven_validation_pass_2026_07_04] {len(FAILURES)} FAILURE(S)")
    sys.exit(1)
print("[verify_datadriven_validation_pass_2026_07_04] ALL CHECKS PASSED")
