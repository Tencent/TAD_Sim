// Fill out your copyright notice in the Description page of Project Settings.

#include "SignalLightManager.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "XmlParser.h"

ASignalLightManager::ASignalLightManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

void ASignalLightManager::Init(const FManagerConfig& _Config)
{
    const FSignalLightManagerConfig* TConfig = Cast_Data<const FSignalLightManagerConfig>(_Config);
    check(TConfig);

    // ReadSceneTrafficFile(TConfig->SceneTrafficPath);

    // Get all signallight actors in scene;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATrafficLight::StaticClass(), FoundActors);
    for (auto& Elem : FoundActors)
    {
        ATrafficLight* NewATrafficLight = Cast<ATrafficLight>(Elem);
        if (NewATrafficLight)
        {
            if (NewATrafficLight->id_EIBP == -1)
            {
                NewATrafficLight->id_EIBP = FCString::Atoi(*(NewATrafficLight->GetName()));
            }
            trafficlightArry.Add(
                TPair<FName, ATrafficLight*>(FName(*FString::FromInt(NewATrafficLight->id_EIBP)), NewATrafficLight));
            FTrafficLightConfig Config;
            NewATrafficLight->Init(Config);
        }
    }

    // for (auto &Elem : trafficlightArry)
    //{
    //     FTrafficLightConfig Config;
    //     Elem->Init(Config);
    // }
}

void ASignalLightManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    const FSignalLightManagerIn* In = Cast_Data<const FSignalLightManagerIn>(Input);
    check(In);

    FSignalLightManagerOut Out;

    // FSignalLightGroup* CurrentPlan = SignalLightPlan.Find(CurrentPlanIndex);
    if (In)
    {
        for (auto& InputElem : In->trafficlightArry)
        {
            FName ID = FName(*FString::FromInt(InputElem.id));
            if (!trafficlightMapping.Contains(ID))
            {
                if (InputElem.PhysicLightsID.Num() > 0)
                {
                    trafficlightMapping.Add(ID, InputElem.PhysicLightsID);
                }
                else
                {
                    TArray<FString> PhysicIDs;
                    PhysicIDs.Add(FString::FromInt(InputElem.id));
                    trafficlightMapping.Add(ID, PhysicIDs);
                }
            }

            if (TArray<FString>* PhysicIDList = trafficlightMapping.Find(ID))
            {
                bool IsMatch = false;
                for (auto& PhysicID : *PhysicIDList)
                {
                    if (ATrafficLight** TrafficLight = trafficlightArry.Find(FName(*PhysicID)))
                    {
                        FTrafficLightOut SLOut;
                        (*TrafficLight)->Update(InputElem, SLOut);
                        Out.trafficlightArry.Add(SLOut);
                        IsMatch = true;
                    }
                }
                if (!IsMatch)
                {
                    UE_LOG(LogTemp, Error, TEXT("TrafficLight Id %d can`t find!"), InputElem.id);
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("trafficlightMapping %s can`t find!"), InputElem.id);
            }
        }
    }

    Output = Out;
}

void ASignalLightManager::BeginPlay()
{
    Super::BeginPlay();
}

void ASignalLightManager::ReadSceneTrafficFile(const FString& SceneTrafficPath)
{
    FXmlFile SceneTrafficFile(SceneTrafficPath);
    if (SceneTrafficFile.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Got SceneTraffic xml"));
        FXmlNode* Traffic = SceneTrafficFile.GetRootNode();
        if (Traffic)
        {
            FXmlNode* signlights = Traffic->FindChildNode(TEXT("signlights"));
            CurrentPlanIndex = FCString::Atoi(*signlights->GetAttribute(TEXT("activePlan")));

            if (signlights)
            {
                for (auto& Elem : signlights->GetChildrenNodes())
                {
                    int32 Plan = FCString::Atoi(*Elem->GetAttribute(TEXT("Plan")));
                    FString LogicID = Elem->GetAttribute(TEXT("id"));
                    FString SignalHead = Elem->GetAttribute(TEXT("signalHead"));
                    TArray<FString> PhysicIDList;
                    if (PhysicIDList.Num() > 0)
                    {
                        SignalHead.ParseIntoArray(PhysicIDList, TEXT(","), true);
                        SignalLightPlan.FindOrAdd(Plan).trafficlightMapping.Add(
                            TPair<FName, TArray<FString>>(FName(*LogicID), PhysicIDList));
                    }
                    else
                    {
                        UE_LOG(
                            LogTemp, Error, TEXT("ReadSceneTrafficFile PhysicIDList is null, logic id : %s"), *LogicID);
                    }
                }
            }
        }
    }
}
