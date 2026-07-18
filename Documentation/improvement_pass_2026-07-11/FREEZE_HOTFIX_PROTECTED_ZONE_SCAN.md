# Hotfix — game-thread freeze + dead movement in late-campaign saves
**Date:** 2026-07-16 (same evening as Pass 5) · **Status:** shipped

Kenny's report on the pass-5 build: "Character movement is almost
non-functional" and "Game is rendered completely frozen and non-functional."

## Reproduction and forensics

No new screenshots, no crash logs, no hang reports — the app was alive but
useless. Reproduced headlessly with his real container save:
`CodeRescueUnreal.app ... -CodeRescueAutoResumeLanguage=Cpp -stdout` resumed
his C++ track (city 03 Chicago) and then **spun at 400%+ CPU with the frame
counter parked at single digits for minutes** — stuck INSIDE a frame, so
nothing logged and no hitch detector fired.

A macOS `sample` profile of the spinning process named the culprit:

```
ACodeRescueAIController::UpdateState
  -> ACodeRescueGameMode::IsLocationInsideProtectedLearningZone
       -> TActorIterator<AActor> (GetObjectsOfClass over the WHOLE world)
```

## Root cause

`IsLocationInsideProtectedLearningZone` iterated **every actor in the world**
(checking five tags each, plus `GetComponentsBoundingBox` on matches) on
every call — and it is called per AI controller, per zombie, and from player
tick paths. A late-campaign city carries a large actor population and his
save resumes into a swarm: O(agents × world) tag scans per frame crossed the
cliff from "heavy" to "one frame takes seconds." Both symptoms are the same
bug — at fractional FPS, movement input feels dead and the render looks
frozen.

Why the pass-5 audits missed it: the 21-token integrated audit runs a FRESH
first level (small actor population, short session) — it passed 21/21 on the
exact binary that froze on a dense resumed save.

## Fix

1. **Cached zone snapshot** (`CodeRescueGameMode.cpp`): the protected anchors
   (terminal labs, challenge zones — rare, mostly static markers) are scanned
   once and snapshotted (bounds + location); queries are pure math against
   the snapshot. Refresh at most every 4 s, keyed per world, rebuilt on world
   change/time reset. All call sites (AI, zombies, player) go through the one
   static function, so one fix covers everything.
2. **Marker tick culling** (`CodeRescueMessageMarkerActor` +
   `CodeRescueBeaconMarkerActor`): the face-the-player turn + bob pushed a
   full transform update through TextRender children every frame per marker
   (hundreds in a built-out city — 19% of the actor tick in the profile).
   Markers beyond 7 000 uu now hold still (`bMarkerAnimationCulled`,
   respected by the beacon subclass pulse too).
3. **`[ResumeHealth]` heartbeat**: under `-CodeRescueAutoResumeLanguage` every
   player character (menu and post-resume — the level swap destroys the first
   one) logs frame counter + `GAverageFPS` + position every 10 s. A hard spin
   logs nothing, so absence-of-logs was ambiguous; now any "frozen" report
   gets a numeric pulse to read.

## Verification

- Editor `-game` resume: bursts ~90–100 FPS post-resume (was: stuck).
- Watchdog: **VERDICT: PASS** (0 REAL).
- Editor integrated audit: **COMPLETE PASS (21/21)**.
- Repackaged; **packaged app resuming Kenny's real save**: `[ResumeHealth]`
  steady **93–104 FPS** across 90+ s in city 03 Chicago with live zombies,
  zero stuck-movement warnings.
- Packaged integrated audit: **COMPLETE PASS (21/21)**.
- Physics-asset scale theory tested and CLEARED first (all reimported
  characters measure bounds r≈100, correct); ADS stuck-state paths audited
  and cleared (EndAim/bUIOpen gating sound).

## Notes / lessons

- The UE log's bracketed frame field is `GFrameCounter % 1000` — it WRAPS;
  don't read a smaller number as a level reload.
- A launch via `Engine/Binaries/Mac/UnrealEditor` (non-.app) relaunches
  itself into the .app bundle and drops `-stdout` piping — pass `-ABSLOG`
  or launch the bundle binary directly.
- Editor-context grep filters (`LogPython.*Error`) hide `LogTemp` markers
  like `[V3PhysFix]` — filter on the marker, not the category.
- Sparse gameplay logs are not a liveness signal; the heartbeat is.
- Occasional `CodeZombieActor_N is stuck and failed to move` warnings appear
  in dense editor swarms (spawn jostling, self-recovering, absent from the
  packaged run) — cosmetic for now, worth a spawn-spacing look someday.
