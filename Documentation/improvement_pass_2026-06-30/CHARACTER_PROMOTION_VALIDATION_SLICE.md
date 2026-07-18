# Character Promotion Validation Slice

This pass continues the June 25 character-animation guidance by turning zombie cast promotion into a native editor validation surface. The prior editor Data Validation slice proved the module path; this slice applies that path to the runtime zombie variant DataTable that actually drives spawned enemy silhouettes.

## Native Validator

Added `UCodeRescueZombieVariantTableValidator`, a `UEditorValidatorBase` subclass that validates any `UDataTable` using `FZombieVariantRow`.

The validator enforces:

- non-empty zombie variant rows
- at least one promoted non-fallback row
- readable `DisplayName` values
- 0.1 to 5.0 health, damage, speed, and mesh-scale multipliers
- 0.0 to 10.0 zone-weight values with non-negative zone indexes
- loadable `SkeletalMesh` soft paths for promoted rows
- loadable `AnimBPClass` soft paths for promoted rows
- no `/Game/Grooms`, `GroomStrands`, or `StrandGroom` paths in promoted runtime character references

`Default` and `BaseMesh` remain explicit fallback exceptions. They may omit a mesh or AnimBP, but any assigned fallback path still has to load and still has to avoid Mac strand-groom content.

## Unreal Smoke

Added `Scripts/verify_character_promotion_validation_unreal.py`. It runs in UnrealEditor-Cmd, verifies `CodeRescueZombieVariantTableValidator` is registered, loads `/Game/CodeRescueAssets/DT_ZombieVariants`, mirrors the promotion checks in Python, and writes `Saved/DataValidation/code_rescue_character_promotion_validation.json`.

The JSON report records:

- validator class path
- DataTable path
- total row count
- promoted row count
- explicit fallback rows
- per-row mesh and AnimBP paths
- any validation errors

## Contract Coverage

Added `Content/CodeRescueData/character_promotion_validation_contract.tsv` to define the promotion rules for zombie variants, fallback rows, survivor MetaHuman imports, friendly NPC cast slots, the player rescue operator, and Mac hair compatibility.

Updated `Content/CodeRescueData/editor_data_validation_contract.tsv` so the character validator is no longer listed as a future-only surface. Updated the creative-development plan and human QA checklist so zombie-family variants and Maya character cleanup route through the new commandlet smoke.

## Verification

Added `Scripts/verify_character_promotion_validation_contract_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

`Run_Full_QA_Audit.command` also runs the Unreal-side smoke script through the existing commandlet helper.

Validation passed with:

- `python3 -m py_compile Scripts/verify_character_promotion_validation_contract_pass.py Scripts/verify_character_promotion_validation_unreal.py Scripts/verify_editor_data_validation_contract_pass.py`
- `python3 Scripts/verify_character_promotion_validation_contract_pass.py`
- `python3 Scripts/verify_editor_data_validation_contract_pass.py`
- adjacent Mac asset import, feature capability, and hair-card compatibility verifiers
- scoped whitespace diff check
- `./Recompile_Module.command < /dev/null`
- UnrealEditor-Cmd character-promotion smoke using `Scripts/verify_character_promotion_validation_unreal.py`
- `./Package_Mac_App.command < /dev/null`
- packaged null smoke
- packaged render smoke

The Unreal smoke wrote `Saved/DataValidation/code_rescue_character_promotion_validation.json` and confirmed:

- validator class `/Script/CodeRescueUnrealEditor.CodeRescueZombieVariantTableValidator`
- DataTable `/Game/CodeRescueAssets/DT_ZombieVariants`
- 1 total row
- 1 promoted row
- 0 fallback rows
- 0 validation errors
- promoted row `EliteBoomer`
- skeletal mesh `/Game/UrbanZombie4/Mesh/Separated/SK_UrbanZombie4_Body.SK_UrbanZombie4_Body`
- AnimBP class `/Game/UrbanZombie4/Demo/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C`
- zone weights `0=0.2`, `1=0.3`, `2=0.55`

Both packaged smoke paths also passed the runtime log-contract checks. The only warnings were the already accepted immediate-exit navigation/crowd warnings and the render-mode CoreAudio sample-rate warning.

## Boundaries

This does not yet validate every future survivor Blueprint, player arms mesh, FriendlyNPC cast Blueprint, IK Retargeter, Control Rig asset, or Physics Asset. It closes the highest-risk active enemy promotion path first: a zombie row can no longer silently promote without a mesh, a locomotion AnimBP, sane tuning, and Mac-safe hair boundaries.
