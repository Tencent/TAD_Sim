// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimalCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BasicInfoComp.h"
#include "Components/SimMoveComponent.h"
#include "Components/InputComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "Kismet/KismetMathLibrary.h"
#include "Managers/Manager.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogAnimal, Log, All);

// Sets default values
AAnimalCharacter::AAnimalCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
    // it.
    PrimaryActorTick.bCanEverTick = true;
    basicInfoComp = CreateDefaultSubobject<UBasicInfoComp>(FName(TEXT("BasicInfo")));
    RootComponent = basicInfoComp;

    simMoveComponent = CreateDefaultSubobject<USimMoveComponent>(FName(TEXT("SimMove")));

    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("SimMesh")));
    meshComp->SetupAttachment(RootComponent);
    meshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    meshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    SetReplicates(false);
}

// Called when the game starts or when spawned
void AAnimalCharacter::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AAnimalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (LastLocation.Equals(GetActorLocation()))
    {
        StayRecordTime += DeltaTime;
        if (StayRecordTime >= 0.5f)
        {
            speed = 0.f;
            basicInfoComp->velocity = FVector(0.f);
            StayRecordTime = 0;
        }
    }
    else
    {
        StayRecordTime = 0.f;
    }
    LastLocation = GetActorLocation();
}

// Called to bind functionality to input
void AAnimalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

const FSimActorConfig* AAnimalCharacter::GetConfig() const
{
    check(basicInfoComp);
    return basicInfoComp->config.Get();
}

void AAnimalCharacter::Init(const FSimActorConfig& _Config)
{
    const FAnimalConfig* AnimalConfig = Cast_Sim<const FAnimalConfig>(_Config);
    check(AnimalConfig);

    check(basicInfoComp);
    // basicInfoComp->SetConfig(*AnimalConfig);
    // basicInfoComp->id = AnimalConfig->id;
    // basicInfoComp->typeName = AnimalConfig->typeName;
    basicInfoComp->Init(_Config);

    //// Load mesh
    // FString MeshPath;
    // FString AnimPath;
    // if (GConfig->GetString(TEXT("TypeList"), *basicInfoComp->typeName, MeshPath, GGameIni))
    //{
    //     FString SuffixName;
    //     MeshPath.Split(TEXT("/"), &AnimPath, &SuffixName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
    //     FString Temp;
    //     FString MeshName;
    //     SuffixName.Split(TEXT("."), &MeshName, &Temp);
    //     AnimPath.Split(TEXT("'"), &Temp, &AnimPath);
    //     AnimPath = TEXT("AnimBlueprint'") + AnimPath;
    //     AnimPath += TEXT("/") + MeshName + TEXT("_AnimBP.") + MeshName + TEXT("_AnimBP_C'");

    //    USkeletalMesh* SMesh = LoadObject<USkeletalMesh>(this, *MeshPath);
    //    UClass* AnimBPClass = LoadClass<UAnimInstance>(NULL, *AnimPath);
    //    // TODO: Load animation blueprint
    //    if (SMesh)
    //    {
    //        meshComp->SetSkeletalMesh(SMesh);
    //        meshComp->AddRelativeRotation(FRotator(0, -90, 0));
    //    }
    //    else
    //    {
    //        UE_LOG(SimLogAnimal, Error, TEXT("Can not load mesh: %s"), *MeshPath);
    //    }
    //    if (AnimBPClass)
    //    {
    //        meshComp->SetAnimInstanceClass(AnimBPClass);
    //    }
    //    else
    //    {
    //        UE_LOG(SimLogAnimal, Error, TEXT("Can not load AnimBP: %s"), *AnimPath);
    //    }
    //}
    // else
    //{
    //    UE_LOG(SimLogAnimal, Error, TEXT("The type is not in list: %s"), *basicInfoComp->typeName);
    //}

    FTransform NewTransform;
    if (simMoveComponent)
    {
        simMoveComponent->GetSnapGroundTransform(
            NewTransform, AnimalConfig->startLocation, AnimalConfig->startRotation, true);
    }
    this->SetActorTransform(NewTransform);

    if (AnimalConfig->CustomMesh)
    {
        UStaticMeshComponent* RuntimeMeshComp = NewObject<UStaticMeshComponent>(this, TEXT("RuntimeMeshComp"));
        if (RuntimeMeshComp)
        {
            RuntimeMeshComp->SetStaticMesh(AnimalConfig->CustomMesh);
            RuntimeMeshComp->AttachToComponent(
                meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT(""));
            RuntimeMeshComp->RegisterComponent();
            meshComp->SetVisibility(false);
        }
    }

    // Register SimActor
    AManager::RegisterSimActor(this);

    UE_LOG(SimLogAnimal, Warning, TEXT("Animal Character Inited, Id: %d"), basicInfoComp->id);
}

void AAnimalCharacter::Update(const FSimActorInput& _Input, FSimActorOutput& _Output)
{
    const FAnimalInput* AnimalIn = Cast_Sim<const FAnimalInput>(_Input);
    check(AnimalIn);

    FTransform NewTransform;
    /*simMoveComponent->GetSnapGroundTransform(NewTransform, AnimalIn->location, directionRot, true);*/
    FVector RaycastStart = AnimalIn->location;
    RaycastStart.Z = this->GetActorLocation().Z;
    simMoveComponent->GetSnapGroundTransform(NewTransform, RaycastStart, AnimalIn->rotation);
    //// Caculated Direction
    // if ((NewTransform.GetLocation() - this->GetActorLocation()).Size() > 0.001)
    //{
    //     directionRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), NewTransform.GetLocation());
    // }
    // NewTransform.SetRotation(directionRot.Quaternion());
    this->SetActorTransform(NewTransform);

    // Veloctiy
    speed = AnimalIn->velocity.Size();

    // SetActorLocationAndRotation(AnimalIn->location, directionRot);

    basicInfoComp->Update(*AnimalIn);

    FAnimalOutput* AnimalOut = Cast_Sim<FAnimalOutput>(_Output);
    *(FSimActorInput*) AnimalOut = _Input;
    // if (WasRecentlyRendered(2.f))
    {
        AnimalOut->locPose = this->GetActorLocation();
        AnimalOut->rotPose = this->GetActorRotation();
        AnimalOut->bHasPose = true;
    }
}

void AAnimalCharacter::Destroy()
{
    Super::Destroy();
}

double AAnimalCharacter::GetTimeStamp() const
{
    if (basicInfoComp)
    {
        return basicInfoComp->timeStamp;
    }
    return -1;
}

float AAnimalCharacter::GetSpeed() const
{
    return basicInfoComp->velocity.Size();
}

void AAnimalCharacter::ApplyCatalogOffset(const FVector OffSet)
{
    FVector LocalOffset = OffSet;
    LocalOffset.Y = -LocalOffset.Y;
    meshComp->SetRelativeLocation(LocalOffset * 100.f);
}
