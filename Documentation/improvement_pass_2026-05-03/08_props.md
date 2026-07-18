# Item 8: Authored Environment Props for Cities

## Summary
Implemented deterministic environment prop spawning for each city: ACodeRescueGameMode::SpawnAuthoredPropsForCity() spawns hardcoded props (benches, sandbags, vehicles, streetlights) at fixed offsets in each city's zone. All coordinates use 50x scaling for city-scale geometry. Props are static actors with simple collision.

## What Changed
- **NEW: CodeRescueGameMode.cpp SpawnAuthoredPropsForCity()**: Massive switch statement per city (Anchorage, Seattle, Tokyo, London, Singapore, Sydney); each case creates prop array with name, position (50x scaled), rotation, scale.
- **CodeRescueGameMode.cpp SpawnWorld()**: After terrain/zombies, calls SpawnAuthoredPropsForCity(CurrentCityIndex) to place props.
- **NEW: Simple Static Mesh References**: Props are hardcoded UPROPERTY references to Unreal default meshes (e.g., `/Engine/BasicShapes/Cube`) or third-party asset package references (e.g., Urban Props pack).
- **No New Classes**: Props are vanilla AStaticMeshActor with default collision.

## Design Decisions
1. **Hardcoded Spawning**: Props array defined in code, not in DataTable, for tight control and determinism across save reloads.
2. **City-Specific Theming**: Anchorage has industrial props (metal containers, sandbags); Seattle has urban clutter (benches, fences); Tokyo has neon signage and futuristic elements (configurable meshes); London has brick structures; etc.
3. **50x Scaling**: All coordinates scaled by 50 (UE unit = 50 cm, so 1 UE unit = 25 meters in-game). Props positioned accordingly.
4. **Simple Collision**: Props use box/sphere collision from mesh, no custom collision shapes.
5. **Static Rendering**: Props are static actors, allowing Unreal to bake them into HLOD/LOD for performance.

## Files Touched
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` (+SpawnAuthoredPropsForCity function)
- **Config Files** (optional): Props can reference asset packs in Engine.ini or project DefaultEngine.ini if using async loading.

## Known Limitations
1. **Hardcoded Coordinates**: Props positions are baked into code; editing requires recompilation. Future: move to DataTable or JSON config.
2. **No Dynamic Asset Loading**: Props reference fixed UE default meshes or project assets. No async loading for large asset packs.
3. **No Variation**: Same props spawn every playthrough; no randomization.
4. **Collision Overhead**: All props have collision enabled. High prop count may impact performance (mitigated by Item 20 HISM pass).
5. **No Destruction**: Props cannot be destroyed or damaged by player actions.

## Follow-Up Work
1. Move prop definitions to DataTable (Anchorage_Props, Seattle_Props, etc.).
2. Implement dynamic asset streaming for third-party prop packs.
3. Add prop variation (random rotations, scale jitter).
4. Implement destructible props (barrels explode on impact, crates break).
5. Create props as HISM instances for massive batch rendering (Item 20).

## Compiler Notes
If using third-party assets, ensure referenced packs are mounted in Content/ and referenced via soft asset pointers (TSoftObjectPtr<UStaticMesh>).
