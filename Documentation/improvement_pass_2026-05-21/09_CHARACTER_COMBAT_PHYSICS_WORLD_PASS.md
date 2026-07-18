# Improvement Pass 2026-05-21 — Character, Combat, and Physics-World Pass

Author: Codex
Date: 2026-05-21 local session

## Goal

Continue improving playable characters, NPCs, zombies, boss enemies, and the
virtual world so the game feels more recognizable, functional, and grounded in
physical rules while still giving the player a forgiving, positive learning
experience.

## Improvements Completed

- Player movement now tracks real falling speed while airborne.
- Player landings now apply fall damage only after a physically meaningful
  speed threshold, so ordinary jumps remain safe while major drops matter.
- Added a training landing assist: if a hard fall would be lethal, the player
  is held at 1 health and stamina is drained instead of instantly failing.
- Enemy hits now apply a small physical knockback impulse to the player, making
  zombie contact read as a body impact instead of only a number change.
- Added a short damage mercy window after enemy contact so stacked hits do not
  instantly erase the player during close-quarters learning moments.
- Corrected magazine firing behavior: shots now consume magazine ammunition
  without incorrectly adding fired rounds back into reserve ammo.
- Zombies now telegraph close-range attacks by shifting their infection light
  from green to bright red shortly before impact.
- Charger zombies now properly reset their dash cooldown when a dash starts.
- Boss health phases now scale from the boss's actual spawned max health,
  including mission-tier and variant multipliers.
- Boss phase-two regeneration now caps at the boss's actual max health instead
  of a hard-coded 600 health value.
- Boss phase-three add spawns are now capped, tracked, and assigned unique IDs,
  preventing endless duplicate-ID spawn pressure.
- Added a `Physics Traversal Yard` to every generated city:
  - collision-enabled ramps,
  - raised platform,
  - cover blocks,
  - gravity gauge,
  - soft landing assist pad,
  - floating training labels.
- Added a reusable rotated-block world-spawn helper so future physically
  readable ramps and angled set-pieces can be built without ad hoc actor code.

## Design Direction

- The world should mostly obey physical expectations: collision blocks movement,
  ramps have angles, falling has consequences, attacks push the player, and
  cover occupies real space.
- The game should still protect learning momentum: hard landings can hurt, but
  the training assist prevents surprise instant failure; enemy hits push and
  damage the player, but the mercy window gives a little recovery latitude.
- The player's review path now has three readable world-improvement anchors:
  the enterable civic safehouse, the mission diorama chain, and the physics
  traversal yard.

## Files Updated

- `Source/CodeRescueUnreal/CodeRescueCharacter.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeZombieActor.h`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`
- `Source/CodeRescueUnreal/BossZombieActor.h`
- `Source/CodeRescueUnreal/BossZombieActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Run_Character_World_Demo.command`
- `progress.md`

## Demo Review Checklist

Open:

`/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Run_Character_World_Demo.command`

Review:

1. Find the `PHYSICS YARD` label near the early mission route.
2. Walk over the ramps and platform; they are real collision set-pieces.
3. Drop from the platform and confirm the experience is forgiving rather than
   punishing.
4. Let a zombie approach and watch for the red infection-light attack tell.
5. Fight a boss and confirm phase messages/add pressure feel capped rather than
   endless.
6. Fire several shots, reload, and confirm ammo no longer increases from
   shooting.

## Verification

Commands run from:

`/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix`

```bash
ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
"$ENGINE_ROOT/Build/BatchFiles/Mac/Build.sh" CodeRescueUnrealEditor Mac Development -Project="$(pwd)/CodeRescueUnreal.uproject" -WaitMutex
"$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" -run=pythonscript -script="$(pwd)/Scripts/verify_character_world_assets.py" -unattended -NoSound -NullRHI -NoLoadStartupPackages -log
"$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" -game -NullRHI -NoSound -Unattended -NoRadioVoice -ExecCmds="Quit" -log
```

Results:

- Editor build: succeeded.
- Character/world asset verification: succeeded with 0 errors.
- Headless runtime smoke: exited cleanly with code 0.
- Remaining warnings are the same known optional bridge/cvar warnings from
  prior passes.
