// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraMasterComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API UCameraMasterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UCameraMasterComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    TMap<FString, UCameraComponent*> cameraMap;

    TArray<FString> cameraNameOrderArry;

    FString currentCameraName;

    void SwitchCamera(FString _CameraName);

public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UCameraComponent* CreateCameraByName_Runtime(FString _CameraName);

    bool RegisterCamera(FString _Name, UCameraComponent* _Camera);

    // void DistroyAllCamera();

    void SwitchCameraByName(FString _CameraName = FString(TEXT("")));

    UCameraComponent* GetCurrentCamera();

    FORCEINLINE FString GetCurrentCameraName();

    bool SetCurrentCamera(FString _Name);

    void SwitchCamera();
};
