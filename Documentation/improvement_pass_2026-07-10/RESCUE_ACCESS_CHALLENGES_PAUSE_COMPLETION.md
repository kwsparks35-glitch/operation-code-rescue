# Rescue Access, Ten-Challenge, Supply, and Pause Completion

Date: 2026-07-10

## Scope

This pass closes the six first-level issues reported after the V5 combat and
world pass: survivor-route arena recovery, usable building and lower-level
access, longer day/night periods, gameplay supply rewards, ten required coding
stations per level, and complete mouse operation on the pause screen. The
changes are implemented in shared campaign/runtime systems, so the ten-station
contract and save progression apply to every campaign city while the authored
access changes remain scoped to the first level.

## Completed Work

### 1. Survivor route and arena recovery

- Moved the arena contract into `FCodeRescueCampaign` so world construction,
  player recovery, audits, survivor gating, and objective routing use the same
  dimensions.
- Expanded the first-level playable area to contain the survivor camp,
  extraction route, three accessible buildings, sidewalks, roads, and coding
  concourse.
- Replaced the old fixed-height recovery with a downward ground trace. Recovery
  now places the capsule on the actual walkable surface instead of rejecting
  legitimate ground-floor positions or forcing the player to an upper deck.
- Expanded the catch floor and boundary walls with the playable footprint, and
  retained Backspace/F8 as an explicit player-controlled recovery route.
- Moved the clinic inside the shared bounds and relocated a sandbag cluster and
  support-team hub that obstructed capsule paths to the survivor/buildings.
- Added outside-to-inside, long street approach, arena-bound, ground
  continuity, player-start, survivor, and extraction sweeps to the first-level
  access audit.

Result: all three authored entrances and their street approaches report clear,
level, bounded access, and moving toward the survivor no longer invokes arena
recovery.

### 2. Building, street, and lower-level access

- Retained the accessible market, clinic, and open cafe with literal 230 cm
  entrances, interior floors, practical lighting, and functional pickups.
- Added sidewalk spurs from the street network to each entrance.
- Disabled collision on the production canopy display surface so it cannot
  become an invisible traversal blocker.
- Enlarged the protected coding-concourse and safehouse floor at the canonical
  ground level.
- Changed the first-level player start to the canonical street plane and made
  recovery honor traced floor height, preserving access to the bottom level.

The final runtime audit reports `clear_doors=3/3`, `street_approach=3/3`,
`bounded_doors=3/3`, and `level_doors=3/3`.

### 3. Ten required coding challenges per level

- Added `RequiredChallengesPerCity = 10` as the single campaign constant.
- Built a protected 5-by-2 coding concourse with ten physical terminals in the
  first level.
- Generated ten deterministic IDs for each of the 465 campaign cities, for
  4,650 globally unique required challenge IDs.
- Preserved each city's original terminal ID as stage one, so existing solved
  progress remains valid. Stages two through ten use the city slug plus a
  stage-specific suffix to prevent lesson keywords from shadowing one another.
- Updated the survivor, HUD, journal, objective beacon, fast travel, encounter
  director, boss state, and fail-safe board to use `x/10` progression.
- The survivor route remains locked at nine solved stations and opens only
  after the tenth. Assisted/bypass solves count as completed stations but do
  not award a clean-solve supply cache.
- Added canonical reference submissions to the same runtime function that
  builds terminal starter code, preventing tests and play from drifting apart.

The runtime audit validates all 10 physical stations and 60 language/station
submissions, then proves that nine remains locked and ten unlocks the survivor
route before restoring the player's in-memory save state.

### 4. Supply rewards

- A first-time clean coding solve drops three ground-snapped pickups: an ammo
  item, scrap, and a utility/medical item.
- Defeating a persistent gameplay zombie drops one deterministic ground-snapped
  supply pickup.
- Test-only and transient audit zombies are excluded from normal reward drops.
- Supply creation is verified at runtime by both challenge and zombie reward
  paths.

### 5. Day/night timing

- Increased the full first-level day/night period to 1,800 seconds.
- This yields approximately 15 minutes across the daylight half and 15 minutes
  across the night half, with continuous day, sunset, night, and sunrise
  transitions.
- The sky audit verifies stars, moon, all four phases, and a minimum acceptable
  cycle duration of 1,200 seconds.

### 6. Pause-screen mouse operation

- Pause now uses UI-only input, releases mouse capture/lock, enables cursor,
  click, hover, and touch events, flushes held gameplay keys, and restores
  gameplay input when closed.
