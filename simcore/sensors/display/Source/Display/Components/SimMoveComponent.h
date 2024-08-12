// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "SimMoveComponent.generated.h"

/**
 * Storing object`s basic information.
 * For example, location, rotation, lonlat, bounding box and so on.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API USimMoveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    USimMoveComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    /* Snap Ground Var */
    FRotator sumRpy = FRotator();
    float sumZ = 0;
    TArray<FRotator> smoothRpyBuf;
    TArray<float> smoothZBuf;
    int32 curIdx = 0;
    bool isEgoSnap = true;
    // ETrafficType trafficType = ETrafficType::ST_TRAFFIC;

    float snapRangeTop = 300;       // unit is cm
    float snapRangeBottom = 300;    // unit is cm
    /* Snap Ground Var */

public:
    // FTransform GetSnapGroundTransform(const FVector& _Location, const FRotator& _Rotation);

    bool GetSnapGroundTransform(
        FTransform& _SnappedTransform, const FVector& _Location, const FRotator& _Rotation, bool _IsForce = false);
    bool GetSnapGroundTransform(FTransform& _SnappedTransform, const FVector& _Location, const FRotator& _Rotation,
        float _RTTop, float _RTBottom, bool _UseAverage = true);
    bool GetSnapGroundTransform_NotAverage(
        FTransform& _SnappedTransform, const FVector& _Location, const FRotator& _Rotation, bool _IsForce = false);

    void MoveSimActor(AActor* _Actor, const FVector& _Location, const FRotator& _Rotation, bool _UseSnapGround);

public:
    // Called every frame
    // virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
    // override;
};
