// Fill out your copyright notice in the Description page of Project Settings.

#include "CreatureManager.h"
#include "Engine/World.h"
#include "PedestrianManager.h"
#include "Objects/SimActorFactory.h"
#include "Data/CatalogDataSource.h"
#include "Framework/DisplayGameInstance.h"
#include "RuntimeMeshLoader.h"
#include "LoaderBPFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogAnimalManager, Log, All);

ACreatureManager::ACreatureManager()
{
    // Load vehicle template
    defaultClass_Animal = LoadClass<AAnimalCharacter>(
        NULL, TEXT("Blueprint'/Game/Blueprints/Creature/Animal/BP_Animal_Default.BP_Animal_Default_C'"));
    if (!defaultClass_Animal.Get())
    {
        defaultClass_Animal = AAnimalCharacter::StaticClass();
    }
}

void ACreatureManager::Init(const FManagerConfig& Config)
{
    const FCreatureManagerConfig* CM_Config = Cast_Data<const FCreatureManagerConfig>(Config);

    check(CM_Config);

    pedestrianManager = GetWorld()->SpawnActor<APedestrianManager>();
    if (pedestrianManager)
    {
        pedestrianManager->Init(CM_Config->pedestrianManager);
        UE_LOG(LogTemp, Log, TEXT("PedestrianManager Spawn!"));
    }
}

void ACreatureManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    const FCreatureManagerIn* CM_Input = Cast_Data<const FCreatureManagerIn>(Input);
    FCreatureManagerOut* CM_Output = Cast_Data<FCreatureManagerOut>(Output);
    check(CM_Input && CM_Output);
    CM_Output->animalArry.Empty();
    CM_Output->animalArry.AddDefaulted(CM_Input->animalArry.Num());
    /* Pedestrian */
    if (pedestrianManager)
    {
        pedestrianManager->Update(CM_Input->pedestrianManager, CM_Output->pedestrianManager);
    }

    /* Animal */
    // Update All Animal
    UDisplayGameInstance* GI = Cast<UDisplayGameInstance>(GetWorld()->GetGameInstance());
    if (!GI)
    {
        return;
    }
    for (size_t i = 0; i < CM_Input->animalArry.Num(); i++)
    {
        bool IsAssigned = false;
        if (i < animalArry.Num() && !animalArry[i]->IsActorBeingDestroyed() &&
            CM_Input->animalArry[i].id == animalArry[i]->GetConfig()->id)    // Check
        {
            animalArry[i]->Update(CM_Input->animalArry[i], CM_Output->animalArry[i]);
            IsAssigned = true;
        }
        if (!IsAssigned)    // Search
        {
            for (auto& Elem : animalArry)
            {
                if (!Elem->IsActorBeingDestroyed() && Elem->GetConfig()->id == CM_Input->animalArry[i].id)
                {
                    Elem->Update(CM_Input->animalArry[i], CM_Output->animalArry[i]);
                    IsAssigned = true;
                }
            }
        }
        if (!IsAssigned)    // Add
        {
            FAnimalConfig AnimalConfig;
            AnimalConfig.id = CM_Input->animalArry[i].id;
            AnimalConfig.startLocation = CM_Input->animalArry[i].location;
            AnimalConfig.startRotation = CM_Input->animalArry[i].rotation;
            AnimalConfig.typeName = CM_Input->animalArry[i].typeName;
            // AnimalConfig.typeName = TEXT("creature/pedestrian.man");
            AnimalConfig.timeStamp = CM_Input->animalArry[i].timeStamp;
            AnimalConfig.type = CM_Input->animalArry[i].type;

            TSubclassOf<AAnimalCharacter> Class = GetBPResource<AAnimalCharacter>(
                CM_Input->animalArry[i].typeName, ECatalogType::CT_Creature, classMap_Animal);
            if (!Class.Get())
            {
                Class = AAnimalCharacter::StaticClass();
                if (GI->GetRuntimeMeshLoader())
                {
                    FString ModelPath =
                        GI->GetCatalogDataSource()->GetModelPathByType(ECatalogType::CT_Creature, AnimalConfig.type);
                    AnimalConfig.CustomMesh =
                        GI->GetRuntimeMeshLoader()->LoadStaticMeshFromFBX(ModelPath, true, EMeshType::MT_CREATURE);
                }
                if (!AnimalConfig.CustomMesh)
                {
                    Class = defaultClass_Animal;
                }
            }
            AAnimalCharacter* NewAnimal =
                ASimActorFactory::SpawnSimActor<AAnimalCharacter>(GetWorld(), Class, AnimalConfig);
            if (NewAnimal)
            {
                if (GI->GetCatalogDataSource())
                {
                    NewAnimal->ApplyCatalogOffset(
                        GI->GetCatalogDataSource()->GetOffset(ECatalogType::CT_Creature, CM_Input->animalArry[i].type));
                }
                animalArry.Add(NewAnimal);
            }
            else
            {
                UE_LOG(SimLogAnimalManager, Error, TEXT("Spawn Animal Failed, ID: %d"), AnimalConfig.id);
            }
            IsAssigned = true;
        }
    }

    // Find SimActor Need To Destroy
    TArray<int32> ToRemoveIndexArry;
    for (int32 i = animalArry.Num() - 1; i >= 0; i--)
    {
        if (animalArry[i] && !animalArry[i]->IsActorBeingDestroyed())
        {
            if (animalArry[i]->GetTimeStamp() != CM_Input->timeStamp)
            {
                animalArry[i]->Destroy();
                ToRemoveIndexArry.Add(i);
            }
        }
        else
        {
            ToRemoveIndexArry.Add(i);
        }
    }

    // Remove SimActor Destroyed In Array
    for (auto& Elem : ToRemoveIndexArry)
    {
        if (animalArry.IsValidIndex(Elem))
        {
            animalArry.RemoveAt(Elem);
        }
    }
}
