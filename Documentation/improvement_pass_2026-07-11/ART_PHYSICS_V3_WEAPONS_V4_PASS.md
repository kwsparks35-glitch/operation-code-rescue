# Art + Physics Pass — CharactersV3, WeaponsV4, Authored-Body Ragdoll (2026-07-11, evening)

Kenny's directive: *"build the characters, weapons, and the character and
world-physics modeling for real-time playability using Blender … so the game
transitions into something more stimulating and inviting … repackage and
document."*

Everything below was authored in Blender 5.1 over the MCP bridge, imported
through one reproducible pipeline, wired in C++, verified in a live resume
soak on the real Cpp save, and repackaged.

---

## 1. What shipped

### CharactersV3 (Blender → `RawArt/CharactersV3/*.fbx` → `/Game/CodeRescueArt/CharactersV3/`)

| Character | Role | New over v2 |
|---|---|---|
| SurvivorKennyV3 | player third-person body + male street NPCs | headlamp w/ EMISSIVE lens, belt, shoulder patch (beacon orange), richer gear; **Wave** celebration |
| SurvivorMayaV3 | rescue survivors + medic/scientist NPCs | hip medic satchel, teal shoulder patch; **Wave** celebration on rescue |
| ZombieShamblerV3 | horde | rib shadows + exposed ribs, EMISSIVE sickly eyes; **Attack / Flinch / Death** keyed one-shots |
| ZombieBruteV3 | heavy horde | 1.38× mass, knobby spine ridge, emissive eyes; Attack/Flinch/Death |
| **ZombieRunnerV3 (NEW)** | fast infected / EliteCharger | lean 0.84× build, frantic sprint-lunge Run loop; Attack/Flinch/Death |

All five: 17-bone proven rig (retarget continuity), 7 facial morph targets
(`Blink, BrowRaise, BrowAngry, Smile, Grimace, JawOpen, Alarm`), ~18–20k verts,
FBX takes for every action. Zombie actions: Idle/Walk/Run + **Attack (22f
windup→whip-down swipe), Flinch (12f), Death (44f collapse with pelvis
translation keys — knees fold, chest pitches, body ends prone)**. Survivor
actions: Idle/Walk/Run + **Wave (48f raise-and-wave)**.

Pipeline: `Scripts/BlenderArt/build_characters_v3.py` — execs the proven v2
script for its helpers, then adds v3 palette/extras/animations. Previews
(including per-action pose stills) in `RawArt/CharactersV3/previews_v3/`.

### WeaponsV4 (Blender → `RawArt/WeaponsV4/*.glb` → `/Game/CodeRescueArt/WeaponsV4/`)

PistolV4, ShotgunV4, RifleV4, SMGV4, CrossbowV4 — real hardware landmarks the
V3 set lacked: front+rear slide serrations, accessory rails, charging handle +
brass deflector + dust cover, 4-prong flash hider, red-dot optic with hood,
vent-rib + side-saddle BRASS shells + ghost ring (shotgun), suppressor cooling
rings + folded wire stock (SMG), split limbs + cams + stirrup + under-rail
quiver (crossbow), **emissive tritium dots / red-dot reticles** (night
readability). Same contract as v3: meters, +X muzzle, origin at grip.
Tri budgets ≤ 900 each — first-person cheap. Pipeline:
`Scripts/BlenderArt/build_weapons_v4.py` (+ previews in `previews_v4/`).

### Character physics — authored-body ragdoll restored

The 2026-07-04 crash fix had REMOVED all skeletal physics from authored
(Blender) zombie bodies — no ragdoll deaths, no physical hit reactions
(physics asset cleared, "fallback only"). This pass restores both, safely:

1. **Deliberate physics assets** — `Source/CodeRescueUnrealEditor/
   CodeRescueV3PhysicsLibrary.{h,cpp}` (new): builds each V3 mesh's physics
   asset analytically from the reference skeleton — **16 bodies** (capsules for
   pelvis/spine/chest/limb segments, spheres for head/hands/feet) + **15
   cone-limited constraints**, adjacent-body collision disabled. The engine's
   auto-fitter was reducing the rig to TWO bodies (pelvis+chest) regardless of
   parameters — see Lessons.
