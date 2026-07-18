# Squad Personality Tactical Readability Slice

## Purpose

This slice continues the June 25 release, UX, and character guidance by making the rescue support team read as named teammates with distinct roles instead of generic support actors.

The game already had a functional five-role squad. This pass binds the runtime team to the `squad_personality_manifest.tsv` identities, surfaces role identity in the HUD, and gives squad orders short role-specific subtitle responses.

## Implemented

- Added runtime personality fields to `ACompanionActor`: `MechanicalIdentity`, `BarkStyle`, and `RoleAccentColor`.
- Added `ConfigureSquadPersonality()` so city spawning can configure each teammate from the manifest-aligned role table.
- Added `GetHudCallsign()` and `GetRoleStatusLabel()` so the HUD can show readable first-name role pips such as `Mira MED`, `Tomas ENG`, and `Briggs HVY`.
- Added `GetOrderResponseBark()` and `PushRoleOrderBark()` so regroup, formation, hold, and follow orders receive concise role-specific subtitle responses.
- Updated the runtime support-team roster to use the manifest names: Mira Hale, Tomas Ives, Ada Cross, Noor Vance, and Briggs Vale.
- Updated the squad HUD status line to wrap into two readable lines with named health pips, role status readouts, medic readiness, support-fire state, formation, hold/follow order, and manual medic prompt.

## Player Impact

Players should now be able to understand who is helping them and why: Mira is the medic, Tomas keeps routes reliable, Ada provides overwatch, Noor watches flanks, and Briggs anchors the rear guard. The Y/U/O/N squad controls now produce short teammate responses, which makes the commands feel acknowledged without adding another menu.

## Validation

- Added `Scripts/verify_squad_personality_tactical_readability_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.
- Updated `squad_personality_manifest.tsv`, `first_ten_minutes_onboarding.tsv`, `human_qa_signoff_checklist.tsv`, and `visual_regression_targets.tsv`.
- Validation passed: Python verifier compilation, squad personality verifier, legacy rescue-team survivability verifier, demo-readiness verifier, C++ module recompile, Mac package archive, packaged null smoke, packaged render smoke, packaged runtime log contracts, and scoped whitespace diff check.

## Remaining QA

This is a code-driven readability pass. Final human QA should still play a combat encounter with all five roles active and confirm the named HUD pips, subtitle barks, medic cooldowns, and hold/follow formation behavior feel distinct without cluttering the screen.
