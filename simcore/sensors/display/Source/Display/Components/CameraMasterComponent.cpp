// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraMasterComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Framework/DisplayPlayerController.h"

// Sets default values for this component's properties
UCameraMasterComponent::UCameraMasterComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

// Called when the game starts
void UCameraMasterComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

void UCameraMasterComponent::SwitchCamera(FString _CameraName)
{
    UCameraComponent** TarCameraPtr = cameraMap.Find(_CameraName);
    if (TarCameraPtr && *TarCameraPtr)
    {
        // Set all camera disactive except target camera.
        for (auto& Elem : cameraMap)
        {
            if (Elem.Key == _CameraName)
            {
                Elem.Value->SetActive(true);
            }
            else
            {
                if (Elem.Value)
                {
                    Elem.Value->SetActive(false);
                }
            }
        }
        // Call PC change view target.
        check(GetWorld());
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->SetViewTargetWithBlend((*TarCameraPtr)->GetOwner());
            if (ADisplayPlayerController* DisplayPC = Cast<ADisplayPlayerController>(PC))
            {
                DisplayPC->OnEgoViewChange.Broadcast(FName(_CameraName));
            }
        }
        currentCameraName = _CameraName;
    }
}

// Called every frame
void UCameraMasterComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

UCameraComponent* UCameraMasterComponent::CreateCameraByName_Runtime(FString _CameraName)
{
    if (!this->GetOwner())
    {
        return NULL;
    }

    UCameraComponent* NewCamera = NewObject<UCameraComponent>(this->GetOwner(), FName(*_CameraName));
    if (NewCamera)
    {
        NewCamera->RegisterComponent();
        cameraMap.Add(_CameraName, NewCamera);
        cameraNameOrderArry.Add(_CameraName);
        return NewCamera;
    }
    return NULL;
}

bool UCameraMasterComponent::RegisterCamera(FString _Name, UCameraComponent* _Camera)
{
    if (!_Camera)
    {
        return false;
    }
    cameraMap.Add(_Name, _Camera);
    cameraNameOrderArry.Add(_Name);
    return true;
}

void UCameraMasterComponent::SwitchCameraByName(FString _CameraName /* = FString(TEXT(""))*/)
{
    if (_CameraName.IsEmpty())
    {
        // UE_LOG(LogTemp, Warning, TEXT("CurrentCameraName Is Null!"));
        if (currentCameraName.IsEmpty())
        {
            if (cameraNameOrderArry.Num() > 0)
            {
                SwitchCamera(cameraNameOrderArry[0]);
            }
        }
        else
        {
            size_t NextIndex = 0;
            for (size_t i = 0; i < cameraNameOrderArry.Num(); i++)
            {
                if (cameraNameOrderArry[i] == currentCameraName)
                {
                    NextIndex = (i + 1) % cameraNameOrderArry.Num();
                }
            }
            SwitchCamera(cameraNameOrderArry[NextIndex]);
        }
    }
    else
    {
        SwitchCamera(_CameraName);
    }
}

UCameraComponent* UCameraMasterComponent::GetCurrentCamera()
{
    UCameraComponent** Camera = cameraMap.Find(currentCameraName);
    if (Camera)
    {
        return *Camera;
    }
    return NULL;
}

FString UCameraMasterComponent::GetCurrentCameraName()
{
    return currentCameraName;
}

bool UCameraMasterComponent::SetCurrentCamera(FString _Name)
{
    UCameraComponent** TarCameraPtr = cameraMap.Find(_Name);
    if (TarCameraPtr && *TarCameraPtr)
    {
        (*TarCameraPtr)->SetActive(true);
        // Set all camera disactive except target camera.
        for (auto& Elem : cameraMap)
        {
            if (Elem.Key == _Name)
            {
                Elem.Value->SetActive(true);
            }
            else
            {
                if (Elem.Value)
                {
                    Elem.Value->SetActive(false);
                }
            }
        }
        currentCameraName = _Name;
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("Cant Find Camera Name: %s!"), *_Name);
    return false;
}

void UCameraMasterComponent::SwitchCamera()
{
    SwitchCameraByName(FString());
}
