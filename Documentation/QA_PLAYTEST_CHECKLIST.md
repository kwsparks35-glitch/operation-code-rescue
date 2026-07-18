# Operation Code Rescue - QA Playtest Checklist

Use this checklist for human sign-off on the packaged app. Automated smoke tests already verify startup, cooked data mounting, map load, and GameMode initialization. This checklist verifies feel and visual/audio quality that cannot be judged from terminal logs.

## Setup

1. Run `Package_Mac_App.command`.
2. Run `Smoke_Test_Packaged_App.command null` for headless package smoke.
3. Run `Smoke_Test_Packaged_App.command render` for visual package smoke.
4. Open `PackagedMac/Mac/CodeRescueUnreal.app`.
5. Use a fresh save for first-pass QA. Then repeat save/load checks with an existing save.

## Core Controls

- WASD moves forward/back/strafe.
- Mouse and arrow keys turn the camera.
- I/K look up/down.
- Space, F, and left mouse fire.
- E, Enter, Tab, and G interact.
- Q uses a medkit.
- R reloads the active weapon.
- H/M show help.
- T jumps to the active level/objective route.
- Backspace and F8 recover the player to the current city arena if stuck or below the floor.
- C and V cycle camera perspective.
- F1, F2, F3, F4, F5, and F6 select FPS, TPS, tactical, top-down, isometric, and side-view cameras.
- 1, 2, 3, 4, 5, 6, 7, 8, 9, and 0 quick-select weapon slots.
- Mouse wheel and bracket keys cycle the full arsenal.
- Y regroups active rescue-team members behind the player.
- U cycles active rescue-team formation spacing between Tight, Standard, and Wide.
- N calls the rescue-team medic and reports success, cooldown, or failure.
- O toggles the rescue team between Hold and Follow orders.
- J toggles the objective journal.
- P opens/closes pause menu.

Pass criteria: every listed input produces the expected behavior and no gameplay action leaks while a modal UI is open.

## Demo Readiness Controls And Settings

- Confirm the pause menu cycles all six difficulty presets: Story, Easy, Normal, Hard, Survival, Nightmare.
- Confirm the selected difficulty label persists after saving and reloading.
- Open Settings and confirm the panel scrolls cleanly at 1280x720.
- Change subtitle size and confirm subtitles render larger or smaller.
- Toggle subtitles and confirm radio/recovery subtitles suppress and resume.
- Toggle high-contrast HUD and confirm live HUD text becomes higher contrast.
- Toggle reduced motion and confirm enemy-hit knockback is visibly softened.
- Toggle simplified input hints and confirm the HUD control line becomes shorter.
- Move aim assist to zero and confirm assisted hits are disabled; move it higher and confirm assisted hit radius/angle becomes more forgiving.
- Run `python3 Scripts/apply_control_remap_profile.py` and confirm `Saved/Config/ControlProfiles/default_controls_profile.json` is produced for binding review.

## First City Flow

