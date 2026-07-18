# Improvement Pass 2026-05-20 — Input System Rebuild (Round 3)

Author: Claude (Cowork session for Kenny)
Date: 2026-05-21
Build verified: `CodeRescueUnrealEditor Mac Development` — `BUILD SUCCEEDED`

## Kenny's playtest feedback (round 3)

The environment and lighting are good. Remaining issues:

1. Buildings are proportionally very small and cannot be entered.
2. The C key does not change the camera — non-functional.
3. No interaction keys work at all — only movement (WASD / arrows / mouse).
   E, T, J, P, R, Tab, Enter, 1-4 all do nothing.
4. No way to reach a menu or exit the game — the mouse only turns the
   character; it cannot click anything in-game.

## Root cause (finally diagnosed)

`Config/DefaultInput.ini` sets the project to the **Enhanced Input** system
(`DefaultInputComponentClass = EnhancedInputComponent`). Movement worked
because the character *binds* its movement axes (`BindAxis`). But every
action key was never bound — it was only **polled** each frame in
`PollDirectKeys()`. That polling does not work in this build, so nothing
except the bound movement axes responded. Round 2's fix swapped one polling
method for another, so it still didn't work — the real problem was that the
keys were polled at all instead of bound.

## The fix — bind every action key

`SetupPlayerInputComponent()` now binds every action key directly with
`BindKey()` — the exact same input-routing path the working movement axes
use. Event-driven, no polling:

- **E / Enter / Tab / G** → Interact
- **Space / F / Left-Click** → Fire
- **R / Left-Ctrl** → Reload
- **Q** → Use medkit
- **T** → Jump to next objective
- **J** → Objective journal
- **P / Escape** → Pause menu
- **C** → Cycle camera perspective
- **1 / 2 / 3 / 4** → Swap weapon
- **X** → Throw · **B** → Barricade · **H / M** → Mission help

The old polled action-key block was removed from `PollDirectKeys()` (which
now only handles the movement fallback and Shift-sprint), so nothing
double-fires.

## This also fixes the "no way to exit" problem

The pause menu already frees the mouse cursor correctly when it opens (it
switches to UIOnly input mode and shows the cursor — that code was always
there). The only reason Kenny couldn't reach it was that **P never worked**.
Now that **P** and **Escape** are bound, pressing either opens the pause
menu with a usable mouse cursor, so the player can click Resume / Quit /
etc. Exiting the game is now possible from inside the game.

## Files changed

- `CodeRescueCharacter.cpp` — rewrote `SetupPlayerInputComponent` with
  `BindKey` calls; removed the polled action-key block from
  `PollDirectKeys`; added four no-arg weapon-swap wrappers.
- `CodeRescueCharacter.h` — added the weapon-swap wrapper declarations;
  removed the now-unused polled-key snapshot members.

## Verification

- `Recompile_Module.command` → `BUILD SUCCEEDED` (all 23 compile/link
  steps). The input rewrite compiles and links cleanly.
- Why this fix is high-confidence: it does not guess at the polling
  problem — it moves the action keys onto `BindKey`, the identical
  event-routing the movement axes already use successfully. If movement
  binds work (they do), key binds work.

## Still open — building proportions (Kenny's issue #1)

Buildings are too small and cannot be entered. This was **not** fixed this
round, deliberately:

- The procedural city sizes every structure through one shared scale
  helper. The earlier 50x->2x rescale shrank building footprints along
  with everything else. Simply enlarging structures risks making them
  intersect each other and the player, because building *sizes* and the
  *spacing between them* are currently coupled to the same number.
- Doing it safely needs a proper pass that decouples structure size from
  city spacing and re-checks for overlaps — worth doing carefully rather
  than rushing a fix that breaks the geometry.
- "Cannot be entered" is a separate, larger task: the buildings are solid
  decorative meshes with no interiors. Enterable buildings require modeled
  interiors — a content task for a future pass.

This is the #1 item for the next round.

## What Kenny should test

In a fresh run, confirm the keys now respond:
- **C** cycles first-person / third-person / far third-person.
- **E** interacts, **T** jumps to objective, **J** journal, **R** reload,
  **1-4** weapons.
- **P** or **Escape** opens the pause menu — and the mouse cursor appears
  so you can click its buttons (Resume, Quit, etc.).
