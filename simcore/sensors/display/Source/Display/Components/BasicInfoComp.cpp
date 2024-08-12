// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicInfoComp.h"

// Sets default values for this component's properties
UBasicInfoComp::UBasicInfoComp()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

// Called when the game starts
void UBasicInfoComp::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

bool UBasicInfoComp::Init(const FSimActorConfig& _Config)
{
    SetConfig(_Config);
    id = _Config.id;
    timeStamp = _Config.timeStamp;
    typeName = _Config.typeName;
    velocity = _Config.initVelocity;
    return true;
}

void UBasicInfoComp::Update(const FSimActorInput& _InData)
{
    if (_InData.id != id)
    {
        UE_LOG(LogTemp, Error, TEXT("Updating SimActor`s Id Does Not Match Data! Name: %s, Id: %d"), *typeName, id);
        return;
    }

    // if (_InData.typeName != typeName)
    //{
    //     UE_LOG(LogTemp, Error, TEXT("Updating SimActor`s TypeName Does Not Match Data! Name: %s, Id: %d"), *typeName,
    //     id); return;
    // }

    double DeltaTimeStampInSecond = (_InData.timeStamp - timeStamp);
    if (DeltaTimeStampInSecond <= 0.0001 && DeltaTimeStampInSecond >= -0.0001)
    {
        UE_LOG(LogTemp, Error, TEXT("Updating TimeStamp Is Repeat! Name: %s, Id: %d"), *typeName, id);
    }
    // else
    //{
    //     FVector DeltaDistanceInMeter = (this->GetComponentLocation() - transform_Last.GetLocation()) / 100.f;
    //     velocity = DeltaDistanceInMeter / DeltaTimeStampInSecond;
    // }
    // transform_Last.SetLocation(this->GetComponentLocation());

    velocity = _InData.velocity;
    timeStamp = _InData.timeStamp;

    // config->timeStamp = _InData.timeStamp;
}

//// Called every frame
// void UBasicInfoComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction*
// ThisTickFunction)
//{
//     Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//     // ...
// }
