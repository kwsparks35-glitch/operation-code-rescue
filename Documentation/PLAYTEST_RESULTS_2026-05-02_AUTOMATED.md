# Operation Code Rescue - Automated Playtest Results

Date: 2026-05-02  
Tester: Codex automated terminal smoke pass  
Build path: `PackagedMac/Mac/CodeRescueUnreal.app`  
Mac model observed by Unreal: Apple M4 Pro  
macOS observed by Unreal: macOS 26.3.1  
Build configuration: Mac Development

## Tests Run

### Gameplay Parameter Repackage

Command shape:

```bash
./Package_Mac_App.command
```

Result: Passed after the 0.6.0 player-resource and tuning pass.

Observed:

- `CodeRescueUnrealEditor Mac Development` built successfully.
- `CodeRescueUnreal Mac Development` built successfully.
- Full cook completed for Mac.
- Staged app was copied into the archive app.
- Archived app contains cooked `Contents/UE` data and packaged pak/ucas/utoc files.

### NullRHI Startup Smoke

Command shape:

```bash
PackagedMac/Mac/CodeRescueUnreal.app/Contents/MacOS/CodeRescueUnreal \
  -NullRHI -nosound -NoRadioVoice -unattended -stdout -FullStdOutLogOutput -ExecCmds="Quit"
```

Result: Passed.

Observed:

- Packaged pak/utoc containers mounted.
- `/Engine/Maps/Entry` loaded.
- `CodeRescueGameMode` initialized.
- App exited cleanly with exit code 0.

### Metal/CoreAudio Startup Smoke

Command shape:

```bash
PackagedMac/Mac/CodeRescueUnreal.app/Contents/MacOS/CodeRescueUnreal \
  -windowed -ResX=1280 -ResY=720 -NoRadioVoice -unattended -stdout -FullStdOutLogOutput -ExecCmds="Quit"
```

Result: Passed.

Observed:

- Metal RHI initialized on Apple M4 Pro.
- CoreAudio mixer initialized.
- Packaged pak/utoc containers mounted.
- Cooked shader libraries loaded.
- `/Engine/Maps/Entry` loaded.
- `CodeRescueGameMode` initialized.
- App exited cleanly with exit code 0.

### Latest Re-Run After Parameter Tuning

Commands:

```bash
./Smoke_Test_Packaged_App.command render
./Smoke_Test_Packaged_App.command null
```

Result: Both passed on the refreshed `PackagedMac/Mac/CodeRescueUnreal.app`.

Observed:

- Package size is approximately `1.1G`.
- `CodeRescueUnreal-Mac.pak`, `.ucas`, and `.utoc` are present.
- `global.ucas` and `global.utoc` are present.
- Render smoke mounted containers, initialized Metal/CoreAudio, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.
- Null smoke mounted containers, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.

### Latest Re-Run After City Access Fix

Commands:

```bash
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command render
./Smoke_Test_Packaged_App.command null
```

Result: Package rebuild passed, then both smoke modes passed on the refreshed `PackagedMac/Mac/CodeRescueUnreal.app`.

Observed:

- `CodeRescueUnrealEditor Mac Development` built successfully after the perimeter-collision fix.
- `CodeRescueUnreal Mac Development` built successfully during packaging.
- Full Mac cook and archive refresh completed.
- Render smoke mounted packaged containers, initialized Metal/CoreAudio, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.
- Null smoke mounted packaged containers, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.
- City-access QA is now represented in the manual checklist because automated startup smoke does not drive the player from the spawn point into the city.

### Latest Re-Run After 50x City Span Expansion

Commands:

```bash
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command render
./Smoke_Test_Packaged_App.command null
```

Result: Package rebuild passed, then both smoke modes passed on the refreshed `PackagedMac/Mac/CodeRescueUnreal.app`.

Observed:

- `CodeRescueUnrealEditor Mac Development` built successfully after adding shared 50x city span helpers.
- `CodeRescueUnreal Mac Development` built successfully during packaging.
- Full Mac cook and archive refresh completed.
- Render smoke mounted packaged containers, initialized Metal/CoreAudio, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.
- Null smoke mounted packaged containers, loaded `/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, and exited cleanly.
- Manual QA must still judge the visible city scale, traversal feel, and objective flow with real input.

## Automated Coverage

Covered:

- Packaged app has cooked data.
- Package launches without descriptor-file failure.
- Renderer startup works.
- Audio startup works.
- Entry map and GameMode startup work.
- Release output is not missing pak/ucas/utoc data.
- The refreshed package includes the player-resource save and gameplay-parameter tuning code.
- The refreshed package includes the city-access collision fix and southwest entry guide visuals.
- The refreshed package includes the 50x city span expansion and traversal/range retuning code.

Not covered by automation:

- Human judgment of controls, combat feel, readability, and subjective pacing.
- Full manual first-city completion.
- Multi-city progression playthrough with real input.
- Manual confirmation of walking from the outside spawn area into the city.
- Manual confirmation that the 50x city span feels readable and playable with real input.

Manual sign-off steps are documented in `Documentation/QA_PLAYTEST_CHECKLIST.md`.

## Release Decision

Ready for human QA sign-off. No automated startup, packaging, parameter-pass, city-access, or 50x city-span packaging blocker remains.
