# First-Level Combat and Interaction Pass

Date: 2026-07-09

Status: implemented, compiled, cooked, packaged, and exercised in the final Mac
application.

Current V5 target-lock, access, sky, and locality details supersede the original
V4 values below and are recorded in
`FIRST_LEVEL_V5_ACCESS_TARGET_LOCK_WORLD_PASS.md`.

## Scope Contract

This pass deliberately targets campaign city index `0`, New York, while shared
character and combat corrections remain available wherever those systems are
used. The new storefront, field armory, triage checkpoint, sandbag cover, warm
practical lights, and first-level placement logic are hard-gated to city index
`0`. The language-selection launch screen, per-language save slots, Resume/New
Run choices, coding terminals, objective journal, and existing pause workflows
remain intact.

## Requirement-to-Implementation Map

### 1. Visible weapon aiming

- The production Manny locomotion pose is copied to a dedicated presentation
  mesh rather than replaced with a static mannequin pose.
- A procedural upper-body layer turns both upper arms toward the camera aim
  vector while right mouse is held and briefly during a shot. V5 also blends
  both arms toward the torso of a physically valid auto-locked target.
- The held weapon remains attached to `hand_R`, so arm, hand, and weapon motion
  read as one action.
- The layer is presentation-only and does not alter capsule movement,
  navigation, or hit solving.

Primary implementation:
`Source/CodeRescueUnreal/CodeRescueCharacter.h` and
`Source/CodeRescueUnreal/CodeRescueCharacter.cpp`.

### 2. First-level art and environment development

Nine new assets were authored reproducibly in Blender 5.1.2:

| Asset | Gameplay purpose |
| --- | --- |
| `FirstLevelStorefrontV4` | Authored street facade replacing one V3 facade slot |
| `FieldArmoryV4` | Readable armory landmark and loadout identity |
| `TriageCheckpointV4` | Medical/survivor storytelling landmark |
| `SandbagCoverV4` | Grounded combat cover and street silhouette |
| `GrenadeV4` | Frag, incendiary, and flash inventory presentation |
| `CombatKnifeV4` | Melee inventory presentation |
| `RocketLauncherV4` | Heavy-weapon inventory presentation |
| `WoundCavityV4` | Localized ballistic wound volume |
| `BiteWoundV4` | Localized infected bite wound volume |

The deterministic Blender source is
`Scripts/BlenderArt/build_first_level_combat_art_v4.py`; import and validation
are handled by `Scripts/import_first_level_combat_art_v4.py` and
`Scripts/inspect_first_level_v4_meshes_unreal.py`. The generated GLBs are under
`Content/CodeRescueArt/Blender/FirstLevelV4`, and the cooked Unreal meshes are
under `Content/CodeRescueArt/FirstLevelV4`.

No Blender MCP server was exposed in this workspace session, so the same local
Blender executable was invoked directly in background mode. This limitation
did not change the authored outputs or the Unreal import/validation path.

### 3. Clickable `P` field armory

- `P` opens the normal pause surface with a full-width `FIELD ARMORY` section.
- The panel renders the selected Unreal weapon mesh in a live, lit 3D viewport.
- Clickable Previous, Next, and Equip controls cycle or apply the selection.
- Left/right arrows and Enter provide equivalent keyboard operation.
- Every weapon shows magazine and reserve ammunition, maximum capacity, damage,
  effective range, fire interval, reload time, delivery type, special behavior,
  and a plain-language tactical description.
- Preview bounds determine scale and centering, and authored off-center pivots
  stay centered while the model rotates.
- The pre-existing Resume, save, load, slot management, settings, tutorial,
  crafting, and skill-tree controls remain on the same pause surface.

Primary implementation:
`Source/CodeRescueUnreal/CodeRescuePauseWidget.h` and
`Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp`.

### 4. Jump

- Space and the gamepad face button now invoke jump; Space no longer fires.
- Jump consumes stamina and respects the existing exhaustion contract.
- First-level movement uses a `680` jump Z velocity and `0.42` air control so
  traversal is useful without becoming floaty.
- Releasing the input calls `StopJumping`.

### 5. Reliable weapon influence

- Hits now combine world occlusion, the weapon trace channel, zombie object
  tracing, and exact segment-to-capsule geometry.
- A zombie is accepted only when its physical intersection precedes blocking
  world geometry.
- V5 target lock is finite: at most 4,200 Unreal units, a 24-degree acquisition
  cone, a 36-degree break cone, and a required clear physical weapon path.
- A deliberate miss has no post-trace proximity fallback and therefore cannot
  damage an off-ray target.
- Piercing uses a tighter 58-unit corridor and still validates geometry.
- Explosives cannot inherit direct-fire aim assist and cannot detonate at a
  remote endpoint when no physical impact occurred.
- Hit point, incoming direction, anatomical zone, and bone context are passed
  to the target; hit-zone multipliers are applied once.

### 6. Localized wounds and grounded reactions

- Bullet impacts create a blood decal and a small `WoundCavityV4` mesh at the
  anatomical hit location, attached to the hit bone when one is available.
- Up to eight recent ballistic wounds remain visible on each zombie.
- Zombie bites create a side-aware decal and `BiteWoundV4` mesh on the injured
  player region.
- Death impulse follows the incoming shot direction, removes launch-scale
  vertical force, and is clamped to grounded values. Ragdoll-capable targets
  receive the strongest reaction; primitive and physical-animation fallbacks
  receive smaller values.
- The blood-decal material instance directory is explicitly included in cooked
  builds, closing the missing-decal warning found during the first package
  audit.

Primary implementation:
`Source/CodeRescueUnreal/CodeZombieActor.h`,
`Source/CodeRescueUnreal/CodeZombieActor.cpp`, and character combat handling.

### 7. Corpse persistence and disappearance

- A defeated zombie remains visible for 9 seconds.
- Its final settled pose is frozen before a 2.8-second gradual sink/scale fade.
- The actor is destroyed only after the fade completes.
- Fallen companion NPCs use the same readable lifecycle with an 8-second hold
  and 3-second fade.
- Audit-only transient zombies cannot update campaign save progress.

## Player Controls

| Action | Input |
| --- | --- |
| Aim | Hold right mouse / gamepad trigger |
| Fire | Existing fire input |
| Jump | Space / gamepad face button |
| Field armory | `P` |
| Inspect previous/next | Click buttons or Left/Right |
| Equip inspected weapon | Click Equip or press Enter |
| Resume | `P` or Escape |

## Visual Evidence

- `Renders/first_level_aim_review.png`
- `Renders/packaged_first_level_armory_cycle_complete.png`
- `Renders/packaged_first_level_combat_wound.png`
- `Renders/packaged_first_level_grounded_corpse.png`
- `Renders/packaged_first_level_corpse_fade.png`

## Deterministic Review Hooks

- `-FirstLevelArmoryCycleAudit` opens the real pause widget, traverses all 17
  weapon previews through player-facing handlers, equips the final entry, takes
  a screenshot, and exits.
- `-FirstLevelCombatRuntimeAudit` isolates a live target or creates a transient
  one when the selected save has already cleared the encounter. It exercises
  jump, bite wounds, two unassisted pistol traces, bullet wounds, death,
  persistence, fade, removal, screenshots, and exit.
- `-FirstLevelIntegratedAcceptanceAudit` performs access, all six language
  solutions, four sky phases, target lock, deliberate miss locality, combat,
  corpse lifecycle, and all 17 armory previews in one packaged run.
- `-VisualReviewStart` bypasses the launch selector only for opt-in review runs;
  ordinary launches still stop at the language-selection start screen.
