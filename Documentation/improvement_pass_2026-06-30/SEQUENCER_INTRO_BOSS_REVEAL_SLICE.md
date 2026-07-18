# Sequencer Intro Boss Reveal Slice

This pass implements the P2 cinematic request for Sequencer intros and boss reveals as a package-safe blocking layer. The June 25 character-animation and release guidance calls for public-demo-ready presentation, Sequencer/Control Rig hooks, readable boss reveals, and reduced-motion fallback behavior. This slice gives every major story beat a visible camera-blocking target that can later be replaced by authored Level Sequence assets without changing gameplay flow.

## Runtime Integration

- Extended `SpawnCinematicStreetLifeLayer()` with a `SEQUENCER BLOCKING REEL` board.
- Added camera rails, tripods, lens markers, labels, and connectors for six beats:
  - intro city establish
  - selected-language continuity
  - safe terminal solve
  - survivor rescue reveal
  - extraction route closure
  - boss reveal fallback
- All blocking actors are tagged with `SequencerIntroBossRevealBlocking`, `CinematicCameraBlockingReady`, `SequencerReadyFallback`, and `ControlRigReadyFallback`.
- Added beat-specific tags such as `SequencerIntroBeat`, `SequencerLanguageBeat`, `SequencerTerminalBeat`, `SequencerSurvivorBeat`, `SequencerExtractionBeat`, and `SequencerBossRevealBeat`.
- Extended `ABossRevealPresentationActor` so the existing boss reveal fallback participates in the same cinematic blocking contract.

## Data And QA

- Added `Content/CodeRescueData/cinematic_sequence_blocking_manifest.tsv`.
- Updated the creative-development inclusion plan so the P2 cinematic row routes through `verify_sequencer_intro_boss_reveal_slice_pass.py`, the existing boss reveal verifier, packaged render smoke, and manual cinematic review.
- Added human QA and visual regression entries for the intro/language/terminal/rescue/extraction/boss camera beat sequence.
- Wired the new verifier into full QA and local CI.

## Validation

The static verifier checks:

- Sequencer blocking tags and beat labels in `SpawnCinematicStreetLifeLayer()`
- the existing `ABossRevealPresentationActor` integration with the new boss reveal beat tags
- the cinematic blocking manifest and review records
- CI wiring, documentation, and progress-log coverage

Full validation should run the new static verifier, the existing boss reveal presentation verifier, module recompile, Mac packaging, packaged null smoke, and packaged render smoke.

## Boundaries

This pass does not create final `.uasset` Level Sequence, Camera Cut Track, Control Rig track, audio stem, or cinematic Blueprint assets. It creates the safe blocking layer and audit contract those authored cinematic assets should replace later.
