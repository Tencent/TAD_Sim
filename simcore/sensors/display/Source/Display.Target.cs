// Copyright 2024 Tencent Inc. All rights reserved.
//

using UnrealBuildTool;
using System.Collections.Generic;

public class DisplayTarget : TargetRules
{
    public DisplayTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        if (Configuration == UnrealTargetConfiguration.Shipping)
        {
            //BuildEnvironment = TargetBuildEnvironment.Unique;
            //bUseChecksInShipping = true;
            //bUseLoggingInShipping = true;
        }
        ExtraModuleNames.AddRange( new string[] { "Display" } );
    }
}
