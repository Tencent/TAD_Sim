// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "PedestrianManager.h"
#include "Objects/Creatures/Animals/AnimalInterface.h"
#include "Objects/Creatures/Animals/AnimalCharacter.h"
#include "CreatureManager.generated.h"

USTRUCT()
struct FCreatureManagerConfig : public FManagerConfig
{
    GENERATED_BODY()
public:
    FPedestrianManagerConfig pedestrianManager;
    TArray<FAnimalConfig> animalArry;
};

USTRUCT()
struct FCreatureManagerIn : public FManagerIn
{
    GENERATED_BODY()
public:
    FPedestrianManagerIn pedestrianManager;
    TArray<FAnimalInput> animalArry;
};

USTRUCT()
struct FCreatureManagerOut : public FManagerOut
{
    GENERATED_BODY()
public:
    FPedestrianManagerOut pedestrianManager;
    TArray<FAnimalOutput> animalArry;
};

/**
 *
 */
UCLASS()
class DISPLAY_API ACreatureManager : public AManager
{
    GENERATED_BODY()
public:
    ACreatureManager();

public:
    virtual void Init(const FManagerConfig& Config);

    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

public:
    APedestrianManager* pedestrianManager = NULL;

    TArray<AAnimalCharacter*> animalArry;

protected:
    UPROPERTY()
    TMap<FString, TSubclassOf<AAnimalCharacter> > classMap_Animal;

    UPROPERTY()
    TSubclassOf<AAnimalCharacter> defaultClass_Animal;
};
