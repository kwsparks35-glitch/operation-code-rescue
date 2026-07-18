# Items 22–25 — Asset import recipes (run on macOS)

These four items can't be done from the Linux scripting environment — they
need the UE editor + Quixel Bridge + the user's microphone or a sample
library. Below are the complete recipes + helper scripts I shipped to
automate the wiring once the assets land.

## Item 22 — Bake 342 radio briefing WAVs + auto-wire

### Step 1: bake the WAVs (5 min)
```bash
cd /Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix
python3 Scripts/generate_radio_voiceovers.py --limit 0
```
This walks `Content/CodeRescueData/radio_briefings.tsv` and uses macOS
`say` + `afconvert` to emit one WAV per city under
`Content/CodeRescueAssets/Audio/RadioSamples/<slug>_radio_briefing.wav`.
Expect ~342 WAVs totaling ~30 MB.

### Step 2: import as USoundWaves in the editor (~2 min)
1. Open the UE editor.
2. In the Content Browser, navigate to
   `Content/CodeRescueAssets/Audio/RadioSamples/`.
3. Drag every WAV from your Mac Finder into that folder.
4. UE imports each as a `USoundWave` asset; accept defaults.
5. Save All (`Ctrl+S` in Content Browser).

### Step 3: auto-wire to GameMode (~10 sec)
Open Window → Developer → Python Console:
```python
>>> import importlib, wire_radio_cues
>>> importlib.reload(wire_radio_cues)
>>> wire_radio_cues.run()
[wire_radio_cues] 342 / 342 cities matched (100.0%)
[wire_radio_cues] Done.
```

### Verification
Spawn into city 0 (Anchorage) — you should hear the briefing play through
the cooked SoundWave path, NOT via macOS `say`.
You can confirm by killing the `say` binary mid-PIE; the briefing still
plays.

---

## Item 23 — MetaHuman survivor

### Step 1: pick + download an archetype (~5 min)
1. Open UE editor.
2. Window → Quixel Bridge.
3. Sign in to your Epic account.
4. Browse MetaHumans → pick a "civilian" preset (e.g., "Olivia").
5. Click **Add** — Bridge downloads ~150 MB and adds to
   `Content/MetaHumans/Olivia/`.
6. Wait for compile to finish (status bar bottom-right).

### Step 2: create BP_SurvivorActor (one-time, ~30 sec)
1. In Content Browser, navigate to `/Game/CodeRescueAssets/`.
2. Right-click → Blueprint Class → search "ASurvivorActor".
3. Name it `BP_SurvivorActor`. Save.
4. Open `BP_CodeRescueGameMode` → set `SurvivorActorClass` to
   `BP_SurvivorActor`. Save.

### Step 3: auto-wire (~10 sec)
Python Console:
```python
>>> import importlib, import_metahuman_survivor
>>> importlib.reload(import_metahuman_survivor)
>>> import_metahuman_survivor.run("Olivia")
[import_metahuman_survivor] Set ProfessionalSurvivorMesh = Olivia_Body
[import_metahuman_survivor] Set ProfessionalSurvivorAnimClass = ABP_Olivia
[import_metahuman_survivor] Done.
```

### Verification
Restart PIE. Survivors render as the MetaHuman, not the cube + sphere.

---

## Item 24 — Three zone ambient cues

### Step 1: source 3 WAVs (~10 min)
Pick 3 ambient sounds, ~30 seconds each, stereo, 44.1 kHz, looping:
- Wind / snow (Anchorage zone)
- Rain / overcast (Seattle zone)
- Urban night / neon hum (Tokyo zone)

Free sources:
- freesound.org (CC0 / CC-BY filter)
- zapsplat.com (free with account)
- youtube-dl + an ambient YouTube channel (check the license)

Save under `Content/CodeRescueAssets/Audio/ZoneAmbient/`. Name them with a
zone hint in the filename (e.g., `Anchorage_Wind_Loop.wav`).

### Step 2: import (~30 sec)
Drag the 3 WAVs into the editor's Content Browser at
`Content/CodeRescueAssets/Audio/ZoneAmbient/`.

### Step 3: auto-wire (~10 sec)
```python
>>> import importlib, import_zone_ambient_cues
>>> importlib.reload(import_zone_ambient_cues)
>>> import_zone_ambient_cues.run()
[import_zone_ambient_cues] Zone 0 → Anchorage_Wind_Loop
[import_zone_ambient_cues] Zone 1 → Seattle_Rain_Loop
[import_zone_ambient_cues] Zone 2 → Tokyo_Urban_Hum
[import_zone_ambient_cues] Done.
```

### Verification
Spawn into a city — you should hear the zone-specific ambient bed at low
volume. Walk between zones (use the helipad fast-travel from item 7) and
hear the bed change.

---

## Item 25 — Megascans props for Anchorage

### Step 1: download props (~10 min)
Window → Quixel Bridge → Megascans → 3D Assets. Pick:
- `urban_car_sedan` (or any vehicle)
- `dumpster_industrial`
- `shipping_container`
- `debris_pile_concrete`

Click Add for each. Bridge writes them under
`Content/Megascans/3D_Assets/`.

### Step 2: replace placeholder cubes
Open `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`,
function `SpawnAuthoredPropsForCity`. Replace the `SpawnBlock` calls with
`GetWorld()->SpawnActor<AStaticMeshActor>` calls referencing the
Megascans static meshes. Or, more elegantly, expose
`TSoftObjectPtr<UStaticMesh> AuthoredPropMeshes[8]` on the GameMode and
let designers assign per-prop in `BP_CodeRescueGameMode`.

The `AuthoredProp` tag I added in this pass means a future cleanup step
can `TActorIterator<AStaticMeshActor>` and replace any tagged actor's
mesh in one editor utility.

### Verification
Anchorage's open spaces fill with realistic props instead of colored cubes.

---

## How to apply
Run all four recipes in one session. Total time end-to-end: ~30 minutes
including downloads. Once each is wired the asset just becomes part of the
cooked build — no further code work required.
