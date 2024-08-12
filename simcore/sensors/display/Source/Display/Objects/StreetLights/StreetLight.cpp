// Fill out your copyright notice in the Description page of Project Settings.

#include "StreetLight.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AStreetLight::AStreetLight()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AStreetLight::BeginPlay()
{
    Super::BeginPlay();
    bIsLightOn = true;
    SetLightOn(false);
}

// Called every frame
void AStreetLight::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AStreetLight::SetLightOn(bool _IsTurnOn)
{
    if (bIsLightOn == _IsTurnOn)
    {
        return;
    }
    for (auto& Lamp : lampArry)
    {
        // Switch LampShader
        for (size_t i = 0; i < Lamp.meshArry.Num(); i++)
        {
            if (Lamp.meshArry[i])
            {
                UMaterialInterface* Mat = Lamp.meshArry[i]->GetMaterial(Lamp.matIndexArry[i]);
                if (Mat)
                {
                    UMaterialInstanceDynamic* MatInsDynamic = UMaterialInstanceDynamic::Create(Mat, this);
                    if (MatInsDynamic)
                    {
                        MatInsDynamic->SetScalarParameterValue(FName(TEXT("LightOn")), _IsTurnOn);
                        Lamp.meshArry[i]->SetMaterial(Lamp.matIndexArry[i], MatInsDynamic);
                    }
                }
            }
        }
        // Switch Light
        for (auto& Light : Lamp.lightArry)
        {
            if (Light)
            {
                Light->SetVisibility(_IsTurnOn);
            }
        }
    }

    if (lampArry.Num() <= 0)
    {
        UE_LOG(LogTemp, Log, TEXT("StreetLight is empty, name: %s"), *this->GetName());
    }
    bIsLightOn = _IsTurnOn;
}

// void AStreetLight::ApplyAllLights()
//{
//     SetLightOn(bAllLightsOn);
// }

void AStreetLight::AddLamps(const FLamp& _Lamp)
{
    lampArry.Add(_Lamp);
}
