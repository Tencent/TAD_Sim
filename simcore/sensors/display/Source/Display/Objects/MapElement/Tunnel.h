// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CameraOcculusionInterface.h"
#include "Engine/StaticMeshActor.h"
#include "Tunnel.generated.h"

UCLASS()
class DISPLAY_API ATunnel : public AStaticMeshActor, public ICameraOcculusionInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATunnel();

    UFUNCTION(BlueprintNativeEvent)
    TArray<FOcculusionAssetInfo> GetOcculusionInfo();
};
