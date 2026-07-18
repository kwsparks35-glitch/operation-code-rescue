# Next 100 Recommended Improvements and Implementation Kickoff

Date: 2026-06-12

## Purpose

This document lists the next 100 recommended improvements for Operation Code
Rescue and records the first implementation slice completed immediately after
the roadmap was written.

## Implemented in this pass

- Added `U` squad formation cycling with Tight, Standard, and Wide spacing.
- Made companion follow offsets, personal-space radius, avoidance radius, and
  regroup placement honor the current squad formation spacing.
- Updated the rescue-team HUD line to show the active formation state and the
  `Y`/`U` squad controls.
- Added emergency auto-medkit resilience: if a hostile hit leaves the player
  in the danger band and a medkit is available, the game can spend one medkit
  immediately and report the recovery.
- Added critical-health attack callouts that include where the attack came
  from when the emergency medkit is unavailable or cooling down.
- Added `Scripts/verify_june12_next100_improvement_pass.py` and wired it into
  `Run_Full_QA_Audit.command`.

## Next 100 Recommended Improvements

1. Squad formation cycling: let the player switch between Tight, Standard,
   and Wide support-team spacing during play.
2. Emergency auto-medkit: spend one medkit automatically when a hostile hit
   would leave the player in the danger band.
3. Squad-control HUD discoverability: keep `Y` regroup and `U` formation
   controls visible while the team is active.
4. Critical-health attack callouts: report attack direction and recovery
   advice when health drops sharply.
5. Static QA coverage for new squad/survivability controls.
6. Per-companion health bars or compact status pips in the squad HUD.
7. Downed-but-revivable companion state before permanent companion removal.
8. Manual medic-call command with cooldown and clear failure messaging.
9. Squad hold-position command for terminals, choke points, and extraction.
10. Squad silent/stealth mode that reduces support fire near learning areas.
11. Companion role icons above each teammate in world space.
12. Companion bark variety for regroup, low health, and reload events.
13. Companion ammo/resupply rules so support fire has visible limits.
14. Companion line-of-sight debug overlay for QA builds.
15. Companion teleport safety check that avoids placing teammates inside
   blocked geometry.
16. Regroup destination nav projection before teleporting teammates.
17. Friendly-fire immunity contract for companion shots and area effects.
18. Support-fire target priority by distance, threat angle, and player aim.
19. Medic triage priority that considers recent damage direction.
20. Engineer repair utility for barricades, doors, and route blockers.
21. Scout ping ability for nearest terminal, survivor, or extraction.
22. Heavy-rescue stagger or shove ability for close zombie pressure.
23. Squad command radial menu for gamepad-friendly support control.
24. Training-mode overlay that explains squad roles once per new save.
25. Save-game persistence for squad formation preference.
26. Save-game persistence for companion health and downed state.
27. Difficulty-specific squad tuning for support damage and medic cooldowns.
28. Onboarding objective that asks the player to test regroup and formation.
29. Better threat proximity meter with direction and distance bands.
30. Health danger vignette that scales with damage and armor state.
31. Armor-plate break feedback with sound, text, and HUD flash.
32. Last-stand cooldown indicator separate from medkit count.
33. More readable medkit/armor pickups with category-specific colors.
34. Pickup attraction radius for essential resources in early cities.
35. Explicit safehouse boundary visualization during combat.
36. Terminal-entry invulnerability grace during the UI transition.
37. Terminal-exit grace period before enemies can damage the player again.
38. Objective-distance smoothing so the HUD does not jitter every tick.
39. Compass strip with objective, survivor, terminal, and extraction bearings.
40. Minimap route overlay for safehouse and survivor paths.
41. Objective route color consistency across HUD, world signs, and beacons.
42. First-city tutorial prompts that disappear after successful use.
43. Input-remap screen for keyboard and controller users.
44. Controller glyph display instead of keyboard-only prompts.
45. Accessibility font-size setting for HUD and terminal text.
46. Colorblind-safe palette option for route markers and threat alerts.
47. Subtitle size and background opacity settings.
48. Audio mix sliders for music, radio, voice, UI, and combat.
49. Radio subtitle history panel for replaying briefing instructions.
50. Improved pause-menu QA page with build, package, and verifier status.
51. Death recap screen showing damage source, direction, and survival tips.
52. Victory recap screen showing cities, rescues, language progress, and
   squad survival.