2. **Runtime gate** (`CodeZombieActor::ApplyProfessionalVisuals`): an authored
   body KEEPS its physics asset only when every body maps onto a real bone and
   `MatchedBodies >= 6` (logged as `[ZombieV3] … physics-eval`). Then
   `bAuthoredBodyPhysicsReady` re-enables `BindPhysicalHitReactionComponent`
   and `TryActivateDeathRagdoll` for that zombie. All 07-04/07-11 crash
   contracts stay in force: pre-physics tick guard, deferred bind until
   `Bodies.Num()>0`, full detach on reset/death.
3. **Kill switches**: `cr.AuthoredBodyPhysics` (default 1) restores the legacy
   cleared-asset behavior instantly; `cr.AuthoredZombieShare` (default 35)
   controls what % of eligible pack variants (BaseMesh / UrbanZombie4 /
   BusinessSuit / EliteCharger) wear authored V3 bodies at all.

### World physics / playability

No world geometry changed this pass; the prior solidity contract stands.
WeaponsV4 import through the same Interchange GLB path as V3 (collision as
before); all new characters run on the existing capsule + ground-snap
contract. Real-time budgets: characters ~18–20k verts, weapons ≤ ~900 tris,
emissives are constant-cost.

---

## 2. Gameplay wiring (C++)

- `CodeZombieActor` — V3-first mesh selection (Runner for EliteCharger,
  Brute/Shambler/Runner rotation otherwise; V2 then packs as fallbacks);
  authored **Attack** one-shot at both attack sites (player + barricade);
  authored **Flinch** on non-fatal hits when the physical-anim layer didn't
  take the hit; authored **Death collapse** when the ragdoll budget is spent
  (authored bodies hide the primitive Body/Head, so the old primitive-corpse
  path could never take them — deaths used to freeze mid-walk);
  `PlayAuthoredOneShot()` helper resumes the locomotion loop afterwards.
- `SurvivorActor` — authored SurvivorMayaV3 is now the DEFAULT body (was: grey
  Quinn mannequin; authored was locked behind `-CodeRescueUsePrototypeCharacters`);
  `TriggerRescueGesture()` now also plays the authored **Wave** and settles
  back to Idle.
- `FriendlyNPCActor` — authored Kenny/Maya V3 bodies are the default street
  NPCs (mannequins remain the asset-missing fallback).
- `CodeRescueCharacter` — third-person player body prefers SurvivorKennyV3
  (Idle/Walk/Run), falls back to v2; `ResolveWeaponPreviewMesh` prefers
  WeaponsV4 (both Interchange nestings tried), falls back V3 → V1 chain.
- Anim asset resolution is importer-naming tolerant (three historical naming
  patterns tried; actual imported names pinned by the import validation log:
  `<File>_Anim_<File>_<Action>`).

## 3. One reproducible import pipeline

`Scripts/import_art_pass_v3_v4.py` (run via
`UnrealEditor <uproject> -ExecutePythonScript=<file> -stdout -unattended`):

1. Forces the LEGACY FBX importer (`Interchange.FeatureFlags.Import.FBX 0`).
2. **Clean-slate deletes** `/Game/CodeRescueArt/CharactersV3` first (see Lessons).
3. Imports 5 character FBX (morphs ON, create_physics_asset ON) + 5 weapon GLB.
4. Rebuilds all physics assets via `CodeRescueV3PhysicsLibrary` (16 bodies each).
5. Sets `used_with_skeletal_mesh` + `used_with_morph_targets` on every V3
   character material.
6. Validates everything (meshes, physics assets, all 28 anims, 5 weapons) and
   prints PASS/FAIL per item.

Standalone equivalents kept for surgical reruns: `fix_v3_physics_assets.py`,
`fix_v3_material_usage.py`, `list_v3_assets.py`,
`run_v3_runtime_soak.sh` (the editor-game resume soak used below).

## 4. Verification

