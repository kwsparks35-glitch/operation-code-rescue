# Creative Development Inclusion Plan - 2026-05-28

## Purpose

This document records the recommended creative and functional inclusions for
the next Operation Code Rescue development passes while Fab, MetaHuman, Maya,
Houdini, and related Unreal assets are actively downloading or being staged.
It is meant to guide asset selection, character/world development, runtime
feature work, and future QA without losing the game's current two-part purpose:

1. Teach coding through protected, selected-language coding challenges.
2. Reward completed challenges with survivor intel, then send the player into
   tense city rescue routes where zombies create navigation and survival risk.

The target creative direction is original third-person survival-horror action:
grounded camera tension, readable tactical resource pressure, hostile urban
spaces, dramatic rescue pacing, and clear learning moments. It should evoke the
quality bar and pacing discipline of classic console survival-horror action
without copying any proprietary characters, maps, weapons, story beats, names,
or assets from another franchise.

## Intake Rule For Downloaded Assets

Every downloaded asset should pass through this order before it becomes part of
normal gameplay:

1. License and source review.
2. Mac compatibility check, including plugin binaries when applicable.
3. Import into a clearly named project destination under `Content/`.
4. Collision, navigation, LOD, Nanite, material, texture-size, and animation
   validation.
5. Prototype replacement in a small review cell before broad campaign use.
6. Gameplay test with safehouse, combat district, survivor route, and packaged
   smoke coverage.
7. Documentation update in this pass note, the handoff note, and the relevant
   TSV/JSON manifest.

Recommended staging destinations:

- Character bodies, outfits, grooms, and rigs:
  `Content/Characters/Imported/`
- City kits, buildings, modular props, and road pieces:
  `Content/World/Imported/`
- Weapons, ammo, gear, and pickups:
  `Content/Weapons/Imported/`
- Audio, radio, UI, stingers, and ambience:
  `Content/Audio/Imported/`
- VFX, Niagara, decals, gore-safe hit effects, smoke, rain, and sparks:
  `Content/VFX/Imported/`
- Plugin or DCC output notes:
  `Content/CodeRescueData/`

## Priority Scale

- P0: Needed for the core game loop to feel playable and reviewable.
- P1: Strongly recommended for commercial-quality presentation.
- P2: Valuable polish or expansion after the core loop is solid.

## P0 Core Loop Inclusions

- Main menu language choice: Java, C, C+, C++, Python, and MATLAB, with each
  run using the selected language only.
- Protected coding safehouse and annexes with no active zombie threat.
- Terminal combat pause while coding, with resume behavior on close.
- Graduated coding missions that unlock survivor intel.
- Survivor intel that points to a specific city district, landmark, door,
  radio beacon, or named NPC rather than vague objective text.
- Combat and rescue districts separated from coding districts.
- Zombie AI that moves directly toward the player when engaged and respects
  safe-zone boundaries.
- Player health gauge, damage feedback, medkit use, and non-instant death.
- Replay, replay from save, save-and-quit, and quit choices after death.
- Working perspective controls and clear camera labels.
- Immediate weapon, ammo, and tactical gear selection with distinct use cases.
- Readable city entry without an outside wall barrier.
- City environments that look like actual urban places: streets, sidewalks,
  storefronts, doors, windows, transit cues, skyline silhouettes, landmarks,
  interiors, and tactical cover.

## Character Development Inclusions

### Playable Character Variants

- Rescue operator body with over-the-shoulder silhouette, shoulder lamp, radio
  harness, tactical coat or jacket, sidearm holster, and readable backpack.
- Field engineer variant with tool belt, tablet, cable spool, repair patches,
  and coding-solver visual identity.
- Medic operator variant with triage pouches, brighter arm marks, medkit slot,
  and healing animation set.
- Signal analyst variant with scanner, compact laptop or tablet, antenna pack,
  and survivor-intel interface props.
- Heavy-rescue variant with armored vest, shotgun/rifle stance support, and
  slower movement profile.
- Civilian survivor playable target for future rescue-to-playable unlocks.
- MetaHuman-ready face/body slots for each playable role.
- Hair/groom fallback cards for Apple GPU performance and packaged reliability.
- Outfit material variants for rain, grime, blood-safe scuffs, dust, and city
  faction colors.
