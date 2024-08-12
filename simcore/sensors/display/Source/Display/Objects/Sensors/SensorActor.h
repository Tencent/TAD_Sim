// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SensorInterface.h"
#include "SensorActor.generated.h"

UCLASS()
class DISPLAY_API ASensorActor : public AActor, public ISensorInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASensorActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    AActor* InstalledActor = NULL;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

public:
    /* Sensor Interface
     */
    virtual bool Init(const FSensorConfig& _Config);

    virtual void Update(const FSensorInput& _Input, FSensorOutput& _Output);

    virtual void Destroy(FString _Reason = "");

    virtual ISimActorInterface* Install(const FSensorConfig& _Config);

    class UDisplayGameInstance* GetDisplayInstance();
};
