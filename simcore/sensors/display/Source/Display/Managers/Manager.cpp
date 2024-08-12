// Fill out your copyright notice in the Description page of Project Settings.

#include "Manager.h"
#include "Framework/DisplayGameModeBase.h"
#include "Engine/EngineTypes.h"
#include "Objects/SimActorInterface.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogManagerDebug, Log, All);

TArray<ISimActorInterface*> AManager::registeredSimActorArry;

// Sets default values
AManager::AManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    // SetReplicates(true);
}

// Called when the game starts or when spawned
void AManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogManagerDebug, Log, TEXT("Manager spawn!"));
}

// void AManager::UpdateSimActorArry(TArray<ISimActorInterface*> _SimActorArray, const FSimActorInput & _Input,
// FSimActorOutput & _Output)
//{
//     // Update All Pedestrians
//     for (size_t i = 0; i < TInput->pedestrianArry.Num(); i++)
//     {
//         bool IsAssigned = false;
//         if (i < pedestrianArry.Num() && TInput->pedestrianArry[i].id == pedestrianArry[i]->GetConfig()->id)//Check
//         {
//             pedestrianArry[i]->Update(TInput->pedestrianArry[i], TOutput->pedestrianArry[i]);
//             IsAssigned = true;
//         }
//         if (!IsAssigned)//Search
//         {
//             for (auto &Elem : pedestrianArry)
//             {
//                 if (Elem->GetConfig()->id == TInput->pedestrianArry[i].id)
//                 {
//                     Elem->Update(TInput->pedestrianArry[i], TOutput->pedestrianArry[i]);
//                     IsAssigned = true;
//                 }
//             }
//         }
//         if (!IsAssigned)//Add
//         {
//             FPedestrianConfig PedestrianConfig;
//             PedestrianConfig.id = TInput->pedestrianArry[i].id;
//             PedestrianConfig.startLocation = TInput->pedestrianArry[i].location;
//             PedestrianConfig.startRotation = TInput->pedestrianArry[i].rotation;
//             PedestrianConfig.typeName = TInput->pedestrianArry[i].typeName;
//             PedestrianConfig.timeStamp = TInput->pedestrianArry[i].timeStamp;
//             APedestrianCharacter* NewPedestrian = ASimActorFactory::SpawnSimActor<APedestrianCharacter>(GetWorld(),
//             pedestrianClass, PedestrianConfig); if (NewPedestrian)
//             {
//                 pedestrianArry.Add(NewPedestrian);
//             }
//             else
//             {
//                 UE_LOG(SimLogPedestrianManager, Error, TEXT("Spawn Pedestrian Failed, ID: %d"), PedestrianConfig.id);
//             }
//             IsAssigned = true;
//         }
//     }
//
//     // Find SimActor Need To Destroy
//     TArray<int32> ToRemoveIndexArry;
//     for (size_t i = 0; i < pedestrianArry.Num(); i++)
//     {
//         if (pedestrianArry[i])
//         {
//             if (pedestrianArry[i]->GetConfig()->timeStamp != TInput->timeStamp)
//             {
//                 pedestrianArry[i]->Destroy();
//                 ToRemoveIndexArry.Add(i);
//             }
//         }
//         else
//         {
//             ToRemoveIndexArry.Add(i);
//         }
//     }
//
//     // Remove SimActor Destroyed In Array
//     for (auto &Elem : ToRemoveIndexArry)
//     {
//         pedestrianArry.RemoveAt(Elem);
//     }
// }

void AManager::Init(const FManagerConfig& Config)
{
}

void AManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
}

void AManager::Output(const FSimActorOutput& Output)
{
}

bool AManager::RegisterSimActor(ISimActorInterface* _Actor)
{
    if (!_Actor)
    {
        return false;
    }

    if (registeredSimActorArry.Contains(_Actor))
    {
        return false;
    }
    else
    {
        registeredSimActorArry.Add(_Actor);
        return true;
    }
}
