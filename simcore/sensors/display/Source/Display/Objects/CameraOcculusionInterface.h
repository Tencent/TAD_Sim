// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CameraOcculusionInterface.generated.h"

USTRUCT(BlueprintType)
struct FOcculusionAssetInfo
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UPrimitiveComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UMaterialInterface*> OriginMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UMaterialInterface*> OcculusionMaterials;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCameraOcculusionInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API ICameraOcculusionInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    TArray<FOcculusionAssetInfo> GetOcculusionInfo();
};
