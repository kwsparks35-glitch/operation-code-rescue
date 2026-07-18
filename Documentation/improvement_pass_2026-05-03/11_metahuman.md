# Item 11: MetaHuman Survivors & Rescue Missions

## Summary
Integrate MetaHuman skeletal meshes as unique NPC survivors in city zones. Implement rescue mission logic: players find survivors in safe zones, receive briefing, and optionally defend them during extraction sequences.

## What Changed
- Created ASurvivorNPC class derived from UCharacter with MetaHuman skeletal setup
- Added rescue mission state machine (Idle → Briefing → Defense → Extraction → Complete)
- Implemented survivor dialogue system with context-sensitive lines
- Added extraction timer (60s countdown, triggered by player proximity)

## Design Decisions
1. **MetaHuman Setup**:
   - Use MetaHuman body mesh from Fab content (high-quality proportions)
   - Clothing variants per city (NYC business attire, LA beachwear, Chicago worker gear, Houston industrial)
   - Each survivor has unique face ID (prevents repetition in missions)

2. **Rescue Mission Flow**:
   - Player finds survivor in safe zone (terminal, shelter, building basement)
   - On interaction: survivor plays greeting animation, displays briefing dialogue
   - Player can accept/decline rescue (decline = no mission progress)
   - On acceptance: 60-second extraction timer starts, enemies may spawn
   - Player must defend survivor until helicopter arrives
   - On extraction: survivor boards helicopter, mission complete

3. **NPC AI**:
   - Survivors do not attack but have movement constraints (stay within 500cm of safe zone)
   - Use idle animations during defense phase
   - Play celebration animation on successful extraction
   - No complex pathfinding; survivors follow preset waypoints

## Files Touched
- `Source/CodeRescue/Character/SurvivorNPC.h` (ASurvivorNPC class, rescue state machine)
- `Source/CodeRescue/Character/SurvivorNPC.cpp` (dialogue system, state transitions)
- `Source/CodeRescue/Game/RescueMissionManager.h` (global mission registry)
- `Source/CodeRescue/Game/RescueMissionManager.cpp` (mission tracking, completion rewards)

## Known Limitations
- MetaHuman rigs require Epic skeleton compatibility (hand-rigged for non-standard proportions)
- Dialogue system is text-based, no voice acting
- Extraction sequence hardcoded to helicopter visual + fade-out (no dynamic camera)
- Survivors cannot be revived if killed during defense phase

## Follow-Up Work
1. Implement voice lines for each survivor (text-to-speech or professional VO)
2. Add survivor skill-based interactions (hacker grants terminal access, medic grants healing)
3. Implement dynamic extraction helicopter flight paths per city
4. Add post-rescue dialogue callbacks (survivors provide intelligence on other cities)

## Compiler Notes
**Mac Build Step**: After `./Recompile_Module.command`, verify MetaHuman initialization:
```
UE_LOG(LogMetaHuman, Warning, TEXT("Survivor %s initialized with mesh: %s"), *SurvivorName, *SkeletalMeshPath);
```
MetaHuman assets must be manually imported from Fab content before survivors spawn. No procedural generation.
