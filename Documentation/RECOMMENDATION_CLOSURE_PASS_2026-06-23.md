# Recommendation Closure Pass - 2026-06-23

## Scope

This pass implements the non-human recommendations from the full project review.
The intentionally excluded item is the human playthrough pass.

## Implemented

1. Cooked radio voice priority now works on macOS. `ACodeRescueGameMode::SpeakRadioBriefing`
   pushes subtitles, checks `bPreferCookedRadioBriefingCues` /
   `-UseCookedRadioVoice`, plays Blueprint-wired or slug-loaded cooked cues when
   available, and only then falls back to `/usr/bin/say`.
2. The Mac bundle identifier is no longer the Unreal placeholder. Source config
   and the current packaged app now use:

   ```text
   com.operationcoderescue.CodeRescueUnreal
   ```

   The current package was re-signed ad-hoc after the plist update so local
   integrity checks can still pass. Developer ID signing and notarization remain
   credential-bound.
3. Campaign scope is now documented as 465 missions: 342 U.S. city missions
   followed by 123 global extension missions. Source-level constants were added
   to `FCodeRescueCampaign` for the U.S. and current full-campaign counts.
4. The pause-menu skill tree is now a real runtime widget with eight unlock
   buttons, RP affordability state, feedback text, save-through unlocks, and
   immediate application to the current pawn.
5. Skill effects are now applied on player `BeginPlay` and guarded with a
   transient per-pawn applied-mask so repeated calls do not stack reload or
   magazine bonuses. Health/stamina bonuses add to current tuned values instead
   of resetting them to stale baselines.
6. MATLAB PATH installs are supported for trusted local QA. `IsLanguageAvailable`
   probes `matlab -batch "exit"` when the fallback is PATH-only, and validation /
   desktop launch execute through `/usr/bin/env matlab` in that case.
7. Trusted external C/C++/Python validators are harder to spoof:
   - C/C++ harnesses undefine challenge-name and output macros before hidden
     tests run.
   - C/C++/Python use a generated per-challenge sentinel instead of the old fixed
     `ALL_TESTS_PASSED` token.
   - Python user code is written to `mission_solution.py` and imported by a
     separate `mission_harness.py`, so early `sys.exit(0)` cannot emit harness
     success.
8. Source-control guidance was updated with the actual dirty-tree scale and
   recommended review slices. No commit was created because the tree already had
   many unrelated pre-existing changes, including files touched by this pass.

## Files Touched

- `Config/DefaultEngine.ini`
- `README_MAC.md`
- `Run_Launch_Menu_Visual_Check.command`
- `Source/CodeRescueUnreal/CodeRescueCampaign.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueSkillTreeWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueSkillTreeWidget.h`
- `Source/CodeRescueUnreal/CodeRunnerLibrary.cpp`
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp`
- Current/release documentation under `Documentation/`

## Verification Plan

Run after this pass:

```bash
git diff --check
zsh -n Run_Launch_Menu_Visual_Check.command
python3 Scripts/verify_june19_playability_readability_fix_pass.py
python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py
python3 Scripts/verify_demo_readiness_pass.py
python3 Scripts/verify_save_compatibility_pass.py
python3 Scripts/verify_asset_budget_pass.py
python3 Scripts/audit_maple_audio_assets.py
python3 Scripts/verify_package_integrity_pass.py --expected-bundle-id com.operationcoderescue.CodeRescueUnreal
python3 Scripts/verify_nonhuman_release_readiness_pass.py
```

## Remaining Credential-Bound Work

External Mac distribution still requires:

1. Apple Developer Team ID.
2. Developer ID Application certificate.
3. Hardened-runtime signing as appropriate for the final app.
4. Notary submission, staple, and Gatekeeper verification.
5. Strict distribution preflight:

   ```bash
   python3 Scripts/verify_package_integrity_pass.py --strict-distribution --expected-bundle-id com.operationcoderescue.CodeRescueUnreal
   ```
