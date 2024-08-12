// Fill out your copyright notice in the Description page of Project Settings.

#include "PedestrianManager.h"
#include "Objects/SimActorFactory.h"
#include "Objects/Creatures/Pedestrians/PedestrianCharacter.h"
#include "Data/CatalogDataSource.h"
#include "Framework/DisplayGameInstance.h"
#include "RuntimeMeshLoader.h"
#include "LoaderBPFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogPedestrianManager, Log, All);

APedestrianManager::APedestrianManager()
{
    // Load vehicle template
    defaultClass = LoadClass<APedestrianCharacter>(
        NULL, TEXT("Blueprint'/Game/Blueprints/Creature/Pedestrian/BP_Pedestrian_Default.BP_Pedestrian_Default_C'"));
    if (!defaultClass.Get())
    {
        defaultClass = APedestrianCharacter::StaticClass();
    }
}

void APedestrianManager::Init(const FManagerConfig& Config)
{
    const FPedestrianManagerConfig* PM_Config = Cast_Data<const FPedestrianManagerConfig>(Config);

    check(PM_Config);
    for (auto& Elem : PM_Config->pedestrianArry)
    {
        FPedestrianConfig NewConfig = Elem;
        TSubclassOf<APedestrianCharacter> Class =
            GetBPResource<APedestrianCharacter>(Elem.typeName, ECatalogType::CT_Creature, classMap);
        if (!Class.Get())
        {
            Class = APedestrianCharacter::StaticClass();
        }
        APedestrianCharacter* PC = ASimActorFactory::SpawnSimActor<APedestrianCharacter>(GetWorld(), Class, Elem);
        if (PC)
        {
            if (UDisplayGameInstance* GI = Cast<UDisplayGameInstance>(GetWorld()->GetGameInstance()))
            {
                if (GI->GetRuntimeMeshLoader())
                {
                    FString ModelPath =
                        GI->GetCatalogDataSource()->GetModelPathByType(ECatalogType::CT_TrafficVehicle, Elem.type);

                    NewConfig.CustomMesh =
                        GI->GetRuntimeMeshLoader()->LoadStaticMeshFromFBX(ModelPath, true, EMeshType::MT_CREATURE);
                    if (!NewConfig.CustomMesh)
                    {
                        Class = defaultClass;
                    }
                }

                if (GI->GetCatalogDataSource())
                {
                    PC->ApplyCatalogOffset(GI->GetCatalogDataSource()->GetOffset(ECatalogType::CT_Creature, Elem.type));
                }
            }
            pedestrianArry.Add(PC);
        }
    }
}

void APedestrianManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    const FPedestrianManagerIn* TInput = Cast_Data<const FPedestrianManagerIn>(Input);
    check(TInput);

    FPedestrianManagerOut* TOutput = Cast_Data<FPedestrianManagerOut>(Output);
    check(TOutput);
    TOutput->pedestrianArry.Empty();
    TOutput->pedestrianArry.AddDefaulted(TInput->pedestrianArry.Num());
    // Update All Pedestrians
    for (size_t i = 0; i < TInput->pedestrianArry.Num(); i++)
    {
        bool IsAssigned = false;
        if (i < pedestrianArry.Num() && !pedestrianArry[i]->IsActorBeingDestroyed() &&
            TInput->pedestrianArry[i].id == pedestrianArry[i]->GetConfig()->id)    // Check
        {
            pedestrianArry[i]->Update(TInput->pedestrianArry[i], TOutput->pedestrianArry[i]);
            IsAssigned = true;
        }
        if (!IsAssigned)    // Search
        {
            for (auto& Elem : pedestrianArry)
            {
                if (!Elem->IsActorBeingDestroyed() && Elem->GetConfig()->id == TInput->pedestrianArry[i].id)
                {
                    Elem->Update(TInput->pedestrianArry[i], TOutput->pedestrianArry[i]);
                    IsAssigned = true;
                }
            }
        }
        if (!IsAssigned)    // Add
        {
            FPedestrianConfig PedestrianConfig;
            PedestrianConfig.id = TInput->pedestrianArry[i].id;
            PedestrianConfig.startLocation = TInput->pedestrianArry[i].location;
            PedestrianConfig.startRotation = TInput->pedestrianArry[i].rotation;
            PedestrianConfig.typeName = TInput->pedestrianArry[i].typeName;
            PedestrianConfig.type = TInput->pedestrianArry[i].type;
            PedestrianConfig.timeStamp = TInput->pedestrianArry[i].timeStamp;
            TSubclassOf<APedestrianCharacter> Class = GetBPResource<APedestrianCharacter>(
                TInput->pedestrianArry[i].typeName, ECatalogType::CT_Creature, classMap);

            if (UDisplayGameInstance* GI = Cast<UDisplayGameInstance>(GetWorld()->GetGameInstance()))
            {
                if (!Class.Get())
                {
                    Class = APedestrianCharacter::StaticClass();
                    if (GI->GetRuntimeMeshLoader())
                    {
                        FString ModelPath = GI->GetCatalogDataSource()->GetModelPathByType(
                            ECatalogType::CT_Creature, PedestrianConfig.type);
                        PedestrianConfig.CustomMesh =
                            GI->GetRuntimeMeshLoader()->LoadStaticMeshFromFBX(ModelPath, true, EMeshType::MT_CREATURE);
                    }
                    if (!PedestrianConfig.CustomMesh)
                    {
                        Class = defaultClass;
                    }
                }

                APedestrianCharacter* NewPedestrian =
                    ASimActorFactory::SpawnSimActor<APedestrianCharacter>(GetWorld(), Class, PedestrianConfig);
                if (NewPedestrian)
                {
                    FVector CatalogOffset = FVector(0);
                    if (PedestrianConfig.type > 100 ||
                        PedestrianConfig.CustomMesh)    // 预设行人因为动画高度改变过大，不启用catalog高度偏移
                    {
                        NewPedestrian->ApplyCatalogOffset(
                            GI->GetCatalogDataSource()->GetOffset(ECatalogType::CT_Creature, PedestrianConfig.type));
                    }

                    pedestrianArry.Add(NewPedestrian);
                }
                else
                {
                    UE_LOG(
                        SimLogPedestrianManager, Error, TEXT("Spawn Pedestrian Failed, ID: %d"), PedestrianConfig.id);
                }
                IsAssigned = true;
            }
        }
    }

    // Find SimActor Need To Destroy
    TArray<int32> ToRemoveIndexArry;
    for (int32 i = pedestrianArry.Num() - 1; i >= 0; i--)
    {
        if (pedestrianArry[i] && !pedestrianArry[i]->IsActorBeingDestroyed())
        {
            if (pedestrianArry[i]->GetTimeStamp() != TInput->timeStamp)
            {
                pedestrianArry[i]->Destroy();
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
        if (pedestrianArry.IsValidIndex(Elem))
        {
            pedestrianArry.RemoveAt(Elem);
        }
    }
}
