# Operation Code Rescue

A C++ survival-horror game, built on Unreal Engine 5.7, whose core progression
mechanic is learning to program: every rescue is gated by writing and passing a
real, graded coding challenge at an in-world terminal. Course project for
CSCE A201 (Computer Programming I), University of Alaska Anchorage, Summer 2026.

Author: Kenneth (Kenny) Sparks. Developed with instructor-approved AI
assistance — Claude (Anthropic): primary AI-assisted C++ implementation,
Blender scripting, and verification tooling; Codex (OpenAI): later integration,
repository reconciliation, and document revision. Use was disclosed and
approved (June 18–19, 2026); details are in the project report (Section 1.3 and
Appendix C).

At the submitted snapshot: 69,358 physical lines across 142 C++ source and
header files under `Source/`; 60 curriculum entries across 10 concept tiers and
6 language tracks (C, C+, C++, Java, Python, MATLAB); 465 campaign mission
records (342 US + 123 international) sharing procedural generators; 171 Python
verifier scripts plus a run-all oversight watchdog. The default packaged build
validates player code with non-executing structural and anti-trivial checks;
external compiler/interpreter execution is opt-in for trusted local QA.

## What is in this repository
- `Source/` — the complete C++ game module (all gameplay, UI, and the in-engine
  code-validation engine; no Blueprint logic).
- `Content/CodeRescueData/` — the declarative JSON curriculum database
  (challenges, concept graph, language tracks).
- `Scripts/` — the Blender art-generation pipeline and the Python verification
  suite (171 verifier scripts plus the oversight watchdog).
- `Documentation/` — dated per-pass engineering records.
- `Config/`, `*.command` — build/run helpers.

## Building
Open `CodeRescueUnreal.uproject` in Unreal Engine 5.7 (macOS, Apple Silicon),
or run `Recompile_Module.command`; package with `Package_Mac_App.command`.

## Video presentation
Slide walkthrough + live demo of the packaged macOS build (one video):
https://drive.google.com/drive/folders/1IwMeEkBgQ-PaNy8-HWPmM50Aq9xp7-W7?usp=sharing
(also listed in `Video Presentation Link.txt`).

Public repository: https://github.com/kwsparks35-glitch/operation-code-rescue

## Note on assets
This public repository contains the project's authored source, curriculum, and
tooling. Large licensed marketplace art packs (Fab) and bulk audio used only by
the packaged build are intentionally excluded and are not required to read or
build the C++ work; they can be re-imported from Fab under the owner's license.
