# Editor Data Validation Contract Slice

This pass creates the first native Unreal Data Validation foothold for Operation Code Rescue. The June 25 character, world, and physics deep dives repeatedly call for `UEditorValidatorBase` / `UObject::IsDataValid` style gates so imported assets fail in editor or CI instead of surprising the packaged game.

## Editor Module

Added `CodeRescueUnrealEditor` as an editor-only module in `CodeRescueUnreal.uproject`. The module depends on:

- `DataValidation`
- `CodeRescueUnreal`
- `UnrealEd`
- `AssetRegistry`

The runtime game module remains unchanged for package play; the validator lives only in the editor target.

## Native Validator

Added `UCodeRescueAssetManifestValidator`, a `UEditorValidatorBase` subclass using the UE 5.7 signatures:

- `CanValidateAsset_Implementation(const FAssetData&, UObject*, FDataValidationContext&)`
- `ValidateLoadedAsset_Implementation(const FAssetData&, UObject*, FDataValidationContext&)`

It validates `UCodeRescueAssetManifest` assets and fails promotion when required professional-asset references are missing:

- zombie skeletal mesh
- survivor skeletal mesh
- city building mesh list
- barricade mesh list
- muzzle flash VFX
- bullet impact VFX
- fire and smoke VFX
- infection cloud VFX
- radio briefing sound
- zombie attack sound

This started as the first narrow validator rather than an enormous catch-all. The same native Data Validation path now covers promoted character rows, city-kit outputs, physics props, Nanite/HLOD candidates, and Mac compatibility rules.

## CI And Manifest Coverage

Added `Scripts/verify_code_rescue_data_validation_unreal.py` to run in UnrealEditor-Cmd. It confirms the editor module registered `CodeRescueAssetManifest` and `CodeRescueAssetManifestValidator`, scans `/Game` for any manifest assets, and writes `Saved/DataValidation/code_rescue_data_validation_contract.json`.

Added `Content/CodeRescueData/editor_data_validation_contract.tsv` to document the current validator surfaces:

- `UCodeRescueAssetManifestValidator`
- `UCodeRescueZombieVariantTableValidator`
- `UCodeRescuePhysicsPromotionValidator`
- `UCodeRescueWorldPromotionValidator`
- `UCodeRescueMacCompatibilityValidator`

Updated the creative-development plan and human QA checklist so Houdini, Maya, and release-package review all include the editor Data Validation contract.

## Verification

Added `Scripts/verify_editor_data_validation_contract_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

`Run_Full_QA_Audit.command` also runs the Unreal-side smoke script through the existing commandlet helper. Validation for this slice is editor compile, static verifier, UnrealEditor-Cmd smoke, package, and packaged smoke. Package smoke remains important because adding an editor module must not break the runtime app.

Completed verification for this slice:

- `python3 -m py_compile Scripts/verify_editor_data_validation_contract_pass.py Scripts/verify_code_rescue_data_validation_unreal.py`
- `python3 Scripts/verify_editor_data_validation_contract_pass.py`
- Adjacent Mac asset/feature/hair/rendering gate verifiers
- `git diff --check` over the touched validation/module/documentation files
- `./Recompile_Module.command < /dev/null`
- UnrealEditor-Cmd smoke for `Scripts/verify_code_rescue_data_validation_unreal.py`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`

The Unreal smoke wrote `Saved/DataValidation/code_rescue_data_validation_contract.json` and confirmed `/Script/CodeRescueUnreal.CodeRescueAssetManifest` plus `/Script/CodeRescueUnrealEditor.CodeRescueAssetManifestValidator` are registered. It found zero current manifest assets, which is acceptable for this gate: the class path is ready before content promotion begins.

## Boundaries

This does not yet validate every imported character Blueprint, skeletal mesh LOD chain, Physics Asset, Geometry Collection, HLOD proxy, or texture-compression setting. It creates and verifies the editor-only Data Validation lane those future rules will use, starting with the project-level professional asset manifest that points gameplay systems at promoted content.
