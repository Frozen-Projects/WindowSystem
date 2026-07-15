// Some copyright should be here...

using UnrealBuildTool;
public class WindowSystem : ModuleRules
{
	public WindowSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
                "InputCore",
				"RHI",
				"RenderCore",
			});
	}
}