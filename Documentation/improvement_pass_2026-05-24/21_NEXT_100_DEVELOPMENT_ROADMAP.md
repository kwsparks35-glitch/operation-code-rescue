# Operation Code Rescue - Next 100 Development and Improvement Items

Date: 2026-05-24

Status: implemented in the 2026-05-24 Next 100 systems pass. These items were completed as a systemic game-improvement layer: campaign audit data, terminal learning coaching, in-world procedural props/signage, character/mentor markers, progression kiosks, accessibility/polish boards, QA hooks, and automated verification.

Completion ledger: see `22_NEXT_100_IMPLEMENTATION_PASS.md` for the item-by-item complete status, touched files, verification commands, and residual polish notes.

## Curriculum and Coding Gameplay

1. Add a curriculum map screen that shows all five learning stages and the player's current city/stage position.
2. Add per-language lesson tracks so Java, C, Python, and MATLAB each show tailored progression notes.
3. Add short in-terminal "why this matters" blurbs for each task type.
4. Add pre-challenge micro-prompts that ask the player to predict the expected output before coding.
5. Add optional worked examples for each lesson kind, gated behind a lower score reward.
6. Add code-trace mini-games where the player steps through a loop or condition before writing code.
7. Add a visual debugger overlay for arrays, strings, booleans, linked-list index chains, and binary-search bounds.
8. Add per-mission mastery badges for clean pass, no-hint pass, first-try pass, and fast pass.
9. Add spaced-review missions that reintroduce older concepts at higher difficulty.
10. Add a "mistake glossary" that explains common failed tests in plain language.
11. Add hidden-test replay explanations after completion so learning does not stop at pass/fail.
12. Add challenge variants for nested loops after the current FizzBuzz/even-filter stage.
13. Add dictionary/map lookup missions for later collection stages.
14. Add stack and queue missions using rescue-route metaphors.
15. Add recursion preview missions for late-stage algorithmic rescue towers.
16. Add sorting-intuition missions before binary-search-heavy cities.
17. Add string parsing missions tied to radio-code cleanup.
18. Add input-validation missions where bad data must be handled safely.
19. Add multi-function challenges once the player clears the first full stage.
20. Add final stage capstone missions that combine two or three concepts in one rescue operation.

## City, World, and Architecture Development

21. Build a visible city-selection atlas with regional color and architecture labels.
22. Add more skyline silhouettes per art kit so major cities feel less repetitive.
23. Add distinct street furniture sets for downtown, waterfront, desert, mountain, historic, and high-tech cities.
24. Add regional transit markers such as metro entrances, ferry terminals, tram stops, or bus hubs.
25. Add city-specific arrival plazas with landmark-inspired geometry.
26. Add environmental storytelling props around each terminal that reflect the coding task.
27. Add district boundary signage so players can read the city's neighborhoods at a glance.
28. Add unique lighting moods per region and time of day.
29. Add small vertical landmarks for navigation, such as towers, cranes, monuments, and beacons.
30. Add city completion transformations that visually restore power, lights, or civic order.
31. Add more non-blocking vista details beyond the playable boundary.
32. Add alley, rooftop, and underpass variants for denser traversal spaces.
33. Add landmark-specific rescue staging areas for the top 25 campaign cities.
34. Add art-kit material variation so buildings do not share the same surface language too often.
35. Add curriculum-themed murals and civic banners in each stage band.
36. Add more water, bridge, rail, and park details in cities whose art kits imply them.
37. Add local relief-camp layouts that vary by climate and region.
38. Add objective-route composition passes for the first 20 cities to maximize early polish.
39. Add special late-game algorithmic districts with more complex spatial puzzles.
40. Add a city beautification pass that tunes scale, color, silhouette, and readability across all art kits.

## Characters, NPCs, and Story

41. Add named mentor NPCs for each programming language.
42. Add survivor archetypes tied to coding concepts, such as network engineer, data medic, systems mechanic, and robotics student.
43. Add short survivor dialogue after each rescue that reinforces the current lesson.
44. Add companion banter when the player changes camera perspective or enters a new district.
45. Add idle animations or pose variation for civilian set pieces.
46. Add NPC role icons above friendly characters.
47. Add mission-specific NPC requests that make coding objectives feel socially motivated.
48. Add recurring characters who appear across multiple cities and react to the player's progress.
49. Add visual differentiation for mentors, survivors, traders, medics, and engineers.
50. Add a codex/journal page for major characters and rescued allies.
51. Add enemy variants that visually communicate threat behavior more clearly.
52. Add boss intro labels and readable ability tells.
53. Add non-combat civilian activity loops near safe areas.
54. Add companion upgrade milestones connected to curriculum progress.
55. Add rescue-team radio callouts for streaks, failed tests, and successful validations.
56. Add post-mission character debriefs that summarize what the player learned.
57. Add special character vignettes in capstone cities.
58. Add crowd-presence proxies in safe zones so restored cities feel alive.
59. Add accessibility-friendly nameplates and interaction outlines for all important characters.
60. Add a character-aesthetic QA script that verifies skeletal meshes, anim classes, labels, and interaction components.