- First-person arms and third-person full-body alignment for all weapon modes.
- Locomotion set: idle, walk, jog, sprint, crouch, strafe, aim-walk, reload,
  heal, interact, terminal use, ladder/stair traversal, stagger, and death.
- Animation retargeting through IK Rig/IK Retargeter for Manny, Quinn,
  MetaHuman, and imported skeletal meshes.
- Camera-safe head/shoulder proportions for tight third-person aiming.

### Survivor Cast

- Named survivor archetypes per city: engineer, medic, teacher, student,
  transit worker, firefighter, civic guide, researcher, radio operator,
  mechanic, supply runner, and family contact.
- Survivor condition variants: hiding, injured, barricaded, exhausted,
  signaling, carrying gear, guarding a door, or guiding another survivor.
- Rescue reaction animations: wave, crouched panic, point toward exit, limp,
  assisted walk, radio call, and safehouse relief.
- Rescue-specific props: blankets, emergency bags, clipboards, radios,
  flashlights, water, medicine, data drives, and map folders.
- Survivor dialogue and subtitle barks tied to completed coding intel.
- City-local visual accents without relying on stereotypes: transit uniforms,
  hospital badges, school IDs, port vests, civic patches, or weather gear.
- Survivor grouping that supports gameplay: one lead, one injured, one
  objective-holder, and optional escort target.

### Friendly NPCs

- Safehouse mentor who explains the selected-language track.
- Field trader with ammo, gear, health, and intel reward economy.
- Medic who teaches resource survival and supports post-rescue recovery.
- Engineer who unlocks doors, restores power, and visually ties coding to
  world-state repairs.
- Radio operator who converts solved challenges into survivor coordinates.
- Companion candidate with pathing, warning barks, and light combat support.
- Local civilian guides who give district flavor and city-specific warnings.
- NPC schedule states: idle, work, panic, hide, guide, heal, trade, repair,
  and evacuate.
- Facial animation slots for important NPCs using Control Rig or MetaHuman
  Animator when locally available.

### Zombie And Enemy Families

- Standard walker with direct player pursuit, clear attack windup, and fair
  recovery windows.
- Sprinter with lower health, louder telegraphing, and short burst pursuit.
- Armored infected using construction, police, fire, or utility gear.
- Bloated infected with area denial, slower speed, and explicit weak points.
- Nurse, office worker, dock worker, subway, storm-weather, and road-crew
  variants for city identity.
- Dog or fast quadruped variant only in combat districts, never in protected
  coding spaces.
- Elite warden enemy for optional boss encounters and survivor route pressure.
- Horde proxy silhouettes at distance that sell scale without overwhelming
  active gameplay.
- Hit reactions, limb-safe stagger states, knockback, stun, and death poses.
- Audio identity per class: breath, foot drag, sprint scrape, armor rattle,
  distant call, and attack warning.
- Navmesh and spawn volumes that keep enemies out of safehouse coding zones.

### Boss And Mini-Boss Concepts

- Redline Warden: tall elite pursuer used as a pressure boss after major intel
  milestones.
- Transit Brute: armored subway or station enemy guarding underground routes.
- Bridge Siren: signal-disrupting enemy tied to radio tower or bridge routes.
- Glass Ward Breaker: safehouse siege enemy for late-game defensive missions.
- Grid Husk: power-substation mini-boss that reacts to restored generators.
- Boss arenas with exits, cover, ammo pressure, healing windows, readable weak
  points, and no forced terminal coding while under attack.

## World Development Inclusions

### City Composition

- Every city should have a recognizable urban plan: entry approach, protected
  coding safehouse, combat district, survivor district, extraction route, and
  optional boss or resource zone.
- Cities should use local landmarks as fictionalized silhouettes rather than
  exact replicas unless licensed or created from owned reference.
- Each city needs street names or district signs, transit identity, skyline
  framing, weather cues, emergency signage, and local infrastructure.
- Streets should have sidewalks, curbs, crosswalks, traffic lights, alleys,
  streetlamps, parking meters, bus stops, benches, trash, fences, road work,
  and damaged vehicles.
