// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../CreatureInterface.h"
#include "AnimalInterface.generated.h"

USTRUCT()
struct FAnimalConfig : public FCreatureConfig
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FAnimalInput : public FCreatureInput
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FAnimalOutput : public FCreatureOutput
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
class UAnimalInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API IAnimalInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
