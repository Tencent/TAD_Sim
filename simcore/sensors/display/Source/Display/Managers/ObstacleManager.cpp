// Fill out your copyright notice in the Description page of Project Settings.

#include "ObstacleManager.h"
#include "Objects/SimActorFactory.h"
#include "Misc/ConfigCacheIni.h"
#include "Data/CatalogDataSource.h"
#include "Framework/DisplayGameInstance.h"
#include "RuntimeMeshLoader.h"
#include "LoaderBPFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogObstacleManager, Log, All);

AObstacleManager::AObstacleManager()
{
    // Load vehicle template
    defaultClass = LoadClass<AObstacleActor>(
        NULL, TEXT("Blueprint'/Game/Blueprints/Obstacle/BP_Obstacle_paperbox.BP_Obstacle_paperbox'"));
    if (!defaultClass.Get())
    {
        defaultClass = AObstacleActor::StaticClass();
    }
}

void AObstacleManager::Init(const FManagerConfig& Config)
{
    const FObstacleManagerConfig* OM_Config = Cast_Data<const FObstacleManagerConfig>(Config);

    check(OM_Config);
    UDisplayGameInstance* GI = Cast<UDisplayGameInstance>(GetWorld()->GetGameInstance());
    if (!GI)
    {
        return;
    }
    for (auto& Elem : OM_Config->obstacleArry)
    {
        FObstacleConfig NewConfig = Elem;
        TSubclassOf<AObstacleActor> Class =
            GetBPResource<AObstacleActor>(NewConfig.typeName, ECatalogType::CT_Obstacle, classMap);
        if (!Class.Get())
        {
            Class = AObstacleActor::StaticClass();
            if (GI->GetRuntimeMeshLoader())
            {
                FString ModelPath =
                    GI->GetCatalogDataSource()->GetModelPathByType(ECatalogType::CT_Obstacle, NewConfig.type);
                NewConfig.CustomMesh =
                    GI->GetRuntimeMeshLoader()->LoadStaticMeshFromFBX(ModelPath, true, EMeshType::MT_OBSTACLE);
            }

            if (!NewConfig.CustomMesh)
            {
                Class = defaultClass;
            }
        }
        AObstacleActor* PC = ASimActorFactory::SpawnSimActor<AObstacleActor>(GetWorld(), Class, NewConfig);
        if (PC)
        {
            if (GI->GetCatalogDataSource())
            {
                PC->ApplyCatalogOffset(
                    GI->GetCatalogDataSource()->GetOffset(ECatalogType::CT_Obstacle, NewConfig.type));
            }
            obstacleArry.Add(PC);
        }
    }
}

void AObstacleManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    const FObstacleManagerIn* TInput = Cast_Data<const FObstacleManagerIn>(Input);
    check(TInput);

    FObstacleManagerOut* TOutput = Cast_Data<FObstacleManagerOut>(Output);
    check(TOutput);
    TOutput->obstacleArry.Empty();
    TOutput->obstacleArry.AddDefaulted(TInput->obstacleArry.Num());
    //// Update All Obstacles
    // for (size_t i = 0; i < TInput->obstacleArry.Num(); i++)
    //{
    //     bool IsAssigned = false;
    //     if (i < obstacleArry.Num() && TInput->obstacleArry[i].id == obstacleArry[i]->GetConfig()->id)//Check
    //     {
    //         obstacleArry[i]->Update(TInput->obstacleArry[i], TOutput->obstacleArry[i]);
    //         IsAssigned = true;
    //     }
    //     if (!IsAssigned)//Search
    //     {
    //         for (auto &Elem : obstacleArry)
    //         {
    //             if (Elem->GetConfig()->id == TInput->obstacleArry[i].id)
    //             {
    //                 Elem->Update(TInput->obstacleArry[i], TOutput->obstacleArry[i]);
    //                 IsAssigned = true;
    //             }
    //         }
    //     }
    //     if (!IsAssigned)//Add
    //     {
    //         FObstacleConfig ObstacleConfig;
    //         ObstacleConfig.id = TInput->obstacleArry[i].id;
    //         ObstacleConfig.startLocation = TInput->obstacleArry[i].location;
    //         ObstacleConfig.startRotation = TInput->obstacleArry[i].rotation;
    //         //ObstacleConfig.typeName = TInput->obstacleArry[i].typeName;
    //         ObstacleConfig.typeName = TEXT("obstacle.box");
    //         ObstacleConfig.timeStamp = TInput->obstacleArry[i].timeStamp;
    //         AObstacleActor* NewObstacle = ASimActorFactory::SpawnSimActor<AObstacleActor>(GetWorld(), obstacleClass,
    //         ObstacleConfig); if (NewObstacle)
    //         {
    //             obstacleArry.Add(NewObstacle);
    //         }
    //         else
    //         {
    //             UE_LOG(SimLogObstacleManager, Error, TEXT("Spawn Obstacle Failed, ID: %d"), ObstacleConfig.id);
    //         }
    //         IsAssigned = true;
    //     }
    // }

    //// Find SimActor Need To Destroy
    // TArray<int32> ToRemoveIndexArry;
    // for (size_t i = 0; i < obstacleArry.Num(); i++)
    //{
    //     if (obstacleArry[i])
    //     {
    //         if (obstacleArry[i]->GetConfig()->timeStamp != TInput->timeStamp)
    //         {
    //             obstacleArry[i]->Destroy();
    //             ToRemoveIndexArry.Add(i);
    //         }
    //     }
    //     else
    //     {
    //         ToRemoveIndexArry.Add(i);
    //     }
    // }

    //// Remove SimActor Destroyed In Array
    // for (auto &Elem : ToRemoveIndexArry)
    //{
    //     obstacleArry.RemoveAt(Elem);
    // }
    //  Update All Obstacles
    UDisplayGameInstance* GI = Cast<UDisplayGameInstance>(GetWorld()->GetGameInstance());
    if (!GI)
    {
        return;
    }

    for (size_t i = 0; i < TInput->obstacleArry.Num(); i++)
    {
        bool IsAssigned = false;
        if (i < obstacleArry.Num() && !obstacleArry[i]->IsActorBeingDestroyed() &&
            TInput->obstacleArry[i].id == obstacleArry[i]->GetConfig()->id)    // Check
        {
            obstacleArry[i]->Update(TInput->obstacleArry[i], TOutput->obstacleArry[i]);
            IsAssigned = true;
        }
        if (!IsAssigned)    // Search
        {
            for (auto& Elem : obstacleArry)
            {
                if (!Elem->IsActorBeingDestroyed() && Elem->GetConfig()->id == TInput->obstacleArry[i].id)
                {
                    Elem->Update(TInput->obstacleArry[i], TOutput->obstacleArry[i]);
                    IsAssigned = true;
                }
            }
        }
        if (!IsAssigned)    // Add
        {
            FObstacleConfig ObstacleConfig;
            ObstacleConfig.id = TInput->obstacleArry[i].id;
            ObstacleConfig.startLocation = TInput->obstacleArry[i].location;
            ObstacleConfig.startRotation = TInput->obstacleArry[i].rotation;
            ObstacleConfig.typeName = TInput->obstacleArry[i].typeName;
            ObstacleConfig.type = TInput->obstacleArry[i].type;
            ObstacleConfig.timeStamp = TInput->obstacleArry[i].timeStamp;
            /* Load BP */
            TSubclassOf<AObstacleActor> Class =
                GetBPResource<AObstacleActor>(TInput->obstacleArry[i].typeName, ECatalogType::CT_Obstacle, classMap);
            if (!Class.Get())
            {
                Class = AObstacleActor::StaticClass();
                if (GI->GetRuntimeMeshLoader())
                {
                    FString ModelPath =
                        GI->GetCatalogDataSource()->GetModelPathByType(ECatalogType::CT_Obstacle, ObstacleConfig.type);
                    ObstacleConfig.CustomMesh =
                        GI->GetRuntimeMeshLoader()->LoadStaticMeshFromFBX(ModelPath, true, EMeshType::MT_OBSTACLE);
                }
                if (!ObstacleConfig.CustomMesh)
                {
                    Class = defaultClass;
                }
            }
            AObstacleActor* NewObstacle =
                ASimActorFactory::SpawnSimActor<AObstacleActor>(GetWorld(), Class, ObstacleConfig);
            // AObstacleActor* NewObstacle = ASimActorFactory::SpawnSimActor<AObstacleActor>(GetWorld(), obstacleClass,
            // ObstacleConfig);
            if (NewObstacle)
            {
                if (GI->GetCatalogDataSource())
                {
                    NewObstacle->ApplyCatalogOffset(
                        GI->GetCatalogDataSource()->GetOffset(ECatalogType::CT_Obstacle, TInput->obstacleArry[i].type));
                }
                obstacleArry.Add(NewObstacle);
            }
            else
            {
                UE_LOG(SimLogObstacleManager, Error, TEXT("Spawn Obstacle Failed, ID: %d"), ObstacleConfig.id);
            }
            IsAssigned = true;
        }
    }

    // Find SimActor Need To Destroy
    TArray<int32> ToRemoveIndexArry;
    for (int32 i = obstacleArry.Num() - 1; i >= 0; i--)
    {
        if (obstacleArry[i] && !obstacleArry[i]->IsActorBeingDestroyed())
        {
            if (obstacleArry[i]->GetTimeStamp() != TInput->timeStamp)
            {
                obstacleArry[i]->Destroy();
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
        if (obstacleArry.IsValidIndex(Elem))
        {
            obstacleArry.RemoveAt(Elem);
        }
    }
}
