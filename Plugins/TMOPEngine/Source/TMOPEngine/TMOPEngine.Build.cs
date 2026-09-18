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
            "DeveloperSettings", "Json", "JsonUtilities", "Niagara", "MediaAssets",
            "AudioMixer", "EngineSettings"
        });
        // One-shot notebook render-target readback; no editor-only preview module.
        PrivateDependencyModuleNames.AddRange(new string[] { "RenderCore", "RHI" });
    }
}
