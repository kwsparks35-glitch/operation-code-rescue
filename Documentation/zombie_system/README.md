# Zombie variant system — improvements log

This folder documents every improvement made on top of the variant scaffolding
that was PIE-verified on 2026-04-29. Items are numbered to match the top-20
roadmap. Each item links to a per-item file with the change summary, files
touched, how to verify, and any open follow-ups.

## Status legend

- **DONE** — code is in main, compiled, and (where applicable) PIE-verified.
- **DONE (code)** — code lands compile-clean; PIE behavior depends on a
  prerequisite that's outside the repo (e.g. real environment assets).
- **PARTIAL** — most of the change is in; some sub-step deferred with a
  reason.
- **DEFERRED** — explicitly NOT done in this pass; reason and next-step
  recipe documented in the per-item file.

| # | Item                                               | Status     | File                                |
|---|----------------------------------------------------|------------|-------------------------------------|
| 1 | Smarter AnimBP picker                              | DONE       | [01_animbp_picker.md](01_animbp_picker.md) |
| 2 | AActor → ACharacter (or AnimBP velocity hookup)    | PARTIAL    | [02_character_locomotion.md](02_character_locomotion.md) |
| 3 | Death montage on `ApplyRescueDamage`               | DONE       | [03_death_montage.md](03_death_montage.md) |
| 4 | Hit-react flinch                                   | DONE       | [04_hit_react.md](04_hit_react.md) |
| 5 | Per-variant tuning pass                            | DONE       | [05_variant_tuning.md](05_variant_tuning.md) |
| 6 | NavMesh                                            | DONE (code) | [06_navmesh.md](06_navmesh.md) |
| 7 | Real `AAIController` + Behavior Tree               | PARTIAL    | [07_ai_controller.md](07_ai_controller.md) |
| 8 | Real environment art per zone                      | DEFERRED   | [08_real_environments.md](08_real_environments.md) |
| 9 | Lighting + post-process pass                       | DEFERRED   | [09_lighting.md](09_lighting.md) |
| 10| Audio cues (variant + ambient)                     | DONE (code) | [10_audio.md](10_audio.md) |
| 11| Player weapon system                               | DONE       | [11_weapon_system.md](11_weapon_system.md) |
| 12| Game-over / death state                            | DONE       | [12_game_over.md](12_game_over.md) |
| 13| HUD pass                                           | DONE       | [13_hud.md](13_hud.md) |
| 14| Crosshair + interaction prompt                     | DONE       | [14_crosshair.md](14_crosshair.md) |
| 15| More challenge types                               | DONE       | [15_challenge_types.md](15_challenge_types.md) |
| 16| Real survivor variety                              | DEFERRED   | [16_survivors.md](16_survivors.md) |
| 17| Objective progression / gating                     | DONE       | [17_objective_gating.md](17_objective_gating.md) |
| 18| Difficulty rebalance + variant cap                 | DONE       | [18_difficulty_cap.md](18_difficulty_cap.md) |
| 19| Verify Shipping Mac build                          | PARTIAL    | [19_shipping_build.md](19_shipping_build.md) |
| 20| Playtest protocol                                  | DEFERRED   | [20_playtesting.md](20_playtesting.md) |

For a chronological story of what landed when, see
[CHANGELOG.md](CHANGELOG.md).

## Why some items are DEFERRED rather than DONE

I am driving this through a screen-automation MCP and a Linux-sandboxed bash.
A few items genuinely cannot be completed responsibly from here:

- **#8 Real environments** — needs a kit-bashed Megascans/PCG content pass that
  I'd have to do entirely through editor clicks. Even one zone (Anchorage
  Medical) takes a half-day of asset-browser + drag/drop. The doc in
  `08_real_environments.md` lays out the exact assets and approach.
- **#9 Lighting** — perceptual pass; needs human eye on a real display, not
  pixel-screenshots over a video stream. Doc spells out the recipe.
- **#16 Real survivors** — needs MetaHuman download + rigging + VO recording.
  Outside the scope of this pass; doc has the bring-up plan.
- **#20 Playtesting** — needs real students. Doc is the structured protocol
  to run when one is available.

For everything else: the code is in, compiled, and I'll note PIE behavior
in the per-item file.

## How to re-run the build/test pipeline

After any C++ change in this folder's items:

1. Quit the editor (Cmd+Q from inside it; macOS app menu → Quit).
2. Double-click `Recompile_Module.command` from the project root in Finder.
   Watch for `BUILD SUCCEEDED` (≈10–60 sec depending on incremental).
3. Double-click `CodeRescueUnreal.uproject` to reopen the editor.
4. (If variant rows changed) bottom dropdown → **Python**, paste:

   ```
   exec(open(r"/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Scripts/build_zombie_variants_table.py").read())
   ```

5. Press **Play** in the toolbar to PIE.
6. Open Outliner, search `CodeZombie`, click one — verify the Details panel
   shows the variant + per-variant stats from the data table.

## Where new content lives

- **Source/CodeRescueUnreal/** — all C++ changes (one new module, no new modules)
- **Content/CodeRescueAssets/** — DT_ZombieVariants and BP_CodeRescueGameMode
- **Scripts/** — Python automation for editor-side asset authoring
- **Documentation/zombie_system/** — this folder