| Gate | Result |
|---|---|
| Editor compile (all edits) | PASS (UBT, first-try on game module; editor module needed `PhysicsUtilities` dep) |
| Import validation | PASS — 5 meshes, 5 physics assets, 28/28 anims, 5/5 weapons |
| Physics rebuild | PASS — 16 bodies + 15 constraints on all five characters |
| Watchdog (sandbox) | PASS — 128 pass / env-only fails only |
| Watchdog (Mac) | **PASS — 129 pass / 0 REAL regressions** |
| Live resume soak (real Cpp save, editor -game) | **PASS** — 20 authored-body zombies spawned (`cr.AuthoredZombieShare=35`), 20/20 kept physics (`matched=16 total=16`), **13 of them died and completed corpse lifecycles through the previously-fatal teardown path with 0 asserts**; 368 total corpse lifecycles; 0 fatals; **0 LogMaterial warnings**; WeaponsV4 confirmed live (`[HeldWeapon] body weapon 'PistolV4'/'RifleV4' attached via hand_R`); `[SurvivorV3]` + 4×`[FriendlyNPCV3]` confirmed |
| Repackage (`Package_Mac_App.command`) | see § Packaged verification below |

### Packaged verification

`Package_Mac_App.command` → BuildCookRun Success. UE5.7 ships IoStore: the
.pak holds only ~1.8k loose files — verify content via the **.utoc directory
index** (`strings CodeRescueUnreal-Mac.utoc | grep <Asset>`), which confirmed
every V3 character (mesh/skeleton/physics asset/all anims incl. Wave/Death)
and every V4 weapon in the 1.17 GB container.

Packaged resume smoke (`PackagedMac` binary, real container save,
`-CodeRescueAutoResumeLanguage=Cpp -stdout`,
`TestLogs/PackagedSmoke_ArtPhysicsV3_2026_07_11.log`):

- 11 authored-body zombies spawned; **11/11 `physics-eval … matched=16
  total=16` and kept their physics asset** in the COOKED build.
- `[HeldWeapon]` V4 weapons on the body, `[SurvivorV3]` + 4×`[FriendlyNPCV3]`.
- **0 fatals / asserts, 0 LogMaterial warnings, 0 default-material hits**
  across a 5+ minute run that progressed through the sunset→night sky cycle
  with the authored-physics swarm live.
- The authored-body death/teardown path (the .200 crash area) was separately
  exercised in the editor soak: 13 authored-body corpse lifecycles, 0 asserts
  (`TestLogs/EditorSoak_ArtPhysicsV3_2026_07_11.log`).

## 5. Lessons (add to the permanent pile)

1. **Blender 5.x slotted actions**: re-assigning `animation_data.action` does
   NOT evaluate until `animation_data.action_slot` is also set. And v2's
   `preview()` hard-resets to frame 1 — pose previews need a frame-preserving
   renderer (`preview_frame`).
2. **UE reimport reuses the ORIGINAL import options.** A crashed first import
   (mesh+skeleton only) poisoned every subsequent `replace_existing` reimport
   — anims/physics assets silently never appeared. Clean-slate delete first.
3. **`-run=pythonscript` commandlet cannot import FBX** (Slate assertion);
   use the full editor with `-ExecutePythonScript` (it inits Slate, runs,
   quits). Display-level LogPython is filtered from `-stdout` there — use
   warning/error level for phase markers you need to see.
4. **The physics auto-fitter cannot handle the authored rigs**: default AND
   tuned `FPhysAssetCreateParams` both produced 2 bodies (pelvis+chest) for a
   17-bone rig. Build physics assets analytically from the reference skeleton
   (`FPhysicsAssetUtils::CreateNewBody` + manual sphyl/sphere elems +
   `CreateNewConstraint`).
5. **`PhysicsAsset.skeletal_body_setups` is not Python-exposed** in 5.7 —
   body-count QA must live in C++ (editor library returns the count).
6. Material usage flags remain the #1 packaged-build foot-gun:
   morph-target skeletal materials need `used_with_morph_targets` or they
   cook as the DEFAULT MATERIAL (only a warning in editor -game).
7. `grep` on a live UE stdout pipe buffers — poll the log FILE instead.

## 6. Rollback / tuning knobs

- `cr.AuthoredZombieShare 0` — no pack zombie gives up its mesh (V3 still
  dresses the no-pack fallback horde).
- `cr.AuthoredBodyPhysics 0` — authored bodies go back to cleared physics
  assets (no ragdoll/hit reactions), exactly the pre-pass behavior.
- WeaponsV4/CharactersV3 assets missing → every wiring site falls back to the
  previous generation automatically (V3 weapons, V2 characters, mannequins).
