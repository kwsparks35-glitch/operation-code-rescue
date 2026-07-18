# Operation Code Rescue — Top 50 Development Roadmap

Compiled 2026-05-20 after a full Mac compile + live playtest of the build.
Items 1–5, 11–12, 33, 45 were **completed this session** (see
`04_VISIBILITY_PLAYABILITY_OVERHAUL.md`). The rest are ordered roughly by
impact-per-effort: the earlier an item, the more it improves the game per
hour of work.

Legend:  [DONE] shipped this session   ·   [NEXT] highest priority   ·
[SOON] strong value   ·   [LATER] polish / scale

## Tier 1 — Visibility & playability (the game must be playable first)

1. [DONE] Fix the "multiple directional lights competing" bug — one shared sun.
2. [DONE] Make night a playable moonlit dusk instead of near-black.
3. [DONE] Raise the SkyLight ambient floor so nothing crushes to black.
4. [DONE] Tune auto-exposure so dark areas brighten to visibility.
5. [DONE] Start a fresh run in bright mid-morning daylight.
6. [NEXT] Fix the player camera burying into terrain and walls — capsule
   collision and camera spring-arm pass. Most-felt bug in the live demo.
7. [NEXT] Slow down or expose the day/night cycle — 240s is a very fast
   full day; make the period a comfort/difficulty setting.
8. [NEXT] Verify the HUD (crosshair, status, resources, minimap) reliably
   mounts and is visible from the first frame of a fresh run.
9. [NEXT] End-to-end test the in-game coding terminal: open, type, validate
   Java/C/Python/MATLAB, close — confirm the core loop actually works.
10. [SOON] Resolve the "[VSM] Non-Nanite Marking Job Queue overflow"
    performance warning seen in the live build.

## Tier 2 — Characters & NPCs

11. [DONE] Brighter, longer-reach survivor rescue beacon.
12. [DONE] Warm, wider friendly-NPC safe-hub glow.
13. [NEXT] Make the imported Fab zombie skeletal meshes the default body —
    retire the procedural cube/sphere fallback to a true last resort.
14. [NEXT] Author/import per-variant zombie animations — idle, walk,
    attack, hit-react, and death montages (C++ montage slots already exist).
15. [SOON] Import per-variant zombie audio — growl/attack/death cues
    (C++ cue slots already exist; rows in DT_ZombieVariants are blank).
16. [SOON] Give survivors real MetaHuman or mannequin meshes plus a spoken
    rescue line.
17. [SOON] Add idle/walk animation to friendly NPCs (currently static).
18. [SOON] Zombie variety pass — distinct silhouette, scale, and tint per
    variant so the player reads the threat type at a glance.
19. [SOON] Boss telegraph — a visible wind-up animation and audio cue
    before boss attacks land.
20. [LATER] Companion NPC polish — follow distance, repathing, no
    doorway-blocking.
21. [LATER] Ambient idle behaviors for diorama civilians so populated
    spaces feel alive.
22. [LATER] Clear zombie hit-reaction feedback — flinch/stagger the player
    can read.

## Tier 3 — World & landscapes

23. [NEXT] Replace procedural cube buildings with modular Megascans / Fab
    kit-bash meshes, one zone at a time (start with Anchorage).
24. [SOON] Real sculpted landscape terrain instead of flat textured ground
    planes.
25. [SOON] Per-zone visual identity — Anchorage cool/snow, Seattle
    overcast/green, Tokyo neon — distinct color palettes and materials.
26. [SOON] Volumetric fog and atmosphere tuning per zone for depth/mood.
27. [SOON] Street set-dressing — vehicles, debris, signage, barricades — so
    cities feel inhabited.
28. [SOON] Bind real Niagara snow/rain/fog weather systems (slots exist).
29. [LATER] Dynamic cloud/sky pass that tracks the day/night cycle.
30. [LATER] Water material and reflections for coastal/lake/river cities.
31. [LATER] Distant-city LOD / impostors so the 342-city campaign scales.
32. [LATER] Placed reflection captures at objective hubs for grounded
    local lighting.

## Tier 4 — Mission objectives

33. [DONE] Colored beacon lights at every objective stop along the route.
34. [NEXT] On-screen waypoint / compass arrow pointing to the current
    objective.
35. [SOON] Objective-complete VFX and audio sting on terminal solve /
    rescue.
36. [SOON] 4–6 new coding-challenge shapes beyond the current 8.
37. [SOON] Wire `GetAdaptiveDifficultyTier()` into terminal challenge
    selection so challenges scale to the player.
38. [LATER] Authored per-city mission briefings (text + voice) replacing
    generic templates.
39. [LATER] Optional side-objectives (collect / escort / defend) for
    replay variety.
40. [LATER] Confirm victory and death screens fire reliably under all
    win/lose paths.

## Tier 5 — Combat & AI

41. [SOON] Zombie AI behavior-tree pass — smarter chase, flanking,
    investigate-on-lost-sight.
42. [SOON] Weapon feel pass — recoil, muzzle flash, impact decals,
    hit-confirm audio.
43. [LATER] Melee and throwable polish — readable arcs, cooldowns, VFX.
44. [LATER] Balance pass on elite zombies and the boss vs. the weapon set.

## Tier 6 — Polish, UX & audio

45. [DONE] Legible HUD — larger fonts and stronger shadows on crosshair,
    status line, and interaction prompt.
46. [NEXT] Author the main-menu map, set it as the launch target, and
    confirm controller navigation.
47. [SOON] Full audio mix — music, ambience, SFX levels; import remaining
    cues so cooked builds don't rely on macOS text-to-speech.
48. [SOON] Settings-menu verification — resolution, quality, volume, and
    sensitivity all actually apply and persist.
49. [SOON] First-run tutorial and onboarding polish aimed at non-gamer
    students.
50. [LATER] Accessibility pass — colorblind modes, subtitle sizing,
    remappable controls.

## Ongoing engineering / QA / release

- Keep running `Recompile_Module.command` after every code change — it is
  fast and is the ground truth for "does it build."
- Package a Shipping Mac build (`Package_Mac_App.command`) and smoke-test
  it (`Smoke_Test_Packaged_App.command null`).
- Playtest with 5 real students using the protocol in
  `Documentation/zombie_system/20_playtesting.md`.
