// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/SignalLights/SignalLightBase.h"
#include "TrafficLight.generated.h"

USTRUCT()
struct FTrafficLightConfig : public FSignalLightConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int age = 0;
};

USTRUCT()
struct FTrafficLightIn : public FSignalLightIn
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int age = 0;

    UPROPERTY()
    TArray<FString> PhysicLightsID;
};

USTRUCT()
struct FTrafficLightOut : public FSignalLightOut
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int age = 0;
};

/**
 *
 */
UCLASS()
class DISPLAY_API ATrafficLight : public ASignalLightBase
{
    GENERATED_BODY()

    ATrafficLight();

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
    // TrafficLight data
    FTrafficLightConfig config;
    int age = -1;

    void SetAge(int _Age);

public:
    UFUNCTION(BlueprintCallable)
    FColor BP_GetColor() const;

    UFUNCTION(BlueprintCallable)
    int BP_GetAge() const;

    UFUNCTION(BlueprintImplementableEvent)
    bool BPI_SetColor(const FColor& _Color);

    UFUNCTION(BlueprintImplementableEvent)
    bool BPI_SwitchLight(bool _IsOn);

    UFUNCTION(BlueprintImplementableEvent)
    void BPI_SetAge(int _Age);

    UPROPERTY(EditAnyWhere)
    int id_EIBP = -1;
};
