// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Objects/Transports/Vehicle/VehicleInterface.h"
#include "ControlDataProcessor.generated.h"

// UENUM()
// enum class EDeviceType : uint8
//{
//     REALCAR,
//     LOGISDK
//
// };
//
// USTRUCT()
// struct FDeviceConfig
//{
//     GENERATED_USTRUCT_BODY();
// public:
//
// };

USTRUCT()
struct FDataInput
{
    GENERATED_USTRUCT_BODY();

public:
    // in radian
    double steering_wheel = 0.f;
    double throttle = 0.f;
    double brake = 0.f;
    int8 gear = 0;
    int8 status = 0;

    double hand_brake = 0.f;
};

USTRUCT()
struct FDataOutput
{
    GENERATED_USTRUCT_BODY();

public:
    FVector location = FVector(0);
    FRotator rotation = FRotator(0);
    double speed = 0.f;
    double acceleration = 0.f;

    double rmp = 0.f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API UControlDataProcessor : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UControlDataProcessor();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    UPROPERTY(config)
    FString device_name = TEXT("");
    FDataInput data_input;
    FDataOutput data_output;

    void RecieveData();

public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SimInit(const FVehicleConfig& _InData);
    void UpdateVehicleInfo(const FDataOutput& _Info);
    void UpdateSimData(const FVehicleIn& _InData);
    FDataInput GetInput() const
    {
        return data_input;
    }
    FDataOutput GetOutput() const
    {
        return data_output;
    }
};
