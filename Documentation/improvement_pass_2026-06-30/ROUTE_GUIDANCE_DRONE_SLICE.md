# Animated Route Guidance Drone Slice

Date: 2026-06-30

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: called for clearer authored route readability and stronger player guidance through rescue spaces.
- `CHARACTER_ANIMATION_DEEPDIVE.pdf`: called for more motion in the scene and better readability around action beats.
- `TOP_50_RECOMMENDATIONS.pdf`: emphasized stronger feedback after player actions and clearer progression cues.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasized reviewable, demo-ready systems that survive packaging and save/load paths.

## Implementation

Added `ARescueRouteGuidanceDroneActor`, a cook-safe animated wayfinding actor built entirely from engine primitives. Each drone contains:

- compact drone body
- glowing nose marker
- two rotating rotor arms
- hanging signal panel
- route-guidance point light

`ACodeRescueGameMode::RevealSolvedTerminalRescueRoute()` now spawns one drone per solved rescue route segment after a coding terminal is completed. The drones patrol between the same route points used by the existing pulse segments and rescue beacons, making the route feel active rather than purely static.

The drones are registered through the existing solved-route response contract:

- `RegisterStreamedActor(Drone)`
- `TagSolvedRoute(Drone)`
- terminal-specific `SolvedRoute_...` duplicate protection
- save reconstruction through the existing `RevealSolvedTerminalRescueRoute(..., bFromLoad=true)` path

## Tags

Each drone is tagged for audit and future replacement:

- `RescueRouteGuidanceDrone`
- `SolvedRouteGuidanceDrone`
- `RescueRouteGuidanceDroneLayer`
- `AnimatedWayfinding`
- `CodingToRescueWorldResponse`
- `TerminalSolvedRouteVisible`
- `CharacterAnimationDeepDive`
- `WorldDevelopmentDeepDive`
- `Top50Recommendations`

## Accessibility

The actor reads the same reduced motion setting used elsewhere in the game. In standard mode, drones patrol and rotor arms spin quickly enough to communicate urgency. In reduced motion mode, patrol speed, hover bobbing, and rotor motion are dampened while the color and light cues remain visible.

## Player Impact

When the player solves a coding terminal, the world now responds in three connected layers:

- static pulse route and beacon markers
- language-specific breach encounter and reward cache
- animated guidance drones that lead from the terminal toward the survivor/extraction path

This improves the coding-as-rescue loop without changing the saved progression contract or requiring imported drone assets.

## Verification

Added `Scripts/verify_route_guidance_drone_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks:

- drone actor class and route tuning fields
- body, nose, rotors, signal panel, and light components
- runtime patrol, rotor, light, and reduced motion behavior
- solved rescue route integration in `RevealSolvedTerminalRescueRoute`
- streamed cleanup and terminal-specific solved-route tagging
- documentation and QA wiring
