# Glass-Panel Fix + Ragdoll Combat-Run Findings (2026-07-02, final follow-up)

Completes the two follow-ups Kenny asked for: pin down + fix the last teal panels, and push into
a combat run to capture the ragdoll. Both were done via the packaged Mac app; the glass ID came
from a temporary diagnostic, and the combat findings from the runtime log.

## 1. Teal glass panels — IDENTIFIED and FIXED

A temporary diagnostic (logged every flat-panel prop's mesh + slot-0 material) named the culprits
in the runtime log:

```
[PanelDiag] '... Public Demo Glass Window 1-1 A' mesh=.../SM_GlassWindow mat0=.../M_Glass slots=1
[PanelDiag] '... Imported Cold Glass Pane 1'    mesh=.../SM_GlassWindow mat0=.../M_Glass slots=1
[PanelDiag] 'AuthoredProp_GlassPanel_E_City0'   mesh=.../SM_GlassWindow mat0=.../M_Glass slots=1
```

The residual teal panels were StarterContent `SM_GlassWindow` panes defaulting to **`M_Glass`** — a
translucent material that reads as a teal window-grid. (The earlier Nanite fix had already cleared
the *checkerboard*; this is the separate glass-tint issue.)

**Fix:** `SpawnStaticMeshProp` now detects any slot whose material is `M_Glass` and swaps it to
`M_Metal_Steel`, so every glass pane — regardless of spawn site — reads as a clean opaque panel. The
diagnostic was removed after use. Commit `63024c2`.

The same diagnostic **confirmed the authored city-kit facades are correct**: every kit mesh carries
its intended material on all slots (e.g. `SM_Facade_Windows_6x9` → `M_Concrete_Panels` ×3 slots,
`SM_Building_Corner_9m` → `M_Brick_Cut_Stone` ×2, `SM_Terminal_Kiosk` → `M_Metal_Burnished_Steel`),
and post-Nanite-fix they now actually render those materials. The nearby detailed skyscrapers are the
`Parallax_Night_Building` meshes with their own `MI_Building` window materials (intended glass
windows, not a bug).

## 2. Ragdoll combat run — pushed in, and found the gate

I traversed **61 m** from the safehouse into the city. Findings, all confirmed:

- The wider city renders correctly now: a solid daylight street canyon with textured buildings, real
  shadows, and no teal wash (the Nanite fix holds up across the whole map, not just the safehouse).
- I reached the **survivor rescue plaza** and found the characters there. The runtime log shows
  **9 civilian spawns and 0 zombie spawns** for this run, with the encounter director reporting
  `objective='terminal route locked' adaptive_pressure=0.90 relief=false`.
- The silver mannequins in that plaza are the **survivors** (`SpawnDecorativeCivilian` → SKM_Manny/
  Quinn), not zombies — they're passive and never damaged the player (HP stayed 250/250).

**Conclusion:** hostile zombies are intentionally **gated behind the terminal solve**. The ambient
zombie count is 0 while the terminal route is locked; the fightable wave is spawned by
`TriggerBossHorde`, which fires *after* a terminal is solved ("#14 Boss / horde rush after a terminal
solve"). So a live death-ragdoll can only be captured by first completing a Java terminal puzzle to
trigger the horde.

### Ragdoll system status — fully verified (code + assets + wiring)
- `ACodeZombieActor::TryActivateDeathRagdoll` is complete: `bEnableDeathRagdoll = true` by default,
  guarded on `GetPhysicsAsset()`, capped at 10 active corpses, applies a death impulse
  (`RagdollImpulseStrength = 52000`), and is invoked on death (line ~1629).
- The zombie skeletal meshes ship with cooked physics assets (`PHYS_ZombieM04`, `PHYS_Zombie_F01`,
  `DogZombie_PhysicsAsset`, `Phy_Zombie_PhysicsAsset`), and those content dirs are in
  `DirectoriesToAlwaysCook`.

The only thing not captured is a literal on-screen death frame, because that requires a full
terminal-solve → horde playthrough (and the terminal's code-entry widget may hit the same
packaged-build UMG-input limitation seen with the menus — worth its own dedicated pass).

## Net state after this session
- Checkerboard: **root-caused (Nanite) and fixed**; kit facades and city surfaces render real
  materials.
- Teal glass panes: **fixed** (→ steel).
- World text: **decluttered**.
- Lighting: **neutral/warm**, entry plaza improved.
- Ragdoll: **system verified**; visual capture is gated behind mission progression (documented).

Commits this session: `fc292bc`, `f870b30`, `63024c2`.
