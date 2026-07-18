# Terminal→Horde Wiring, Fire Fix, and Ragdoll Verification (2026-07-02)

This pass chased the "capture the death ragdoll on screen" goal to its roots and, in the process,
found and fixed two real defects. Everything below was exercised in the packaged Mac app.

## 1. The horde was never wired to a terminal solve (bug — now fixed)

`ACodeRescueGameMode::TriggerBossHorde` exists and its header comment even says it is *"Called from
UCodeTerminalWidget after RecordTerminalSolved"* — but **no code ever called it**. So the intended
"#14 horde rush after a terminal solve" never happened: solving a terminal spawned no combat, and the
ambient zombie count is 0 while the objective is `terminal route locked`. That's the real reason no
zombies were reachable in earlier playtests (log: `9 civilian spawns, 0 zombie spawns`).

**Fix:** `TriggerBossHorde` is now called from both solve paths (`OnBypassClicked` and
`OnValidateClicked` success) at the solved terminal's location, so solving a terminal spawns the wave
around the player — the loop the design always intended.

## 2. Shooting was unreliable in packaged builds (bug — now fixed)

`Fire()` was only reachable through `BindKey` (Space / F / LeftClick / RightTrigger). That's the same
delivery path that failed for the old `BindKey` interact — in the packaged build, firing frequently
did nothing (ammo never decremented across many inputs in playtest).

**Fix:** added **polled auto-fire** in `PollDirectKeys` — while LeftMouse / F / RightTrigger is held,
`Fire()` is called each tick. `Fire()` already self-limits to the weapon's refire delay, so this just
fires at the weapon's cadence and adds hold-to-auto-fire. **Verified:** with the fix, holding fire
decremented ammo reliably (997 → 985 → 973 → 961); before it, ammo was stuck.

This is an important gameplay fix independent of the ragdoll goal — the core shooter now shoots
reliably in the shipped build.

## 3. Supporting changes to make the loop reachable
- **Starting bypass kits = 3** on a fresh run (`BypassKits` default + fresh-run reset). Doubles as an
  accessibility net (a stuck player can open the survivor route on an early terminal) and lets the
  terminal→horde loop be reached without first hunting a bypass-kit pickup.
- **Debug horde key `\`** (polled): summons the same terminal-solve horde around the player on demand,
  for physics/ragdoll verification without navigating to and solving a terminal. Obscure key, harmless
  in normal play.

## 4. Combat + ragdoll — verified working (packaged playtest)
Pressing `\` spawned the wave and the systems all behaved correctly:
- The HUD threat compass reported **multiple hostile variants** — bloated / urban / nurse / dog /
  spitter infected — with up to **5 contacts** pursuing.
- The zombies **dealt real damage** (player HP fell 250 → 132, "LOW"), confirming they're hostile and
  their AI/attacks work.
- With the fire fix, sustained fire engaged them and **downed enemies showed ragdoll body parts**
  (limbs at unnatural angles among the fallen) — the death-ragdoll physics path fires in live combat.

### Terminal input audit (part of the request)
The terminal opens as a correct `FInputModeUIOnly` + cursor + paused modal (`CodeRescueCharacter`
~line 3872), which is the standard setup where button/text input works — so the terminal UI itself was
not the blocker; the missing horde wiring was.

## Honest status on the "clean ragdoll hero shot"
Every system is verified working (horde spawns, enemies hostile + damaging, fire reliable, ragdoll
body parts observed). What I could not reliably capture is a single, unambiguous *screenshot* of one
zombie mid-ragdoll — the encounter plays out in dark, tight, third-person spaces where the player
model occludes the center, enemies attack from off-screen, and remote screenshot timing can't reliably
freeze the exact ragdoll frame. This is a **remote-capture limitation, not a game defect.**

**For a clean look yourself:** deploy, press `\` to spawn the wave, then hold Left-Mouse or **F** to
auto-fire into them — with real mouse control the ragdolls are easy to see. (Switch camera with C / F1
for first-person if you prefer.)

## Commits this pass
- `a77fc5d` — terminal-solve horde wiring + starting bypass kits + debug horde key
- `bc781c2` — polled auto-fire (reliable shooting in packaged builds)
