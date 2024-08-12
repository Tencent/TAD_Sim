// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "Objects/SignalLights/TrafficLight.h"
#include "SignalLightManager.generated.h"

USTRUCT()
struct FSignalLightManagerConfig : public FManagerConfig
{
    GENERATED_BODY()
public:
    TArray<FTrafficLightConfig> trafficlightArry;

    FString SceneTrafficPath;
};

USTRUCT()
struct FSignalLightManagerIn : public FManagerIn
{
    GENERATED_BODY()
public:
    TArray<FTrafficLightIn> trafficlightArry;
};

USTRUCT()
struct FSignalLightManagerOut : public FManagerOut
{
    GENERATED_BODY()
public:
    TArray<FTrafficLightOut> trafficlightArry;
};

struct FSignalLightGroup
{
public:
    // 逻辑灯与物理灯组的映射关系
    TMap<FName, TArray<FString>> trafficlightMapping;
};

/**
 *
 */
UCLASS()
class DISPLAY_API ASignalLightManager : public AManager
{
    GENERATED_BODY()
    ASignalLightManager();

public:
    // SimManager Interface
    virtual void Init(const FManagerConfig& Config);
    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    void ReadSceneTrafficFile(const FString& SceneTrafficPath);

    TMap<FName, ATrafficLight*> trafficlightArry;
    TMap<int32, FSignalLightGroup> SignalLightPlan;
    TMap<FName, TArray<FString>> trafficlightMapping;
    int32 CurrentPlanIndex;
};
