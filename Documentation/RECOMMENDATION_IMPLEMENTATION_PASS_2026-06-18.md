# Recommendation Implementation Pass - 2026-06-18

## Scope

This pass implements items 1-6 from the June 18 recommendation review for
Operation Code Rescue. The goal was to convert the review findings into
shippable source changes, runtime/static QA coverage, and clear follow-up
documentation without disturbing the large pre-existing worktree.

Supersession note: the later June 18 next-20 demo-readiness pass completed the
Maple generation/import/wiring work that was still pending here. Current Maple
coverage is `230/230`, the cooked Mac package is `2.03 GB`, and the latest
evidence lives in `Documentation/NEXT_20_IMPLEMENTATION_PASS_2026-06-18.md`
and `Saved/Release/release_manifest_latest.json`.

## Implemented Items

### 1. Dynamic-material warning fix

- Added `Source/CodeRescueUnreal/CodeRescueMaterialUtils.h` and
  `Source/CodeRescueUnreal/CodeRescueMaterialUtils.cpp`.
- Centralized material loading, tinted dynamic-material creation, and mesh
  application through `CodeRescueMaterials`.
- Added `ResolveDynamicMaterialParent` so newly created dynamic materials use a
  stable authored parent/base material instead of nesting from an existing
  `UMaterialInstanceDynamic`.
- Updated GameMode material application paths to use the shared utility.
- Verified the fresh headless and packaged logs no longer emit the previous
  `LogMaterial` warning, `not a valid parent for MaterialInstanceDynamic`, or
  `MID_MID_` material chain.

### 2. Source-control hygiene and handoff

- Added `.gitattributes` with Unreal binary/media Git LFS patterns and LF
  normalization for code, scripts, config, data, and documentation.
- Extended `.gitignore` for generated Python caches and local virtual
  environments.
- Removed generated local noise observed during this pass, including
  `.DS_Store`, `__pycache__` folders, and an accidental empty top-level
  `Source` directory outside the project.
- Added `Documentation/SOURCE_CONTROL_HANDOFF_2026-06-18.md` to explain the
  LFS expectations, recommended commit slices, and why no commit was created
  from this pass.

No git commit was made because the repository already contained many unrelated
modified and untracked files from earlier work. Future commits should stage the
June 18 hardening files intentionally rather than using a broad `git add .`.

### 3. Runtime/visual assertions for critical static contracts

- Added `Scripts/verify_runtime_log_contracts.py`.
- The verifier checks fresh runtime logs for required public-demo markers:
  `[CodeRescueArenaConfinement]`, `[CodeRescueUSCityIdentity]`,
  `[CodeRescueUnrealSystems]`, `[CodeRescuePublicDemoQuality]`,
  `[CodeRescueSafeLearning]`, `[CodeRescueCreativeImplementation]`, and
  `[CodeRescueEntryAccess]`.
- The verifier also checks the New York signature/district strings and
  Backspace/F8 recovery guidance.
- The verifier forbids the material warning patterns fixed by item 1.
- Wired the verifier into both `Run_Full_QA_Audit.command` and
  `Smoke_Test_Packaged_App.command` so full QA and packaged smoke tests fail
  if those runtime contracts regress.

### 4. Maple narration status made honest

- Kept the Maple sinister female-narration pipeline in place while downgrading
  the completion language to match the current artifact state.
- Updated `Tools/MapleVoice/README.md`,
  `Generate_Maple_Sinister_Narrations.command`,
  `Scripts/import_and_wire_maple_narrations.py`,
  `Scripts/verify_maple_sinister_narration_pass.py`, and
  `Documentation/improvement_pass_2026-06-12/44_MAPLE_SINISTER_FEMALE_NARRATION_PASS.md`.
- Added `Documentation/MAPLE_NARRATION_STATUS_2026-06-18.md`.
- At the time of this pass, generated cue coverage was documented as `0/230`
  and female-voiced cities continued to use the existing fallback until
  generation/import could run. This is superseded by the next-20
  demo-readiness pass, which completed Maple at `230/230`, imported/wired the
  cues, added a native slug-based runtime cue fallback, and cooked the radio
  sample directory into the package.

### 5. External code execution disabled by default

- Added the runtime gate `CodeRescue.AllowExternalCodeValidation`, defaulting
  to `0`.
- Added `-AllowExternalCodeValidation` as a trusted QA/development command-line
  opt-in.
- Exposed `UCodeRunnerLibrary::AreExternalValidatorsAllowed()`.
- Updated language availability, dependency messaging, challenge validation,
  and MATLAB desktop launch behavior so the public default does not spawn local
  `javac`, `clang`, `python3`, or MATLAB processes.
- Kept trusted full QA coverage by running
  `Scripts/verify_curriculum_validator_shapes.py` with
  `-AllowExternalCodeValidation`.
- Updated `Documentation/DISTRIBUTION_GUIDE_MAC.md` with public-release safety
  instructions.

### 6. GameMode split

- Added `Source/CodeRescueUnreal/CodeRescueGameModeSpawning.cpp`.
- Moved block, rotated block, textured block, static mesh prop, and decorative
  civilian spawning helpers out of `CodeRescueGameMode.cpp`.
- Left declarations in `CodeRescueGameMode.h` so the existing class contract
  stays intact.
- Updated `Scripts/verify_june12_city_realization_pass.py` so static city
  wardrobe checks read both GameMode source files after the split.

## Static Hardening Verifier

Added `Scripts/verify_june18_public_hardening_pass.py` to lock this pass in
place. It checks:

- Material utility creation and use.
- LFS and ignore hygiene.
- Runtime log verifier wiring.
- Maple status documentation.
- External validator gate default/off behavior plus trusted QA opt-in.
- GameMode split source coverage.

## Fresh Validation

The following commands passed after the implementation:

```bash
python3 Scripts/verify_june18_public_hardening_pass.py
python3 Scripts/verify_maple_sinister_narration_pass.py
python3 Scripts/verify_june12_city_realization_pass.py
zsh -n Run_Full_QA_Audit.command
git diff --check
./Recompile_Module.command
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

`./Run_Full_QA_Audit.command` passed with fresh
`Saved/Logs/HeadlessFullQASmoke.log` runtime log coverage. The log contract
verifier passed and confirmed the material warning patterns were absent.

Packaged app validation passed for both null and render smoke modes. The smoke
scanner allowed only the known unattended/immediate-quit diagnostics:

- `LogNavigationDirtyArea` in headless/immediate quit.
- `LogCrowdFollowing` RecastNavMesh diagnostic in headless/immediate quit.
- macOS CoreAudio sample-rate query warning in render smoke.

## Fresh Package Evidence

```text
Path: /Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
Size: 1.9G
Timestamp: Jun 18 14:49:01 AKDT 2026
```

## Remaining Follow-Up

- Maple generation/import/wiring has since been completed by the next-20
  demo-readiness pass. Future Maple follow-up is limited to licensing review,
  subjective listen-through, and any deliberate cue regeneration.
- Create intentional git commits in small slices because the working tree still
  contains many unrelated modified and untracked assets/source files from
  earlier work.
- Continue splitting `CodeRescueGameMode.cpp` by feature ownership in future
  passes once the current spawning extraction has had runtime soak time.
