using UnrealBuildTool;

public class TMOPEngineEditor : ModuleRules
{
    public TMOPEngineEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "TMOPEngine",
            "UnrealEd",
            "NavigationSystem",
            "PropertyEditor",
            "Slate",
            "SlateCore",
            "ToolMenus",
            "LevelEditor",
            "InputCore"
        });
    }
}
