# Collision Channel Gameplay Contract Slice

Date: 2026-06-30

## Source Guidance

- `GAME_PHYSICS_DEEPDIVE.md`: calls out custom object channels for player, zombie, cover, and pickup objects, plus dedicated trace channels for weapon, AI sight, and interaction behavior.
- `TOP_50_RECOMMENDATIONS_2026-06-25.md`: recommendation 21 makes the collision channel scheme a P0 physics item.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keeps packaged Mac playability and auditable validation ahead of hidden runtime assumptions.

## Implementation

- Added `CodeRescueCollisionChannels.h` as the single source for the project channel mapping.
- Switched player weapon fire and elite spitter acid from `ECC_Visibility` to `CodeRescueCollision::WeaponTrace`.
- Switched AI line-of-sight and zombie barricade checks from `ECC_Visibility` to `CodeRescueCollision::AISightTrace`.
- Switched the player interaction trace to `CodeRescueCollision::InteractionTrace`.
- Marked terminals, survivors, friendly NPCs, pickups, case files, helipads, and the Jeep as explicit interaction trace targets.
- Marked the player capsule, zombie capsule, barricades, pickups, and case files with custom object-channel setup and readable QA tags.

## Player Impact

- Combat hits, zombie perception, and E prompts now have separate collision paths, which reduces false positives as the city gains more glass, foliage, props, and destructible cover.
- Interactions stay focused on usable gameplay objects while cover and enemies keep their own combat/perception roles.
- Future asset imports can be validated against a named project contract instead of inheriting broad default profiles by accident.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueCollisionChannels.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueAIController.cpp`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`
- `Source/CodeRescueUnreal/BarricadeActor.cpp`
- `Source/CodeRescueUnreal/PickupActor.cpp`
- `Source/CodeRescueUnreal/CaseFilePickupActor.cpp`
- `Source/CodeRescueUnreal/CodingTerminalActor.cpp`
- `Source/CodeRescueUnreal/SurvivorActor.cpp`
- `Source/CodeRescueUnreal/FriendlyNPCActor.cpp`
- `Source/CodeRescueUnreal/HelipadActor.cpp`
- `Source/CodeRescueUnreal/JeepActor.cpp`
- `Content/CodeRescueData/collision_channel_gameplay_contract_manifest.tsv`
- `Scripts/verify_collision_channel_gameplay_contract_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Validation

- Static verifier: `python3 Scripts/verify_collision_channel_gameplay_contract_slice_pass.py`
- Adjacent verifiers: creative physics world, protected learning zone AI, selected-language terminal flow, tactical gear pickups, case-file collectibles, standard pursuit zombies, destructible cover, surface impact physics, physics promotion contract.
- Build/package/smoke should be run because this changes runtime trace channels and component collision responses.

## Human QA Notes

- Start a selected-language run and confirm E targeting still opens the start flow, terminal, survivor, friendly NPCs, case files, pickups, helipad, and Jeep interactions.
- Fight zombies near cover and confirm weapon fire still damages zombies and barricades while AI pursuit remains readable.
- Confirm the Jeep surface probe still works; it intentionally keeps a terrain/material probe separate from the combat, sight, and interaction trace migration.
