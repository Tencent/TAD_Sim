// Copyright 2024 Tencent Inc. All rights reserved.
//

using UnrealBuildTool;
using System.Collections.Generic;

// display editor target
public class DisplayEditorTarget : TargetRules
{
    // display editor target
    public DisplayEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        // additional modules that should be loaded during startup
        ExtraModuleNames.AddRange( new string[] { "Display" } );
    }
}