- All 18 pause actions are button-bound and mouse-enabled. Text and decorative
  layers are hit-test-invisible so they cannot intercept a button click.
- Added a preview-phase geometry router as a fallback for Slate child routing;
  it invokes only the action under the pointer and prevents duplicate dispatch.
- Weapons can be selected with mouse Previous/Next and Equip controls, while
  keyboard/controller navigation remains available.
- Added a functional crafting panel with Flare, Stim, and Grenade recipes,
  resource-aware enable states, immediate inventory updates, and persistent
  save on craft.

The runtime audit verifies 18 bindings, disabled hit interception on the 3D
preview, the pointer router, 17 weapon previews, Equip, crafting open/close,
three recipes, and save-on-craft behavior.

## Save and Start-Screen Behavior

The existing per-language save architecture remains intact. Solved station
IDs, inventory rewards, survivor progression, selected language, and crafted
items are saved to that language's run. Normal packaged launch still presents
the six-language selector before play and does not select a language
automatically; Resume and New Run remain available for each language profile.

The packaged launch was held at the chooser for more than 20 seconds. The log
contains both launch-widget markers, no automatic selection, no recovery
warning, and no gameplay acceptance markers before graceful shutdown.

## Acceptance Evidence

| Gate | Result | Evidence |
| --- | --- | --- |
| Rescue/access/challenge/pause static contract | PASS, 20/20 | `Scripts/verify_rescue_access_challenge_pause_pass_2026_07_10.py` |
| Integrated V5/V6 static contract | PASS | `Scripts/verify_first_level_integrated_v5_pass_2026_07_09.py` |
| Combat implementation contract | PASS | `Scripts/verify_first_level_combat_experience_pass_2026_07_09.py` |
| Legacy gameplay-regression contract | PASS | `Scripts/verify_gameplay_fixes_2026_07_07.py` |
| Editor integrated acceptance | COMPLETE PASS | `TestLogs/FirstLevelIntegratedAcceptanceAuditV6Exact.log` |
| External-toolchain challenge matrix | PASS, 60/60 | `TestLogs/FirstLevelChallengeExternalToolchainsV6.log` |
| Packaged integrated acceptance | COMPLETE PASS | `TestLogs/PackagedFirstLevelIntegratedAcceptanceAuditV6.log` |
| Normal packaged language gate | PASS, 20+ second hold | `TestLogs/PackagedLaunchLanguageGateV6.log` |
| Packaged NullRHI smoke | PASS | `TestLogs/PackagedSmoke_null.log` |
| Packaged Metal smoke | PASS | `TestLogs/PackagedSmoke_render.log` |
| Asset budget | PASS | `Scripts/verify_asset_budget_pass.py` |
| Package integrity | PASS, local ready | `TestLogs/package_integrity_latest.json` |
| Deep code-sign verification | PASS | `codesign --verify --deep --strict` |

The final packaged marker is:

```text
[FirstLevelIntegratedAudit] COMPLETE PASS world=1 access=1 sky=1 day_period=1 challenges=1 progression=1 supplies=1 target_lock=1 combat=1 corpse=1 armory=1 pause_mouse=1 crafting=1
```

The warning scanner permits only Unreal Development-environment diagnostics
for navigation dirty-area setup, viewport hit-proxy storage, crowd-manager
shutdown, and the host audio sample-rate query. No failed package load, linker
warning, assertion, ensure, exception, fatal, or gameplay error appears in the
final packaged acceptance log.

## Pointer Automation Note

The in-game pause audit verifies the complete Slate binding and pointer-routing
contract. A separate desktop automation click was attempted, but the desktop
tool translated the Retina coordinate outside the Unreal window and reported
`windowNotFoundAtPosition`; no click reached the game, so that attempt is not
represented as a literal pointer success. The visible armory was inspected,
the packaged runtime confirmed no mouse capture, and the deterministic
in-engine mouse contract passed.

## Package

- Application: `PackagedMac/Mac/CodeRescueUnreal.app`
- Bundle identifier: `com.operationcoderescue.CodeRescueUnreal`
- Bundle version: `51494982.0.197`
- Archive size: 2,051.5 MB
- Cook: 1,815 packages, zero cook errors, zero cook warnings
- Local signature: valid on disk and satisfies its designated requirement

The archive is ready for local Mac play. Apple Developer ID signing,
notarization, and Gatekeeper approval require distribution credentials and are
correctly reported as pending external-release steps.

