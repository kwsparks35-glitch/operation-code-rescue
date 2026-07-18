#include "CodeRescueLeaderboards.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FString UCodeRescueLeaderboards::PathFor(ELeaderboardKind Kind)
{
    static const TCHAR* Names[] = {
        TEXT("fastest_five_city"), TEXT("most_rescues"),
        TEXT("most_headshots"), TEXT("longest_no_resupply")
    };
    const int32 Idx = static_cast<int32>(Kind);
    const FString Dir = FPaths::ProjectSavedDir() / TEXT("Leaderboards");
    IFileManager::Get().MakeDirectory(*Dir, true);
    return Dir / FString::Printf(TEXT("%s.json"), Names[FMath::Clamp(Idx, 0, 3)]);
}

TArray<FLeaderboardEntry> UCodeRescueLeaderboards::Fetch(ELeaderboardKind Kind)
{
    TArray<FLeaderboardEntry> Out;
    FString Raw;
    if (!FFileHelper::LoadFileToString(Raw, *PathFor(Kind))) return Out;
    TArray<TSharedPtr<FJsonValue>> Items;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
    if (!FJsonSerializer::Deserialize(Reader, Items)) return Out;
    for (const TSharedPtr<FJsonValue>& Item : Items)
    {
        const TSharedPtr<FJsonObject>* Obj;
        if (!Item->TryGetObject(Obj)) continue;
        FLeaderboardEntry E;
        E.PlayerName = (*Obj)->GetStringField(TEXT("name"));
        E.Score = (int64)(*Obj)->GetNumberField(TEXT("score"));
        E.Timestamp = (*Obj)->GetStringField(TEXT("ts"));
        Out.Add(E);
    }
    return Out;
}

bool UCodeRescueLeaderboards::Submit(ELeaderboardKind Kind, const FString& PlayerName, int64 Score)
{
    TArray<FLeaderboardEntry> List = Fetch(Kind);
    FLeaderboardEntry New;
    New.PlayerName = PlayerName;
    New.Score = Score;
    New.Timestamp = FDateTime::UtcNow().ToString();
    List.Add(New);
    // Sort: high-score-wins for most_rescues/headshots/no_resupply,
    // low-score-wins for fastest_five_city.
    const bool bAscending = (Kind == ELeaderboardKind::FastestFiveCity);
    List.Sort([bAscending](const FLeaderboardEntry& A, const FLeaderboardEntry& B)
    {
        return bAscending ? (A.Score < B.Score) : (A.Score > B.Score);
    });
    if (List.Num() > 10) List.SetNum(10);

    // Serialize back.
    TArray<TSharedPtr<FJsonValue>> Items;
    for (const FLeaderboardEntry& E : List)
    {
        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetStringField(TEXT("name"), E.PlayerName);
        Obj->SetNumberField(TEXT("score"), (double)E.Score);
        Obj->SetStringField(TEXT("ts"), E.Timestamp);
        Items.Add(MakeShared<FJsonValueObject>(Obj));
    }
    FString Raw;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Raw);
    FJsonSerializer::Serialize(Items, Writer);
    return FFileHelper::SaveStringToFile(Raw, *PathFor(Kind));
}

FString UCodeRescueLeaderboards::PrettyPrint(ELeaderboardKind Kind)
{
    static const TCHAR* Names[] = {
        TEXT("Fastest 5-City"), TEXT("Most Rescues"),
        TEXT("Most Headshots"), TEXT("Longest No-Resupply")
    };
    FString Out = FString::Printf(TEXT("LEADERBOARD — %s\n"), Names[static_cast<int32>(Kind)]);
    int32 Rank = 1;
    for (const FLeaderboardEntry& E : Fetch(Kind))
    {
        Out += FString::Printf(TEXT("  %2d. %-16s  %lld   %s\n"),
            Rank++, *E.PlayerName, (long long)E.Score, *E.Timestamp);
    }
    if (Rank == 1) Out += TEXT("  (no entries yet)\n");
    return Out;
}
