// Fill out your copyright notice in the Description page of Project Settings.

#include "SensorActor.h"
#include "Engine/World.h"
#include "Framework/DisplayPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"
#include "Objects/Transports/Vehicle/VehicleInterface.h"
#include "Framework/DisplayGameInstance.h"

// Sets default values
ASensorActor::ASensorActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    // PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASensorActor::BeginPlay()
{
    Super::BeginPlay();
    // UE_LOG(LogTemp, Log, TEXT("Spawn Camera!"));
}

// Called every frame
void ASensorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool ASensorActor::Init(const FSensorConfig& _Config)
{
    UE_LOG(LogTemp, Log, TEXT("Init %s! Id: %d"), *_Config.typeName, _Config.id);
    return true;
}

void ASensorActor::Update(const FSensorInput& _Input, FSensorOutput& _Output)
{
}

void ASensorActor::Destroy(FString _Reason)
{
    UE_LOG(LogTemp, Log, TEXT("Destroy: %s"), *_Reason);
    Super::Destroy();
}

ISimActorInterface* ASensorActor::Install(const FSensorConfig& _Config)
{
    configBase = _Config;
    check(GetWorld());
    if (_Config.targetType != ETrafficType::ST_NONE)
    {
        ISimActorInterface* SimActor = nullptr;
        if (GetWorld() && GetWorld()->GetGameState<ADisplayGameStateBase>())
        {
            auto gamestate = GetWorld()->GetGameState<ADisplayGameStateBase>();
            if (gamestate->syncSystem && gamestate->syncSystem->transportManager.IsValid() &&
                gamestate->syncSystem->transportManager->vehicleManager)
            {
                SimActor = gamestate->syncSystem->transportManager->vehicleManager->GetVehicle(
                    _Config.targetType, _Config.targetId);
            }
        }
        if (SimActor)
        {
            AVehiclePawn* Vehicle = Cast<AVehiclePawn>(SimActor);
            if (Vehicle)
            {
                if (_Config.installSlot == TEXT("C1") && Vehicle->containerMeshComp->SkeletalMesh)
                {
                    UE_LOG(LogTemp, Log, TEXT("Install C1"));
                    this->AttachToComponent(
                        Vehicle->containerMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                    this->SetActorRelativeLocation(_Config.installLocation - Vehicle->GetContainerOffset());
                    this->SetActorRelativeRotation(_Config.installRotation);
                    // this->AttachToComponent(Vehicle->GetRootComponent(),
                    // FAttachmentTransformRules::KeepRelativeTransform);
                    this->SetOwner(Vehicle);
                    InstalledActor = Vehicle;
                    return Vehicle;
                }
            }
            AActor* Actor = Cast<AActor>(SimActor);
            if (Actor)
            {
                UE_LOG(LogTemp, Log, TEXT("Install C0"));
                this->AttachToComponent(
                    Actor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                this->SetActorRelativeLocation(_Config.installLocation);
                this->SetActorRelativeRotation(_Config.installRotation);
                this->SetOwner(Actor);
                InstalledActor = Actor;
                return SimActor;
            }
        }
    }
    else
    {
        this->SetActorLocation(_Config.installLocation);
        this->SetActorRotation(_Config.installRotation);
        InstalledActor = nullptr;
    }

    return nullptr;
}

UDisplayGameInstance* ASensorActor::GetDisplayInstance()
{
    if (!GetWorld())
    {
        return nullptr;
    }
    UGameInstance* GI = GetWorld()->GetGameInstance();
    UDisplayGameInstance* DGI = NULL;
    if (GI)
    {
        DGI = Cast<UDisplayGameInstance>(GI);
    }
    return DGI;
}
