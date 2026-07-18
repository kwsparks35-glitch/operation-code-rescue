using UnrealBuildTool;
using System.Collections.Generic;

public class CodeRescueUnrealEditorTarget : TargetRules
{
    public CodeRescueUnrealEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        // UE 5.7 uses stricter default build settings than earlier UE 5.x versions.
        // The previous project target used backward-compatible settings, which made
        // UnrealBuildTool report: UndefinedIdentifierWarningLevel: Off != Error.
        // Using V6 aligns the project target with UE 5.7's default editor build environment.
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
ExtraModuleNames.Add("CodeRescueUnreal");
    }
}
