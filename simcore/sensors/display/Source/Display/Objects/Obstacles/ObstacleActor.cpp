// Fill out your copyright notice in the Description page of Project Settings.

#include "ObstacleActor.h"
#include "Components/BasicInfoComp.h"
#include "Misc/ConfigCacheIni.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/Manager.h"
#include "Components/SimMoveComponent.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogObstacle, Log, All);

// Sets default values
AObstacleActor::AObstacleActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    basicInfoComp = CreateDefaultSubobject<UBasicInfoComp>(FName(TEXT("BasicInfo")));
    RootComponent = basicInfoComp;

    simMoveComponent = CreateDefaultSubobject<USimMoveComponent>(FName(TEXT("SimMove")));

    // sMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("StaticMesh")));
    // sMeshComp->SetupAttachment(RootComponent);

    // skMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("SkeletalMesh")));
    // skMeshComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AObstacleActor::BeginPlay()
{
    Super::BeginPlay();
}

FORCEINLINE const FSimActorConfig* AObstacleActor::GetConfig() const
{
    check(basicInfoComp);
    return basicInfoComp->config.Get();
}

void AObstacleActor::Init(const FSimActorConfig& _Config)
{
    const FObstacleConfig* ObstacleConfig = Cast_Sim<const FObstacleConfig>(_Config);

    check(ObstacleConfig);

    check(basicInfoComp);
    basicInfoComp->SetConfig(_Config);
    basicInfoComp->id = _Config.id;
    basicInfoComp->typeName = _Config.typeName;
    basicInfoComp->timeStamp = _Config.timeStamp;

    //// Load mesh
    // FString MeshPath;
    // if (GConfig->GetString(TEXT("TypeList"), *basicInfoComp->typeName, MeshPath, GGameIni))
    //{
    //     UStaticMesh* SMesh = LoadObject<UStaticMesh>(this, *MeshPath);
    //     if (SMesh)
    //     {
    //         sMeshComp->SetStaticMesh(SMesh);
    //     }
    //     else
    //     {
    //         USkeletalMesh* SKMesh = LoadObject<USkeletalMesh>(this, *MeshPath);
    //         if (SKMesh)
    //         {
    //             // TODO: Load animation assert
    //             skMeshComp->SetSkeletalMesh(SKMesh);
    //         }
    //         else
    //         {
    //             UE_LOG(SimLogObstacle, Error, TEXT("Can not load mesh: %s"), *MeshPath);
    //         }
    //     }
    // }
    // else
    //{
    //     UE_LOG(SimLogObstacle, Error, TEXT("The type is not in list: %s"), *basicInfoComp->typeName);
    // }

    FTransform NewTransform;
    if (simMoveComponent)
    {
        simMoveComponent->GetSnapGroundTransform(NewTransform, _Config.startLocation, _Config.startRotation, true);
    }
    this->SetActorTransform(NewTransform);
    // this->SetActorLocationAndRotation(_Config.startLocation, _Config.startRotation);

    if (ObstacleConfig->CustomMesh)
    {
        UStaticMeshComponent* RuntimeMeshComp = NewObject<UStaticMeshComponent>(this, TEXT("RuntimeMeshComp"));
        if (RuntimeMeshComp)
        {
            RuntimeMeshComp->SetStaticMesh(ObstacleConfig->CustomMesh);
            RuntimeMeshComp->AttachToComponent(
                basicInfoComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT(""));
            RuntimeMeshComp->RegisterComponent();
        }
    }

    AManager::RegisterSimActor(this);

    // UE_LOG(SimLogObstacle, Warning, TEXT("Obstacle Inited, Id: %d"), basicInfoComp->id);
    // UE_LOG(SimLogObstacle, Warning, TEXT("Obstacle Loc: %s"), *this->GetActorLocation().ToString());
}

void AObstacleActor::Update(const FSimActorInput& _Input, FSimActorOutput& _Output)
{
    const FObstacleInput* ObstacleIn = Cast_Sim<const FObstacleInput>(_Input);
    check(ObstacleIn);

    basicInfoComp->Update(*ObstacleIn);

    FObstacleOutput* ObstacleOut = Cast_Sim<FObstacleOutput>(_Output);
    *(FSimActorInput*) ObstacleOut = _Input;
    // if (WasRecentlyRendered(2.f))
    {
        ObstacleOut->locPose = this->GetActorLocation();
        ObstacleOut->rotPose = this->GetActorRotation();
        ObstacleOut->bHasPose = true;
    }
}

void AObstacleActor::Destroy()
{
    Super::Destroy();
}

double AObstacleActor::GetTimeStamp() const
{
    if (basicInfoComp)
    {
        return basicInfoComp->timeStamp;
    }
    return -1;
}

void AObstacleActor::ApplyCatalogOffset(const FVector OffSet)
{
    FVector LocalOffset = OffSet;
    LocalOffset.Y = -LocalOffset.Y;

    TArray<UActorComponent*> Components;
    GetComponents(UMeshComponent::StaticClass(), Components);
    for (auto Component : Components)
    {
        if (UMeshComponent* MC = Cast<UMeshComponent>(Component))
        {
            MC->SetRelativeLocation(LocalOffset * 100.f);
        }
    }
}
