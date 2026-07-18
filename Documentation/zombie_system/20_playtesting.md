# Item 20 — Playtest protocol

**Status:** DEFERRED — needs real students. Documented as the structured
session protocol to run when one is available.

## Goal

Confirm that the loop "find terminal → write code → solve challenge →
rescue survivor → repeat" is fun and educational for the target
audience (students learning to code). Everything else is
infrastructure; this is the only test that tells us whether the
infrastructure adds up to a *game*.

## Recruit profile

Five students at minimum, at the level the curriculum is designed for
(intro programming — high school CS, college freshman intro, or
self-taught early). Mix of Java/C/Python/MATLAB familiarity. None
should have seen the project before.

## Session structure (~45 min per student)

### Pre-session (5 min)

- One-line context: "This is a coding-rescue prototype. You're trying
  to evacuate a zombie-overrun city. Some doors only open if you write
  the right code at the terminal. Just play and think out loud — I'll
  watch."
- DON'T explain mechanics. The HUD prompts and on-screen guidance text
  are doing that work; this test reveals whether they're enough.

### Session (30 min)

- Camera on screen + face if recording. Take handwritten notes on a
  3-column page: **Time | Player did X | I noticed**.
- Watchpoints (top priority for early sessions):
  1. Did they figure out **WASD + LMB shoot** within 60 seconds?
  2. Did they realize **terminals open on E** without being told?
     (Crosshair-yellow + on-screen prompt are the only cues — see
     item 13/14.)
  3. When they opened a terminal, did the prompt language make sense?
     Did they know what function shape to write?
  4. On their first wrong submission, did the validation feedback help
     them iterate, or did they get stuck? (See `UCodeRunnerLibrary`
     — the regex shapes are designed to be permissive but the player
     has to know what's expected.)
  5. Did they get killed? If so, did the **death widget** make sense?
     Did they pick Restart-from-Save vs Restart-Fresh by intent or
     accident?
  6. Did they ever find the **Tokyo metro zone** without prompting?
     (Far-zone discoverability is a known concern.)

### Debrief (10 min)

Don't ask "did you have fun?" (compliance bias). Instead:

- "What confused you most?"
- "What did you wish was there that wasn't?"
- "When did you feel stuck?"
- "When did you feel like you 'got it'?"

Write these verbatim, not paraphrased.

## Specific prompts to validate

For each new challenge type added in item 15, test that at least one
real-target student can solve it without external help:

- `hospital_string_reverse` — Anchorage
- `dock_palindrome_check` — Seattle
- `metro_fizzbuzz_signal` — Tokyo
- `triage_even_filter` — Anchorage

If a student can't solve `fizzbuzz` (the most universally taught
beginner exercise) the prompt copy needs work, not the validator.

## Success threshold

After 5 students:

- ≥ 4 of 5 found and used WASD/E/LMB without verbal help
- ≥ 3 of 5 solved their first terminal in under 5 minutes
- ≥ 4 of 5 reached at least one survivor rescue
- 0 of 5 got stuck on a non-coding bug (collision, soft-lock, audio,
  HUD mis-information)

If any of these miss, file specific tickets against the per-item docs
and iterate before the next round.

## Why this can't be done in-session

Needs real humans, real attention, real recording. The other 19
infrastructure items can be measured against runtime behavior; this one
only against a person.