- Start in New York.
- Confirm the player starts inside the playable level route without being blocked by an outside city wall, gate, or perimeter rail.
- Confirm the open-route beacons and green route stripe visually lead into the playable level space.
- Confirm entry, armory, safehouse, launch language marker, terminal, survivor, and helipad access points are not blocked by set dressing or physics props.
- Confirm the city perimeter is visibly locked with rescue-boundary walls/lights and that the player cannot leave the city arena from the north, south, east, west, or corner edges.
- Confirm the perimeter does not block the entry pad, armory, safehouse, launch language marker, terminal, survivor area, helipad, or route stripe.
- Press Backspace and F8 from normal play and confirm the player returns to a safe current-city arena location without closing or reopening the app.
- If a floor/fall bug is reproduced, confirm automatic arena recovery returns the player to the city instead of saving or leaving the player below the floor.
- Confirm buildings, props, civilians, and zombies read at a compact human-scale proportion suitable for tense survival-horror navigation.
- Confirm the New York U.S. city identity layer is visible and reads as New York: harbor/dense-island landscape cues, tight grid, brownstone/high-rise architecture, transit/taxi vehicle cues, and coat/businesswear civilian clothing cues.
- Confirm New York includes the harbor-statue signature silhouette and that it is decorative rather than a blocker.
- Confirm New York includes district micro-scenes such as waterfront/riverwalk, transit, historic/civic, and local clothing accessory cues, and that these are decorative rather than blockers.
- Confirm the city identity set dressing does not block the entry pad, armory, safehouse, launch language marker, terminal, survivor area, helipad, or the green route stripe.
- Confirm the level remains traversable without needing to search for an exterior city-gate opening.
- Confirm HUD appears and text is readable at 1280x720 and native display size, including the large player health label.
- Confirm the main menu offers Java, C, C+, C++, Python, and MATLAB before starting a run.
- Choose one language and confirm the campaign immediately starts without needing a second New Game click.
- Start with one selected deployment language and confirm the HUD/terminal uses only that track for the city.
- Confirm no active-play language switching stations appear after deployment.
- Confirm zombies, survivor teams, the player character, and pickup supplies appear grounded on visible solid platforms.
- Confirm world information markers are mostly compact symbols, while action prompts and keybinds remain readable text.
- Confirm the five-member rescue support squad appears near the entry rally point and follows in formation.
- Confirm the rescue-team HUD line reports active squad count, compact teammate health pips, medic state, support-fire status, formation, and hold/follow order.
- Press Y and confirm active rescue-team members regroup behind the player without blocking movement.
- Confirm the rescue-team HUD line normally shows `Y REGROUP` and briefly changes to `REGROUPED N` after regrouping.
- Press U and confirm the rescue-team HUD cycles through Tight, Standard, and Wide formation states.
- After changing formation, press Y and confirm regroup placement respects the selected spacing.
- Press N at low health and confirm the medic call heals immediately, reports cooldown, or reports a clear failure if no medic is operational.
- Press O and confirm the rescue team holds current positions while still using support fire and medic utility when relevant.
- Press O again and confirm the rescue team resumes following formation.
- Press Y while O Hold is active and confirm the squad regroups then holds the new ordered positions.
- Confirm squad members do not physically block the player when reversing, strafing, entering safehouse spaces, or moving through narrow access points.
- Walk to the protected coding safehouse and confirm no zombies are inside the learning zone.
- Open the New York terminal and confirm combat pauses while the code editor is open.
- Validate starter code or a known-correct solution.
- Confirm terminal success reports survivor-location intel.
- Confirm terminal helper markers disappear immediately after success.
- Rescue the New York survivor team.
- Confirm survivor helper markers disappear immediately after rescue.
- Fight at least one zombie.
- Confirm zombies face and move toward the player directly instead of sliding sideways across the route.
- Confirm one zombie hit does not immediately kill a healthy player and the HUD health gauge updates.
- Confirm the attack alert states the hit direction, source, damage, and source distance.
- At low health with a medkit available, confirm the emergency auto-medkit can trigger once, consumes a medkit, and reports where the attack came from.
- While the emergency auto-medkit is cooling down or unavailable, confirm critical-health callouts recommend Q medkit use or regrouping with the medic.
- Lower health below half and confirm the medic support member can heal the player when nearby.
- Confirm zombie marker disappears immediately after neutralization.
- Pick up ammo, medkit, armor plate, flare, smoke, stim, and scrap by walking through them.
- Look at ammo, medkit, armor plate, flare, smoke, stim, and scrap and press E to confirm interact pickup also works.
- Fill ammo or medkits to the cap, then confirm a full inventory does not consume extra pickups.
- Save through the pause menu.
- Quit and relaunch.
- Load the save and confirm terminal/survivor/zombie state, health, ammo, and medkits persist.

## City Transition Flow

- Press T after graduating New York.
- Confirm Los Angeles streams in and the previous city-local actors are gone.
- Confirm the streamed city's entry perimeter is passable from the player start.
- Confirm Los Angeles uses the same open-level entry treatment with no outside city wall barrier.
- Confirm Los Angeles visually differs from New York with dry-basin/palm landscape cues, wide boulevards/freeway signage, stucco/studio architecture cues, SUVs/convertibles, and casual sunwear clothing cues.
- Confirm Los Angeles includes the hillside-letter/media-district signature silhouette.
- Complete Los Angeles terminal and survivor.
- Press T and confirm Chicago streams in.
- Confirm Chicago visually differs from both prior cities with lake/river/grid cues, brick/steel architecture, transit rail cues, bus/taxi vehicles, and cold-weather clothing cues.
- Confirm Chicago includes a river-bridge/elevated-grid signature silhouette.
- Use J journal and confirm:
  - Completed cities show `[X]`.
  - Active city shows `[>]`.
  - Locked future cities show `[L]`.
  - The journal scrolls and can show later rows.

## Lesson Coverage

Test at least one city for each lesson family:

