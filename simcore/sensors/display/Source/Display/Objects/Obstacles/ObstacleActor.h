// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../SimActorInterface.h"
#include "ObstacleInterface.h"
#include "ObstacleActor.generated.h"

UCLASS()
class DISPLAY_API AObstacleActor : public AActor, public ISimActorInterface, public IObstacleInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AObstacleActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Basic root comp
    class UBasicInfoComp* basicInfoComp = NULL;

    //// Static mesh comp
    // class UStaticMeshComponent* sMeshComp = NULL;

    //// Skeletal mesh comp
    // class USkeletalMeshComponent* skMeshComp = NULL;

    // Sim actor movement
    class USimMoveComponent* simMoveComponent = NULL;

public:
    // Get the configuration from SimActor
    virtual const FSimActorConfig* GetConfig() const;

    // Init SimActor
    virtual void Init(const FSimActorConfig& _Config);

    // Update SimActor
    virtual void Update(const FSimActorInput& _Input, FSimActorOutput& _Output);

    // Destroy the SimActor
    virtual void Destroy();

    // Get current timestamp
    virtual double GetTimeStamp() const;

    void ApplyCatalogOffset(const FVector OffSet);
};
