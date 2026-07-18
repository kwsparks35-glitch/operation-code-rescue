# Character, Streetscape & Night-Sky Wiring + Live Verification (2026-07-04)

## C++ wiring

**Player (`CodeRescueCharacter`)**
- Body prefers `/Game/CodeRescueArt/CharactersV2/SurvivorKenny` (mannequin fallback
  intact). Single-node animation with speed-based Idle/Walk/Run switching
  (`UpdateV2BodyLocomotion`, thresholds 40/430 uu/s).
- `UCodeRescueFacialExpressionComponent` registered at BeginPlay: morph-target driver
  with hold/fade envelopes + autonomous blink (2.2–5.5s cadence). Safe no-op on
  meshes without morphs.
- `RefreshFirstPersonWeapon()`: the existing hidden `FirstPersonWeaponSilhouette`
  component now shows the authored weapon mesh for the active weapon (pistol-family →
  SM_Pistol_Compact, shotgun-family → SM_Shotgun_Breacher, knife → SM_Machete_Field,
  Grenade → hidden, everything else → SM_Rifle_Scout). Hooked into `SwapWeapon`.
- `ApplyDamage` → Grimace 1.2s.

**Survivors/NPCs** — `SurvivorActor` prefers SurvivorMaya (accent-tint skipped for
authored clothing; idle loop + blink component). `FriendlyNPCActor` picks Maya for
Medic/Scientist roles, Kenny otherwise; same idle/blink treatment; mannequin+ABP
fallbacks preserved everywhere.

**Terminal/GameMode emotion hooks** — solve success (both bypass and validate paths)
→ Smile 3s; `TriggerBossHorde` → Alarm 2.5s.

**Streetscape** (`SpawnStreetscapeLayer`, called with the 07-01 kit layer): 7 road
segments + 14 sidewalks + crosswalk + 4 vehicles + 5 oaks + 2 dead trees + 5 bushes +
stop sign + 2 traffic lights = 41 placements, every one ground-snapped via `GroundZAt`
and spawned SOLID (complex-as-simple collision).

**Night sky** (`SpawnNightSkyLayer` + `UpdateNightSkyVisibility` in the day/night
tick): star dome (scale 600 → ~600m radius, 20,000uu below player) + moon at a fixed
NW-high offset; both follow the player, cast no shadows, tagged `SkyLayer`, hidden
outside the TimeOfDay 0.47–0.995 night window.

## Import pipeline results (editor bridge, live)

- Job `0200_import_art_pass_v2.json`: 19 assets — SurvivorKenny + SurvivorMaya (mesh,
  shared skeleton, 3 anims each, morph targets ON) + 5 weapons + 3 vehicles +
  3 nature + 2 sky + 5 street pieces. Asset paths match the C++ exactly (validated by
  `does_asset_exist` on all 12 critical paths). Kenny's imported morph list:
  `Blink, BrowRaise, BrowAngry, Smile, Grimace, JawOpen, Alarm` — all 7.
- **Gotcha found:** UE 5.7 Interchange FBX import REFUSED the two zombie FBX with
  `cannot merge bone tree with the existing skeleton` (it reuses skeleton-compatible
  assets; Maya merged into Kenny's skeleton silently, zombies were rejected).
  **Fix that worked:** disable Interchange for FBX (`Interchange.FeatureFlags.Import.FBX 0`),
  import through the legacy FbxFactory, re-enable. Zombies landed at
  `/Game/CodeRescueArt/CharactersV2/ZombieShamblerV2/...` and `.../ZombieBruteV2/...`
  (own folders). Remember this for every future character import.

## Live playtest verification (standalone -game, 2026-07-04 ~16:00)

Runtime log (`~/Library/Logs/CodeRescueUnreal/CodeRescueUnreal.log`):

```
[CityKit] Authored art: 27 spawned, 0 failed (dir=/Game/CodeRescueArt/CityKit/)
[Streetscape] 01 New York, NY: 41 spawned, 0 failed
[NightSky] dome=ok moon=ok
[CharacterV2] Player body = SurvivorKenny (anims idle=1 walk=1 run=1)
```
Plus ZERO `LogMaterial: Warning` lines (no Default-Material/checkerboard regressions).

Screen-verified: language menu renders → NEW Java RUN deploys → tutorial dismisses →
HUD live → tactical third-person shows the NEW PLAYER BODY (olive jacket, backpack,
hair — not the silver mannequin) → T-travel to the terminal route shows sidewalk,
poles, NPC group down a dusk street and a cyan beacon glyph. Loot pickups, movement,
camera cycling all responsive.

Operational notes (repeat lessons): the `-game` instance logs to
`~/Library/Logs/CodeRescueUnreal/`, the EDITOR logs to
`~/Library/Logs/Unreal Engine/CodeRescueUnrealEditor/`; the editor can hang on quit
with an unsaved untitled level (force-quit before launching `-game`, or the new
instance exits with "Existing instance is the same age or newer").
