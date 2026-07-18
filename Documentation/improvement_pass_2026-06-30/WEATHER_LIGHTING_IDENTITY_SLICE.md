# Weather and lighting identity slice

This pass implements the P1 world-development request to make survival-horror atmosphere readable by district, not only through global sky/fog settings. The project already had per-city climate tokens, fog, color grade, and optional Niagara weather hooks; this slice adds visible, nonblocking district cues that players and reviewers can inspect in the packaged game.

## Runtime implementation

- Added `SpawnWeatherLightingIdentityLayer` and call it immediately after `SpawnWeatherForCity`.
- The layer reuses the active city climate contract from `BuildUSCityVisualProfile` and `BuildUSCityRealizationParams`.
- It maps city cloud families into text-first weather identities:
  - Clear: `CLEAR SKY CONTRAST`
  - Overcast: `OVERCAST SHELTER LIGHTING`
  - Marine layer: `MARINE FOG ROUTE LIGHTING`
  - Warm haze: `WARM HAZE HEAT MIRAGE`
  - Humid glow: `HUMID STORM GLOW`
  - Snow sky: `COLD SNOW SKY`
- It spawns three district review cues:
  - Entry Weather Shelter
  - Safehouse Light Pool
  - Rescue Route Sky Cue
- Each cue has ground reflection strips, weather streaks, a climate sign, text label, and a point light tuned from the active cloud/grade family.

## Tags and review hooks

Weather cue actors receive:

- `WeatherLightingIdentity`
- `WeatherLightingIdentityReady`
- `DistrictWeatherCue`
- `NonBlockingWeatherCue`
- `WorldDevelopmentDeepDive`
- `Top50Recommendations`

Signal lights additionally receive `WeatherLightingSignalLight`.

Runtime smoke logs now include `[CodeRescueWeatherLightingIdentity]` with the city label, weather title, cloud token, grade token, and fog density.

## Data and QA updates

- Added `Content/CodeRescueData/weather_lighting_identity_manifest.tsv`.
- Updated the creative inclusion plan, visual-regression targets, human QA checklist, and accessibility manifest.
- Added `Scripts/verify_weather_lighting_identity_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.

## Validation

Required validation for this slice:

- `python3 -m py_compile Scripts/verify_weather_lighting_identity_slice_pass.py`
- `python3 Scripts/verify_weather_lighting_identity_slice_pass.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`
- Runtime log contract scan on packaged smoke logs
- Runtime log confirmation of `[CodeRescueWeatherLightingIdentity]`
- `git diff --check`
- Touched-file trailing-whitespace scan
