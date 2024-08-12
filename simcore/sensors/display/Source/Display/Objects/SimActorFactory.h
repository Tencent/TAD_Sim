// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimActorInterface.h"
#include "Engine/World.h"
#include "SimActorFactory.generated.h"

class ISimActorInterface;
class UWorld;
struct FSimActorConfig;

UCLASS()
class DISPLAY_API ASimActorFactory : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASimActorFactory();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Spawn SimActor and init it.
    // static AActor* SpawnSimActor(UWorld* _World, UClass* _Class, const FSimActorConfig& _InitParam);

    template <class T = AActor>
    static T* SpawnSimActor(UWorld* _World, UClass* _Class, const FSimActorConfig& _InitParam)
    {
        if (!_World)
        {
            return NULL;
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        T* TActor = _World->SpawnActor<T>(_Class, Params);
        if (!TActor)
        {
            return NULL;
        }
        ISimActorInterface* SimActor = Cast<ISimActorInterface>(TActor);
        if (!SimActor)
        {
            return NULL;
        }
        SimActor->Init(_InitParam);
        // UE_LOG(LogTemp, Warning, TEXT("SimActor Spawn! TypeName: "));
        return TActor;
    }
};
