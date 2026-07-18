# Item 12: Radio Communication & Story Narration

## Summary
Implement background radio broadcasts for story narration, mission briefings, and atmospheric flavor. Use UI radio widget with tuning, channel selection, and dynamic audio playback based on in-game story progression.

## What Changed
- Created URadioWidget for player UI with tunable frequency display
- Added ARadioStation actor with channel list and broadcast schedules
- Implemented broadcast timeline (mission briefing, ambient chatter, warning alerts)
- Integrated radio audio playback with story progression triggers

## Design Decisions
1. **Radio Channels** (10 frequencies, each ~10 minutes of content):
   - 88.5 FM: KDOOM (military/tactical channel, mission briefings)
   - 89.3 FM: KSURVIVE (civilian emergency alerts, rescue coordination)
   - 90.1 FM: KZONE (zombie activity reports, threat warnings)
   - 91.5 FM: KCULTURE (pre-apocalypse music/entertainment, story flavor)
   - 92.7 FM: KMUTE (static/dead air, narrative tension)

2. **Broadcast Timing**:
   - Broadcasts trigger on mission completion or city entry
   - Each broadcast plays WAV file in sequence (no looping within missions)
   - Player can switch channels during broadcast (audio crossfades over 0.5s)
   - Radio mutes when UI is closed (players can disable radio via settings)

3. **Story Integration**:
   - Briefing channel (88.5) provides mission objective summary
   - Threat channel (90.1) updates zombie pack locations in real-time
   - Survivor channel (89.3) announces rescue targets and extraction times
   - Warning channel activates during story cutscenes

## Files Touched
- `Source/CodeRescue/UI/RadioWidget.h` (URadioWidget class, frequency tuning)
- `Source/CodeRescue/UI/RadioWidget.cpp` (channel switching, audio playback)
- `Source/CodeRescue/Game/RadioStation.h` (ARadioStation, broadcast schedule)
- `Source/CodeRescue/Game/RadioStation.cpp` (broadcast timeline management)

## Known Limitations
- Radio broadcasts are static WAV files, no real-time synthesis
- Channel tuning is discrete (no realistic frequency sweep), only 10 channels
- Broadcasts do not adapt to player performance (no dynamic difficulty narration)
- Radio audio cannot be interrupted by gameplay events (fires to completion)

## Follow-Up Work
1. Implement dynamic radio content based on mission success/failure
2. Add procedural radio chatter generation (text-to-speech NPC voices)
3. Implement radio signal jamming mechanics (certain areas lose reception)
4. Add radio easter eggs (hidden frequencies with bonus lore)

## Compiler Notes
**Mac Build Step**: After `./Recompile_Module.command`, verify radio initialization:
```
UE_LOG(LogRadio, Warning, TEXT("Radio station initialized with %d channels"), NumChannels);
```
Radio audio files must be in WAV format, placed at: `Content/Audio/Radio/Broadcasts/{Channel}_{BroadcastID}.wav`
