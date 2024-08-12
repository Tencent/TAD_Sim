// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimalInterface.h"
#include "../../SimActorInterface.h"
#include "AnimalCharacter.generated.h"

UCLASS()
class DISPLAY_API AAnimalCharacter : public ACharacter,
                                     public ISimActorInterface,
                                     public ICreatureInterface,
                                     public IAnimalInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AAnimalCharacter();

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

    /* Calculated Direction */
    FRotator directionRot = FRotator();

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Get the configuration from SimActor
    virtual const FSimActorConfig* GetConfig() const;

    // Init SimActor
    virtual void Init(const FSimActorConfig& _Config);

    // Update SimActor
    virtual void Update(const FSimActorInput& _Input, FSimActorOutput& _Output);

    // Destroy the SimActor
    virtual void Destroy();

    virtual double GetTimeStamp() const;

    void ApplyCatalogOffset(const FVector OffSet);

    UFUNCTION(BlueprintCallable)
    float GetSpeed() const;
    UPROPERTY(BlueprintReadWrite)
    float speed = 0.f;

    // record last frame location
    FVector LastLocation = FVector(0.f);

    // stay time in place
    float StayRecordTime = 0;
};
