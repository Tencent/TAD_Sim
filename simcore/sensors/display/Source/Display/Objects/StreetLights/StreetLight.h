// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StreetLight.generated.h"

USTRUCT(BlueprintType)
struct FLamp
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    TArray<class UStaticMeshComponent*> meshArry;
    // UPROPERTY(BlueprintReadWrite)
    //     TArray<class UMaterialInterface*> mat;
    UPROPERTY(BlueprintReadWrite)
    TArray<int> matIndexArry;
    UPROPERTY(BlueprintReadWrite)
    TArray<class UPointLightComponent*> lightArry;
};

UCLASS()
class DISPLAY_API AStreetLight : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AStreetLight();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    UPROPERTY()
    bool bIsLightOn = false;
    UPROPERTY()
    TArray<FLamp> lampArry;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void SetLightOn(bool _IsTurnOn);

    // void ApplyAllLights();

    UFUNCTION(BlueprintCallable)
    void AddLamps(const FLamp& _Lamp);

    // static bool bAllLightsOn;
};
// bool AStreetLight::bAllLightsOn = false;
