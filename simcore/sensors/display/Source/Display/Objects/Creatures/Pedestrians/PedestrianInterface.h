// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../CreatureInterface.h"
#include "PedestrianInterface.generated.h"

USTRUCT()
struct FPedestrianConfig : public FCreatureConfig
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FPedestrianInput : public FCreatureInput
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FPedestrianOutput : public FCreatureOutput
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
class UPedestrianInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API IPedestrianInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual FTransform GetSnapGroundTransform(const FVector& _Location, const FRotator& _Rotation);
};
