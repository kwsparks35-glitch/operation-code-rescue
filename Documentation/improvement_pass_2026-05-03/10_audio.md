# Item 10: Audio Attenuation & Spatial Sound

## Summary
Implement spatial audio with distance-based attenuation for zombie vocalizations, gunfire, and environmental ambience. Use UE5 attenuation curves and reverb zones to create immersive soundscape feedback.

## What Changed
- Added ACodeZombieActor::PlayVocalization() with attenuation settings
- Created UAudioAttenuationComponent for spatial sound positioning
- Implemented SoundAttenuation curves in CodeRescueSoundSettings
- Added reverb zone detection for audio environment adaptation

## Design Decisions
1. **Attenuation Curves**:
   - Zombie vocalizations: 400cm min distance, 3000cm max distance, -6dB/octave rolloff
   - Gunfire: 500cm min distance, 5000cm max distance, -3dB/octave rolloff (more intense)
   - Footsteps: 200cm min distance, 1000cm max distance, -12dB/octave rolloff (close-range cue)

2. **Spatial Sound Priority**:
   - Only play vocalization if player within attenuation range
   - Cull zombie sounds beyond max distance (performance optimization)
   - Gunfire audio plays even outside max range for narrative impact

3. **Reverb Zones**:
   - Indoor areas (subway, parking garage): high reverb tail (2.5s decay)
   - Outdoor areas (city streets): minimal reverb (0.2s decay)
   - Transitional spaces: fade reverb over 1.0s

## Files Touched
- `Source/CodeRescue/Character/CodeZombieActor.h` (PlayVocalization signature)
- `Source/CodeRescue/Character/CodeZombieActor.cpp` (vocalization attenuation logic)
- `Source/CodeRescue/Game/CodeRescueSoundSettings.h` (attenuation curve definitions)
- `Source/CodeRescue/Game/CodeRescueSoundSettings.cpp` (spatial audio initialization)

## Known Limitations
- Attenuation curves are linear approximations, not physically accurate
- Reverb zone detection uses simple box overlap, not complex geometry
- Audio source pooling not implemented (potential for audio thread starvation)
- Doppler effects not simulated (listener moving relative to source)

## Follow-Up Work
1. Implement spatial audio using 3D audio libraries (Windows Sonic, Dolby Atmos)
2. Add audio occlusion for walls/buildings blocking sound propagation
3. Implement footstep audio pool for NPC zombie movement
4. Integrate dynamic music layers with threat intensity scaling

## Compiler Notes
**Mac Build Step**: After `./Recompile_Module.command`, verify audio initialization:
```
UE_LOG(LogAudio, Warning, TEXT("Audio attenuation initialized: %d zones"), NumReverbZones);
```
Sound assets already defined in project. No additional WAV/OGG imports needed for this item.
