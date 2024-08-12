// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DisplayHUD.generated.h"

/**
 *
 */
UCLASS()
class DISPLAY_API ADisplayHUD : public AHUD
{
    GENERATED_BODY()
    ADisplayHUD();

public:
    /** The Main Draw loop for the hud.  Gets called before any messaging.  Should be subclassed */
    virtual void DrawHUD();

protected:
    bool bShowNetMode = false;
};
