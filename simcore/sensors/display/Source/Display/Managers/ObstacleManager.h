// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "Objects/Obstacles/ObstacleActor.h"
#include "ObstacleManager.generated.h"

USTRUCT()
struct FObstacleManagerConfig : public FManagerConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FObstacleConfig> obstacleArry;
};

USTRUCT()
struct FObstacleManagerIn : public FManagerIn
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FObstacleInput> obstacleArry;
};

USTRUCT()
struct FObstacleManagerOut : public FManagerOut
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FObstacleOutput> obstacleArry;
};

/**
 *
 */
UCLASS()
class DISPLAY_API AObstacleManager : public AManager
{
    GENERATED_BODY()
public:
    AObstacleManager();
    /* Manager Interface */
    virtual void Init(const FManagerConfig& Config);
    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

protected:
    UPROPERTY()
    TSubclassOf<AObstacleActor> defaultClass;

    UPROPERTY()
    TMap<FString, TSubclassOf<AObstacleActor> > classMap;

public:
    TArray<AObstacleActor*> obstacleArry;
};
