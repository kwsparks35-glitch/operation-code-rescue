#!/usr/bin/env python3
"""Static verifier for the 2026-07-01 UX pass:
   (1) Enter-to-select on the language screen, and
   (2) world-text declutter -> hovering id markers + a separate scrollable reader.
Static only; a Mac compile + playtest is the Definition-of-Done gate.
"""
from __future__ import annotations
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "Source/CodeRescueUnreal"
errors: list[str] = []

def read(p: Path) -> str:
    if not p.exists():
        errors.append(f"FAIL: missing {p.relative_to(ROOT)}")
        return ""
    return p.read_text(encoding="utf-8", errors="replace")

def need(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(f"FAIL: {msg}")

# --- reader widget (separate scrollable screen) ---
rh = read(SRC / "CodeRescueMessageReaderWidget.h")
rc = read(SRC / "CodeRescueMessageReaderWidget.cpp")
need("UCodeRescueMessageReaderWidget" in rh, "reader widget class present")
need("UScrollBox" in rh and "UScrollBox" in rc, "reader must use a scroll box (scrollable)")
need("OpenReader" in rh and "OpenReader" in rc, "reader exposes OpenReader")
need("NativeOnKeyDown" in rc and "Close" in rc, "reader closes on key")
need("#pragma once" in rh and "CodeRescueMessageReaderWidget.generated.h" in rh, "reader header UE hygiene")

# --- hovering id marker ---
mh = read(SRC / "CodeRescueMessageMarkerActor.h")
mc = read(SRC / "CodeRescueMessageMarkerActor.cpp")
need("ACodeRescueMessageMarkerActor" in mh, "marker actor class present")
need("OpenMessageReader" in mh and "OpenMessageReader" in mc, "marker exposes OpenMessageReader (interact dispatch)")
need("ConfigureMessage" in mh and "ConfigureMessage" in mc, "marker configurable with id/title/body")
need('MessageMarker' in mc, "marker tags itself MessageMarker for interaction")
need("OpenReader" in mc, "marker opens the reader on interact")
need("PromptRange" in mh, "marker shows a proximity read prompt")
need("/Engine/BasicShapes/" in mc, "marker is cook-safe (engine primitives)")

# --- gamemode routes SpawnGuideText through markers ---
gm = read(SRC / "CodeRescueGameMode.cpp")
need("ACodeRescueBeaconMarkerActor" in gm or "ACodeRescueMessageMarkerActor" in gm,
     "gamemode spawns markers (2026-07-04: beacon subclass carries the marker contract)")
need(("bMultiWord" in gm or "bSubstantiveMessage" in gm) and "ConfigureMessage" in gm,
     "SpawnGuideText routes multi-word text to a marker/beacon")
need("NoHoverMarkers" in gm, "legacy world-text path kept behind -NoHoverMarkers")

# --- character interact recognizes + dispatches the marker (no new dependency) ---
ch = read(SRC / "CodeRescueCharacter.cpp")
need(ch.count('MessageMarker') >= 3, "character recognizes MessageMarker (IsInteractable + assist + dispatch)")
need("OpenMessageReader" in ch, "character Interact dispatches OpenMessageReader by name")

# --- language screen: Enter selects + launches ---
menu = read(SRC / "CodeRescueMainMenuWidget.cpp")
menuh = read(SRC / "CodeRescueMainMenuWidget.h")
need("NativeOnKeyDown" in menuh and "NativeOnKeyDown" in menu, "menu handles key input")
need("EKeys::Enter" in menu and "StartLanguageRun" in menu, "Enter confirms + starts the selected language")
need("NativeSupportsKeyboardFocus" in menu, "menu accepts keyboard focus so Enter is received")
need("CycleSelectedLanguage" in menu, "arrow-key language navigation present")

if errors:
    print(f"[verify_ui_declutter_pass] {len(errors)} problem(s):")
    for e in errors:
        print("  " + e)
    sys.exit(1)
print("[verify_ui_declutter_pass] PASS - Enter-to-select + world-text declutter (markers + scrollable reader) present")
print("  NOTE: static check only; compile on Mac + playtest is the Definition-of-Done gate.")
