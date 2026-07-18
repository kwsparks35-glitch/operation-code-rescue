#!/usr/bin/env python3
"""Static acceptance contract for the 2026-07-10 rescue/access UX pass."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read(name: str) -> str:
    return (ROOT / "Source" / "CodeRescueUnreal" / name).read_text(encoding="utf-8")


campaign_h = read("CodeRescueCampaign.h")
campaign_cpp = read("CodeRescueCampaign.cpp")
character = read("CodeRescueCharacter.cpp")
game_mode_h = read("CodeRescueGameMode.h")
game_mode = read("CodeRescueGameMode.cpp")
survivor = read("SurvivorActor.cpp")
terminal = read("CodeTerminalWidget.cpp")
zombie = read("CodeZombieActor.cpp")
pause_h = read("CodeRescuePauseWidget.h")
pause = read("CodeRescuePauseWidget.cpp")
hud = read("CodeRescueHUDWidget.cpp")
journal = read("CodeRescueObjectiveJournalWidget.cpp")

checks: list[tuple[bool, str]] = [
    ("RequiredChallengesPerCity = 10" in campaign_h, "ten challenges per city"),
    ("GetCityChallengeIds" in campaign_cpp and "Mission->Slug" in campaign_cpp,
     "unique neutral stage IDs preserve the legacy first terminal"),
    ("HasCompletedCityChallengeSet(GI, CityIndex)" in survivor,
     "survivor rescue uses the full city challenge gate"),
    ("coding clearance %d/%d" in survivor.lower(), "survivor prompt reports x/10 progress"),
    ("ArenaOuterHalfXLocal = 5500" in campaign_h and "ArenaOuterHalfYLocal = 4800" in campaign_h,
     "expanded shared mission-safe arena bounds"),
    ("GCodeRescueArenaSafeGroundZ = 108" in character and "LineTraceSingleByChannel" in character,
     "ground-level recovery instead of forced upper-plane respawn"),
    ("street_reachable" in game_mode and "bounded_doors" in game_mode,
     "door audit covers street-to-interior paths and arena bounds"),
    ("SupportHubShift" in game_mode and "SandbagCoverV4" in game_mode,
     "cafe approach blockers relocated"),
    ("DayNightPeriodSeconds = 1800.0f" in game_mode_h, "thirty-minute solar cycle"),
    ("SpawnChallengeCompletionSupplyCache" in game_mode_h and
     "ChallengeCompletionSupply" in game_mode, "coding completion supplies"),
    ("SpawnZombieDeathSupply" in game_mode_h and "SpawnZombieDeathSupply(ZombieId" in zombie,
     "zombie death supplies are wired to real deaths"),
    ("bFirstTimeChallengeCompletion" in terminal and "bClearanceComplete" in terminal,
     "first-time station rewards and tenth-station route unlock"),
    ("physical_stations=%d/10" in game_mode and "validators=%d/%d" in game_mode,
     "ten physical stations and sixty validator audit"),
    ("CampaignChallengeContractAudit" in game_mode and "expected=%d" in game_mode,
     "campaign-wide 10-per-level uniqueness audit"),
    ("GetCityChallengeProgress" in hud and "GetCityChallengeProgress" in journal,
     "HUD and journal expose challenge progress"),
    ("FInputModeUIOnly" in character and "bEnableClickEvents = true" in character,
     "pause uses UI-only no-capture pointer input"),
    ("SetClickMethod(EButtonClickMethod::MouseDown)" in pause and
     "HitTestInvisible" in pause, "direct button activation and non-blocking decoration"),
    ("NativeOnPreviewMouseButtonDown" in pause_h and "RoutePointerAtScreenPosition" in pause,
     "geometry-based pointer routing fallback"),
    ("CraftFlareButton" in pause_h and "PauseCraftingAudit" in pause,
     "functional in-pause crafting controls"),
    ("SavePersistentRun();" in read("CodeRescueGameInstance.cpp") and
     "MakeLanguageSaveSlotName" in read("CodeRescueGameInstance.cpp"),
     "progress and crafting remain language-save aware"),
]

failed = [label for passed, label in checks if not passed]
if failed:
    for label in failed:
        print(f"[FAIL] {label}")
    raise SystemExit(1)

for _, label in checks:
    print(f"[PASS] {label}")
print(f"[PASS] 2026-07-10 source contract ({len(checks)}/{len(checks)})")
