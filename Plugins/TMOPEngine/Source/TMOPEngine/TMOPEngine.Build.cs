using UnrealBuildTool;

public class TMOPEngine : ModuleRules
{
    public TMOPEngine(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
            "AIModule", "NavigationSystem", "GameplayTasks", "UMG", "Slate", "SlateCore",
            "DeveloperSettings", "Json", "JsonUtilities"
        });
    }
}
