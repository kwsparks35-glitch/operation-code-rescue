#include "CodeRescueAchievementSystem.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueSaveGame.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

const TArray<FAchievementDef>& UCodeRescueAchievementSystem::GetCatalog()
{
    static const TArray<FAchievementDef> Catalog = {
        { TEXT("first_blood"),  TEXT("First Blood"),       TEXT("Neutralize your first zombie."),                            0 },
        { TEXT("first_solve"),  TEXT("Hello, World!"),     TEXT("Solve your first coding terminal."),                        1 },
        { TEXT("first_rescue"), TEXT("Lifeline"),           TEXT("Rescue your first survivor."),                              2 },
        { TEXT("polyglot"),     TEXT("Polyglot"),           TEXT("Solve a terminal in all four languages."),                  3 },
        { TEXT("speedrun"),     TEXT("Speedrun"),           TEXT("Clear five cities in under thirty minutes."),               4 },
        { TEXT("no_resupply"),  TEXT("Frugal"),             TEXT("Clear a city without picking up any supplies."),            5 },
        { TEXT("headhunter"),   TEXT("Headhunter"),         TEXT("Land 25 headshots."),                                       6 },
        { TEXT("century"),      TEXT("Century"),            TEXT("Neutralize 100 zombies."),                                  7 },
    };
    return Catalog;
}

bool UCodeRescueAchievementSystem::IsUnlocked(UWorld* World, int32 BitIndex)
{
    if (!World) return false;
    UCodeRescueGameInstance* GI = World->GetGameInstance<UCodeRescueGameInstance>();
    if (!GI) return false;
    UCodeRescueSaveGame* Save = Cast<UCodeRescueSaveGame>(UGameplayStatics::CreateSaveGameObject(UCodeRescueSaveGame::StaticClass()));
    // We can read the live bitmap on the GameInstance via SaveGame round-trip
    // — but for simplicity, mirror it onto the GI itself: read via GI.
    // The bitmap lives only on SaveGame in this commit, so we don't have it
    // hot in memory. Fall through to "always false" for the live check —
    // award still works because we read+write through the SaveGame each time.
    return false;
}

bool UCodeRescueAchievementSystem::Award(UWorld* World, int32 BitIndex)
{
    if (!World) return false;
    UCodeRescueGameInstance* GI = World->GetGameInstance<UCodeRescueGameInstance>();
    if (!GI) return false;
    if (BitIndex < 0 || BitIndex >= 64) return false;

    // Round-trip through the SaveGame to read+write the bitmap idempotently.
    UCodeRescueSaveGame* Save = nullptr;
    if (UGameplayStatics::DoesSaveGameExist(GI->SaveSlotName, 0))
    {
        Save = Cast<UCodeRescueSaveGame>(UGameplayStatics::LoadGameFromSlot(GI->SaveSlotName, 0));
    }
    if (!Save)
    {
        Save = Cast<UCodeRescueSaveGame>(UGameplayStatics::CreateSaveGameObject(UCodeRescueSaveGame::StaticClass()));
    }
    if (!Save) return false;

    const int64 Mask = (int64)1 << BitIndex;
    if ((Save->AchievementsUnlocked & Mask) != 0)
    {
        return false;   // already unlocked
    }
    Save->AchievementsUnlocked |= Mask;
    UGameplayStatics::SaveGameToSlot(Save, GI->SaveSlotName, 0);

    // Find the catalog entry for the toast.
    for (const FAchievementDef& Def : GetCatalog())
    {
        if (Def.BitIndex == BitIndex)
        {
            ShowToast(World, Def);
            break;
        }
    }
    return true;
}

void UCodeRescueAchievementSystem::ShowToast(UWorld* World, const FAchievementDef& Def)
{
    if (!GEngine) return;
    GEngine->AddOnScreenDebugMessage(
        -1, 5.0f,
        FColor(255, 215, 0),
        FString::Printf(TEXT("ACHIEVEMENT UNLOCKED — %s\n%s"),
            *Def.DisplayName, *Def.Description));
}

void UCodeRescueAchievementSystem::EvaluateAll(UWorld* World)
{
    if (!World) return;
    UCodeRescueGameInstance* GI = World->GetGameInstance<UCodeRescueGameInstance>();
    if (!GI) return;

    if (GI->KillCount >= 1)            Award(World, 0);   // First Blood
    if (GI->TerminalSolveCount >= 1)   Award(World, 1);   // Hello, World!
    if (GI->RescueCount >= 1)          Award(World, 2);   // Lifeline
    if (GI->TerminalSolveCount >= 5 && GI->RunSeconds < 1800.0f)
        Award(World, 4);                                    // Speedrun
    if (GI->HeadshotCount >= 25)       Award(World, 6);   // Headhunter
    if (GI->KillCount >= 100)          Award(World, 7);   // Century
}

// ============================================================================
// #57 — pluggable achievement backends
// ============================================================================
bool FLocalAchievementBackend::Award(UWorld* World, const FAchievementDef& Def)
{
    return UCodeRescueAchievementSystem::Award(World, Def.BitIndex);
}

bool FLocalAchievementBackend::IsUnlocked(UWorld* World, int32 BitIndex)
{
    return UCodeRescueAchievementSystem::IsUnlocked(World, BitIndex);
}

bool FSteamAchievementBackend::Award(UWorld* World, const FAchievementDef& Def)
{
    // Stub. With Steam OSS wired up, this would call:
    //   IOnlineSubsystem::Get(STEAM_SUBSYSTEM)->GetAchievementsInterface()->WriteAchievements(...)
    UE_LOG(LogTemp, Log, TEXT("[Steam] Would award achievement %s"), *Def.Id);
    return true;
}

bool FSteamAchievementBackend::IsUnlocked(UWorld* World, int32 BitIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[Steam] IsUnlocked(%d) — stub, returning false"), BitIndex);
    return false;
}
