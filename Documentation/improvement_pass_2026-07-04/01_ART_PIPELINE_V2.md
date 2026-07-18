# Art Pipeline v2 (2026-07-04) — Blender-authored characters, weapons, world, sky

Blender 5.1 is now installed on the Mac with the MCP addon connected, so this pass was
authored LIVE in Blender (iterative preview renders reviewed at each stage), then locked
into two reproducible pipelines:

- `Scripts/BlenderArt/build_characters_v2.py`
- `Scripts/BlenderArt/build_world_art_v2.py`

Either runs headless (`blender --background --python <file>`), from the Scripting tab,
or over the MCP (`exec(open(<file>).read())`, optional `BUILD_ONLY=[...]`).

## Characters (RawArt/Characters/*.fbx)

| Character | Role | Look |
|---|---|---|
| SurvivorKenny | player body + male NPCs | olive field jacket, gunmetal chest rig w/ mag pouches, backpack, gloves, cargo pants, knee pads, thigh holster, watch, cropped dark hair |
| SurvivorMaya | rescue survivors + medic/scientist NPCs | teal-grey medic shell, red-cross armband, ponytail + hair band, slimmer proportions |
| ZombieShamblerV2 | infected | necrotic olive skin, sunken cheeks/belly, torn sleeves/shins, patchy scalp, 4 wounds, hunched idle/shamble |
| ZombieBruteV2 | heavy infected | 1.35× muscle mass, bald, milky eyes, 5 wounds, torn rags |

Shared build: one-piece profile-lofted torso (no stacked-box look), axis-projected
tapered limbs with elbow/knee/deltoid fillers, mitt-with-thumb glove hands, layered
clothing shells, and a fully featured head (eyes + iris, upper lids, brow ridges, nose,
subtle mouth line + upper lip, chin, ears, hair shell with a real hairline).

**Facial shape keys exported as UE morph targets:** `Blink, BrowRaise, BrowAngry,
Smile, Grimace, JawOpen, Alarm` — magnitudes tuned for game-distance readability.

**Rig/anim:** the proven 17-bone skeleton (same names as v1 for retarget continuity),
per-part rigid skinning + smoothstep joint-blend bands at shoulders/elbows/knees,
three NLA actions per character (`Idle` 60f, `Walk` 32f — zombies get a drag-leg
shamble variant, `Run` 20f with forward lean). FBX export: `-Y forward, Z up`,
NLA strips as takes, morphs on.

**QA:** `validate_parts()` runs inside the pipeline — every part's world bbox is
checked against its declared intent (location drift > 9cm or >2.2× oversize fails the
build). This gate exists because of the two real bugs found during authoring (below).

## The Blender 5.1 world-space lesson (important for every future art pass)

On this Blender build, `primitive_*_add(location=…)` followed by
`transform_apply(scale=True)` leaves VERTEX COORDINATES IN WORLD SPACE (object at
origin). Consequences we hit and fixed:

1. "Local" vertex edits silently no-op or misfire — the deltoid flatten collapsed both
   shoulder pads to hip height, and the lips-taper divisor turned a 5cm mouth into a
   1.65m plank (bbox caching hid it until subsurf).
2. `transform_apply(rotation=True)` after setting `rotation_euler` orbits the part
   around the WORLD origin → the brows teleported to ±0.19m. Use the pipeline's
   `tilt()` (rotates about the mesh's own centroid) instead.
3. Face-plane clamps must run BEFORE jaw/chin pushes, or the mouth region folds
   inside-out (rendered as a dark "open mouth" panel).

## World art (GLB, real-world meters; UE glTF import lands at correct scale)

- **Weapons** (`RawArt/Weapons`): SM_Rifle_Scout, SM_Pistol_Compact,
  SM_Shotgun_Breacher, SM_Machete_Field, SM_Wrench_Heavy — hard-surface, gunmetal/
  worn-wood palette, grip near origin for FPS attach.
- **Vehicles** (`RawArt/Vehicles`): SM_Sedan_Wreck (crumple + one flat tire),
  SM_Van_Delivery, SM_Police_Cruiser (emissive lightbar).
- **Nature** (`RawArt/Nature`): SM_Tree_Oak_8m (6-blob canopy), SM_Tree_Dead_6m,
  SM_Bush_Round.
- **Street kit** (`RawArt/CityKit`): SM_Road_Straight_12m (lane dashes + edge lines),
  SM_Crosswalk_8m (zebra), SM_Sidewalk_6m (curb + expansion seams),
  SM_StreetSign_Stop, SM_TrafficLight (3 emissive lamps).
- **Sky** (`RawArt/Sky`): SM_SkyDome_Stars — inward-facing dome, bottom third removed,
  420 emissive star quads oriented to the viewer; SM_Moon — emissive sphere with
  darker maria patches.

All props keep their base at z=0 so the game's ground-snap rests them on pavement.
Materials are named (`CRW_*` / `CRV2_*`) and import as assets, same as the 07-01 kit.

Previews for review: `RawArt/Characters/previews_v2/` and `RawArt/previews_v2/`
(per-character full+head renders, per-group galleries).
