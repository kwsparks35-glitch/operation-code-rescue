# Pass 2026-07-04 Part 4 — Scaffold Fading, Spaced Review, Guidance Trail, Photo Mode

Roadmap items 31, 29, 45, 50 + one honesty fix. Same discipline: code-verified,
live-verified in the crashing-combo smoke (editor -game + bypass), then repackaged
and soaked from the shipped pak.

## Item 31 — Worked-example fading (the worked-example effect, operationalized)

The teach panel's worked example now FADES as competence grows, per concept, from
saved progress (`ConceptProgress.SuccessfulValidations`):

- 0 solves in the concept → full worked example (as before);
- 1 solve → "Fading scaffold": the example is truncated at its arrow ("c=100 -> …")
  and the player is asked to finish the trace themselves;
- 2+ solves → "Scaffold faded (N solves…)": recall prompt only — say the first
  step aloud, then code it.

This turns the curriculum's static examples into an adaptive ramp with zero new
data required — exactly why R1/R2 had to land first.

## Item 29 — Spaced repetition of weak concepts

Every third city (`CityIndex % 3 == 2`), terminal selection checks saved concept
stats for the WEAKEST concept (success rate < 50% across ≥ 2 attempts) and, if a
compatible curriculum entry exists, serves THAT instead of the next ladder step —
logged as `[SpacedReview] city N reviews weak concept '…' (…% success)`. Falls
through to the normal tier ladder when the player has no qualifying weakness.
Also aligned the telemetry keys: validation attempts now record under the
data-driven challenge's own concept when one is active, so the meter, the fading,
and the review all read the same ledger.

## Item 45 — Guidance trail to the main terminal

Each city now lays 4–14 small emissive cyan strips (count scales with distance)
in a straight ground-snapped line from the arrival plaza to the MAIN terminal —
never the secret one (guarded by `TerminalId` match). New players always have a
physical path into the core loop. Live log: `[GuidanceTrail] new_york_ny_sum:
4 strips over 21m`.

## Item 50 — Photo mode

**F10** toggles photo mode: every viewport widget is hidden (original visibility
recorded and restored exactly — including the damage vignette, which is why photo
shots suddenly show the world's real lighting), time dilation drops to 12% for
posing shots, and F12 remains the engine screenshot key. Refuses to trigger while
a modal UI is open. C/V still cycle all camera perspectives inside photo mode.
Live-verified: `[PhotoMode] ON — HUD hidden (2 widgets)` + a captured HUD-free
frame; OFF restores everything.

## Honesty fix

`Run_Character_World_Demo.command` claimed "5 FPS | 6 TPS | …" — those number
keys have been WEAPON slots since the input rebuild. Help text now says: cameras
cycle on C/V, numbers select weapons, F10 is photo mode. (Found while
playtesting part 3 — the stale hint cost real debugging time.)

## Verification

- `verify_fading_review_trail_photomode_2026_07_04c.py` — ALL CHECKS PASSED.
- `Recompile_Module.command` → Result: Succeeded (first try).
- Live editor-game+bypass smoke: trail + photo-mode log lines above, appError=0.
- Packaged: build #5 + shipped-pak soak results appended below by the wrap-up.

Known follow-ups (numbered for the next pass): arena-recovery point sits too close
to a wall (spring arm collapses — old item 6); dedicated v2 first-person arms +
weapon-socket composition (item 41 extension).
