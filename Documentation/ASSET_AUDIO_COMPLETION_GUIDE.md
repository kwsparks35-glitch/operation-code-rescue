# Operation Code Rescue - Asset And Audio Completion Guide

The current project is complete as a procedural C++ prototype. This guide captures the optional art/audio path for a higher-fidelity release.

## Visual Asset Slots Already Exposed

- `ACodeRescueGameMode::SurvivorActorClass`
- `ACodeRescueGameMode::ZombieActorClass`
- `ACodeRescueGameMode::ZombieVariantTable`
- `ASurvivorActor::ProfessionalSurvivorMesh`
- `ASurvivorActor::ProfessionalSurvivorAnimClass`
- `ACodeZombieActor::ProfessionalZombieMesh`
- `ACodeZombieActor::ProfessionalZombieAnimClass`
- `ACodeZombieActor` hit, death, attack, growl, and VFX references

## Recommended Visual Replacement Order

1. Zombies: skeletal mesh, AnimBP, hit react, death, attack, growl, and death cue.
2. Survivors: MetaHuman or compatible skeletal mesh and idle animation.
3. Weapons: muzzle flash, tracer/impact VFX, shot audio.
4. City art kits: replace procedural kit blocks with authored static meshes.
5. Terminal/language-station props: replace helper cubes with readable interactable props.
6. UI skins: final fonts, colors, icons, and screen layout pass.

## Audio Completion Options

Current implementation:

- macOS system speech can speak generated radio briefings.
- `Content/CodeRescueData/radio_briefings.tsv` stores city briefing data.
- `Scripts/generate_radio_voiceovers.py` can generate WAV files.
- One sample briefing WAV exists for New York.

Optional full audio pass:

1. Generate all briefing WAVs with `Scripts/generate_radio_voiceovers.py --limit 0`.
2. Import generated WAVs into Unreal as `SoundWave` assets.
3. Map each city slug to its `SoundWave`.
4. Play the imported `SoundWave` instead of calling `/usr/bin/say`.
5. Keep `-NoRadioVoice` as a test override for smoke runs.

## Acceptance Criteria For Art/Audio Upgrade

- No objective uses an unintentional placeholder block unless it is part of the visual style.
- Zombies animate during idle, movement, attack, hit reaction, and death.
- Survivors read clearly as rescue targets from gameplay distance.
- Terminal and language stations are visually distinct.
- Radio/audio works in packaged builds without requiring external files.
- Packaged app size remains acceptable for the intended distribution channel.
