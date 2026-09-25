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
        PrivateDependencyModuleNames.AddRange(new string[] { "RenderCore", "RHI", "Projects" });
        // Runtime atlas JSON must be staged into packaged builds, not just available in the editor.
        RuntimeDependencies.Add("$(PluginDir)/Content/WorldAtlas/world.json", StagedFileType.UFS);
        RuntimeDependencies.Add("$(PluginDir)/Content/WorldAtlas/coastlines.json", StagedFileType.UFS);
    }
}
