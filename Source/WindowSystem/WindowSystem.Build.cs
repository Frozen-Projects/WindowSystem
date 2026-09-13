// Some copyright should be here...

using UnrealBuildTool;
public class WindowSystem : ModuleRules
{
	public WindowSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		CppCompileWarningSettings.DeprecationWarningLevel = WarningLevel.Error;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "Slate", "SlateCore", "UMG", "InputCore", "RHI", "RenderCore" });
		PrivateDependencyModuleNames.Add("Renderer");
	}
}
