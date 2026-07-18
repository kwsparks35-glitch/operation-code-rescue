# Friendly Safehouse NPC Service Slice

## Summary

This slice continues the June 25 P0 character guidance by turning the Civilian Support Hub into a clearer, save-backed safehouse service loop. The game already spawned Engineer, Medic, Scientist, and Trader NPCs, but their daily cooldown was actor-local and their HUD prompt only said `[E] talk`. The new path gives each support role a stable service ID, saves used services inside the selected-language profile, restores cooldowns after relaunch, and teaches the loop through HUD prompts and in-world signage.

## Runtime Work

- Added `UsedFriendlyNPCServiceIds` and `bHasFriendlyNPCServiceState` to `UCodeRescueSaveGame`.
- Added `UCodeRescueGameInstance` helpers for safehouse services:
  - `HasFriendlyNPCServiceCooldown`
  - `MarkFriendlyNPCServiceUsed`
  - `ClearFriendlyNPCServiceCooldown`
  - `ResetFriendlyNPCServiceCooldowns`
  - `GetFriendlyNPCServiceSummary`
- Threaded the service list through `ResetRun`, save serialization, load restoration, and world-state apply.
- Added `AFriendlyNPCActor` helpers for:
  - stable service IDs such as `FriendlyNPC_City000_Medic`
  - role display names
  - service summaries
  - role-specific interaction prompts
  - saved cooldown restore
- Updated `AFriendlyNPCActor::Interact` so successful services save immediately to the active language profile after granting the role benefit.
- Updated `UCodeRescueHUDWidget` so friendly NPC crosshair prompts show the exact service or cooldown instead of generic talk text.
- Updated `ACodeRescueGameMode::Tick` so the day-night transition clears saved NPC service cooldowns and then refreshes live actors.
- Updated `SpawnFriendlyNPCsForCity` signage so the support hub, role stations, and NPC labels state that services save per language and reset on the day-night cycle.

## Player-Facing Behavior

- Engineer grants `+1 scrap for repairs`.
- Medic grants `+25 health when injured`.
- Scientist grants `+1 research point`.
- Trader exchanges `5 scrap for +1 research`.
- A used service remains unavailable after closing and reopening the game until the day-night cycle resets it.
- The same support-service state is isolated to the selected language save slot, preserving the start-screen language resume contract.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueSaveGame.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/FriendlyNPCActor.h`
- `Source/CodeRescueUnreal/FriendlyNPCActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Content/CodeRescueData/friendly_safehouse_npc_service_manifest.tsv`
- `Scripts/verify_friendly_safehouse_npc_service_slice_pass.py`

## Validation Plan

- `python3 -m py_compile Scripts/verify_friendly_safehouse_npc_service_slice_pass.py`
- `python3 Scripts/verify_friendly_safehouse_npc_service_slice_pass.py`
- Existing character/world asset verifiers for friendly NPC coverage.
- Module recompile.
- Mac package.
- Packaged null and render smoke.
- Manual QA: use Engineer/Medic/Scientist/Trader in one selected-language run, save/relaunch, confirm cooldowns persist, wait for day-night shift, confirm services refresh, then start or resume a different language and confirm its service state is separate.
