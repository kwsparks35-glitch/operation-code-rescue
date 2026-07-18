#include "CodeRescueCurriculumLibrary.h"
#include "CodeRescueCampaign.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
FString CampaignLessonKindName(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Sum: return TEXT("Sum");
    case ECampaignLessonKind::Lock: return TEXT("Lock");
    case ECampaignLessonKind::Reverse: return TEXT("Reverse");
    case ECampaignLessonKind::Palindrome: return TEXT("Palindrome");
    case ECampaignLessonKind::FizzBuzz: return TEXT("FizzBuzz");
    case ECampaignLessonKind::EvenFilter: return TEXT("EvenFilter");
    case ECampaignLessonKind::LinkedListTraverse: return TEXT("LinkedListTraverse");
    case ECampaignLessonKind::BinarySearch: return TEXT("BinarySearch");
    default: return TEXT("Unknown");
    }
}
}

FString UCodeRescueCurriculumLibrary::GetCurriculumDatabasePath()
{
    return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("CodeRescueData"), TEXT("curriculum_database.json"));
}

static void ReadStringArray(const TSharedPtr<FJsonObject>& Object, const FString& FieldName, TArray<FString>& OutValues)
{
    OutValues.Reset();
    const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
    if (Object.IsValid() && Object->TryGetArrayField(FieldName, Values))
    {
        for (const TSharedPtr<FJsonValue>& Value : *Values)
        {
            OutValues.Add(Value->AsString());
        }
    }
}

bool UCodeRescueCurriculumLibrary::LoadCurriculumEntries(TArray<FCodeRescueCurriculumEntry>& OutEntries, FString& OutError)
{
    OutEntries.Reset();
    OutError.Empty();

    FString JsonText;
    const FString Path = GetCurriculumDatabasePath();
    if (!FFileHelper::LoadFileToString(JsonText, *Path))
    {
        OutError = FString::Printf(TEXT("Could not read curriculum database at %s"), *Path);
        return false;
    }

    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        OutError = TEXT("Curriculum database JSON is invalid.");
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
    if (!Root->TryGetArrayField(TEXT("entries"), Entries))
    {
        OutError = TEXT("Curriculum database is missing an 'entries' array.");
        return false;
    }

    for (const TSharedPtr<FJsonValue>& EntryValue : *Entries)
    {
        TSharedPtr<FJsonObject> EntryObject = EntryValue->AsObject();
        if (!EntryObject.IsValid()) continue;

        FCodeRescueCurriculumEntry Entry;
        Entry.Id = EntryObject->GetStringField(TEXT("id"));
        Entry.Title = EntryObject->GetStringField(TEXT("title"));
        Entry.Language = EntryObject->GetStringField(TEXT("language"));
        Entry.Concept = EntryObject->GetStringField(TEXT("concept"));
        Entry.Difficulty = EntryObject->GetIntegerField(TEXT("difficulty"));
        ReadStringArray(EntryObject, TEXT("strategies"), Entry.Strategies);
        ReadStringArray(EntryObject, TEXT("common_mistakes"), Entry.Mistakes);
        OutEntries.Add(Entry);
    }

    return true;
}

TArray<FCodeRescueCampaignAuditEntry> UCodeRescueCurriculumLibrary::GetCampaignAuditEntries()
{
    TArray<FCodeRescueCampaignAuditEntry> OutEntries;
    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    OutEntries.Reserve(Missions.Num());

    for (const FCodeRescueCityMission& Mission : Missions)
    {
        FCodeRescueCampaignAuditEntry Entry;
        Entry.Rank = Mission.Rank;
        Entry.CityName = Mission.CityName;
        Entry.StateName = Mission.StateName;
        Entry.Slug = Mission.Slug;
        Entry.TerminalId = Mission.TerminalId;
        Entry.TerminalTitle = Mission.TerminalTitle;
        Entry.LessonKind = CampaignLessonKindName(Mission.LessonKind);
        Entry.CurriculumStageName = Mission.CurriculumStageName;
        Entry.CurriculumFocus = Mission.CurriculumFocus;
        Entry.MissionBrief = Mission.MissionBrief;
        Entry.RegionName = Mission.RegionName;
        Entry.DistrictStyle = Mission.DistrictStyle;
        Entry.LandmarkName = Mission.LandmarkName;
        Entry.ArtKitName = Mission.ArtKitName;
        Entry.ArchitectureSignature = Mission.ArchitectureSignature;
        Entry.NovelGameplayDetail = Mission.NovelGameplayDetail;
        Entry.LanguageTrackText = Mission.LanguageTrackText;
        Entry.LearningSupportText = Mission.LearningSupportText;
        Entry.VisualDebuggerPlan = Mission.VisualDebuggerPlan;
        Entry.ProgressionPlan = Mission.ProgressionPlan;
        Entry.CharacterStoryPlan = Mission.CharacterStoryPlan;
        Entry.GameplayFlowPlan = Mission.GameplayFlowPlan;
        Entry.AccessibilityPolishPlan = Mission.AccessibilityPolishPlan;
        Entry.QAVerificationPlan = Mission.QAVerificationPlan;
        Entry.HintText = Mission.HintText;
        Entry.VisibleTestBrief = Mission.VisibleTestBrief;
        Entry.HiddenTestBrief = Mission.HiddenTestBrief;
        Entry.RadioBriefing = Mission.RadioBriefing;
        Entry.RadioVoiceName = Mission.RadioVoiceName;
        Entry.DifficultyTier = Mission.DifficultyTier;
        Entry.EncounterIntensity = Mission.EncounterIntensity;
        OutEntries.Add(Entry);
    }

    return OutEntries;
}
