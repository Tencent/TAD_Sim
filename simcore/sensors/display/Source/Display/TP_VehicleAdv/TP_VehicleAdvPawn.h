// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "TP_VehicleAdvPawn.generated.h"

class UPhysicalMaterial;
class UCameraComponent;
class USpringArmComponent;
class UTextRenderComponent;
class UInputComponent;
class UAudioComponent;

UCLASS(config = Game)
class ATP_VehicleAdvPawn : public AWheeledVehicle
{
    GENERATED_BODY()

    /** Spring arm that will offset the camera */
    UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* SpringArm;

    /** Camera component that will be our viewpoint */
    UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* Camera;

    /** SCene component for the In-Car view origin */
    UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class USceneComponent* InternalCameraBase;

    /** Camera component for the In-Car view */
    UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* InternalCamera;

    /** Text component for the In-Car speed */
    UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UTextRenderComponent* InCarSpeed;

    /** Text component for the In-Car gear */
    UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UTextRenderComponent* InCarGear;

    /** Audio component for the engine sound */
    UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UAudioComponent* EngineSoundComponent;

public:
    ATP_VehicleAdvPawn();

    /** The current speed as a string eg 10 km/h */
    UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
    FText SpeedDisplayString;

    /** The current gear as a string (R,N, 1,2 etc) */
    UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
    FText GearDisplayString;

    UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
    /** The color of the incar gear text in forward gears */
    FColor GearDisplayColor;

    /** The color of the incar gear text when in reverse */
    UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
    FColor GearDisplayReverseColor;

    /** Are we using incar camera */
    UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
    bool bInCarCameraActive;

    /** Are we in reverse gear */
    UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
    bool bInReverseGear;

    /** Initial offset of incar camera */
    FVector InternalCameraOrigin;

    // Begin Pawn interface
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
    // End Pawn interface

    // Begin Actor interface
    virtual void Tick(float Delta) override;

protected:
    virtual void BeginPlay() override;

public:
    // End Actor interface

    /** Handle pressing forwards */
    void MoveForward(float Val);

    /** Setup the strings used on the hud */
    void SetupInCarHUD();

    /** Update the physics material used by the vehicle mesh */
    void UpdatePhysicsMaterial();

    /** Handle pressing right */
    void MoveRight(float Val);
    /** Handle handbrake pressed */
    void OnHandbrakePressed();
    /** Handle handbrake released */
    void OnHandbrakeReleased();
    /** Switch between cameras */
    void OnToggleCamera();
    /** Handle reset VR device */
    void OnResetVR();

    static const FName LookUpBinding;
    static const FName LookRightBinding;
    static const FName EngineAudioRPM;

private:
    /**
     * Activate In-Car camera. Enable camera and sets visibility of incar hud display
     *
     * @param    bState true will enable in car view and set visibility of various
     */
    void EnableIncarView(const bool bState);

    /** Update the gear and speed strings */
    void UpdateHUDStrings();

    /* Are we on a 'slippery' surface */
    bool bIsLowFriction;
    /** Slippery Material instance */
    UPhysicalMaterial* SlipperyMaterial;
    /** Non Slippery Material instance */
    UPhysicalMaterial* NonSlipperyMaterial;

public:
    /** Returns SpringArm subobject **/
    FORCEINLINE USpringArmComponent* GetSpringArm() const
    {
        return SpringArm;
    }
    /** Returns Camera subobject **/
    FORCEINLINE UCameraComponent* GetCamera() const
    {
        return Camera;
    }
    /** Returns InternalCamera subobject **/
    FORCEINLINE UCameraComponent* GetInternalCamera() const
    {
        return InternalCamera;
    }
    /** Returns InCarSpeed subobject **/
    FORCEINLINE UTextRenderComponent* GetInCarSpeed() const
    {
        return InCarSpeed;
    }
    /** Returns InCarGear subobject **/
    FORCEINLINE UTextRenderComponent* GetInCarGear() const
    {
        return InCarGear;
    }
    /** Returns EngineSoundComponent subobject **/
    FORCEINLINE UAudioComponent* GetEngineSoundComponent() const
    {
        return EngineSoundComponent;
    }

    //// Custom : UDP component
    // UPROPERTY()
    //     class UVehicleSimUDPComponent* vehsimUDP = NULL;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    // UPROPERTY(config)
    //     bool bUseRemoteMannedControl = false;
    UPROPERTY(config)
    float weightThrottle = 1.0;
    UPROPERTY(config)
    float weightBrake = 1.0;
    UPROPERTY(config)
    float weightSteeringWheel = 1.0;

    UPROPERTY(config)
    float weightScale = 2.5;

    UPROPERTY()
    FVector velocity_Sim;

    UPROPERTY()
    FVector acceleration_Sim;    // cm/s^2

private:
    UPROPERTY()
    FVector lastLoc;

    UPROPERTY(config)
    float longSlipThreshold = 5;
    UPROPERTY(config)
    float latSlipThreshold = 5;
};
