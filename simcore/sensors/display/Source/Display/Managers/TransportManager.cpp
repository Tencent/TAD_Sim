// Fill out your copyright notice in the Description page of Project Settings.

#include "TransportManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogTransportManager, Log, All);

// Sets default values
ATransportManager::ATransportManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    vehicleManagerClass = AVehicleManager::StaticClass();
}

// Called when the game starts or when spawned
void ATransportManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(SimLogTransportManager, Warning, TEXT("Spawn TransportManager!"));
}

void ATransportManager::Init(const FManagerConfig& Config)
{
    const FTransportManagerConfig* TConfig = Cast_Data<const FTransportManagerConfig>(Config);
    check(TConfig);

    check(GetWorld());
    // FActorSpawnParameters Params;
    vehicleManager = GetWorld()->SpawnActor<AVehicleManager>(vehicleManagerClass);

    check(vehicleManager);
    vehicleManager->Init(TConfig->vehicleManagerConfig);
}

void ATransportManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    const FTransportManagerIn* TInput = Cast_Data<const FTransportManagerIn>(Input);
    check(TInput);
    FTransportManagerOut* TOutput = Cast_Data<FTransportManagerOut>(Output);
    check(TOutput);

    check(vehicleManager);
    vehicleManager->Update(TInput->vehicleManagerIn, TOutput->vehicleManagerOut);
}