- Building size must stay proportional to player scale: doors, windows,
  floors, stairs, awnings, counters, and cover pieces should read at human
  scale.
- Interiors should be enterable where mission-critical: safehouse, pharmacy,
  data center, school, hospital, police or civic office, transit station,
  supply store, apartment lobby, garage, warehouse, and rooftop access.
- Exterior spaces should support tactical navigation: corners, alleys, cover,
  sightlines, choke points, escape loops, and recognizable return routes.
- No outer city wall should block the player from beginning the level.

### Recommended City District Kits

- Downtown financial core: glass towers, lobby interiors, office cubicles,
  security gates, elevators, atriums, and rooftop signal points.
- Hospital and triage district: ambulance bay, emergency tents, pharmacy,
  med carts, oxygen tanks, privacy curtains, and survivor care rooms.
- Transit district: subway entrance, platform, turnstiles, bus loop, rail
  bridge, maintenance tunnels, ticket booths, and map boards.
- Port or waterfront district: containers, cranes, wet asphalt, mooring lines,
  floodlights, warehouses, piers, and emergency boats.
- Industrial district: power substations, generators, fenced yards, pipes,
  cooling towers, rail spurs, forklifts, and repair consoles.
- Residential district: apartment lobbies, stoops, courtyards, laundry rooms,
  playgrounds, alleys, and barricaded stairwells.
- Civic district: city hall, courthouse, library, fire station, police
  staging area, press boards, and public shelters.
- Education district: school hallway, classroom coding lab, gym shelter,
  library terminal room, backpacks, desks, whiteboards, and science props.
- Commercial strip: pharmacy, diner, hardware store, electronics shop, supply
  room, back alley, shutters, glass fronts, and signage.
- Weather or regional overlay: snow, rain, desert dust, coastal fog, river
  mist, heat haze, or storm debris.

### Major U.S. City Identity Targets

- New York: dense vertical streets, subway access, emergency command signage,
  narrow alleys, rooftop signal points, and glass/brick contrast.
- Los Angeles: wide roads, overpasses, parking lots, palm-lined commercial
  strips, studio-lot facades, hillside silhouettes, and wildfire haze.
- Chicago: river bridges, elevated tracks, brick industrial blocks, cold wind,
  civic towers, and waterfront rescue corridors.
- Houston: medical center, freeway underpasses, industrial energy sites,
  flood-control channels, and wide rescue routes.
- Phoenix: desert light, solar fields, low commercial blocks, dry washes,
  heat-haze streets, and utility substations.
- Philadelphia: historic civic blocks, rowhouse streets, transit stations,
  brick alleys, and emergency archive sites.
- San Antonio: riverwalk-inspired passages, mission-style stone, civic plazas,
  floodlit bridges, and southern evacuation signage.
- San Diego: harbor, naval-adjacent rescue staging, beach fog, trolley stops,
  and coastal supply routes.
- Dallas/Fort Worth: freeway grids, rail yards, logistics warehouses,
  commercial towers, and storm-dark skyboxes.
- San Jose/San Francisco/Oakland: tech offices, bay bridges, steep streets,
  data centers, transit hubs, and fog/rain atmosphere.
- Seattle/Portland: wet streets, overpasses, waterfront industry, evergreen
  skyline cues, and moody rain lighting.
- Denver/Anchorage: mountain silhouette, cold-weather gear, utility yards,
  snow or frost decals, and distant rescue lights.
- Washington, DC: civic barricades, command posts, museum/library interiors,
  federal-style architecture, and radio intelligence hubs.
- Miami/Tampa/Jacksonville/New Orleans: storm surge cues, waterfront rescues,
  bright signage, evacuation shelters, flooded roads, and generator noise.

## Learning And Coding Inclusions

### Curriculum Structure

- One selected language per run, chosen before gameplay begins.
- C+ should be treated as a project-specific teaching bridge track, since C+
  is not a standard production language in the way C and C++ are.
- C++ should use clang++ validation and gradually introduce types, functions,
  vectors/arrays, loops, conditionals, string handling, and small algorithms.
- C should use clang validation and focus on functions, loops, arrays,
  strings, pointers where appropriate, and simple memory safety lessons.
