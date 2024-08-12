// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../CreatureInterface.h"
#include "PedestrianInterface.h"
#include "../../SimActorInterface.h"
#include "PedestrianCharacter.generated.h"

class UBasicInfoComp;
class USkeletalMeshComponent;

UCLASS()
class DISPLAY_API APedestrianCharacter : public ACharacter,
                                         public ISimActorInterface,
                                         public ICreatureInterface,
                                         public IPedestrianInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    APedestrianCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Basic root comp
    class UBasicInfoComp* basicInfoComp = NULL;

    // Sim move component
    class USimMoveComponent* simMoveComponent = NULL;

    // Skeleton mesh comp
    UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* meshComp = NULL;

    //// Camera comp
    // class UCameraMasterComponent* cameraMasterComp = NULL;

    //// Camera for view
    // class UCameraComponent* camera_BirdView = NULL;
    // class UCameraComponent* camera_FPS = NULL;
    // class UCameraComponent* camera_TPS = NULL;
    // class UCameraComponent* camera_Free = NULL;
    //// arm for free camera
    // class USpringArmComponent* springArm_Free = NULL;

    /* Snap Ground Var */
    FRotator sumRpy = FRotator();
    float sumZ = 0;
    TArray<FRotator> smoothRpyBuf;
    TArray<float> smoothZBuf;
    int32 curIdx = 0;
    /* Snap Ground Var */

    /* Calculated Direction */
    FRotator directionRot = FRotator();

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // virtual void SwitchCamera(FString _Name);

    // void SwitchCamera();
public:
    // Get the configuration from SimActor
    virtual const FSimActorConfig* GetConfig() const;

    // Init SimActor
    virtual void Init(const FSimActorConfig& _Config);

    // Update SimActor
    virtual void Update(const FSimActorInput& _Input, FSimActorOutput& _Output);

    // Destroy the SimActor
    virtual void Destroy();

    void ApplyCatalogOffset(const FVector OffSet);

    // virtual FTransform GetSnapGroundTransform(const FVector& _Location, const FRotator& _Rotation);

    UFUNCTION(BlueprintCallable)
    float GetSpeed() const;
    UPROPERTY(BlueprintReadWrite)
    float speed = 0.f;

    // record last frame location
    FVector LastLocation = FVector(0.f);

    // stay time in place
    float StayRecordTime = 0;
    virtual double GetTimeStamp() const;
};
