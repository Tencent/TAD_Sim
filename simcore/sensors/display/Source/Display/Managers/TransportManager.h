// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "CoreMinimal.h"
// #include "GameFramework/Actor.h"
#include "Display/Managers/Manager.h"
#include "VehicleManager.h"
#include "TransportManager.generated.h"

// class AVehicleManager;
// struct FVehicleManagerConfig;

USTRUCT()
struct FTransportManagerConfig : public FManagerConfig
{
    GENERATED_BODY();

    UPROPERTY()
    FVehicleManagerConfig vehicleManagerConfig;
};

USTRUCT()
struct FTransportManagerIn : public FManagerIn
{
    GENERATED_BODY();

    UPROPERTY()
    FVehicleManagerIn vehicleManagerIn;
};

USTRUCT()
struct FTransportManagerOut : public FManagerOut
{
    GENERATED_BODY();

    UPROPERTY()
    FVehicleManagerOut vehicleManagerOut;
};

UCLASS()
class DISPLAY_API ATransportManager : public AManager
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATransportManager();

    AVehicleManager* vehicleManager;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    TSubclassOf<AVehicleManager> vehicleManagerClass;

public:
    virtual void Init(const FManagerConfig& Config);

    virtual void Update(const FManagerIn& Input, FManagerOut& Output);
};
