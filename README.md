# Operation Code Rescue

A C++ survival-horror game, built on Unreal Engine 5.7, whose core progression
mechanic is learning to program: every rescue is gated by writing and passing a
real, graded coding challenge at an in-world terminal. Course project for
CSCE A201 (Computer Programming I), University of Alaska Anchorage, Summer 2026.

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

## Note on assets
This public repository contains the project's authored source, curriculum, and
tooling. Large licensed marketplace art packs (Fab) and bulk audio used only by
the packaged build are intentionally excluded and are not required to read or
build the C++ work; they can be re-imported from Fab under the owner's license.
