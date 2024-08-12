// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "Objects/Creatures/Pedestrians/PedestrianCharacter.h"
#include "PedestrianManager.generated.h"

USTRUCT()
struct FPedestrianManagerConfig : public FManagerConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FPedestrianConfig> pedestrianArry;
};

USTRUCT()
struct FPedestrianManagerIn : public FManagerIn
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FPedestrianInput> pedestrianArry;
};

USTRUCT()
struct FPedestrianManagerOut : public FManagerOut
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FPedestrianOutput> pedestrianArry;
};

/**
 *
 */
UCLASS()
class DISPLAY_API APedestrianManager : public AManager
{
    GENERATED_BODY()

public:
    APedestrianManager();
    /* Manager Interface */
    virtual void Init(const FManagerConfig& Config);
    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

protected:
    UPROPERTY()
    TSubclassOf<APedestrianCharacter> defaultClass;

    UPROPERTY()
    TMap<FString, TSubclassOf<APedestrianCharacter> > classMap;

public:
    TArray<APedestrianCharacter*> pedestrianArry;
};
