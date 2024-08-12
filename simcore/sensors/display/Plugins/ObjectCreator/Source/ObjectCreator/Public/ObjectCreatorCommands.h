// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ObjectCreatorStyle.h"

class FObjectCreatorCommands : public TCommands<FObjectCreatorCommands>
{
public:
    FObjectCreatorCommands()
        : TCommands<FObjectCreatorCommands>(TEXT("ObjectCreator"),
              NSLOCTEXT("Contexts", "ObjectCreator", "ObjectCreator Plugin"), NAME_None,
              FObjectCreatorStyle::GetStyleSetName())
    {
    }

    // TCommands<> interface
    virtual void RegisterCommands() override;

public:
    TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
