# Regression Repair Implementation and Test Report

Date: 2026-07-11

## Scope

This report records the implementation and verification for all eight issues
reported with `Code_Accepted_Screen.png`. Changes are shared where the defect was
systemic, while authored presentation and acceptance coverage remain focused on
the first playable level.

## Issue Traceability

### 1. Overlapping narration

Root cause: a resumed run could briefly construct one city and then restore a
different city. Each load launched an independent cooked cue or `/usr/bin/say`
process, and GameMode did not own or stop the prior voice.

Implementation:

- GameMode now owns one `UAudioComponent` or one system speech process.
- A city-switch debounce waits 0.85 seconds and speaks only for the city that
  remains active.
- Every replacement and `EndPlay` stops, closes, and resets the prior voice.
- Runtime telemetry records active voice count and replacement stops.

Packaged proof: the real-save city-switch run logged one active narrator,
`previous_voice_stopped=1`, and left no `/usr/bin/say` process running.

### 2. Elevated or inaccessible ground regions

Root cause: the old grounding pass traced downward through the mission floor,
hit the emergency catch floor, and moved playable geometry down to it. Recovery
then reused the below-world hit and repeatedly placed the pawn near `Z=-42`.

Implementation:

- Every city owns an explicitly tagged canonical mission floor.
- The emergency catch floor moved to local `Z=-620` and can never be selected
  as playable ground.
- Broad near-ground traversal slabs normalize against city origin instead of a
  downward collision trace.
- Recovery ignores catch-floor actors, rejects hits below the mission plane,
  validates final height, and falls back to the canonical city entry.
- A dedicated audit loads city two, places the pawn below ground, calls the real
  recovery function, and checks height, bounds, ground continuity, and enemies.

Packaged proof: New York and Los Angeles both report zero elevated regions;
Los Angeles recovered at relative `Z=92.00` with no repeated recovery.

### 3. Terminal contrast and modal readability

Implementation:

- The code editor now uses near-white `(0.960, 0.985, 1.000)` text on an
  explicit near-black `(0.002, 0.005, 0.008)` brush in normal, hovered, focused,
  and read-only states.
- The measured linear-color contrast ratio is `18.89:1`, above the `7:1`
  enhanced-contrast target.
- Gameplay HUD rendering is suppressed while a modal owns input.
- Transient Unreal debug guidance is cleared when the terminal opens, so no
  gameplay text overlaps the lesson or code editor.

Visual proof: `Screenshots/first_level_terminal_contrast.png`.

### 4. Missing zombies

Root cause: every regular zombie ID was saved as permanently neutralized and
then destroyed on every resume. A mature language save could therefore remove
the entire renewable city population.

Implementation:

- Bosses and named minibosses retain permanent defeat state.
- Regular, director, horde, encounter, and dog populations repopulate each
  session; their historical IDs remain available for aggregate statistics.
- Each non-sandbox city must contain at least six live renewable threats after
  saved objective state is applied.

Packaged proof: the real Java save contained 130 renewable death IDs. They were
correctly ignored for spawning, and both tested cities loaded 124 renewable
enemies, including 120 regular and four director zombies.

### 5. Valid C++ reverse solution rejected

Root cause: the reverse validator recognized postfix `i--` but not the equally
valid prefix form `--i` used in the captured player submission.

Implementation:

- The structural validator now accepts prefix decrement, postfix decrement,
  and `-= 1` backward loops.
- The exact reported C++ implementation is embedded in the runtime challenge
  audit as a permanent alternate-solution regression case.

Proof: all 60 canonical stage/language solutions passed external toolchains;
the reported prefix-decrement solution separately scored 100.

### 6. Random, inescapable CODE ACCEPTED screen

Root cause: saved solved-state restoration replayed the solved route for every
terminal. This spawned duplicate geometry and an interactive `CODE ACCEPTED`
world message. The reader was not focusable and could stack at z-order 1000
above the pause armory, intercepting its mouse and close keys.

Implementation:

- Saved terminals are marked solved silently; the city route reconstructs once
  from the primary station only.
- The secret terminal cannot unlock the required ten-station rescue route.
- The obsolete solved-route `CODE ACCEPTED / RESCUE ROUTE OPEN` reader spawn was
  removed; HUD and world route feedback remain.
- The message reader is focusable, owns the UI lock, pauses the world, supports
  E/Enter/Escape, and includes a visible mouse CLOSE button.
- Reader, journal, and pause are mutually exclusive. Pressing pause while the
  reader is open closes the reader and returns without stacking another modal.

