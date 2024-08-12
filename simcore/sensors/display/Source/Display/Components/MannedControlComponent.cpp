// Fill out your copyright notice in the Description page of Project Settings.

#include "MannedControlComponent.h"
#include "TP_VehicleAdv/TP_VehicleAdvPawn.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"
#include "Engine/World.h"
#include "Framework/DisplayPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UMannedControlComponent::UMannedControlComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;

    // ...
}

// Called when the game starts
void UMannedControlComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

// Called every frame
void UMannedControlComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Drive mode
    if (GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->GetDrivingMode() == 1)
    {
        if (GetMode() == EDriveMode::Auto)
        {
            SwitchToManned(pawnAuto);
            // begin communication
            // vehsimComponent->StartCommunication();
        }
    }
    else
    {
        if (GetMode() == EDriveMode::Manned)
        {
            SwitchToAuto();
            // close communication
            // vehsimComponent->EndCommunication();
        }
    }
}

bool UMannedControlComponent::SwitchToManned(ATransportPawn* _AutoPawn)
{
    if (!_AutoPawn)
    {
        return false;
    }

    pawnAuto = _AutoPawn;

    if (pawnManned && pawnManned->IsValidLowLevel())
    {
        pawnManned->Destroy();
        pawnManned = NULL;
    }
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    pawnManned = GetWorld()->SpawnActor<ATP_VehicleAdvPawn>(
        _AutoPawn->GetActorLocation() + spawnOffset, _AutoPawn->GetActorRotation(), Params);
    if (!pawnManned)
    {
        return false;
    }
    pawnManned->SetActorHiddenInGame(true);
    // FAttachmentTransformRules Rules = FAttachmentTransformRules::KeepWorldTransform;
    // Rules.RotationRule = EAttachmentRule::KeepRelative;
    // pawnAuto->AttachToActor(pawnManned, Rules);
    pawnAuto->SetOwner(pawnManned);
    pawnAuto->physicProxyActor = pawnManned;

    GetWorld()->GetFirstPlayerController()->Possess(pawnManned);
    GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(pawnAuto);
    pawnAuto->SwitchCamera(TEXT("Camera_Driver"));

    mode = EDriveMode::Manned;
    return true;
}

bool UMannedControlComponent::SwitchToAuto()
{
    if (!pawnManned || !pawnAuto)
    {
        return false;
    }

    // pawnAuto->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    pawnAuto->SetOwner(nullptr);
    pawnAuto->physicProxyActor = nullptr;

    GetWorld()->GetFirstPlayerController()->Possess(pawnAuto);
    GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(pawnAuto);

    pawnManned->Destroy();
    pawnManned = NULL;

    mode = EDriveMode::Auto;
    return true;
}
