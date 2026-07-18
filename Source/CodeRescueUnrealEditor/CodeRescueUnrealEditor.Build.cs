using UnrealBuildTool;

public class CodeRescueUnrealEditor : ModuleRules
{
    public CodeRescueUnrealEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "DataValidation",
            "CodeRescueUnreal"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "AssetRegistry",
            "UnrealEd",
            "PhysicsUtilities"   // FPhysicsAssetUtils (CodeRescueV3PhysicsLibrary)
        });
    }
}
