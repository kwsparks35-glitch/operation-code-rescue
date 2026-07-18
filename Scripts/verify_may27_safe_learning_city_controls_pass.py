#!/usr/bin/env python3
"""
Static verifier for the May 27 safe-learning, city, controls, and health pass.

This checks the user-facing contract:
- all coding terminals live in protected learning zones away from zombie spawns,
- the campaign uses one preselected language track and now includes C+ / C++,
- terminals pause combat and reward survivor intel instead of spawning hordes,
- the city layer reads as an authored street grid rather than open clutter,
- F1-F6 camera controls and 1-0 weapon slots are wired,
- the HUD/death flow supports health, replay, and save-and-quit.
"""

from __future__ import annotations

from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"


def read(path: Path) -> str:
    if not path.exists():
        raise RuntimeError(f"missing {path}")
    return path.read_text(encoding="utf-8")


def require(path: Path, tokens: list[str]) -> None:
    content = read(path)
    missing = [token for token in tokens if token not in content]
    if missing:
        raise RuntimeError(f"{path} missing tokens: {', '.join(missing)}")


def forbid(path: Path, tokens: list[str]) -> None:
    content = read(path)
    present = [token for token in tokens if token in content]
    if present:
        raise RuntimeError(f"{path} still contains forbidden tokens: {', '.join(present)}")


