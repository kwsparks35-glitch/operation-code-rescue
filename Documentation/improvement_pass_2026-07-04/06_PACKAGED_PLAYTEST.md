# Packaged-App Verification (2026-07-04, Part 2)

The whole point of this pass: `PackagedMac/Mac/CodeRescueUnreal.app` now CONTAINS
everything — double-click it and play.

## Package build

- `Package_Mac_App.command` → RunUAT BuildCookRun (build+cook+stage+pak+archive)
  → **BUILD SUCCESSFUL**; fresh pak set written 2026-07-04 17:37
  (`CodeRescueUnreal-Mac.pak` 386.3 MB, `.ucas` 1.151 GB — ~6 MB larger than the
  07-02 build; the delta is the v2 art).
- `/Game/CodeRescueArt` is in `DirectoriesToAlwaysCook`, so all pass-1 content
  (CharactersV2 incl. both zombies, Weapons, Vehicles, Nature, Sky, street kit)
  cooked in automatically — no config change needed.

## Packaged runtime proof (this exact .app, launched with the shipping binary)

Launched `CodeRescueUnreal.app/Contents/MacOS/CodeRescueUnreal
-CodeRescueBypassLaunchLanguageMenu` (the project's own automation flag — skips the
language menu straight into active play, so world spawn is exercised without input).
Runtime log (`~/Library/Containers/com.operationcoderescue.CodeRescueUnreal/Data/
Library/Logs/CodeRescueUnreal/CodeRescueUnreal.log`):

```
[CityKit]    Authored art: 27 spawned, 0 failed (dir=/Game/CodeRescueArt/CityKit/)
[Streetscape] 01 New York, NY: 41 spawned, 0 failed
[NightSky]   dome=ok moon=ok
[CharacterV2] Player body = SurvivorKenny (anims idle=1 walk=1 run=1)
LogMaterial warnings: 0
```

Meaning, in the SHIPPED build: the authored player character (with all three
locomotion anims + morph targets) loads from the pak; all 41 streetscape pieces
(roads/sidewalks/crosswalk/vehicles/trees/signals) and the 27-kit layer spawn with
zero failures; the star dome + moon are live; and there are no Default-Material/
checkerboard regressions.

A normal double-click launch (no flags) was also booted and reaches the language
menu cleanly; the earlier -game playtest (pass 1) visually confirmed the identical
content: SurvivorKenny body in third person, streetscape, beacon glyph, menu → NEW
Java RUN → tutorial dismiss → HUD.

## For Kenny

Just double-click `PackagedMac/Mac/CodeRescueUnreal.app`. Notable since your last
packaged build (07-02): you now play as the authored survivor (watch him smile after
a terminal solve, wince when hit, and blink); your equipped weapon is visible in
first person and changes when you swap; the arrival street has real roads,
sidewalks, wrecked vehicles, trees, and signals — all solid; stars + moon come out
at night; multi-word world text is now beaming symbols (walk up + E to read);
zombies without pack meshes are the new v2 infected; and every terminal in every
city now teaches from the full 36-entry curriculum with validation driven by each
challenge's own tests.