Proof: the integrated editor and package runs both passed the reader routing
audit with open, close, mouse button, world-unpause, and pause-exclusivity checks.

### 7. Player animation and aiming

Implementation:

- Production Manny now uses an explicit six-state controller for idle, walk,
  run, jump, fall, and land instead of depending on sample AnimBP variables that
  this custom pawn did not drive.
- Walk and run playback rates follow real ground speed.
- The supplied `MM_Land` clip is additive and invalid as a standalone cooked
  node, so landing uses a cooked-safe idle base plus visible procedural pelvis,
  spine, and first-person arm compression.
- The existing pose-copy layer now combines those locomotion states with
  physical left/right upper-arm aiming and auto target lock.

Proof: the Metal run observed idle, jump, fall, land compression, and two-arm
aim at runtime; no additive-animation warning remained. See
`Screenshots/first_level_combat_wound.png`.

### 8. Pause controls and mouse selection

Implementation:

- All 18 controls remain bound to direct mouse-down activation.
- Child text and the 3D preview are hit-test-invisible where appropriate.
- Both preview and bubble pointer routes resolve the exact button under the
  pointer, with action telemetry for diagnosis.
- The pause UI uses a bounded real-time presentation delta so the weapon preview
  can rotate independently of paused game time.
- The integrated Metal audit opened/closed crafting, cycled all 17 weapons,
  equipped the final selection, and verified every action binding.

Desktop Computer Use was also attempted. macOS Retina/Stage Manager translated
the visible coordinates outside the Unreal window (`windowNotFoundAtPosition`),
so that external click did not reach the game and is not claimed as success.
The packaged Slate binding, geometry routing, cursor, capture, and action audits
all passed.

## Verification Matrix

| Gate | Result | Evidence |
| --- | --- | --- |
| Editor compilation | PASS | UnrealBuildTool, Development editor target |
| July 11 source contract | PASS, 24/24 | `TestLogs/static_regression_contract.txt` |
| Final runtime evidence contract | PASS, 17/17 | `TestLogs/runtime_evidence_contract.txt` |
| V5 integrated source contract | PASS | `TestLogs/integrated_static_contract.txt` |
| External challenge matrix | PASS, 60/60 | `TestLogs/FirstLevelChallengeRegressionV7Final.log` |
| Exact prefix-decrement solution | PASS, score 100 | same challenge log |
| City-two recovery and population | PASS | `TestLogs/CampaignGroundRecoveryRegressionV7Final.log` |
| Terminal Metal review | PASS, 18.89:1 | `TestLogs/TerminalContrastReviewV7Final.log` |
| Editor integrated Metal run | COMPLETE PASS | `TestLogs/FirstLevelIntegratedRegressionV7Metal.log` |
| Full cook | PASS, 1,815/1,815 | `TestLogs/CookV7.log` |
| UAT build/stage/archive | PASS, ExitCode 0 | `TestLogs/PackagingBuildV7.log` |
| Packaged real-save recovery | COMPLETE PASS | `TestLogs/PackagedCampaignGroundRecoveryV7.log` |
| Packaged integrated Metal run | COMPLETE PASS | `TestLogs/PackagedFirstLevelIntegratedV7.log` |
| Normal packaged language gate | PASS, six languages, no world start | `TestLogs/PackagedNormalLaunchGateV7.log` |
| Packaged warning scan | PASS | `TestLogs/packaged_warning_scan.txt` |
| Deep code-sign verification | PASS | `TestLogs/codesign_verify.txt` |
| Local package integrity | PASS | `TestLogs/package_integrity.txt` |

The warning scanner found only the established Development-build diagnostics:
one empty navigation dirty bound, missing Recast crowd manager at shutdown, and
packaged viewport hit-proxy storage. It found no fatal error, assertion, ensure,
failed package load, gameplay failure marker, or animation warning.

## Start Screen and Saves

Normal packaged launch still stops at the language chooser. Java, Python, C,
C+, C++, and MATLAB stations and menu entries are present; no language is
selected automatically and no campaign world is spawned during the hold. The
per-language saves remain intact, including the tested Java profile.

## Package Handoff

- App: `PackagedMac/Mac/CodeRescueUnreal.app`
- Bundle identifier: `com.operationcoderescue.CodeRescueUnreal`
- Bundle version: `51494982.0.198`
- Archive size: 2,051.6 MB
- Local signature: valid on disk and satisfies its designated requirement
- Local integrity: ready
- External distribution: Developer ID signing, notarization, and Gatekeeper
  approval still require Apple distribution credentials
