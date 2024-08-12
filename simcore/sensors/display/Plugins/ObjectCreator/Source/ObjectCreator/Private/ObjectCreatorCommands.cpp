// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectCreatorCommands.h"

#define LOCTEXT_NAMESPACE "FObjectCreatorModule"

void FObjectCreatorCommands::RegisterCommands()
{
    UI_COMMAND(OpenPluginWindow, "ObjectCreator", "Bring up ObjectCreator window", EUserInterfaceActionType::Button,
        FInputChord());
}

#undef LOCTEXT_NAMESPACE