- Java, Python, MATLAB, C, C+, and C++ should each have equivalent mission
  shapes so no language feels less supported.
- Each solved challenge should unlock a precise rescue clue, not just score.
- Challenge rooms should visually match the concept being taught: loops as
  beacon cycles, arrays as supply shelves, conditionals as gate states,
  strings as radio messages, functions as repair modules, and search/sort as
  map/intel reconstruction.
- Coding failure should give useful feedback and never kill the player.
- Difficulty should graduate from syntax to reasoning to rescue planning.

### Challenge Categories

- Variables and types: inventory counts, health values, radio IDs.
- Conditionals: gate unlocks, safe route checks, triage priority.
- Loops: beacon sweeps, patrol timing, repeated scans, ammo totals.
- Arrays/lists: survivor names, supply caches, route distances.
- Strings: radio decoding, call signs, encrypted messages.
- Functions: restore power, compute safe path, validate code phrase.
- Search: find survivor by district clue or ID.
- Sort/rank: order evacuation priorities or route risks.
- Debugging: repair broken code to reveal coordinates.
- Data structures: queues for evacuation, stacks for logs, maps for city intel.
- Algorithmic missions: shortest safe corridor, resource allocation, sensor
  sweep, and countdown scheduling.

## Gameplay System Inclusions

### Weapons And Tactical Gear

- Knife or close tool for emergency stagger, crate opening, and low-ammo play.
- Handgun as reliable baseline sidearm.
- Shotgun for close crowd control and door defense.
- SMG for short burst pressure at medium range.
- Rifle for precise weak-point engagement and distant threats.
- Magnum or heavy pistol for rare high-impact shots.
- Crossbow or silent launcher analogue for stealth/resource strategy.
- Grenade, flashbang, smoke, and mine/trip tool categories with clear limits.
- Healing items: first aid spray analogue, medkits, bandages, and temporary
  stabilizer.
- Tactical gear: flashlight, radio scanner, survivor beacon, lock bypass kit,
  armor vest, ammo pouch, and battery pack.
- Distinct ammo types and HUD readouts for each weapon family.
- Visual weapon pickups and safehouse armory tables before combat districts.
- Number-key quick slots plus wheel/bracket cycling.
- Weapon-specific animation, recoil, spread, range, reload, sound, and purpose.

### Survival And Player Feedback

- Health gauge with color and percent.
- Damage flash, directional hit indicator, camera shake tuning, and audio cue.
- Stamina or exertion meter for sprint/crouch balance.
- Armor or temporary buffer for late-game tactical choices.
- Infection or contamination meter only if it supports learning/rescue pacing.
- Clear death state with replay/save-and-quit flow.
- Difficulty options that tune enemy count, damage, ammo, and puzzle hints.
- Accessibility toggles for aim assist, camera shake, subtitles, color-safe UI,
  simplified reload timing, and larger coding text.

### Camera And Controls

- Over-the-shoulder survival camera as the default.
- First-person inspection/coding-friendly view.
- Tactical wide view for navigation.
- Cinematic rescue view for survivor moments.
- Debug/editor review views for QA only.
- Smooth perspective switching with no input lockout.
- Controller mappings equivalent to keyboard/mouse mappings.
- Clear in-game control labels that do not clutter the screen.

## Mission And Progression Inclusions

- Every city starts in a protected safehouse or learning district.
- The first terminal reveals the first survivor route.
- Survivor rescue unlocks the next coding mission or city intel.
- Optional boss or resource objective unlocks extra gear, not required learning
  progress.
- City completion should have an extraction beat: rooftop, ambulance, bridge,
  helipad, rail car, boat, convoy, or safehouse evacuation.
- Campaign map should show completed cities, rescued survivors, active
  language, current challenge topic, and next intel clue.
- Save system should preserve selected language, city progress, rescued
  survivors, health, gear, ammo, and challenge completion.
- Replayable challenge archive should let the player practice without combat.
- Reward loop should include intel, story notes, gear upgrades, safehouse
  improvements, survivor roster growth, and city map clarity.

## AI And Encounter Inclusions

