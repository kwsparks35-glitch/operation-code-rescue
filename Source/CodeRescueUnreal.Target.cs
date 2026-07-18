using UnrealBuildTool;
using System.Collections.Generic;

public class CodeRescueUnrealTarget : TargetRules
{
    public CodeRescueUnrealTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

        // UE 5.7-compatible target settings.
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
ExtraModuleNames.Add("CodeRescueUnreal");
    }
}
