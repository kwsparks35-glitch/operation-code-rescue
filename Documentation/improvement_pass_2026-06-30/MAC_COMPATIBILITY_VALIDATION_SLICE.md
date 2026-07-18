# Mac Compatibility Validation Slice

## Purpose

This slice converts the remaining future-only Mac compatibility row in the editor Data Validation contract into a live validator.

The June 25 guidance is explicit that Apple Silicon / Metal support needs conservative promotion gates for strand grooms, Nanite/SM6 content, VSM/Lumen feature cost, shader-heavy materials, texture memory, and imported mesh LODs. Earlier slices made those rules visible; this slice adds the native editor validator and Unreal commandlet report that keep those rules enforceable.

## Implemented

- Added `UCodeRescueMacCompatibilityValidator`, a native `UEditorValidatorBase` subclass in the editor module.
- Groom and HairStrands-like assets are treated as `GroomStrandReviewOnlyMac` inputs unless a card or mesh fallback is documented through `MacHairCardRuntimeReady`.
- Runtime-promoted skeletal meshes must provide renderable LOD data; strict Mac runtime promotions require an authored LOD chain or explicit hero exception.
- Runtime-promoted static meshes must provide renderable LOD data and cannot be treated as Nanite-only without `MacNaniteSM6ReviewGate` evidence plus a `MacNonNaniteFallbackReady` path.
- Material and material-instance candidates receive a Mac shader/texture review warning so shader-complexity, VFX/translucency, texture-memory, and packaged render smoke evidence stay attached to promotion decisions.
- Added `Scripts/verify_mac_compatibility_validation_unreal.py`, which confirms the validator binding in `UnrealEditor-Cmd`, scans groom-like, skeletal, static, and material candidates, verifies renderer/source/manifest gates, and writes `Saved/DataValidation/code_rescue_mac_compatibility_validation.json`.
- Added `Content/CodeRescueData/mac_compatibility_validation_contract.tsv`.
- Replaced the old `FutureMacCompatibilityValidator` row in `editor_data_validation_contract.tsv` with `UCodeRescueMacCompatibilityValidator`.
- Wired the static and Unreal verifiers into `Run_Full_QA_Audit.command`; wired the static verifier into `Run_Local_CI_Readiness.command`.

## Gameplay Boundary

The validator does not disable high-end rendering features globally, delete groom assets, convert MetaHuman hair, or author final Nanite fallback meshes. It keeps the current playable Mac fallback intact while ensuring risky imported content is visibly review-only until it carries the required fallback, budget, and package-smoke evidence.

## Validation

Validation passed.

- Python syntax check passed for the new static and Unreal scripts plus adjacent editor / Mac contract verifiers.
- Static verifier passed for `Content/CodeRescueData/mac_compatibility_validation_contract.tsv`.
- Adjacent editor / Mac hair / Mac feature / Mac asset / Mac rendering / character / physics / world validation contracts passed.
- `./Recompile_Module.command < /dev/null` passed.
- UnrealEditor-Cmd smoke passed through `Scripts/verify_mac_compatibility_validation_unreal.py` and wrote `Saved/DataValidation/code_rescue_mac_compatibility_validation.json`.
- `./Package_Mac_App.command < /dev/null` passed and archived `PackagedMac/Mac/CodeRescueUnreal.app`.
- Packaged null smoke passed with only the already-allowed immediate-exit navigation/crowd warnings.
- Packaged render smoke passed with `SF_METAL_SM6` startup and Metal library mmap evidence, with only the already-allowed CoreAudio sample-rate warning plus immediate-exit navigation/crowd warnings.

Commandlet report highlights:

- Validator class: `/Script/CodeRescueUnrealEditor.CodeRescueMacCompatibilityValidator`
- Native validator base: `UEditorValidatorBase`
- Groom-like assets reviewed: 359
- Skeletal mesh candidates reviewed: 55
- Static mesh candidates reviewed: 22
- Material candidates reviewed: 143
- Strict runtime-promoted Mac assets currently found: 0
- Filesystem review counts: 213 groom `.uasset` files, 5 MetaHuman `.mhpkg` sources, and 2 groom art-source `.mhpkg` sources
- Source gates passed: native validator contract, Mac contract rows, hair fallback runtime gate, Nanite/VSM runtime gate, import budget runtime gate, TAA Mac baseline, renderer feature review config, no runtime groom hard reference, no forced `/Game/Nanite` cook, and hair / feature / asset budget manifest alignment
- Errors: 0

## Honesty Boundary

This is a validation and promotion-control slice, not the final art pass. It does not create final MetaHuman hair cards, final Nanite hero geometry, final HLOD proxy budgets, or target-hardware frame captures for every future content pack. It makes those requirements concrete and hard to bypass.