- Zombie AI states: idle, investigate sound, pursue, attack, stagger, recover,
  search last known position, retreat/stumble, and die.
- NPC AI states: safehouse work, guide player, hide, rescue follow, heal, trade,
  radio report, and evacuate.
- Encounter director that chooses pressure based on player health, ammo,
  current objective, safehouse distance, and learning status.
- No enemy spawn inside active coding rooms.
- Combat district spawn pacing that avoids instant surround.
- Cover and pathing markers for both player readability and AI navigation.
- EQS/StateTree/Behavior Tree promotion for imported AI packages only after
  Mac compile and packaged smoke validation.
- Crowd silhouettes should sell outbreak scale while active enemies stay
  readable and fair.

## Physics And Interactivity Inclusions

- Movable barricades, carts, doors, cones, crates, chairs, and shelving.
- Breakable glass, sparks, hanging signs, flickering lights, and loose papers.
- Physics cover that can be pushed, toppled, or used to slow zombies.
- Async physics test area before gameplay-wide promotion.
- Doors with clear open/locked/barricaded states.
- Generators, fuse boxes, elevators, gates, turnstiles, and shutters tied to
  coding rewards.
- Projectile collision and weak-point validation for all enemy classes.
- Traversal supports: stairs, ramps, rooftops, ladders where animation-ready,
  and blocked routes with visual alternatives.

## Visual Art Inclusions

- Cohesive palette: cold emergency lighting, wet asphalt, warm safehouse light,
  red danger accents, cyan/green intel accents, and readable objective colors.
- Materials: grime, rain streaks, concrete, worn metal, glass, tile, paper,
  cloth, plastic, medical vinyl, asphalt, brick, and emergency paint.
- Decals: footprints, scuffs, bullet marks, quarantine signs, route arrows,
  water stains, power marks, survivor notes, and map annotations.
- Lighting: safehouse warmth, harsh street lamps, flashing emergency lights,
  deep alley shadow, rooftop beacons, and practical interior lamps.
- Weather: rain, fog, dust, snow/frost, wind-blown paper, dripping water, and
  distant fire smoke.
- VFX: muzzle flash, impact sparks, hit dust, smoke, water splash, flashlight
  cone, radio pulse, terminal scan, rescue beacon, and generator startup.
- UI visual identity: tactical terminal panels, survivor dossiers, city map
  overlays, language track badges, and gear icons.

## Audio And Narrative Inclusions

- City-specific radio briefings.
- Survivor rescue barks.
- Safehouse ambience: generators, radios, distant rain, murmurs, doors.
- Combat ambience: distant sirens, zombie calls, metal knocks, wind, alarms.
- Weapon sounds by weapon role.
- Footsteps by surface: asphalt, tile, metal, wood, wet concrete, gravel.
- UI sounds for terminal typing, compile success, compile failure, intel
  reveal, gear selection, health pickup, and save.
- Mission music layers: safehouse, exploration, combat pressure, boss, rescue,
  and extraction.
- Subtitles for all critical dialogue and radio calls.
- Narrative collectibles: case files, city notes, survivor messages, outbreak
  reports, and coding concept hints.

## Unreal Pipeline Inclusions

- MetaHuman: playable/NPC body targets, face rigs, groom assets, LODs,
  animation retargeting, and performance fallbacks.
- Maya: custom character cleanup, rig checks, weapon attachment sockets,
  authored animation edits, and FBX export validation.
- Houdini: modular city generation, facade variation, rubble, road damage,
  procedural clutter, safehouse cells, and PCG handoff.
- Fab: buildings, interiors, props, vehicles, zombie variants, weapon props,
  materials, VFX, audio packs, and plugins only after license/Mac review.
- PCG: district placement, sidewalk clutter, interior dressing, safehouse
  layouts, survivor route dressing, and skyline dressing.
- World Partition/Data Layers: city streaming, safehouse layer, combat layer,
  rescue layer, skyline layer, and QA review layer.
- Control Rig: facial animation slots, rescue cinematics, boss introductions,
  and character presentation.
- IK Retargeter: movement consistency across Manny, Quinn, MetaHuman, and
  downloaded skeletal meshes.
