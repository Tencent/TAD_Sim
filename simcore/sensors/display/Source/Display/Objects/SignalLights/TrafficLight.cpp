// Fill out your copyright notice in the Description page of Project Settings.

#include "TrafficLight.h"

ATrafficLight::ATrafficLight()
{
}

const FSimActorConfig* ATrafficLight::GetConfig() const
{
    return &config;
}

void ATrafficLight::Init(const FSimActorConfig& _Config)
{
    const FTrafficLightConfig* TConfig = Cast_Sim<const FTrafficLightConfig>(_Config);
    check(TConfig);

    // config.id = TConfig->id;
    config.age = TConfig->age;
    config.color = TConfig->color;

    SetColor(color);
    SetAge(age);
}

void ATrafficLight::Update(const FSimActorInput& _Input, FSimActorOutput& _Output)
{
    const FTrafficLightIn* TrafficLightIn = Cast_Sim<const FTrafficLightIn>(_Input);
    check(TrafficLightIn);

    // if (id_EIBP == TrafficLightIn->id)
    //{
    color = TrafficLightIn->color;
    age = TrafficLightIn->age;
    //}
    // else
    //{
    // UE_LOG(LogTemp, Warning, TEXT("Id is wrong!id :%d input id: /d"), id_EIBP, TrafficLightIn->id);
    //}

    SetColor(color);
    SetAge(age);
}

void ATrafficLight::Destroy()
{
    Super::Destroy();
}

FColor ATrafficLight::GetColor() const
{
    return FColor();
}

bool ATrafficLight::SetColor(const FColor& _Color)
{
    return BPI_SetColor(_Color);
}

bool ATrafficLight::SwitchLight(bool _IsOn)
{
    return BPI_SwitchLight(_IsOn);
}

bool ATrafficLight::IsSwitchOn() const
{
    return false;
}

void ATrafficLight::SetAge(int _Age)
{
    BPI_SetAge(_Age);
}

FColor ATrafficLight::BP_GetColor() const
{
    return color;
}

int ATrafficLight::BP_GetAge() const
{
    return age;
}
