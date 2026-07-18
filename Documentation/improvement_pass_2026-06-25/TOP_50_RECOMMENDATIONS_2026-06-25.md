# Top 50 Recommendations for Continued Improvement — 2026-06-25

Prioritized, project-specific recommendations to move Operation Code Rescue from a strong prototype
toward a fully functional release. Status tags:

- **[DONE]** implemented in this pass
- **[STARTED]** begun this pass; rollout continues
- **[SPEC]** specified here / in the deep-dive PDFs for an upcoming pass

Priority bands: **P0** (release-critical), **P1** (high impact), **P2** (valuable polish).

---

## A. UX / UI & Player Experience (your emphasis)

1. **[DONE] P0 — Centralized UI design system.** Added `CodeRescueUITheme` (semantic color, type
   scale, spacing, helpers) so every screen shares one visual language. *Why:* the UI used dozens of
   ad-hoc literals. See `UX_OVERHAUL_GUIDE.md`.
2. **[DONE] P0 — Refactor menus onto the system.** Main Menu, Pause, Settings, Death, Victory
   restyled; Settings gained a readable backdrop.
3. **[DONE] P0 — Accessibility actually drives the UI.** High-contrast, reduced-motion, and text
   scale now flow from settings into `Theme()` and affect rendering (incl. the critical-HP vignette).
4. **[STARTED] P0 — Roll the theme into the HUD.** Replace HUD literals; drive the health bar color
   by health fraction (green→amber→red); gate the headshot pop behind reduced motion.
5. **[SPEC] P1 — Diegetic terminal restyle.** Give `CodeTerminalWidget` a CRT-flavored but readable
   look: phosphor-green prompts, red compile errors, monospace, subtle scanline material.
6. **[SPEC] P1 — First-time-user onboarding flow.** A guided 3–4 step overlay on first launch
   (move, look, interact `E`, language pick) using the existing tutorial widget + `first_ten_minutes` data.
7. **[SPEC] P1 — Consistent input glyphs.** Show key/controller glyphs in prompts that match the
   active control scheme; respect the existing "simplified hints" flag.
8. **[SPEC] P1 — UI motion language.** Add short, tasteful transitions (fade/slide ≤150 ms) via UMG
   animations, all suppressed under reduced motion, so the UI feels alive without distraction.
9. **[SPEC] P1 — Objective clarity pass.** Always-visible current objective + distance/direction;
   a non-intrusive waypoint marker in the world; clearer "terminal solved / survivor rescued" toasts.
10. **[SPEC] P2 — Pause-menu information architecture.** Group the 11 pause buttons into sections
    (Resume/Save · Loadout: Crafting/Skills · Options · Quit) to reduce a long flat list.
11. **[SPEC] P2 — Settings polish.** Add live value readouts next to each slider, a "Reset to
    defaults," and an explicit separate **UI text scale** (distinct from subtitle scale).
12. **[SPEC] P2 — Colorblind validation.** Verify the three modes against the new semantic palette;
    ensure danger/safe never rely on hue alone (add shape/icon cues).

## B. Character Art & Animation (release-critical fidelity) — see `CHARACTER_ANIMATION_*.pdf`

13. **[SPEC] P0 — Assign real skeletal meshes to the unassigned slots.** Wire the Fab zombie packs
    and a player/survivor mesh into `ProfessionalZombieMesh`/survivor slots; retire primitive fallbacks.
14. **[SPEC] P0 — Locomotion Animation Blueprints.** Author AnimBP + blendspaces for zombies,
    survivors, companion; FPS arms for the player. (Motion Matching is still experimental in 5.7 —
    use hand-authored state machines.)
15. **[SPEC] P0 — IK Retargeter sharing.** One source locomotion set retargeted across body types
    (player, survivors, zombie variants) to multiply animation coverage cheaply.
16. **[SPEC] P1 — Hit reactions & death.** Additive hit reacts + blended ragdoll death (ties to the
    physics pass) so combat reads.
17. **[SPEC] P1 — Unique silhouettes per character.** Art-direct distinct shapes/materials for the
    boss, dog, female, urban, modular zombies so each reads instantly in survival-horror lighting.
18. **[SPEC] P1 — Companion + survivor personality.** Idle variations, look-at, and rescue-moment
    montages so NPCs feel alive.