53. Post-mission debrief for what the player learned in the coding challenge.
54. Per-language mastery goals that unlock cosmetic city signage.
55. Challenge hint ladder that escalates after repeated failed attempts.
56. Unit-test preview in terminal before final validation.
57. Better compiler error summarization for beginner-friendly wording.
58. Optional challenge timer mode for advanced players only.
59. No-pressure practice terminal accessible from the main menu.
60. City-specific terminal props that match the selected language theme.
61. Stronger survivor rescue ceremony with squad reactions and audio.
62. Survivor names and bios visible in the objective journal.
63. Optional side objectives that grant medkits, armor, or research points.
64. Dynamic zombie pacing based on player health and terminal progress.
65. Clearer boss/warden telegraphing before optional fights.
66. Boss arena boundary clarity and safe retreat route.
67. Zombie hit reactions by weapon category.
68. More varied zombie families with readable silhouettes.
69. Hit-zone VFX for head, torso, and limb shots.
70. Weapon recoil and recovery tuning per weapon role.
71. Weapon inspect or comparison panel in pause/journal UI.
72. Per-weapon ammo pickup balancing and scarcity rules.
73. Throwable trajectory preview for flare, smoke, and stim.
74. Smoke cloud visibility and AI behavior validation pass.
75. Flare lure radius HUD feedback and expiry warning.
76. Stim side-effect or cooldown so it remains tactical.
77. Barricade placement preview and blocked-placement feedback.
78. Barricade health indicator and repair affordance.
79. Jeep entry/exit polish and collision safety near access points.
80. Fast-travel confirmation when nearby enemies are active.
81. Performance budget dashboard for actor counts and tick costs.
82. Automated screenshot capture for key cities and HUD states.
83. Packaged-app startup screenshot smoke for visual regressions.
84. Broader smoke duration that survives at least 30 seconds of gameplay.
85. Automated input smoke for weapon cycling, medkit, regroup, and formation.
86. Static verifier for all documented default controls.
87. Static verifier for every roadmap item marked implemented.
88. Cook/package warning classification document with owners and expiry dates.
89. Full QA summary artifact emitted after every audit run.
90. Human playtest report template with severity and reproduction fields.
91. Main-menu build stamp showing package timestamp and git branch.
92. Distribution ZIP creation command with checksum output.
93. External notarization checklist for future signed Mac sharing.
94. Asset provenance manifest for imported content and license notes.
95. Fab/MetaHuman import status panel visible inside the editor map.
96. More distinct city identity pass for the first ten cities.
97. Lighting and fog pass for readability on laptop screens.
98. Audio ambience pass for safehouse, street, terminal, and extraction zones.
99. Narrative continuity pass across radio, mission boards, survivors, and
   squad barks.
100. External playtest package rubric covering install, launch, controls,
   performance, combat, learning flow, accessibility, and release readiness.

## Validation Plan

Minimum validation for this pass:

```bash
python3 Scripts/verify_june12_next100_improvement_pass.py
python3 Scripts/verify_june01_rescue_survivability_pass.py
./Recompile_Module.command
./Run_Full_QA_Audit.command
```

Recommended package validation before sharing a new app:

```bash
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

Validation completed on June 12 after implementation:

```bash
python3 Scripts/verify_june12_next100_improvement_pass.py
python3 Scripts/verify_june01_rescue_survivability_pass.py
./Recompile_Module.command
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

Fresh Mac package:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
Size: 1.9G
Timestamp: Jun 12 15:14:58 AKDT 2026
```

No new blocking regressions were found. The smoke scanner allowed only the
known immediate-quit navigation dirty-area diagnostic, the immediate-quit
crowd-following RecastNavMesh diagnostic, and the unattended macOS CoreAudio
sample-rate query warning in packaged render smoke.

## Manual Playtest Focus

- Press `U` several times and confirm the squad HUD cycles through Tight,
  Standard, and Wide formation states.
- Press `Y` after changing formation and confirm regroup placement honors the
  selected spacing.
- Take hostile damage at low health with at least one medkit available and
  confirm the emergency medkit fires once, consumes a medkit, and reports where
  the attack came from.
- Repeat the low-health case while the emergency medkit is cooling down and
  confirm the critical-health callout recommends `Q` or medic regroup instead.
- Verify the new HUD text remains readable at 1280x720.
