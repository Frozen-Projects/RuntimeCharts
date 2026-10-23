using UnrealBuildTool;

public class RuntimeCharts : ModuleRules
{
    public RuntimeCharts(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG", "SlateCore" });
        PrivateDependencyModuleNames.AddRange(new[] { "Slate", "InputCore", "RenderCore", "RHI", "ImageCore" });
    }
}
