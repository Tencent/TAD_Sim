// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <string>
#include "CatalogDataSource.generated.h"

UENUM()
enum class ECatalogType : uint8
{
    CT_EgoVehicle,
    CT_TrafficVehicle,
    CT_Creature,
    CT_Obstacle
};

USTRUCT()
struct FCommonCatalogData
{
    GENERATED_BODY()

    virtual ~FCommonCatalogData()
    {
    }

    UPROPERTY()
    FVector CenterOffset;

    UPROPERTY()
    int32 ModelID;

    UPROPERTY()
    FString TypeName;

    UPROPERTY()
    FString ModelPath;

    virtual FString GetModelPath()
    {
        return ModelPath;
    }
};

USTRUCT()
struct FTruckCatalogData : public FCommonCatalogData
{
    GENERATED_BODY()

    virtual ~FTruckCatalogData()
    {
    }

    UPROPERTY()
    FVector TruckHeadOffset;

    UPROPERTY()
    FVector TruckTrailerOffset;

    UPROPERTY()
    FVector CombinationTrailerOffset;

    UPROPERTY()
    FString TruckHeadModelPath;

    UPROPERTY()
    FString TruckTrailerModelPath;

    virtual FString GetModelPath() override
    {
        return TruckHeadModelPath + TEXT("__") + TruckTrailerModelPath;
    }
};

/**
 *
 */
UCLASS(config = game)
class DISPLAY_API UCatalogDataSource : public UObject
{
    GENERATED_BODY()

public:
    FVector GetOffset(const FString& TypeName);    // for ego catalog

    FTruckCatalogData GetTruckOffset(const FString& TypeName);    // for ego truck catalog

    FVector GetOffset(ECatalogType CatalogType, int32 Type);

    FString GetModelPathByType(ECatalogType CatalogType, const int32 Type);

    FString GetModelPathByTypeName(ECatalogType CatalogType, const FString& ModelPath);

    TArray<FString> GetTypeNameListWithModelPath(ECatalogType CatalogType, const FString& ModelPath);

    bool LoadSceneBuffer(const std::string& Buffer);

    void ClearData();

    FORCEINLINE const TMap<FString, TPair<FString, FVector>>& GetMapModelData()
    {
        return MapModelData;
    };

private:
    TMap<ECatalogType, TMap<FString, TSharedPtr<FCommonCatalogData>>> TrafficCatalogData;

    TMap<FString, TSharedPtr<FTruckCatalogData>> TruckOffsetData;

    TMap<FString, TPair<FString, FVector>> MapModelData;

    static TMap<int32, FString> VehicleMap_Name_Type;
    static TMap<int32, FString> CreatureMap_Name_Type;
    static TMap<int32, FString> Obstacle_Name_Type;
};
