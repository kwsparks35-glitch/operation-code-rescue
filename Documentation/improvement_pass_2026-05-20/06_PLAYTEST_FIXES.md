# Improvement Pass 2026-05-20 — Playtest Fixes (Round 2)

Author: Claude (Cowork session for Kenny)
Date: 2026-05-20
Build verified: `CodeRescueUnrealEditor Mac Development` — `BUILD SUCCEEDED`

## Kenny's playtest feedback

After playing the visibility-overhaul build, Kenny reported four issues:

1. The game opened completely black and only became illuminated after a few
   minutes of play.
2. Only the movement controls worked — WASD, mouse, and arrow keys. No
   interaction keys (E, T, J, P, etc.) did anything.
3. The world was a "seemingly endless field" with no visible objectives,
   reference points, or characters to interact with.
4. Request: add the option to view the player from multiple camera
   perspectives instead of only first-person ("through the character's
   eyes").

All four are addressed below.

## Fix 1 — Interaction inputs now work (E / T / J / P / R / 1-4 / etc.)

**Cause.** The player polls keys directly each frame. Movement used
`IsInputKeyDown()` (which works), but every interaction key used
`WasInputKeyJustPressed()`, which did not fire reliably in the running
game — so only movement responded.

**Fix.** Replaced `WasInputKeyJustPressed()` with our own press detection:
each frame `PollDirectKeys()` snapshots every action key via
`IsInputKeyDown()` (the call that works) and diffs it against the previous
frame to detect fresh presses. The snapshot runs before any early-return so
no press is missed. Files: `CodeRescueCharacter.h`, `CodeRescueCharacter.cpp`.

Reload is now also bound to **R** (in addition to the existing keys).

## Fix 2 — The empty field: player now spawns inside the content

**Cause.** `FCodeRescueCampaign::GetCitySpanScale()` returned **50.0**. A
prior change had expanded every city 50×, which scattered terminals,
survivors, NPCs, and the objective route kilometres apart. The player
spawned roughly 320 metres from the first objective, so the world read as
an endless empty field.

**Fix.** Changed the span scale from **50.0 → 2.0**. Every position routes
through this one function, so the whole city contracts uniformly: the
objective route, its characters, terminals, and set-pieces now sit within
a compact ~130 m area the player can see and reach from spawn.
Player movement speed was lowered to match (`WalkSpeed` 9000 → 900 cm/s) —
9000 was superhuman and only existed to cope with the broken 50× city.
Files: `CodeRescueCampaign.cpp`, `CodeRescueCharacter.h`.

## Fix 3 — Bright from the first frame (no more dark open)

**Cause.** `SpawnWorld()` enabled Lumen global illumination. On Mac, Lumen
takes many seconds (sometimes minutes) to converge — so the world stayed
black until it finished, which is exactly the "dark for a few minutes"
Kenny saw.

**Fix.** Disabled Lumen GI and Lumen reflections; the scene is now lit
instantly by direct sunlight plus a strong SkyLight ambient. Virtual Shadow
Maps were also disabled (they spammed a performance warning on the
procedural geometry). Less "photoreal", but reliably visible from frame
one — the right trade for an educational game on school Macs.
File: `CodeRescueGameMode.cpp`.

## Fix 4 — Multiple camera perspectives

**Added.** A spring-arm + third-person camera rig on the player. Press **C**
in game to cycle three perspectives:

- **First-person** — the original "through the character's eyes" view.
- **Third-person** — camera behind the shoulder.
- **Third-person (far)** — pulled further back for a wider view.

The spring arm runs a collision test so the camera never clips through
walls. The player also now has a visible mannequin body (loaded from the
project's existing `SKM_Manny` asset) that shows in the third-person modes
and is hidden in first-person. Files: `CodeRescueCharacter.h`,
`CodeRescueCharacter.cpp`.

## Verification

- **Compile:** `Recompile_Module.command` → `BUILD SUCCEEDED`. All 25
  compile/link steps passed.
- **Runs:** the game launches and runs (confirmed alive and active in
  Activity Monitor — not hung, not crashed).
- **Fix 2 & 3 visually confirmed live:** the game now opens **bright**
  (daytime, blue sky, lit ground) and the player spawns **right next to the
  content** — a zombie, a glowing objective beacon, structures, and
  floating objective text were all visible at spawn. This is a dramatic
  change from the black, empty field.
- **Fix 1 & 4 (input + camera):** coded and compile-verified. They could
  not be confirmed through this session's screenshot tooling — the editor
  game window does not reliably surface live frames to screen capture when
  it is not being actively played. They are best confirmed by Kenny in a
  normal play session (see below).

## What Kenny should test

In a fresh run, please confirm:

- **C** cycles the camera: first-person → third-person → far → back.
- **E** interacts (at a terminal it should open the code editor).
- **T** jumps to the next objective; **J** opens the journal; **P** pauses.
- **R** reloads; **1 / 2 / 3 / 4** swap weapons.
- The world is bright immediately and objectives/characters are nearby.

If any key still does nothing, tell me exactly which — that would point to
a deeper input-routing issue to chase next.
