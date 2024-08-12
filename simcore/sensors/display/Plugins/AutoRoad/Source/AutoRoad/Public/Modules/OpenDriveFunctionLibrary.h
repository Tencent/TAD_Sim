// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OpenDriveFunctionLibrary.generated.h"

UCLASS()
class AUTOROAD_API UOpenDriveFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    // 获取中心点经纬度
public:
    static FVector GetMapCenterLonlat(
        const FString& pathIput, const double& RefX = 0, const double& RefY = 0, const double& RefZ = 0);

    UFUNCTION(BlueprintCallable)
    static FVector BPGetMapCenterLonlat(
        const FString& pathIput, const float& RefX = 0, const float& RefY = 0, const float& RefZ = 0);

    UFUNCTION(BlueprintCallable)
    static void RenameObjName(AActor* Target, const FString& NewName);
};
