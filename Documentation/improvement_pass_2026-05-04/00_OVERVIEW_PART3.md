# Improvement Pass 2026-05-04 — Part 3 (items 61–68)

This pass closes the seven *deferred designer-wiring* items that were
flagged at the end of Part 2 and adds **two new content systems** so the
six downloaded zombie packs and the world itself feel more populated.

All work in this pass is purely additive on top of the prior 60 items;
none of the existing call sites were rewritten.

| #  | Item                                                         | Status |
| -- | ------------------------------------------------------------ | ------ |
| 61 | Pause-menu **Crafting** + **Skill Tree** buttons             | DONE   |
| 62 | Spawn `ABossZombieActor` + `AJeepActor` per city             | DONE   |
| 63 | Spawn `ACompanionActor` from first survivor rescue           | DONE   |
| 64 | Music hooks: menu / city / horde stinger                     | DONE   |
| 65 | `UCodeRescueModLoader::LoadAllMods()` from `GameInstance::Init` | DONE |
| 66 | Victory widget submits to `UCodeRescueLeaderboards`          | DONE   |
| 67 | Per-city themed **set-pieces** (lab vault / radio tower /    | DONE   |
|    | dog-pack den / hospital triage / drone wreckage)             |        |
| 68 | Ambient **friendly NPCs** (Engineer / Medic / Scientist /    | DONE   |
|    | Trader) per city, day-cycle perks                            |        |

Per-item docs:

- [`61_pause_widget_crafting_skill_tree.md`](61_pause_widget_crafting_skill_tree.md)
- [`62_spawn_boss_and_jeep.md`](62_spawn_boss_and_jeep.md)
- [`63_companion_on_first_rescue.md`](63_companion_on_first_rescue.md)
- [`64_music_hooks.md`](64_music_hooks.md)
- [`65_mod_loader_init.md`](65_mod_loader_init.md)
- [`66_victory_leaderboard_submit.md`](66_victory_leaderboard_submit.md)
- [`67_city_set_pieces.md`](67_city_set_pieces.md)
- [`68_friendly_npcs.md`](68_friendly_npcs.md)

## Files added

- `Source/CodeRescueUnreal/FriendlyNPCActor.{h,cpp}` (#68)

## Files modified

- `Source/CodeRescueUnreal/CodeRescuePauseWidget.{h,cpp}` (#61)
- `Source/CodeRescueUnreal/CodeRescueGameMode.{h,cpp}` (#62, #64, #67, #68)
- `Source/CodeRescueUnreal/MainMenuGameMode.cpp` (#64)
- `Source/CodeRescueUnreal/SurvivorActor.cpp` (#63)
- `Source/CodeRescueUnreal/CodeRescueGameInstance.{h,cpp}` (#63 flag, #65 init)
- `Source/CodeRescueUnreal/CodeRescueVictoryWidget.cpp` (#66)
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` (#68 interact dispatch)
- `Scripts/build_zombie_variants_table.py` (added Elite variant rows)

## Mac validation gate (designer/user)

```
./Recompile_Module.command 2>&1 | tail -80
./Package_Mac_App.command 2>&1 | tail -40
./Smoke_Test_Packaged_App.command null
```

## Re-author DT_ZombieVariants (one-time)

The variant data-table now has rows for the three Elite variants too, so
re-run the script in the editor's Python console:

```
exec(open(r"/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Scripts/build_zombie_variants_table.py").read())
```

Then save the table. Existing six pack rows are overwritten with the same
content; three new rows (`EliteSpitter`, `EliteCharger`, `EliteBoomer`)
are appended.

## Net new gameplay surface area

- **Boss fight per city** — gated voucher reward; 3-phase escalation.
- **Drivable jeep at every helipad** — fast traversal across the 50× city.
- **AI companion** — appears after first rescue; persists until killed.
- **Ambient NPC hub** — 4 friendly NPCs near each language-station plaza
  with daily-resetting perks (free scrap / heal / research / 5↔1 trade).
- **5 themed set-pieces** rotated deterministically by `CityIndex % 5`.
- **Boss horde** now plays the boss-stinger music cue when triggered.
- **Victory screen** writes 4 metrics into per-kind top-10 leaderboards.
- **Mods folder scan** runs at startup so user content can extend
  challenges without any code change.
