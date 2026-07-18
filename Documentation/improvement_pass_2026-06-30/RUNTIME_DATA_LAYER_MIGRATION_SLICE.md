# Runtime Data Layer Migration Slice

Date: 2026-06-30

## Purpose

This slice continues the `WORLD_DEVELOPMENT_DEEPDIVE` World Partition/Data Layer roadmap and `TOP_50_RECOMMENDATIONS` item 31. The project still has a C++ generated city, so this pass does not pretend authored World Partition maps, Data Layer assets, Level Instances, or OFPA actor files already exist. Instead, it adds a playable runtime bridge that marks the current generated city with the same state, time, and mode boundaries the authored migration must preserve.

## Implemented

- Added `ACodeRescueGameMode::ApplyRuntimeDataLayerTags` so runtime actors can carry stable Data Layer membership tags while the city is still generated in C++.
- Extended `RegisterStreamedActor` with `RuntimeWorldPartitionStreamCell`, `CurrentCppWorldPartitionFallback`, `OneFilePerActorMigrationReady`, and `WorldPartitionReady` tags for every active streamed city actor.
- Added `SpawnRuntimeDataLayerMigrationLayer` to every campaign city after the creative recommendation layer. The station creates five nonblocking review cards: streaming cell, safehouse mode, combat mode, objective state, and weather/time.
- Tagged the protected terminal hub and terminal actor as `RuntimeDataLayer_State_SafeBeat`, `RuntimeDataLayer_Mode_CodingSafehouse`, and `RuntimeDataLayer_Mode_SelectedLanguageOnly`.
- Tagged encounter-director staging as combat/overrun plus route-locked or route-open pressure.
- Tagged solved-terminal world-response actors as rescue-route-open traversal.
- Tagged weather/lighting identity actors with day-night, weather-token, and grade-token Data Layer membership.

## Player-Facing Behavior

Players can walk to the review station in a loaded city and see how the current playable fallback maps to future Data Layers. The station is visual only and nonblocking: it does not bypass the language start screen, alter selected-language saves, change terminal validation, or move combat into protected learning spaces.

## Migration Boundary

This is not the final authored World Partition conversion. Remaining work still includes converting the entry map to World Partition, enabling OFPA actor files, creating real Data Layer assets for Pristine/Overrun, weather, sandbox, and combat modes, moving safehouse and rescue spaces into Level Instances, and validating streaming budgets on Apple Silicon. This slice makes those future seams explicit in the runtime world so later authored content has a contract to match.

## Validation

Run:

```zsh
python3 Scripts/verify_runtime_data_layer_migration_slice_pass.py
python3 Scripts/verify_world_promotion_validation_contract_pass.py
python3 Scripts/verify_weather_lighting_identity_slice_pass.py
python3 Scripts/verify_protected_learning_zone_ai_slice_pass.py
python3 Scripts/verify_adaptive_encounter_director_pressure_slice_pass.py
python3 Scripts/verify_coding_world_response_slice_pass.py
./Recompile_Module.command < /dev/null
./Package_Mac_App.command < /dev/null
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```
