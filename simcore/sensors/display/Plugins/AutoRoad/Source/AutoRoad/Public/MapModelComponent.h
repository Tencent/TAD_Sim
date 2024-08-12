// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MapModelComponent.generated.h"

struct FMapModelData
{
    FMapModelData(const FString& _Path, const FVector& _Center) : Path(_Path), Center(_Center)
    {
    }

    FString Path;

    FVector Center;
};

UCLASS()
class AUTOROAD_API UMapModelComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMapModelComponent()
    {
    }

    void Init(const TMap<FString, TPair<FString, FVector>>& _ModelData, const FString& ModelRootPath);

    UStaticMesh* GetModelByName(const FString& ModelName);

private:
    TMap<FString, FMapModelData> ModelDataMap;
    FString ModelAssetPath;
};
