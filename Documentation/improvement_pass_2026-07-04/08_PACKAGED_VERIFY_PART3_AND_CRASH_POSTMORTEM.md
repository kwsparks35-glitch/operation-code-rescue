# Part 3 — Packaged Verification + Frame-1 Physics-Race Crash Postmortem

## The playtest paid for itself: a real crash found, root-caused, and fixed

While soak-testing this slice in the editor `-game` + `-CodeRescueBypassLaunchLanguageMenu`
combo (first time that exact combo was ever run), the game asserted ~15s after boot:

```
Assertion failed: (Index >= 0) & (Index < ArrayNum)  [Array.h:1095]
Array index out of bounds: 1 into an array of size 0
  UPhysicalAnimationComponent::UpdatePhysicsEngineImp()
```

**Forensics (breadcrumb logging at the bind site) showed the truth:** the crashing
zombies were PACK zombies (SK_Zombie_M04 / ZombieFemale_Nurse) — not the new v2
infected — binding the physical-animation hit-reaction profile while their skeletal
meshes reported `Bodies.Num()==0` (physics states not yet created during the
frame-1 world spawn), or holding a stale profile across a mesh swap. A one-step
bisect (stash to part-2, rebuild, rerun) proved the crash predates this slice; the
combo had simply never been exercised — every packaged run, including bypass
smokes, was and is clean (`appError=0` in every container log since 07-02).

**Fixes (all shipped):**
1. `BindPhysicalHitReactionComponent` refuses to bind while `Bodies.Num()==0` —
   retries every 0.5s up to 5×, then falls back to pose-only hit reactions
   (logged: `[PhysAnimBind] ... -> pose-only hit reactions`).
2. `ApplyProfessionalVisuals` detaches the physical-anim component BEFORE any mesh
   swap (stale body-indexed drive data across swaps was the second trigger).
3. A pre-physics per-tick guard disarms the component any frame the mesh's bodies
   are missing — the actor ticks pre-physics, physical anim ticks post-physics, so
   the guard is deterministic. The crash is now impossible by construction.
4. (Belt & braces from earlier in the session) v2 zombies never bind physical
   animation and carry no physics asset.

**Soak proof (editor -game + bypass, the crashing combo):** 160+ seconds alive,
`appError=0`, all world markers present, live combat (dog infected killed), retry/
fallback breadcrumbs behaving as designed.

## First-person presentation fixes (found in the same session)

- With the v2 body, first person previously showed the inside of Kenny's own head:
  `ApplyCameraPerspective` now sets `OwnerNoSee(bFirstPerson)` on the body mesh.
- The legacy silver-mannequin FP arms clipped the near-plane and clashed with the
  authored survivor — hidden when the v2 body is equipped (dedicated v2 FP arms =
  follow-up, extension of top-50 item 41).
- FP weapon model pushed clear of the near-plane (long guns 58,24,-22; compact
  46,20,-18). Composition polish continues next pass.
- Operational note: the launcher help text "5 FPS | 6 TPS…" is stale — number keys
  are WEAPON SLOTS; cameras cycle with C/V. Also: tight spawn corners collapse the
  third-person spring arm to zero (old roadmap item 6) — walk clear or cycle C.

## Final packaged build of the day

- `Package_Mac_App.command` #4 → BUILD SUCCESSFUL (see `/tmp`-logged output copied
  in commit message); pak refreshed with: curriculum-60, prediction drill, mastery
  meter, debrief, minimap beacons, crash fixes, FP fixes.
- Shipped-pak soak (bypass boot from the .app binary): `[CityKit] 27/0`,
  `[Streetscape] 41/0`, `[NightSky] dome=ok moon=ok`, `[CharacterV2] SurvivorKenny`,
  `LogMaterial warnings: 0`, **`appError: 0` over a ≥2-minute soak** — the packaged
  verification checklist now permanently includes the crash grep + soak time
  (lesson: a marker grep at 60s is not a playtest).
