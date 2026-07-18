# Item 9: Post-Processing & Visual Effects

## Summary
Implement basic post-process volume support for visual fading/transitions and screen shader effects (bloom, color grading, vignette) to enhance atmosphere during zombie encounters and environmental hazards.

## What Changed
- Added post-process volume scaffolding in ACodeRescueGameMode::InitializeLevel()
- Created APostProcessVolume actor references in game world
- Established framework for enabling/disabling post-process effects based on game state
- Implemented basic bloom and color grading parameters

## Design Decisions
1. **Effect Triggers**: Post-process effects activate on:
   - Player entering heavy zombie density zones (red tint, bloom)
   - Environmental hazards (vignette at screen edges)
   - Story transitions (fade-in/fade-out with color grading)

2. **Performance**: Effects use low-cost volume approach rather than camera shader:
   - Post-process volumes are world-space, not per-frame
   - Only active volumes apply effects
   - Effect parameters blended over 0.5s transitions to avoid pop-in

3. **Customization**: Each city has dedicated post-process volume:
   - New York: Heavy bloom, desaturated color palette
   - Los Angeles: Subtle vignette, warm color grading
   - Chicago: High contrast, cool blue tint
   - Houston: Hazard orange warning tint

## Files Touched
- `Source/CodeRescue/Game/CodeRescueGameMode.h` (PostProcessVolumeMap declaration)
- `Source/CodeRescue/Game/CodeRescueGameMode.cpp` (InitializeLevel post-process setup)
- `Source/CodeRescue/Character/CodeRescueCharacter.h` (CurrentPostProcessState enum)
- `Source/CodeRescue/Character/CodeRescueCharacter.cpp` (UpdatePostProcessEffects method)

## Known Limitations
- Post-process effects are volume-based, not dynamic per frame
- Transition blending requires manual timeline management
- Effects do not follow player between cities (volume-based containment)
- Screen shake effects not yet integrated with post-processing

## Follow-Up Work
1. Integrate post-process blending with UI fade transitions
2. Add screen shake callbacks to post-process intensity scaling
3. Implement dynamic bloom scaling based on zombie proximity
4. Add particle effect integration with post-process color spaces

## Compiler Notes
**Mac Build Step**: After `./Recompile_Module.command`, verify post-process volumes load:
```
UE_LOG(LogPostProcess, Warning, TEXT("Post-process volume initialized for city: %s"), *CurrentCityName);
```
No Blueprint changes required. Post-process volumes already exist in UE5 engine.
