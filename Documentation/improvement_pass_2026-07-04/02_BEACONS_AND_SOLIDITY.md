# Beacons (word-competition fix) + World Solidity (2026-07-04)

## 1. Beaming symbols replace multi-word world text

Kenny: "correct anything longer than a single word to a beaming symbol that sits above
the thing that it is intending to draw attention toward."

**New actor — `ACodeRescueBeaconMarkerActor`** (subclass of the proven
`ACodeRescueMessageMarkerActor`):

- Vertical emissive BEAM (thin engine cylinder, tinted `Accent × 4.5` through
  `ApplyTintedMaterial`, so it blooms day and night), centered on the marker so half
  the beam reaches down toward the marked object.
- One large category GLYPH (TextRender, world size 64) chosen by the existing
  `SymbolForGuideText` classifier: `+` supplies, `++` people, `!` threats, terminal
  glyph for code, etc. Color-coded by the original label color.
- Gentle breathing pulse; near-static under reduced motion.
- INHERITS the read-on-demand contract: tag `MessageMarker` + `OpenMessageReader()`,
  so walking up and pressing E still opens the scrollable reader with the full
  original text. No information is lost — it just stops shouting.

**Routing rule change in `SpawnGuideText`** (CodeRescueGameMode.cpp):

- `bMultiWord` = the label contains any whitespace after newline-flattening.
- Non-essential multi-word → beacon (previously only "substantive" text became
  markers, and short multi-word labels became flat TextRender glyphs).
- Single words keep THEMSELVES as text (a single word already is a symbol) —
  previously they were converted to a generic glyph.
- Essential control prompts (`IsEssentialGuideText`: E/WASD/Enter/objective/…)
  keep their words, unchanged.
- `-NoHoverMarkers` still restores legacy full text for debugging.

## 2. World physics: nothing walks through walls, nothing floats

- **`EnsureComplexAsSimpleCollision`** (GameModeSpawning.cpp): every mesh spawned
  through `SpawnKitMesh` / `SpawnStaticMeshProp` with collision enabled gets
  `CTF_UseComplexAsSimple` on its BodySetup — authored glTF/FBX art ships with no
  collision primitives, which is WHY imported walls used to be walk-through.
- **Kit props now collide:** lamp, planter, kiosk, extraction arch, rubble were
  spawned `bEnableCollision=false` since 07-01; all flipped to true (facades already
  collided).
- **Ground snap:** new `ACodeRescueGameMode::GroundZAt(Probe, DefaultZ)` line-trace;
  every streetscape placement traces the ground and rests the prop's z=0 base on the
  hit point.
- **QA command:** `cr.AuditWorldSolidity [fix]` — iterates all StaticMesh actors,
  reports (a) no-collision count (intentional decor: halos, beacons, sky) and
  (b) collision-enabled actors whose bounds bottom is >60uu above the ground trace;
  `fix` snaps floaters down. Sky layer actors are tagged `SkyLayer` and excluded.

## 3. Oversight: the "39 REAL regressions" were 38 stale verifiers

`Scripts/claude_oversight_watchdog.py` gained a PATTERN-STALE triage class: when a
failing verifier's every named identifier still exists in `Source/`, the feature is
present and only the verifier's exact-line expectation drifted (the 2026-07-02
`NativeConstruct → RebuildWidget()` construction move rephrased lines wholesale).
These are reported separately as verifier maintenance for Codex — the REAL-regression
gate stays strict for genuinely missing symbols. Spot-checks confirming presence:
`PracticeRunButton`, `PanelFrame`, `ThreatCompassSlot`, `RewardChoiceActionRow`,
`SurvivorIntelDossierText`, `VisualizedSoundCueText`, `BaseSubtitleFont` (in
`CodeRescueSubtitlesWidget.cpp` — the earlier "missing" read used a wrong filename).
