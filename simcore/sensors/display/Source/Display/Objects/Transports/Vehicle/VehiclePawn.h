// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Transports/TransportPawn.h"
#include "VehicleInterface.h"
#include "Data/CatalogDataSource.h"
#include "VehiclePawn.generated.h"

/**
 *
 */
UCLASS(config = game /*, perObjectConfig*/)
class DISPLAY_API AVehiclePawn : public ATransportPawn, public IVehicleInterface
{
    GENERATED_BODY()
public:
    AVehiclePawn();

    // Overridable function called whenever this actor is being removed from a level
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
    /* ~ Simulator Interface ~ */

    // Init SimActor
    virtual void Init(const FSimActorConfig& _Config);

    // Update SimActor
    virtual void Update(const FSimActorInput& _Input, FSimActorOutput& _Output);

    // Destroy the SimActor
    virtual void Destroy();

    virtual void PossessedBy(AController* NewController);

    virtual bool SwitchDrivingMode(EDrivingMode _DrivingMode);

    void ApplyTruckCatalogOffset(const FTruckCatalogData& Data);

    FORCEINLINE bool IsUseContainer()
    {
        return bUseContainer;
    }

    FORCEINLINE FVector GetContainerOffset()
    {
        return ContainerOffset;
    };

    float GetMeshHead();

    float GetMeshEnd();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    ////virtual void SetupLights();

    // virtual void SwitchStopLamp(bool _IsOn);
    // virtual void SwitchLowBeam(bool _IsOn);
    // virtual void SwitchHighBeam(bool _IsOn);
    // virtual void SwitchFogLamp(bool _IsOn);
    // virtual void SwitchClearanceLamp(bool _IsOn);
    // virtual void SwitchLeftTurningSignalLamp(bool _IsOn);
    // virtual void SwitchRightTurningSignalLamp(bool _IsOn);
    // virtual void SwitchBackupLamp(bool _IsOn);
    // virtual void SwitchDoubleJumpLamp(bool _IsOn);

    // Light for lamp
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USpotLightComponent* lamp_HighBeam_L = nullptr;
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USpotLightComponent* lamp_HighBeam_R = nullptr;

    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class UControlDataProcessor* driving_processor = nullptr;

    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class UDriveWidget* driving_ui = nullptr;

    virtual void SetupLights();

protected:
    UPROPERTY(config)
    int32 uuid = 0;

public:
    UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USceneComponent* containerSceneComp = nullptr;

    // Skeleton container mesh component
    UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* containerMeshComp = nullptr;

    UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* RuntimeMeshComp = nullptr;

    UPROPERTY(config)
    bool bActiveDrivingUI = false;

protected:
    /* Snap Ground Var */
    UPROPERTY(config)
    bool bUseAverageManned = false;
    UPROPERTY(config)
    int32 smoothN = 6;
    // UPROPERTY(config)
    //     float maxPitch = 10;
    FRotator sumRpy = FRotator();
    TArray<FRotator> smoothRpyBuf;
    int32 curIdx = 0;
    FRotator prePhysicRpy = FRotator();
    UPROPERTY(config)
    bool bUseFixedManned = false;

    UPROPERTY(EditAnyWhere)
    bool bUseContainer = false;

    FVector ContainerOffset = FVector(0.f);

    // record last frame location
    FVector LastLocation = FVector(0.f);

    // stay time in place
    float StayRecordTime = 0;
    /* Snap Ground Var */

    // save init config
    FVehicleConfig vehicleConfig;
};