- Sum: `totalPower` / `total_power`.
- Lock: `shouldUnlock` / `should_unlock`.
- Reverse: `reverseString` / `reverse_string`.
- Palindrome: `isPalindrome` / `is_palindrome`.
- FizzBuzz: `fizzBuzz` / `fizz_buzz`.
- Even filter: `evenNumbers` / `even_numbers`.

Pass criteria: correct starter code validates, intentionally broken code fails, and terminal success records exactly once.

## Language Coverage

For each language, verify dependency banner and validation behavior:

- Java.
- C.
- C+.
- C++.
- Python.
- MATLAB, including fallback messaging if MATLAB is not installed.

Pass criteria: available toolchains run compiler-backed validation, unavailable toolchains produce clear fallback messaging, and successful validation still records mission progress.

## Combat And Balance

- Early city zombie encounters are survivable without perfect aim.
- Assisted fire helps only when aiming near the target and does not hit targets behind the player.
- Direct hits feel stronger than assisted hits.
- Ammo pickups are enough to recover from missed shots.
- Medkits are useful but not required for the first city.
- Armor, hit mercy, medkits, and medic support make the player resilient enough to survive early contact.
- Emergency auto-medkit makes surprise lethal pressure survivable when supplies are available, without hiding the attack direction from the player.
- Ammo and medkit caps prevent runaway stockpiling.
- Mouse wheel, bracket keys, number quick slots, and gamepad shoulder controls can cycle or select the full weapon roster.
- Story difficulty keeps combat pressure low enough for learning-first play.
- Easy difficulty remains approachable.
- Normal difficulty feels like the intended default.
- Hard difficulty feels harder through health/damage, not broken.
- Survival difficulty pressures resource management.
- Nightmare difficulty is dangerous but still protected by single-hit safety and recovery systems.

Pass criteria: combat supports mission flow rather than blocking coding progression.

## UI Readability

- HUD does not obscure critical interactables.
- Health, armor, ammo, medkit, and attack-alert text do not overlap at 1280x720 or native display size.
- The rescue-team status line does not overlap the health, reload, tactical, or weapon readouts.
- Crosshair prompt changes for terminals, survivors, pickups, launch language marker if present, and zombies.
- Terminal editor text wraps and remains usable.
- Dependency banner is readable.
- Pause/death/victory screens lock and release input correctly.
- Death screen offers play again from save, play again fresh, save and quit, and quit.
- Journal remains readable with the current 465-city campaign catalog.
- Settings exposes master/SFX/music volume, mouse sensitivity, FOV, subtitles, subtitle size, aim assist, high contrast, reduced motion, simplified input hints, fullscreen, and VSync.

## Audio And Visuals

- Audio initializes and does not crackle or hang.
- Optional macOS radio speech plays once per streamed city if `-NoRadioVoice` is not used.
- Generated Maple narration WAVs play for imported female-voiced cities after the Maple import/wire editor script is run.
- Lighting is bright enough to navigate.
- City art-kit geometry, landmarks, terminals, survivors, zombies, and pickups are visually distinguishable.
- Representative U.S. city families are visually distinguishable through environment cues, signature silhouettes, and district micro-scenes: coastal/tropical, desert, mountain, Northeast historic, Midwest industrial, Southern river/freeway, Pacific Northwest, college-town, tech-suburb, capital/civic, and entertainment/neon profiles.

## Release Evidence

- Run `python3 Scripts/generate_visual_regression_manifest.py --min-count 1`.
- Run `python3 Scripts/generate_release_manifest.py`.
- Run `python3 Scripts/create_support_bundle.py`.
- Confirm `Saved/Release/release_manifest_latest.json` records package evidence, logs, Maple coverage, visual screenshots, known allowed warnings, feature flags, and git status.
- Confirm `Saved/VisualRegression/visual_regression_manifest_latest.json` records screenshot hashes and dimensions.
- Confirm a support zip appears under `Saved/SupportBundles`.
- Confirm `python3 Scripts/verify_save_compatibility_pass.py`, `python3 Scripts/verify_asset_budget_pass.py`, and `python3 Scripts/verify_demo_readiness_pass.py` pass.

## Release Sign-Off

Record playtest results in `Documentation/PLAYTEST_RESULTS_YYYY-MM-DD.md` using this format:

```text
Build path:
Tester:
Date:
Mac model:
macOS version:

Passed:
- ...

Issues:
- ...

Release decision:
- Ready / Ready with notes / Needs fixes
```
