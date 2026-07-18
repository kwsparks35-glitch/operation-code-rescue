# Physics Promotion Validation Slice

This pass continues `GAME_PHYSICS_DEEPDIVE` by converting physics promotion from a future-only recommendation into a native editor validation and commandlet smoke path. The focus is the content that can damage playability fastest as the city grows: zombie ragdoll Physics Assets, destructible-cover debris, throwable impulse props, future Geometry Collections, and the Mac active-body budget.

## Native Validator

Added `UCodeRescuePhysicsPromotionValidator`, a `UEditorValidatorBase` subclass registered by the existing `CodeRescueUnrealEditor` module.

The validator now covers:

- `UPhysicsAsset` assets with authored simple bodies
- runtime zombie ragdoll candidates with at least six bodies
- runtime zombie ragdoll candidates with joint constraints
- `GeometryCollection` assets as a visible manual gate for fixed fracture seed, live-piece budget, cached set-piece path, and sleep/disable retirement evidence

The Geometry Collection branch is intentionally conservative for the current project state: no promoted Geometry Collections exist yet, so the validator surfaces the required review contract without blocking current package flow.

## Runtime Physics Hygiene

`ABarricadeActor` now carries the same Mac physics-budget gate in runtime tags that the asset manifests already expose. The barricade body is tuned with the deep-dive mass/damping values for heavy cover, and debris chunks now add `ChaosDebrisSleepDisableFallback`.

Each debris chunk still gets a readable Chaos burst on break, but `ScheduleDebrisSleepDisable` retires it after `DebrisSleepDisableDelay` by:

- putting the rigid body to sleep
- zeroing linear and angular velocity
- disabling physics simulation
- switching collision to query-only
- tagging the chunk with `ChaosDebrisSleepDisabled`

This is the lightweight runtime equivalent of the deep dive's Sleep/Disable Fields recommendation until authored Geometry Collections become the promoted destruction path.

## Unreal Smoke

Added `Scripts/verify_physics_promotion_validation_unreal.py`. It runs in UnrealEditor-Cmd, verifies `CodeRescuePhysicsPromotionValidator` is registered, loads `/Game/CodeRescueAssets/DT_ZombieVariants`, and checks promoted zombie rows for ragdoll readiness.

The smoke writes `Saved/DataValidation/code_rescue_physics_promotion_validation.json` with:

- validator class path
- promoted zombie row physics readiness
- skeletal mesh path
- assigned Physics Asset path
- Physics Asset body and constraint counts
- sampled zombie Physics Asset inventory
- Geometry Collection count
- source/config checks for substepping, physical surfaces, collision channels, debris retirement, and ragdoll asset gates
- validation errors, if any

## Contract Coverage

Added `Content/CodeRescueData/physics_promotion_validation_contract.tsv` to define promotion rules for zombie ragdoll assets, promoted zombie DataTable physics, destructible cover debris, Geometry Collections, throwable radial impulse props, the future Jeep Chaos vehicle path, and Mac active physics budgets.

Updated `Content/CodeRescueData/editor_data_validation_contract.tsv` so physics is no longer listed as a future-only validator surface. Updated the creative-development plan, Mac asset import budget gate, performance budget, and human QA checklist so physics promotion validation is visible in the normal review trail.

## Verification

Added `Scripts/verify_physics_promotion_validation_contract_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

`Run_Full_QA_Audit.command` also runs the Unreal-side smoke script through the existing commandlet helper.

Validation passed with:

- `python3 -m py_compile Scripts/verify_physics_promotion_validation_unreal.py Scripts/verify_physics_promotion_validation_contract_pass.py`
- `python3 Scripts/verify_physics_promotion_validation_contract_pass.py`
- adjacent editor Data Validation, character promotion, Mac asset budget, destructible cover, and zombie death physics verifiers
- `./Recompile_Module.command < /dev/null`
- UnrealEditor-Cmd physics-promotion smoke using `Scripts/verify_physics_promotion_validation_unreal.py`
- `./Package_Mac_App.command < /dev/null`
- packaged null smoke
- packaged render smoke

The Unreal smoke wrote `Saved/DataValidation/code_rescue_physics_promotion_validation.json` and confirmed:

- validator class `/Script/CodeRescueUnrealEditor.CodeRescuePhysicsPromotionValidator`
- 1 promoted zombie physics row
- promoted row `EliteBoomer`
- skeletal mesh `/Game/UrbanZombie4/Mesh/Separated/SK_UrbanZombie4_Body.SK_UrbanZombie4_Body`
- Physics Asset `/Game/UrbanZombie4/Mesh/Phy_UrbanZombie4_PhysicsAsset.Phy_UrbanZombie4_PhysicsAsset`
- 17 Physics Asset bodies
- 16 Physics Asset constraints
- `ragdoll_promotion_ready=true`
- 23 total project Physics Assets
- 20 sampled zombie-related Physics Assets
- 0 current Geometry Collections
- substepping, 60 Hz substep delta, physical surfaces, custom trace channels, debris retirement, and ragdoll asset gates all present
- 0 validation errors

Both packaged smoke paths passed runtime log-contract checks. The only warnings were the already accepted immediate-exit navigation/crowd warnings and the render-mode CoreAudio sample-rate warning.

## Boundaries

This does not yet author final Geometry Collections, Chaos Vehicle wheel assets, cached destruction set pieces, or CVD determinism diff captures. It establishes the validation gate those assets must pass before they can replace the current fallback implementations.
