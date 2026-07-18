# Next 100 Recommended Improvements: Squad Command and Status Continuation

Date: 2026-06-12

## Purpose

This document lists the next 100 recommended improvements after the formation
and emergency-medkit package pass, then records the first command/status slice
started immediately from that roadmap.

## Implemented in this pass

- Added compact squad health/status pips in the HUD.
- Added direct `N` medic call for low-health recovery.
- Added `O` squad hold/follow order.
- Added hold-aware companion behavior that preserves support fire and medic
  utility while teammates remain at ordered positions.
- Added static verifier coverage for these new command/status contracts.

## Next 100 Recommended Improvements

1. Compact companion health/status pips in the squad HUD.
2. Manual medic-call command with clear success, cooldown, and failure text.
3. Squad hold/follow command for terminals, choke points, and extraction.
4. Hold-aware companion movement that preserves support fire.
5. HUD discoverability for medic and hold commands.
6. Static QA coverage for squad pips, medic call, and hold/follow behavior.
7. Downed-but-revivable companion state before permanent removal.
8. Companion revive command with medic/engineer role gating.
9. Companion rescue beacon when a teammate is downed behind the player.
10. Save-game persistence for squad formation preference.
11. Save-game persistence for squad hold/follow order.
12. Save-game persistence for companion health.
13. Save-game persistence for companion downed/revived state.
14. Per-role companion armor/health tuning.
15. Per-role movement speed tuning.
16. Per-role support-fire range tuning.
17. Per-role cooldown HUD icons.
18. Medic call controller binding and glyph text.
19. Hold/follow controller binding and glyph text.
20. Squad command radial menu for gamepad users.
21. Companion role icon billboards above teammates.
22. Companion outline colors by role.
23. Companion low-health world marker.
24. Companion bark variety for hold, follow, medic, and regroup.
25. Companion reload bark with cooldown throttling.
26. Companion out-of-ammo state and resupply rules.
27. Companion ammo pips in the HUD.
28. Companion support-fire target priority by threat angle.
29. Companion support-fire target priority by distance to player.
30. Companion support-fire target priority by player aim focus.
31. Friendly-fire immunity test for companion shots.
32. Companion line-of-sight debug overlay for QA builds.
33. Regroup nav projection before teleporting teammates.
34. Hold-position nav projection before storing hold points.
35. Hold-position blocked-geometry fallback.
36. Hold-position maximum leash with explicit return warning.
37. Squad scatter command for area attacks.
38. Squad silent mode near learning safehouses.
39. Squad loud mode for extraction defense.
40. Squad defend-survivor order.
41. Scout ping for nearest terminal.
42. Scout ping for nearest survivor.
43. Scout ping for extraction route.
44. Engineer route-clear assist for light blockers.
45. Engineer barricade repair assist.
46. Heavy-rescue stagger ability for close zombie pressure.
47. Heavy-rescue shove cooldown HUD state.
48. Rifle-support suppressive fire state.
49. Medic triage priority based on recent attack direction.
50. Medic pulse overheal cap for easier difficulty only.
51. Difficulty-specific medic cooldown tuning.
52. Difficulty-specific support-fire damage tuning.
53. Difficulty-specific companion survivability tuning.
54. First-city objective teaching `Y`, `U`, `N`, and `O`.
55. One-time squad command tutorial prompt.
56. Pause-menu command reference for squad controls.
57. Main-menu command reference for squad controls.
58. Settings toggle for auto-medkit behavior.
59. Settings toggle for companion chatter frequency.
60. Settings toggle for squad HUD verbosity.
61. Accessibility scale for squad HUD text.
62. Colorblind-safe squad role palette.
63. Threat-direction compass strip.
64. Low-health vignette tied to armor and medkit availability.
65. Armor break flash and audio feedback.
66. Manual medkit cooldown/readiness HUD affordance.
67. Last-stand cooldown indicator.
68. Death recap with final damage source and squad status.
69. Victory recap with squad survival summary.
70. Post-mission debrief with teammate contributions.
71. Survivor rescue ceremony with squad reactions.
72. Survivor objective journal entries with rescue context.
73. Optional squad-rescue side objectives.
74. Dynamic zombie pacing based on player health.
75. Dynamic zombie pacing based on companion losses.
76. Safehouse boundary visualization.
77. Terminal-entry invulnerability grace.
78. Terminal-exit invulnerability grace.
79. Terminal solve reward that refreshes one squad cooldown.
80. Route marker consistency between HUD, minimap, and world signs.
81. Minimap squad dots by role.
82. Minimap nearest medic indicator.
83. Minimap held-position marker.
84. Automated input smoke for `Y`, `U`, `N`, and `O`.
85. Automated screenshot smoke for squad HUD states.
86. Packaged startup screenshot smoke.
87. Thirty-second gameplay smoke with squad commands.
88. Full QA summary artifact emitted after every audit run.
89. Cook/package warning classification with owners.
90. Distribution ZIP creation with checksum output.
91. Main-menu build/package timestamp stamp.
92. External playtest rubric for squad controls.
93. Human playtest report template with reproduction fields.
94. Asset provenance manifest for companion meshes and audio.
95. Audio ambience pass for command and medic feedback.
96. Lighting readability pass for combat with full squad.
97. First-ten-city identity pass for landmarks and safehouses.
98. Radio continuity pass for squad role narration.
99. Narrative continuity pass for survivor, squad, and terminal arcs.
100. Release readiness gate combining QA, package, smoke, and playtest status.

## Validation Plan

Minimum validation for this pass:

```bash
python3 Scripts/verify_june12_squad_command_status_pass.py
python3 Scripts/verify_june12_next100_improvement_pass.py
python3 Scripts/verify_june01_rescue_survivability_pass.py
./Recompile_Module.command
```

Recommended broader validation before sharing:

```bash
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

Validation completed on June 12 for this code/QA slice:

```bash
python3 Scripts/verify_june12_squad_command_status_pass.py
python3 Scripts/verify_june12_next100_improvement_pass.py
python3 Scripts/verify_june01_rescue_survivability_pass.py
./Recompile_Module.command
./Run_Full_QA_Audit.command
```

The full QA audit rebuilt the module, passed static verifiers including
`verify_june12_squad_command_status_pass.py`, ran the Unreal commandlets,
launched headless runtime smoke, and completed the smoke log scan.

No new blocking regressions were found. The smoke scanner allowed only the
known immediate-quit navigation dirty-area diagnostic and the known
immediate-quit crowd-following RecastNavMesh diagnostic.

Package note: this pass did not rebuild `PackagedMac/Mac/CodeRescueUnreal.app`;
the current package still points to the previous June 12 package until a new
package/smoke cycle is run.

## Manual Playtest Focus

- Confirm the squad HUD shows compact health/status pips for active teammates.
- Press `N` at low health with an operational medic and confirm an immediate
  manual medic pulse or an exact cooldown/failure message.
- Press `O` to hold the squad, move away, and confirm teammates remain near
  their held positions while still firing and medic-supporting when relevant.
- Press `O` again and confirm squad members resume following.
- Press `Y` while hold is active and confirm the squad regroups then keeps the
  newly ordered held positions.
