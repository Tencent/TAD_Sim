// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SignalLightInterface.h"
#include "Display/Objects/SimActorInterface.h"
#include "SignalLightBase.generated.h"

UCLASS()
class DISPLAY_API ASignalLightBase : public AActor, public ISimActorInterface, public ISignalLightInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASignalLightBase();

public:
    /* ~ Object Interface ~ */
    // Get the configuration from SimActor
    FORCEINLINE virtual const FSimActorConfig* GetConfig() const;
    // Init SimActor
    virtual void Init(const FSimActorConfig& _Config);
    // Update SimActor
    virtual void Update(const FSimActorInput& _Input, FSimActorOutput& _Output);
    // Destroy the SimActor
    virtual void Destroy();

    /* Signal light interface */
    virtual FColor GetColor() const;
    virtual bool SetColor(const FColor& _Color);
    virtual bool SwitchLight(bool _IsOn);
    virtual bool IsSwitchOn() const;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // int id = -1;
    FColor color = FColor::Black;
    // FVector statrLocation = FVector(0);
    // FRotator startRotator = FRotator(0);

public:
    //// Called every frame
    // virtual void Tick(float DeltaTime) override;

    // UFUNCTION(BlueprintImplementableEvent)
    //     bool BPI_SetColor(const FColor& _Color);
};
