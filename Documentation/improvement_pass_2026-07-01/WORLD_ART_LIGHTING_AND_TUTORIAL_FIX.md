# World Art, Lighting, and Tutorial-Dismiss Pass (2026-07-01 → 07-02)

This pass closed the four queued items from the prior session and then chased the
"world still looks like grey boxes / everything is teal" problem to its real roots.
Every player-facing change below was verified in the **packaged Mac app** via live
playtest (build it, play it, prove it) — not reasoned from source.

## 1. Tutorial overlay is now dismissable (the headline fix)

**Symptom:** the first-launch tutorial rendered correctly (proving the earlier
invisible-UMG sweep worked) but pressing Space/Skip did nothing — Space leaked past
the overlay and fired the weapon, and the semi-transparent panel blocked the view.

**Root cause (two layers):**
1. The tutorial used `FInputModeUIOnly` + Slate focus. In packaged builds those key
   events are not delivered to the widget's `NativeOnKeyDown`, so its own
   Space/Skip/Next handlers never ran.
2. Even the pawn couldn't help, because `UIOnly` routes keys to Slate, starving the
   pawn's `PC->WasInputKeyJustPressed(...)` poll.

**Fix — drive it from the pawn, exactly like the (working) language gate:**
- `UCodeRescueTutorialWidget` now keeps a `static TWeakObjectPtr<...> ActiveInstance`
  with `IsShowing()`, `DriveAdvance()`, `DriveDismiss()` (set in `NativeConstruct`,
  cleared in `NativeDestruct`).
- Its input mode changed from `UIOnly` to **`FInputModeGameOnly`** so the
  PlayerController owns input and the pawn poll can see the keys.
- `ACodeRescueCharacter::PollDirectKeys` gained a tutorial block *before* the
  `bUIOpen` gate: **Space/Enter/E advance a page, Esc/Backspace skip the whole
  tutorial**, then it returns and zeroes bound movement so nothing behind it fires.

**Playtest result:** Space advanced Page 1 → 2 (content + cards changed, "Page 2/8"),
and **Backspace closed the tutorial entirely**, revealing the world. (Escape alone is
swallowed by the UE viewport — Backspace is the reliable skip; both are wired.)

## 2. Authored city kit now actually appears — path + material fix

**Symptom:** "environment/characters do not look noticeably modified." The 27-piece
authored kit (`/Game/CodeRescueArt/CityKit`) was being requested every city spawn but
nothing showed.

**Root cause #1 — wrong object path.** The glTF importer nests each mesh at
`<Name>/StaticMeshes/<Name>`, not flat at `<Name>`. The old flat path resolved to a
*folder*, so `LoadObject<UStaticMesh>` returned null and **zero** kit meshes spawned —
silently. `SpawnAuthoredCityKitLayer`'s `Kit()` helper now builds
`/Game/CodeRescueArt/CityKit/<Name>/StaticMeshes/<Name>.<Name>`.

**Instrumentation.** `SpawnKitMesh` calls are now tallied and logged:
`[CityKit] Authored art: 27 spawned, 0 failed`, with an on-screen warning if any fail.
The live log confirmed **27 spawned, 0 failed** after the fix.

**Root cause #2 — no surface material.** The kit imported without a usable material and
rendered as the engine checkerboard. `SpawnKitMesh` gained a `MaterialPath` parameter
and applies a solid cooked StarterContent material across every slot:
- facades → `M_Concrete_Panels` / `M_Brick_Clay_Old` / `M_Brick_Cut_Stone` (varied)
- lamps + kiosk → `M_Metal_Burnished_Steel`
- planters → `M_Concrete_Grime`, rubble → `M_CobbleStone_Rough`
`/Game/StarterContent/Materials` is already in `DirectoriesToAlwaysCook`, so these are
guaranteed in the pak.

## 3. Lighting de-teal pass

**Symptom:** the open entry plaza blew out to a bright teal/cyan haze that washed all
surface detail.

**Cause:** a stack of blue-biasing settings in `SpawnWorld()` plus the per-zone grade —
a bright blue-sky `SkyLight` (intensity 3.0), blue fog inscatter (0.55,0.65,0.85),
bloom 1.1, auto-exposure bias +1.0 with max-brightness 2.6, a blue color-saturation
push, and the per-zone "cool/blue" grade (0.85,0.85,1.05).

**Changes (all toward a neutral, readable image; night readability preserved):**
- Directional sun 7.0 → **4.5**
- SkyLight 3.0 → **1.4** (near-white color) — this is the biggest de-teal lever; the
  blue-sky ambient no longer dominates, so the warm sun is the key light
- Fog: density 0.0014 → 0.0008, inscatter blue → near-neutral cool-grey (0.62,0.64,0.70)
- Bloom 1.1 → **0.45**; auto-exposure bias 1.0 → **0.25**, max 2.6 → **1.3**
- Per-zone preset 0 ("cool/blue") and the "CoolOvercast" US-city grade eased to
  near-neutral so they stop stacking into a teal cast

**Playtest result:** away from the bright open plaza, the world now reads correctly —
warm sun, real shadows, sandy/concrete ground, the player character visible in third
person, an NPC figure in the mid-distance, textured buildings on the skyline, **no teal
wash.** The entry plaza specifically still over-exposes (bright fully-open area); see
follow-ups.

## Verified this pass (packaged app)
- Language select → Enter deploys ✓
- Tutorial: Space advances, Backspace skips ✓
- HUD minimal single line ✓
- Kit art: 27 meshes spawn, 0 fail, with solid materials ✓ (log + on-screen)
- Lighting neutral + character + NPC + shadows in enclosed areas ✓

## Known follow-ups (honest status)
- **Entry plaza over-exposure.** The one bright open area still adapts to a washed
  look. Next lever: clamp auto-exposure harder there or a small local PP volume /
  manual exposure. Needs its own playtest loop.
- **A few checkerboard placeholder cubes** remain in the safehouse hub (specific
  primitive props without a material) — cosmetic.
- **Wall/'safehouse' guide-text density.** Some decal-style guide text
  ("Foundations. Curriculum focus…", ammo/slot labels) is still wordy; candidate for
  the marker-ization treatment used elsewhere.
- **Zombie ragdoll** (added earlier) not yet visually confirmed in a death.
- **Validation** still archetype-keyed rather than fully data-driven.

## Build / package facts
- Engine: `/Users/Shared/UE_5.7`. Editor gate: `Build.sh CodeRescueUnrealEditor Mac
  Development`. Ship: `Package_Mac_App.command` (BuildCookRun, ~1.5–3 min incremental).
- Packaged app: `PackagedMac/Mac/CodeRescueUnreal.app`; runtime log lands in
  `~/Library/Containers/com.operationcoderescue.CodeRescueUnreal/Data/Library/Logs/CodeRescueUnreal/CodeRescueUnreal.log`.
