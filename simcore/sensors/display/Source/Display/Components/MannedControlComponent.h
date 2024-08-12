// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MannedControlComponent.generated.h"

UENUM()
enum class EDriveMode : uint8
{
    Manned,
    Auto
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), config = game)
class DISPLAY_API UMannedControlComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UMannedControlComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    UPROPERTY()
    EDriveMode mode = EDriveMode::Auto;

public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY()
    class ATP_VehicleAdvPawn* pawnManned = NULL;

    UPROPERTY()
    class ATransportPawn* pawnAuto = NULL;

    UFUNCTION()
    bool SwitchToManned(class ATransportPawn* _AutoPawn);

    UFUNCTION()
    bool SwitchToAuto();

    UFUNCTION()
    EDriveMode GetMode() const
    {
        return mode;
    }

    // void SetSteeringWheel(float _Value);
    // void SetBrake(float _Value);
    // void SetGear(int32 _Value);
    // void SetThrottle(float _Value);

    UPROPERTY(config)
    FVector spawnOffset = FVector(0.f);

    class ATP_VehicleAdvPawn* GetMannedPawn()
    {
        return pawnManned;
    }
    class ATransportPawn* GetAutoPawn()
    {
        return pawnAuto;
    }
};
