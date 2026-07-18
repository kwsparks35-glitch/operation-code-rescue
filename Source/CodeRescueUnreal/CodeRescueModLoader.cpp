#include "CodeRescueModLoader.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool UCodeRescueModLoader::ParseManifest(const FString& JsonPath, FCodeRescueModManifest& Out)
{
    FString Raw;
    if (!FFileHelper::LoadFileToString(Raw, *JsonPath)) return false;
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) return false;

    Out.Name        = Root->GetStringField(TEXT("name"));
    Out.Author      = Root->GetStringField(TEXT("author"));
    Out.Version     = Root->GetStringField(TEXT("version"));
    Out.Description = Root->GetStringField(TEXT("description"));

    const TArray<TSharedPtr<FJsonValue>>* ChallengesArr = nullptr;
    if (Root->TryGetArrayField(TEXT("custom_challenges"), ChallengesArr) && ChallengesArr)
    {
        for (const TSharedPtr<FJsonValue>& V : *ChallengesArr)
        {
            const TSharedPtr<FJsonObject>* Obj;
            if (!V->TryGetObject(Obj)) continue;
            FChallengeSpec C;
            C.Id = (*Obj)->GetStringField(TEXT("id"));
            C.Title = (*Obj)->GetStringField(TEXT("title"));
            C.MissionBrief = (*Obj)->GetStringField(TEXT("brief"));
            const FString LangStr = (*Obj)->GetStringField(TEXT("language"));
            if      (LangStr == TEXT("Java"))   C.Language = ECodingLanguage::Java;
            else if (LangStr == TEXT("C"))      C.Language = ECodingLanguage::C;
            else if (LangStr == TEXT("Python")) C.Language = ECodingLanguage::Python;
            else if (LangStr == TEXT("MATLAB")) C.Language = ECodingLanguage::MATLAB;
            else if (LangStr == TEXT("C+"))     C.Language = ECodingLanguage::CPlus;
            else if (LangStr == TEXT("C++"))    C.Language = ECodingLanguage::Cpp;
            Out.CustomChallenges.Add(C);
        }
    }
    return true;
}

TArray<FCodeRescueModManifest> UCodeRescueModLoader::LoadAllMods()
{
    TArray<FCodeRescueModManifest> Out;
    const FString ModsRoot = FPaths::ProjectSavedDir() / TEXT("Mods");
    if (!IFileManager::Get().DirectoryExists(*ModsRoot))
    {
        return Out;
    }

    TArray<FString> SubDirs;
    IFileManager::Get().FindFiles(SubDirs, *(ModsRoot / TEXT("*")), false, true);
    for (const FString& Sub : SubDirs)
    {
        const FString Manifest = ModsRoot / Sub / TEXT("manifest.json");
        if (!FPaths::FileExists(Manifest)) continue;
        FCodeRescueModManifest M;
        if (ParseManifest(Manifest, M))
        {
            Out.Add(M);
            UE_LOG(LogTemp, Log, TEXT("[ModLoader] loaded mod '%s' v%s by %s (%d challenges)"),
                *M.Name, *M.Version, *M.Author, M.CustomChallenges.Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[ModLoader] failed to parse %s"), *Manifest);
        }
    }
    return Out;
}

FString UCodeRescueModLoader::PrettyPrintLoadedMods()
{
    FString Out = TEXT("LOADED MODS\n");
    int32 N = 0;
    for (const FCodeRescueModManifest& M : LoadAllMods())
    {
        Out += FString::Printf(TEXT("  %s v%s by %s — %s (%d challenges)\n"),
            *M.Name, *M.Version, *M.Author, *M.Description, M.CustomChallenges.Num());
        ++N;
    }
    if (N == 0) Out += TEXT("  (none in Saved/Mods/)\n");
    return Out;
}
