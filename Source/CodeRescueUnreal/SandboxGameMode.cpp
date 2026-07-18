#include "SandboxGameMode.h"

ASandboxGameMode::ASandboxGameMode()
{
    bSandboxMode = true;
    bEnableSystemRadioVoice = false;   // sandbox is silent — no radio briefing
    // Force min counts to zero so any zombie-budget knob the base GameMode
    // computed gets nullified by the bSandboxMode early-out in SpawnWorld.
    ZombieBaseCount = 0;
    ZombieMinCount = 0;
    ZombieMaxCount = 0;
}
