# Art Development Pass — 2026-07-01

Kenny asked for character and world art development. The honest constraint was never model
capability — it's environment: no Blender is installed on the Mac (the Blender MCP can't connect),
and the Unreal editor was closed during this pass. So this pass delivers art in three layers:
real mesh assets generated now, a complete character pipeline that runs the moment Blender exists,
and queued editor commands that import and stage everything automatically on next open.

## Layer 1 — Modular city kit: REAL meshes, on disk now (Top-50 item 29, P0)

`RawArt/CityKit/*.glb` — 10 structurally validated glTF-binary static meshes, authored
procedurally (recessed-window facade, storefront with door recess + awning + glow strip, corner
pilaster, roof parapet, jersey barricade with hazard cap, coding-terminal kiosk with emissive
screen, extraction arch with emissive strip, planter, lamp post, seeded rubble pile). Flat-shaded,
2–4 PBR materials each (concrete/trim/glass/emissive accents), ~1,150 tris total — deliberately
lightweight, readable survival-horror silhouettes that Nanite/LOD never struggle with.
UE 5.7 imports .glb natively (Interchange); meters → cm conversion is automatic.

## Layer 2 — Character pipeline: rigged + animated, one double-click away (Top-50 items 13–17)

`Scripts/BlenderArt/build_characters.py` + `Run_Build_Characters_Blender.command`

Builds THREE characters — **Survivor**, **ZombieShambler** (hunched silhouette), **ZombieBrute**
(broad silhouette) — each with:
- a 17-part stylized low-poly body, per-character palette;
- a 16-bone armature with deterministic per-part skinning (vertex groups, no auto-weight surprises);
- **Idle** (60f) and **Walk** (32f) actions exported as NLA takes;
- a rendered preview PNG (`RawArt/Characters/previews/`) for instant visual review;
- UE-friendly FBX export (`RawArt/Characters/*.fbx`).

Runs three ways: me, live over the Blender MCP (once Blender is connected); the double-click
`.command` (background Blender, no add-on needed); or Blender's Scripting tab.

## Layer 3 — Queued auto-import (runs on next editor open)

- `Saved/ClaudeBridge/inbox/0100_import_code_rescue_art.json` — imports `RawArt/CityKit/*.glb` →
  `/Game/CodeRescueArt/CityKit` and `RawArt/Characters/*.fbx` → `/Game/CodeRescueArt/Characters`
  (skeletal + animations), then saves.
- `Saved/ClaudeBridge/inbox/0110_place_art_showcase.json` — places an inspection row of the imported
  kit meshes at (0, −3000, 0), tagged `CodeRescueArtShowcase`, so the new art is reviewable in one glance.

## What Kenny enables (the actual blockers, both small)

1. **Install Blender** (free, blender.org). Then either double-click
   `Run_Build_Characters_Blender.command` (produces the 3 FBX + previews with no add-on), or also
   enable the **Blender MCP add-on** so Claude can model/rig/animate live in future sessions.
2. **Open the Unreal project** — the two queued bridge commands import everything and stage the showcase.

## Honest status / limits

- Kit meshes are validated glTF and the character script is valid Python authored to Blender's bpy
  API — but **nothing here has been through a UE import or a Blender run yet** (no Blender/UE in the
  sandbox). Visual review on the Mac is the Definition-of-Done gate.
- A few hand-authored planes (glass panes, kiosk screen) may need their material set two-sided in UE
  if they render invisible from the front — a one-checkbox fix, or I regenerate with corrected winding.
- This is deliberate **stylized programmer-art**: clean readable silhouettes intended to replace grey
  blocks and prove the pipeline — not a substitute for the Fab packs' sculpted fidelity. The Fab
  zombie packs remain the ceiling for character fidelity; these give the game a coherent authored
  look everywhere those packs don't cover, plus the survivor/mentor characters the packs lack.

## Verify

```
python3 Scripts/verify_art_pipeline_pass.py
python3 Scripts/claude_oversight_watchdog.py
```
