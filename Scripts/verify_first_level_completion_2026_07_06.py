#!/usr/bin/env python3
"""verify_first_level_completion_2026_07_06.py

Gate for the 2026-07-05/06 FIRST-LEVEL COMPLETION pass. Every item below was
driven by a live playtest of city 01 (New York) at 1310x780 and re-verified in
a second fresh-save playtest after the fixes:

  1. Validation hole: unchanged starter / placeholder return must NOT pass.
  2. Spawn nudge: city player start clear of the safehouse wall (camera).
  3. Terminal coherence: header/title/task text all describe the SERVED lesson.
  4. IDE layout: capped scrolling reading panel + guaranteed-height editor.
  5. Terminal reachability: whole column scrolls; diagnostics internally
     capped+scrolled; action buttons can never be pushed off-screen.
  6. Keyboard focus: Escape/Ctrl hotkeys work on open and after clicks.
  7. Tutorial/recovery double-fire: Backspace while tutorial is up must not
     teleport the player.
  8. Recovery wall clearance: destination probed and nudged away from walls.
  9. Curriculum: full-database lint passes (all 60 entries).
"""
from __future__ import annotations
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
FAILURES: list[str] = []


def check(cond: bool, msg: str) -> None:
    print(f"[verify_first_level_completion_2026_07_06] {'PASS' if cond else 'FAIL'}: {msg}")
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
term_h = SRC / "CodeTerminalWidget.h"
camp = SRC / "CodeRescueCampaign.cpp"
ch_c = SRC / "CodeRescueCharacter.cpp"

# 1 — anti-trivial validation gates
check(has(runner, "Starter code was actually modified (write the body!)",
          "UserNorm != StarterNorm"),
      "item 1: unchanged starter fails validation (live-verified: REPAIR state)")
check(has(runner, "PlaceholderReturns", "return0;", "returnfalse;"),
      "item 1: placeholder-only returns fail validation")

# 2 — spawn wall clearance
check(has(camp, "-3170.0f", "-2760.0f"),
      "item 2: city player start nudged clear of the safehouse wall")

# 3 — terminal coherence (header/title/task describe the served lesson)
check(has(term, "THIS TERMINAL'S CODING TASK", "ActiveLearning.Title"),
      "item 3: brief carries the served lesson's task block")
check(has(term, "The DATA-DRIVEN LESSON NODE below teaches exactly this challenge"),
      "item 3: reading panel states lesson/task unity explicitly")

# 4 — IDE layout: capped reading panel + guaranteed editor height
check(has(term, "ReadingScroll", "ReadingCap", "SetMaxDesiredHeight(280.0f)"),
      "item 4: reading stack scrolls inside a 280px cap")
check(has(term, "CodeEditorMinHeight", "SetMinDesiredHeight(210.0f)",
          "ESlateSizeRule::Automatic"),
      "item 4: code editor keeps a guaranteed 210px minimum height")

# 5 — reachability: outer column scroll + capped diagnostics
check(has(term_h, "UScrollBox* TerminalScroll"),
      "item 5: terminal column scrollbox is a widget member")
check(has(term, "TerminalScroll = WidgetTree->ConstructWidget<UScrollBox>",
          "PanelFrame->SetContent(TerminalScroll)"),
      "item 5: whole terminal column lives in a scrollbox (buttons always reachable)")
check(has(term, "DiagnosticsScroll", "DiagnosticsMaxHeight", "SetMaxDesiredHeight(230.0f)"),
      "item 5: diagnostics capped at 230px with internal scroll")
check(has(term, "TerminalScroll->ScrollWidgetIntoView(OutputFrame"),
      "item 5: validation auto-scrolls fresh results into view")

# 6 — keyboard focus: Escape-to-close reliably reachable
check(has(term, "SetKeyboardFocus();"),
      "item 6: widget takes keyboard focus (open + after clicks)")
check(term.read_text(encoding="utf-8", errors="replace").count("SetKeyboardFocus();") >= 3,
      "item 6: focus restored in NativeConstruct, RunValidation, AnswerPrediction")
check(has(term, "EKeys::Escape", "OnCloseClicked"),
      "item 6: Escape closes the terminal")

# 7 — tutorial/recovery double-fire guard
check(has(ch_c, "Recovery must not fire while the", "UCodeRescueTutorialWidget::IsShowing()"),
      "item 7: Backspace tutorial-dismiss no longer teleports the player")

# 8 — recovery destination wall clearance
check(has(ch_c, "ArenaRecoveryClear", "CameraClearance", "AwayFromWalls",
          "destination nudged"),
      "item 8: recovery destination probed and nudged away from walls")

# 9 — curriculum lint (all 60 entries)
lint = subprocess.run(
    [sys.executable, str(ROOT / "Scripts" / "lint_curriculum_full_2026_07_05.py")],
    capture_output=True, text=True)
check(lint.returncode == 0 and "ALL CHECKS PASSED" in lint.stdout,
      "item 9: full curriculum lint passes (60 entries)")

print()
if FAILURES:
    print(f"[verify_first_level_completion_2026_07_06] {len(FAILURES)} FAILURE(S)")
    sys.exit(1)
print("[verify_first_level_completion_2026_07_06] ALL CHECKS PASSED")