19. **[SPEC] P2 — Facial/voice sync.** With 230 voiced briefings, add audio-driven facial later
    (note: MetaHuman Animator isn't on Mac yet — defer or use a Windows/cloud step).
20. **[SPEC] P2 — Sequencer beats.** Short Control-Rig cinematics for intro, boss reveal, extraction.

## C. Physics — see `GAME_PHYSICS_*.pdf`

21. **[SPEC] P0 — Collision channel scheme.** Define explicit object/trace channels for
    player/zombie/projectile/cover/world; stop relying on defaults.
22. **[SPEC] P0 — Ragdoll & Physical Animation.** PhysicsAssets for zombies; `PhysicalAnimationComponent`
    for blended hit reactions and convincing deaths.
23. **[SPEC] P1 — Real throwable impulses.** Give `AThrowableActor` projectile movement + radial
    impulse on detonation; physical materials for surface-specific impacts.
24. **[SPEC] P1 — Chaos Vehicles for the Jeep.** Replace floating-pawn movement with `ChaosVehicles`
    (wheels/suspension) tuned arcade-leaning for feel.
25. **[SPEC] P1 — Determinism & fixed tick.** Enable async/substepped physics with a fixed timestep
    so feel is frame-rate independent across Macs.
26. **[SPEC] P2 — Chaos Destruction for cover.** Convert tagged "destructible" cover into Geometry
    Collections with a strict performance budget.
27. **[SPEC] P2 — Combat "juice".** Camera shake, hit-stop, impact Niagara + decals, controller
    rumble — most are cheap and dramatically raise game-feel.
28. **[SPEC] P2 — Cloth/secondary motion.** Optional Chaos Cloth on coats/flags for atmosphere.

## D. World, Level Design & Art — see `WORLD_DEVELOPMENT_*.pdf`

29. **[SPEC] P0 — Modular city kit.** Build one high-quality modular building/prop/trim-sheet kit
    (Nanite where sensible) that all 465 cities reuse — the key to quality *and* scale.
30. **[SPEC] P0 — Evolve runtime-procedural into PCG + authored hybrid.** Keep the C++ spawn spine,
    but drive block/prop/clutter placement through PCG and hand-author "hero" areas per city family.
31. **[SPEC] P0 — World Partition + streaming.** Adopt World Partition/Data Layers/Level Instances
    so authored content streams; a per-city data model selects kit + landmarks + mission beats.
32. **[SPEC] P1 — Lighting & atmosphere for mood + readability.** Lumen (mind Apple-Silicon RT
    tiers), VSM, volumetric/height fog tuned per city; use light to guide the eye and sell horror.
33. **[SPEC] P1 — Landmarks & wayfinding.** Distinct silhouettes/landmarks per city so navigation
    and memory work; tie to the minimap and objective markers.
34. **[SPEC] P1 — Encounter & pacing design.** Deliberate rhythm of tension (combat) and relief
    (terminal "safe" beats); design the boss arena and rescue set-pieces as authored spaces.
35. **[SPEC] P2 — Environmental storytelling.** Props, graffiti, and scene-setting that convey the
    outbreak and the "coding rescues people" premise without exposition.
36. **[SPEC] P2 — World bible & lore.** A short canonical document: premise, factions, why coding
    matters, tone — to keep 465 cities coherent and guide audio/art.

## E. Combat & Core Gameplay

37. **[DONE] P1 — Reduced-motion combat feedback.** Critical-HP vignette holds steady instead of
    pulsing when reduced motion is on (`CodeRescueDamageFeedbackWidget`).
38. **[SPEC] P1 — Weapon feel pass.** Distinct fire cadence, recoil, reload timing, and audio per
    weapon; clear ammo/reload HUD states (ties to rec 4).
39. **[SPEC] P1 — Enemy variety & telegraphs.** Differentiate zombie types by speed/health/attack
    and give readable wind-ups; make the boss a real mechanic, not a bullet sponge.
40. **[SPEC] P2 — Stealth/avoidance option.** Line-of-sight + noise so players can sometimes evade
    rather than fight — fits the survival-horror and "think first" coding theme.

## F. AI

41. **[SPEC] P1 — Behavior Tree / StateTree migration.** Promote the C++ chase fallback into a
    Behavior Tree or StateTree + EQS for cover, flanking, and search — easier to tune and extend.
42. **[SPEC] P2 — Director-driven pacing.** A lightweight encounter director that modulates spawns
    to the pacing curve (rec 34) instead of flat difficulty.

## G. Audio

43. **[SPEC] P1 — Spatial + reactive audio.** Attenuation/occlusion, a music system that responds to
    threat state, and authored squad/zombie barks to replace remaining procedural lines.
44. **[SPEC] P2 — Mix & accessibility.** Real per-bus volumes (Sound Class/Submix), plus a "mono /
    visualize sounds" accessibility option.

## H. Progression, Content & Education

45. **[SPEC] P1 — Tighten the teach-then-apply loop.** Ensure each new coding concept is introduced,
    practiced, and reinforced; add post-solve explanations for advanced lessons.
46. **[SPEC] P2 — Meta-progression clarity.** Make skill tree / crafting / research legible and
    rewarding; show mastery and streaks prominently.
47. **[SPEC] P2 — Difficulty & onboarding matrix.** Validate Story→Nightmare feel and the
    first-ten-minutes path with a real human pass.

## I. Performance & Technical Debt

48. **[SPEC] P0 — Split `CodeRescueGameMode.cpp` (~12k lines).** Refactor by ownership (city
    identity, spawning, set-pieces, world gen) into separate units — reduces risk for every future change.
49. **[SPEC] P1 — Performance budgets on Apple Silicon.** Establish frame budgets; use HLODs,
    instancing, LODs; add per-layer runtime telemetry (static profiling already exists).

## J. Release Readiness

50. **[SPEC] P0 — The human + distribution gates.** A scheduled first-ten-minutes human playtest,
    per-difficulty balance review, then Apple Developer signing/notarization and (eventually) hosted
    Mac CI. These are the documented final boundaries to shipping.

---

### Suggested sequencing

- **Now / this pass:** A1–A4 (UX), E37 (done); begin A4/A5 HUD+terminal rollout.
- **Next (fidelity vertical slice):** C21–C23 + B13–B16 + D29–D31 on **one** city → a polished,
  representative slice proving the release pipeline end-to-end.
- **Then (scale + finish):** propagate the kit/PCG to city families, AI migration, audio, the
  GameMode refactor (48), and the release gates (50).
