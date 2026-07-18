# Mac Hair-Card Compatibility Slice

This pass implements the macOS groom guidance from the June 25 `CHARACTER_ANIMATION_DEEPDIVE`. The project has local `Content/Grooms` assets plus staged `MetaHuman_Downloads` and `Content/Grooms/ArtSource` `.mhpkg` packages, so the game needed an explicit rule that those sources are review material rather than assumed packaged Mac runtime hair.

## Implemented Runtime Contract

`SpawnUnrealSystemsCharacterWorldLayer` now presents the character stage as MetaHuman-ready with `Mac hair-card fallback`. Mika Stone's role and greeting identify the promoted Mac path as MetaHuman body, Maya/Houdini, Control Rig, IK, and hair-card/mesh assets. The same actors receive `MacHairCardRuntimeReady` and `GroomStrandReviewOnlyMac` tags so screenshots, inspectors, and future validators can detect the contract.

`SpawnCreativeRecommendationsLayer` now separates the intake panels into `Groom review` and `Hair cards`. The board text states that groom `.mhpkg` sources stay review-only on Mac, while body plus hair-card/mesh fallback assets are the promotion path.

## Manifest Coverage

Added `Content/CodeRescueData/mac_hair_compatibility_manifest.tsv` with classifications for:

- local `Content/Grooms` strand assets
- staged MetaHuman groom packages
- zombie hair mesh packs
- the Mika Stone novel cast slot
- the creative download intake board

Updated the Unreal systems manifest, novel character manifest, creative inclusion plan, human QA checklist, visual regression targets, and active asset intake scanner/table to use the same language.

## Verification

Added `Scripts/verify_mac_hair_card_compatibility_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the C++ gameplay labels/tags, confirms runtime code does not hard-reference `/Game/Grooms`, confirms `CodeRescueUnreal.uproject` does not explicitly enable HairStrands as a gameplay dependency, classifies local groom and `.mhpkg` sources, and checks every manifest/documentation/QA entrypoint. Package validation remains compile, package, packaged null smoke, and packaged render smoke.

## Boundaries

This does not delete groom assets, convert Alembic strand files into cards, or author final MetaHuman hairstyles. It establishes the playable Mac rule now: strand grooms are source/review inputs, and card or mesh hair is the runtime fallback unless a future Apple GPU validation pass proves otherwise.