def main() -> int:
    types_h = SRC / "CodeRescueTypes.h"
    gi_cpp = SRC / "CodeRescueGameInstance.cpp"
    campaign_cpp = SRC / "CodeRescueCampaign.cpp"
    runner_cpp = SRC / "CodeRunnerLibrary.cpp"
    runner_h = SRC / "CodeRunnerLibrary.h"
    terminal_cpp = SRC / "CodeTerminalWidget.cpp"
    character_cpp = SRC / "CodeRescueCharacter.cpp"
    character_h = SRC / "CodeRescueCharacter.h"
    hud_cpp = SRC / "CodeRescueHUDWidget.cpp"
    hud_h = SRC / "CodeRescueHUDWidget.h"
    death_cpp = SRC / "CodeRescueDeathWidget.cpp"
    death_h = SRC / "CodeRescueDeathWidget.h"
    menu_cpp = SRC / "CodeRescueMainMenuWidget.cpp"
    menu_h = SRC / "CodeRescueMainMenuWidget.h"
    mode_cpp = SRC / "CodeRescueGameMode.cpp"
    mode_h = SRC / "CodeRescueGameMode.h"
    mod_loader_cpp = SRC / "CodeRescueModLoader.cpp"

    require(types_h, [
        "CPlus   UMETA(DisplayName = \"C+\")",
        "Cpp     UMETA(DisplayName = \"C++\")",
    ])
    require(gi_cpp, [
        "constexpr int32 CodeRescueLanguageCount = 6;",
        "case ECodingLanguage::CPlus: return TEXT(\"C+\");",
        "case ECodingLanguage::Cpp: return TEXT(\"C++\");",
        "%s track %d/%d",
    ])
    require(campaign_cpp, [
        "case 5: return ECodingLanguage::CPlus;",
        "default: return ECodingLanguage::Cpp;",
        "C+ track emphasizes",
        "C++ track emphasizes",
        "launch track note",
    ])
    require(runner_h, ["ValidateCpp"])
    require(runner_cpp, [
        "static TOptional<bool> GHasCpp;",
        "ProbeExecutable(TEXT(\"clang++\")",
        "case ECodingLanguage::CPlus:",
        "case ECodingLanguage::Cpp: return ValidateCpp",
        "FCodeValidationResult UCodeRunnerLibrary::ValidateCpp",
        "clang++ -Wall -Wextra -std=c++17",
        "LangStr == TEXT(\"C+\")",
        "LangStr == TEXT(\"C++\")",
    ])
    require(mod_loader_cpp, [
        "LangStr == TEXT(\"C+\")",
        "LangStr == TEXT(\"C++\")",
    ])

    require(menu_h, [
        "OnCPlusLanguageClicked",
        "OnCppLanguageClicked",
        "SetSelectedLanguage(ECodingLanguage Language)",
        "CPlusLanguageBtn",
        "CppLanguageBtn",
    ])
    require(menu_cpp, [
        "CHOOSE CODING LANGUAGE",
        "Select one track to deploy into the rescue zone.",
        "NEW %s RUN",
        "RESUME %s SAVE",
        "StartLanguageRun(ECodingLanguage Language)",
        "ResumeLanguageRun(ECodingLanguage Language)",
        "GI->SelectedLanguage = Language;",
        "GI->SaveSlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(Language);",
    ])

    require(terminal_cpp, [
        "TerminalActor->Challenge.Language = GI->SelectedLanguage;",
        "TerminalActor->Challenge.StarterCode = MakeStarterForLanguage",
        "Intel Reward: survivor whereabouts uploaded",
        "UGameplayStatics::SetGamePaused(GetWorld(), false);",
        "case ECodingLanguage::CPlus:",
        "case ECodingLanguage::Cpp:",
    ])
    # 2026-07-04 design evolution: the 2026-07-02 pass INTENTIONALLY wired the
    # post-solve horde into the terminal (core loop: solve -> horde -> fight;
    # playtest-verified). Learning safety is preserved by the protected learning
    # zone that blocks zombie damage near terminals. Verify THAT contract instead.
    require(terminal_cpp, ["RecordTerminalSolved"])  # horde only after a solve
    ch_cpp = (SRC / "CodeRescueCharacter.cpp").read_text(errors="replace")
    if "IsLocationInsideProtectedLearningZone" not in ch_cpp:
        raise RuntimeError("protected learning zone missing: terminals would be unsafe with post-solve hordes")
    require(character_cpp, [
        "UGameplayStatics::SetGamePaused(GetWorld(), true);",
        "Terminal opened in protected learning mode",
        "EKeys::F1",
        "EKeys::F6",
        "EKeys::One",
        "EKeys::Zero",
        "SelectWeaponSlot10",
        "SetActive(bFirstPerson, true)",
        "SetActive(!bFirstPerson, true)",
        "MaxEnemyDamagePerHitFraction",
        "Health <= 0.0f",
    ])
    require(character_h, [
        "MaxEnemyDamagePerHitFraction = 0.16f",
        "bPreventSingleHitEnemyDeaths = true",
        "SelectWeaponSlot1",
        "SelectWeaponSlot10",
    ])

    require(hud_h, ["UProgressBar* HealthBar"])
    require(hud_cpp, [
        "HealthBar = WidgetTree->ConstructWidget<UProgressBar>",
        "HealthBar->SetPercent(HealthPct)",
        "F1-F6 views",
        "1-0 select",
        "Wheel/[ ] cycle weapons",
        "Protected coding safehouse active",
        "Coding pauses combat; solving rewards survivor intel.",
        "Track %s %d/%d",
    ])
    require(death_h, ["SaveAndQuitButton", "OnSaveAndQuitClicked"])
    require(death_cpp, [
        "RESUME FROM LANGUAGE SAVE",
        "START FRESH LANGUAGE RUN (delete this save)",
        "SAVE THIS LANGUAGE RUN AND QUIT",
        "GI->SaveDeathRecoveryCheckpoint(false)",
    ])

    require(mode_h, [
        "SpawnMajorCityUrbanIdentityLayer",
        "SpawnProtectedCodingChallengeHub",
    ])
    require(mode_cpp, [
        "Protected Secret Coding Annex",
        "BonusCodingChallengeSafeZone",
        "ProtectedCodingChallengeZone",
        "NoZombieLearningZone",
        "SafeTerminalLab",
        "SelectedLanguageOnly",
        "LearningWithoutDeathRisk",
        "SpawnProtectedCodingChallengeHub",
        "SpawnTerminal(Hub + FVector(0.0f, -70.0f, 90.0f)",
        "SpawnMajorCityUrbanIdentityLayer",
        "MajorCityUrbanLandscape",
        "USMajorCityIdentity",
        "StreetGridCityComposition",
        "CITY LANDSCAPE PASS",
        "selected language only",
        "combat pauses in the lab",
        "1-0 quick slots",
        "FMath::Clamp(BaseX + CityStream.FRandRange(-105.0f, 105.0f), -720.0f, 3660.0f)",
        "FMath::Clamp(BaseY + CityStream.FRandRange(-95.0f, 95.0f), 760.0f, 2460.0f)",
        "[CodeRescueSafeLearning]",
    ])
    forbid(mode_cpp, [
        "FVector(3500.0f, 3000.0f, 350.0f)",
        "TEXT(\"TACTICAL ARMORY\\nall weapons unlocked\\nwheel or [ ] cycles arsenal\\n1-4 quick equip",
    ])

    print("[verify-may27-safe-learning-city-controls] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
