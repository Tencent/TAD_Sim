// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "Objects/Transports/Vehicle/VehicleInterface.h"
#include "Misc/ConfigCacheIni.h"
#include "VehicleManager.generated.h"

class AVehiclePawn;
class UDataTable;

USTRUCT()
struct FVehicleManagerConfig : public FManagerConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FVehicleConfig> egoConfigArry;

    UPROPERTY()
    TArray<FVehicleConfig> trafficConfigArry;
};

USTRUCT()
struct FVehicleManagerIn : public FManagerIn
{
    GENERATED_BODY()
public:
    UPROPERTY()
    double timeStamp = 0.f;

    UPROPERTY()
    TArray<FVehicleIn> egoVehicleInputArry;

    UPROPERTY()
    TArray<FVehicleIn> trafficVehicleInputArry;
};

USTRUCT()
struct FVehicleManagerOut : public FManagerOut
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FVehicleOut> trafficOutArry;

    UPROPERTY()
    TArray<FVehicleOut> egoOutArry;
};

/**
 *
 */
UCLASS()
class DISPLAY_API AVehicleManager : public AManager
{
    GENERATED_BODY()
public:
    AVehicleManager();

    TArray<ISimActorInterface*> egoArry;

    TArray<ISimActorInterface*> trafficArry;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    TSubclassOf<AVehiclePawn> vehicleClass;

    // static bool bIsLoadTextTable;
    UDataTable* vehicleTypeTable;

    UPROPERTY()
    TMap<FString, TSubclassOf<AVehiclePawn> > vehicleClassMap;

public:
    virtual void Init(const FManagerConfig& Config);

    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

public:
    ISimActorInterface* GetCurrentEgo();

    ISimActorInterface* GetVehicle(ETrafficType _Type, int64 _Id);

    static FVehicleManagerConfig TranslateData(struct FSimResetIn& _Data);
    // static FVehicleManagerIn TranslateData(struct FSimUpdateIn& _Data);

    const UDataTable* GetVehicleTypeTable();

    class UDisplayGameInstance* GetGameInstance();
};
// bool AVehicleManager::bIsLoadTextTable = true;
// UDataTable* AVehicleManager::vehicleTypeTable = NULL;