## Gameplay Flow, Objectives, and Progression

61. Add a clearer first-session campaign path from tutorial plaza to first terminal to first rescue.
62. Add objective-chain scoring that rewards solving, rescuing, and extracting without confusion.
63. Add mission replay from the city journal.
64. Add an optional "practice only" terminal mode that does not affect campaign score.
65. Add difficulty tuning bands for beginner, normal, and challenge learning modes.
66. Add encounter pacing rules that reduce combat pressure while the terminal UI is open.
67. Add stronger return-to-objective markers after combat distractions.
68. Add a city completion ceremony for every stage transition.
69. Add route previews before entering large cities.
70. Add fast-travel previews showing current stage, lesson, difficulty, and reward.
71. Add player-facing explanation for hidden tests and why they matter.
72. Add objective fail-safe logic if a spawned terminal, survivor, or extraction point becomes unreachable.
73. Add optional side objectives tied to code quality, comments, and edge cases.
74. Add recovery objectives that help the player regain resources after repeated failures.
75. Add clearer boss-horde pacing after terminal completion.
76. Add per-city reward choices, such as ammo, medkit, companion boost, or score multiplier.
77. Add stage-end recap screens.
78. Add long-term player profile stats for concepts mastered.
79. Add a city recommendation system that suggests review missions when the player struggles.
80. Add save-slot preview details for stage, city, language, and recent mastery badges.

## UI, Accessibility, Audio, and Polish

81. Add scalable UI settings for terminal font size, HUD size, subtitle size, and contrast.
82. Add colorblind-safe variants for lesson colors, objective markers, and test result states.
83. Add keyboard remapping for camera, interact, terminal, journal, and combat controls.
84. Add controller navigation for all terminal and menu flows.
85. Add screen-reader-friendly text summaries for mission briefs and test feedback where feasible.
86. Add better terminal error formatting with line wrapping and severity labels.
87. Add audio stingers for stage completion, no-hint validation, rescue completion, and city graduation.
88. Add ambient loops per art kit or region.
89. Add optional low-combat learning mode for players who want the coding loop without pressure.
90. Add photo-mode or inspect-mode for reviewing city architecture and character scenes.
91. Add main-menu access to curriculum progress, settings, save slots, and credits.
92. Add loading-screen tips tied to the next mission's coding concept.
93. Add subtitle-safe radio briefing layout that never overlaps the terminal UI.
94. Add HUD state cleanup tests for pause, death, victory, terminal, journal, and fast-travel transitions.
95. Add clearer visual hierarchy to objective text, city labels, and coding prompts.

## QA, Tooling, Performance, and Release Readiness

96. Add a runtime spawn audit that samples representative cities from each stage and verifies all required actors appear.
97. Add automated smoke tests for tutorial flow, terminal validation, rescue, extraction, city graduation, and fast travel.
98. Add performance budgets for active enemies, background proxies, city props, lights, and text actors.
99. Add packaged-build validation for the new curriculum audit and asset compatibility paths.
100. Add a weekly release checklist covering build, package, smoke, curriculum audit, asset audit, accessibility review, and manual playtest sign-off.

## Recommended Implementation Order

1. Start with items 1, 7, 8, 10, 60, 72, 81, 94, 96, and 97 because they improve learning clarity and test reliability.
2. Next complete items 21, 25, 30, 38, 41, 43, 61, 63, 87, and 98 to improve presentation and first-session feel.
3. Then move through the remaining curriculum expansions and city-art passes in stage order so the game stays coherent as it grows.

## Documentation Notes

- This roadmap is now implemented through the systemic Next 100 pass.
- The completion ledger is stored in `22_NEXT_100_IMPLEMENTATION_PASS.md`.
- Any item that creates or changes gameplay behavior should include at least one commandlet, smoke, or manual QA note in the session report.
