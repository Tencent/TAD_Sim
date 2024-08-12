// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../SimActorInterface.h"
#include "Objects/Transports/TransportInterface.h"
#include "ObstacleInterface.generated.h"

USTRUCT()
struct FObstacleConfig : public FTransportConfig
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FObstacleInput : public FSimActorInput
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FObstacleOutput : public FSimActorOutput
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FVector locPose;
    UPROPERTY()
    FRotator rotPose;
    UPROPERTY()
    bool bHasPose = false;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UObstacleInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API IObstacleInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
