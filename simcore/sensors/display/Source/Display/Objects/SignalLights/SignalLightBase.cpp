// Fill out your copyright notice in the Description page of Project Settings.

#include "SignalLightBase.h"

// Sets default values
ASignalLightBase::ASignalLightBase()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASignalLightBase::BeginPlay()
{
    Super::BeginPlay();
}

//// Called every frame
// void ASignalLightBase::Tick(float DeltaTime)
//{
//     Super::Tick(DeltaTime);
//
// }

const FSimActorConfig* ASignalLightBase::GetConfig() const
{
    return nullptr;
}

void ASignalLightBase::Init(const FSimActorConfig& _Config)
{
}

void ASignalLightBase::Update(const FSimActorInput& _Input, FSimActorOutput& _Output)
{
}

void ASignalLightBase::Destroy()
{
    Super::Destroy();
}

FColor ASignalLightBase::GetColor() const
{
    return FColor();
}

bool ASignalLightBase::SetColor(const FColor& _Color)
{
    return false;
}

bool ASignalLightBase::SwitchLight(bool _IsOn)
{
    return false;
}

bool ASignalLightBase::IsSwitchOn() const
{
    return false;
}