- Niagara: rain, fog, sparks, smoke, muzzle flash, impact, and radio scan VFX.
- Chaos/async physics: movable cover, breakables, thrown tools, doors, debris,
  and projectile hit behavior.
- MetaSounds or audio routing: layered city ambience and combat stingers.
- Sequencer: intro, survivor reveal, extraction, boss reveal, and campaign
  transition shots.
- Data Validation: asset naming, missing material checks, skeleton checks,
  collision checks, and cooked package checks.

## UI, UX, And Accessibility Inclusions

- Main menu: language selection, save slots, settings, play, continue, and
  campaign status.
- HUD: health, ammo, selected weapon, gear slot, current objective, distance,
  survivor intel, city name, language track, and safehouse/combat status.
- Terminal UI: selected language only, readable font size, syntax help,
  compile feedback, challenge goal, and intel reward reveal.
- Inventory UI: weapon categories, ammo counts, healing, tactical gear, and
  purpose labels.
- Map/journal UI: city districts, survivor clues, coding challenge archive,
  rescued roster, and active extraction route.
- Death UI: replay from save, replay fresh, save and quit, quit.
- Pause/settings UI: controls, audio, video, accessibility, language track
  display, and save options.
- Accessibility: subtitles, high-contrast objective markers, colorblind-safe
  health/ammo indicators, remappable controls, camera shake toggle, larger
  terminal text, reduced combat pressure option, and aim assist.

## QA And Packaging Inclusions

- Static manifest validation after every asset-intake pass.
- Unreal commandlet coverage for curriculum, character/world assets, runtime
  smoke contracts, camera/roster, production track, and downloaded asset
  references.
- Packaged null smoke and packaged render smoke after any content import that
  touches materials, plugins, skeletons, maps, input, or startup flow.
- Manual first-city playtest: language select, protected coding, intel reward,
  route to survivor, combat, rescue, extraction, death/replay, save/quit.
- Multi-city playtest for progression persistence and city identity.
- Performance capture on the target Mac: frame time, GPU memory, shader hitch,
  texture pool, navmesh build, animation count, and audio warnings.
- Visual review capture for character scale, building scale, safehouse access,
  zombie separation, gear visibility, and city readability.
- Regression checklist whenever a new plugin is enabled.

## Recommended Next Integration Passes

1. Download inventory pass:
   scan the Fab/Vault/MetaHuman staging folders, classify assets, record
   license/Mac status, and update the intake tracker.
2. Character promotion pass:
   replace the strongest player/NPC prototype roles with imported MetaHuman or
   skeletal assets, retarget animations, validate grooms/fallbacks, and run
   camera-readability tests.
3. Zombie family promotion pass:
   add downloaded zombie variants only to combat districts, wire direct pursuit,
   validate attack damage, and confirm safehouse exclusion.
4. City kit promotion pass:
   choose one top-quality modular city/interior kit and integrate it into New
   York first, then expand to regional city archetypes.
5. Weapon and gear art pass:
   replace placeholder gear with imported props, sockets, pickup models, icon
   references, sounds, and clear functional roles.
6. Mission-curriculum art pass:
   make every challenge room visually express the coding concept and reward a
   specific survivor clue.
7. AI/physics pass:
   promote compatible AI, async physics, or extended library plugins behind
   feature flags and commandlet tests.
8. Audio/cinematic pass:
   add city radio, survivor barks, extraction stingers, boss reveals, and
   Sequencer review scenes.
9. Performance and packaging pass:
   trim asset sizes, generate LODs, verify shaders, rebuild, package, and smoke
   test the Mac app.

## Documentation Expectations

Every pass that uses downloaded assets should document:

- Asset name and source.
- License or account dependency.
- Project path after import.
- Whether the asset is prototype-only or gameplay-active.
- What system uses it: player, NPC, zombie, world, weapon, UI, audio, VFX,
  mission, curriculum, physics, or AI.
- Validation command run.
- Packaged smoke result.
- Any known Mac caveat.
- Any manual review still needed.

The machine-readable intake companion for this plan is:

```text
Content/CodeRescueData/creative_development_inclusion_plan.tsv
```

This document should be updated after the active downloads finish and again
after the first imported-asset gameplay pass.
