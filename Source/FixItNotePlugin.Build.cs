// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

using UnrealBuildTool;
 
public class FixItNotePlugin : ModuleRules {
    public FixItNotePlugin (TargetInfo Target) {
        PublicDependencyModuleNames.AddRange (
            new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				"Landscape",
				"PropertyEditor",
				"UnrealEd",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"PlacementMode",
                "RHI",
                "RenderCore",
                "Projects",
                "XmlParser",
            }
        );

        PrivateIncludePaths.AddRange (new string[] { "FixItNotePlugin/Private" });
        PublicIncludePaths.AddRange (new string[] { "FixItNotePlugin/Public" });
    }
}

