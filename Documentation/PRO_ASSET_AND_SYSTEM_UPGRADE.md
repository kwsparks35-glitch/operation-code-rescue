# Professional Asset + Systems Upgrade

This upgraded Unreal project adds the scaffolding needed for professional-quality assets and deeper gameplay systems while still remaining buildable as a source project.

## What was added

### 1. Professional asset slots

The project now includes Blueprint/C++ slots for:

- animated zombie skeletal meshes
- survivor skeletal meshes
- muzzle flash Niagara VFX
- bullet impact Niagara VFX
- fire/smoke Niagara VFX
- infection aura/cloud Niagara VFX
- radio briefing audio
- zombie attack audio
- real-world-inspired environment asset packs

See:

- `Source/CodeRescueUnreal/CodeRescueAssetManifest.h`
- `Content/CodeRescueData/asset_manifest_template.json`
- `Content/CodeRescueAssets/`

### 2. Zombie and survivor character model support

`ACodeZombieActor` and `ASurvivorActor` still work with primitive fallback shapes, but now expose optional skeletal mesh slots. This means the prototype remains playable immediately, but you can replace the primitives with licensed or personally-created animated characters in Unreal.

### 3. Niagara VFX hooks

The player, zombies, and survivors now expose Niagara hooks for:

- muzzle flashes
- bullet impacts
- hit effects
- death effects
- infection aura effects
- survivor rescue beacon effects

### 4. Persistent save system

The project now includes a `UCodeRescueSaveGame` class and save/load functions in the game instance.

Tracked values include:

- selected language
- survivors rescued
- zombies neutralized
- coding score
- terminals solved
- mission progress structures
- concept mastery structures

See:

- `Source/CodeRescueUnreal/CodeRescueSaveGame.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h/.cpp`

### 5. Deeper curriculum database

The project now includes a JSON curriculum database that can drive coding missions and strategy tips for:

- Java
- C
- Python
- MATLAB
- cross-language debugging

See:

- `Content/CodeRescueData/curriculum_database.json`
- `Source/CodeRescueUnreal/CodeRescueCurriculumLibrary.h/.cpp`

## Important limitation

This project does not bundle paid Marketplace/Fab assets, MetaHumans, or commercial city packs. Import those through Unreal/Fab according to their licenses, then assign them to the exposed Blueprint properties or create a `UCodeRescueAssetManifest` data asset.

## Suggested asset import workflow

1. Open the Unreal project.
2. Import or add your licensed assets through Fab/Marketplace or Content Browser.
3. Place them under `Content/CodeRescueAssets/`.
4. Create Blueprint children of:
   - `ACodeZombieActor`
   - `ASurvivorActor`
   - `ACodeRescueCharacter`
5. Assign skeletal meshes and Niagara systems in the Details panel.
6. Replace C++ spawn classes with Blueprint subclasses in `ACodeRescueGameMode`, or set up a DataAsset-driven spawn table.

## Recommended asset categories

- Zombie character pack with walk/run/attack/death animations
- Civilian/survivor character pack or MetaHuman survivors
- Urban ruin / hospital / harbor / metro environment pack
- Niagara weapon VFX pack
- Niagara smoke/fire/sparks pack
- Radio/ambient horror sound pack

## Best practical next production step

Create these Blueprint subclasses in Unreal:

- `BP_CodeZombie_Animated`
- `BP_Survivor_MedicalWorker`
- `BP_Survivor_Dockworker`
- `BP_Survivor_TransitEngineer`
- `BP_PlayerRescueRanger`
- `BP_Terminal_Java`
- `BP_Terminal_C`
- `BP_Terminal_Python`
- `BP_Terminal_MATLAB`

Then assign the imported professional assets to those Blueprints and use them in the procedural world spawner.
