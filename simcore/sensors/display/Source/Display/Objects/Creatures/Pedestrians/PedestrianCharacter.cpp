// Fill out your copyright notice in the Description page of Project Settings.

#include "PedestrianCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BasicInfoComp.h"
#include "Components/SimMoveComponent.h"
#include "Components/CameraMasterComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "Kismet/KismetMathLibrary.h"
#include "Managers/Manager.h"
#include "GameFramework/SpringArmComponent.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogPedestrian, Log, All);

// Sets default values
APedestrianCharacter::APedestrianCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
    // it.
    PrimaryActorTick.bCanEverTick = true;

    basicInfoComp = CreateDefaultSubobject<UBasicInfoComp>(FName(TEXT("BasicInfo")));
    // RootComponent = basicInfoComp;

    simMoveComponent = CreateDefaultSubobject<USimMoveComponent>(FName(TEXT("SimMove")));
    basicInfoComp->SetupAttachment(RootComponent);
    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("SimMesh")));
    meshComp->SetupAttachment(RootComponent);
    // meshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    meshComp->SetCollisionProfileName(FName(TEXT("SimVehicle")));

    // cameraMasterComp = CreateDefaultSubobject<UCameraMasterComponent>(FName(TEXT("CameraMaster")));

    // camera_BirdView = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Camera_BirdView")));
    // camera_BirdView->SetupAttachment(meshComp, TEXT("BirdView"));

    // springArm_Free = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("SpringArm_Free")));
    // springArm_Free->SetupAttachment(meshComp, TEXT("FreeView"));
    // springArm_Free->TargetArmLength = 500;
    // springArm_Free->bDoCollisionTest = true;
    // springArm_Free->ProbeChannel = ECollisionChannel::ECC_GameTraceChannel2;

    // camera_Free = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Camera_Free")));
    // camera_Free->SetupAttachment(springArm_Free);

    SetReplicates(false);
}

// Called when the game starts or when spawned
void APedestrianCharacter::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void APedestrianCharacter::Tick(float DeltaTime)
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

void APedestrianCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // PlayerInputComponent->BindAction("SwitchCamera", IE_Released, this, &APedestrianCharacter::SwitchCamera);
}

// void APedestrianCharacter::SwitchCamera(FString _Name)
//{
//     if (cameraMasterComp)
//     {
//         cameraMasterComp->SwitchCameraByName(_Name);
//     }
//     else
//     {
//         UE_LOG(LogTemp, Warning, TEXT("Switch Camera Failed! cameraMasterComp Is Null!"))
//     }
// }
//
// void APedestrianCharacter::SwitchCamera()
//{
//     SwitchCamera(FString());
// }

FORCEINLINE const FSimActorConfig* APedestrianCharacter::GetConfig() const
{
    check(basicInfoComp);
    return basicInfoComp->config.Get();
}

void APedestrianCharacter::Init(const FSimActorConfig& _Config)
{
    const FPedestrianConfig* PedestrianConfig = Cast_Sim<const FPedestrianConfig>(_Config);
    check(PedestrianConfig);

    check(basicInfoComp);
    // basicInfoComp->SetConfig(*PedestrianConfig);
    // basicInfoComp->id = PedestrianConfig->id;
    // basicInfoComp->typeName = PedestrianConfig->typeName;
    basicInfoComp->Init(*PedestrianConfig);

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
    //        UE_LOG(SimLogPedestrian, Error, TEXT("Can not load mesh: %s"), *MeshPath);
    //    }
    //    if (AnimBPClass)
    //    {
    //        meshComp->SetAnimInstanceClass(AnimBPClass);
    //    }
    //    else
    //    {
    //        UE_LOG(SimLogPedestrian, Error, TEXT("Can not load AnimBP: %s"), *AnimPath);
    //    }
    //}
    // else
    //{
    //    UE_LOG(SimLogPedestrian, Error, TEXT("The type is not in list: %s"), *basicInfoComp->typeName);
    //}

    //// Set camera transform
    // if (camera_BirdView)
    //{
    //     camera_BirdView->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
    //     TEXT("BirdView"));
    // }
    // if (camera_FPS)
    //{
    //     camera_FPS->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
    //     TEXT("FPSView"));
    // }
    // if (camera_TPS)
    //{
    //     camera_TPS->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
    //     TEXT("TPSView"));
    // }

    FTransform NewTransform;
    if (simMoveComponent)
    {
        simMoveComponent->GetSnapGroundTransform(
            NewTransform, PedestrianConfig->startLocation, PedestrianConfig->startRotation, true);
    }
    this->SetActorTransform(NewTransform);
    // SetActorLocationAndRotation(PedestrianConfig->startLocation, PedestrianConfig->startRotation);
    if (PedestrianConfig->CustomMesh)
    {
        UStaticMeshComponent* RuntimeMeshComp = NewObject<UStaticMeshComponent>(this, TEXT("RuntimeMeshComp"));
        if (RuntimeMeshComp)
        {
            RuntimeMeshComp->SetStaticMesh(PedestrianConfig->CustomMesh);
            RuntimeMeshComp->AttachToComponent(
                meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT(""));
            RuntimeMeshComp->SetCollisionProfileName(FName(TEXT("SimVehicle")));
            RuntimeMeshComp->RegisterComponent();
            meshComp->SetVisibility(false);
        }
    }

    // Register SimActor
    AManager::RegisterSimActor(this);

    UE_LOG(SimLogPedestrian, Warning, TEXT("Pedestrian Character Inited(Id: %d, Name: %s, Loc: %s"), basicInfoComp->id,
        *basicInfoComp->typeName, *GetActorLocation().ToString());
}

void APedestrianCharacter::Update(const FSimActorInput& _Input, FSimActorOutput& _Output)
{
    const FPedestrianInput* PedestrianIn = Cast_Sim<const FPedestrianInput>(_Input);
    check(PedestrianIn);

    FTransform NewTransform;
    /*simMoveComponent->GetSnapGroundTransform(NewTransform, PedestrianIn->location, directionRot, true);*/
    FVector RaycastStart = PedestrianIn->location;
    RaycastStart.Z = this->GetActorLocation().Z;
    simMoveComponent->GetSnapGroundTransform(NewTransform, RaycastStart, PedestrianIn->rotation);
    //// Caculated Direction
    // if ((NewTransform.GetLocation() - this->GetActorLocation()).Size() > 0.1)
    //{
    //     directionRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), NewTransform.GetLocation());
    // }
    // NewTransform.SetRotation(directionRot.Quaternion());
    this->SetActorTransform(NewTransform);

    // Veloctiy
    speed = PedestrianIn->velocity.Size();

    // SetActorLocationAndRotation(PedestrianIn->location, directionRot);

    basicInfoComp->Update(*PedestrianIn);
    // basicInfoComp->timeStamp = PedestrianIn->timeStamp;
    // UE_LOG(SimLogPedestrian, Warning, TEXT("Pedestrian Character Updated(Id: %d, Loc: %s, MeshLoc: %s"),
    // basicInfoComp->id, *GetActorLocation().ToString(), *meshComp->GetComponentLocation().ToString());

    FPedestrianOutput* PedestrianOut = Cast_Sim<FPedestrianOutput>(_Output);
    *(FSimActorInput*) PedestrianOut = _Input;
    // if (WasRecentlyRendered(2.f))
    {
        PedestrianOut->locPose = this->GetActorLocation();
        PedestrianOut->rotPose = this->GetActorRotation();
        PedestrianOut->bHasPose = true;
    }
}

void APedestrianCharacter::Destroy()
{
    Super::Destroy();
}

void APedestrianCharacter::ApplyCatalogOffset(const FVector OffSet)
{
    FVector LocalOffset = OffSet;
    LocalOffset.Y = -LocalOffset.Y;
    meshComp->SetRelativeLocation(LocalOffset * 100.f);
}

// FTransform APedestrianCharacter::GetSnapGroundTransform(const FVector & _Location, const FRotator & _Rotation)
//{
//     FVector RTLocation;
//     FRotator RTRotation;
//     FVector RTStart = _Location;
//     FVector RTEnd = _Location;
//     RTStart.Z = 2000;
//     RTEnd.Z = -2000;
//     FHitResult Result;
//     //FCollisionQueryParams CQParam;
//     //FCollisionObjectQueryParams COQParam;
//     //GetWorld()->LineTraceSingleByObjectType(Result, RTStart, RTEnd, , CQParam);
//     //DrawDebugLine(GetWorld(), RTStart, RTEnd, FColor::Red, false, 0.1, 0, 10);
//     int32 smoothN = 6;
//     if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Result, RTStart, RTEnd,
//     ECollisionChannel::ECC_GameTraceChannel2))
//     {
//         RTRotation = FQuat::FindBetweenNormals(FVector(0, 0, 1), Result.ImpactNormal).Rotator();
//         RTLocation = Result.ImpactPoint;
//
//         sumRpy += RTRotation;
//         sumZ += RTLocation.Z;
//         if (smoothRpyBuf.Num() < smoothN)
//         {
//             smoothRpyBuf.Add(RTRotation);
//             smoothZBuf.Add(RTLocation.Z);
//             curIdx = 0;
//         }
//         else
//         {
//             sumRpy -= smoothRpyBuf[curIdx];
//             sumZ -= smoothZBuf[curIdx];
//             smoothRpyBuf[curIdx] = RTRotation;
//             smoothZBuf[curIdx] = RTLocation.Z;
//             curIdx = (curIdx + 1) % smoothN;
//         }
//         RTRotation = sumRpy * (1.0f / smoothRpyBuf.Num());
//         RTLocation = _Location;
//         RTLocation.Z = sumZ / smoothRpyBuf.Num();
//
//         //SetActorLocation(RTLocation);
//         //SetActorRotation(_Input.rotation);
//         //AddActorWorldRotation(RTRotation);
//
//         //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("ImpactPoint: %s"),
//         Result.ImpactPoint.ToString()));
//         //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Name: %s"),
//         *Result.Actor->GetFName().ToString()));
//         //DrawDebugLine(GetWorld(), RTStart, RTLocation, FColor::Red, false, 0.1, 0, 10);
//
//         FTransform RTTransform;
//         RTTransform.SetLocation(RTLocation);
//         RTTransform.SetRotation(RTRotation.Quaternion()*_Rotation.Quaternion());
//         return RTTransform;
//     }
//     else
//     {
//         //SetActorLocation(_Input.location);
//         //SetActorRotation(_Input.rotation);
//         FTransform RTTransform;
//         RTTransform.SetLocation(_Location);
//         RTTransform.SetRotation(_Rotation.Quaternion());
//         return RTTransform;
//     }
// }

float APedestrianCharacter::GetSpeed() const
{
    return basicInfoComp->velocity.Size();
}

double APedestrianCharacter::GetTimeStamp() const
{
    if (basicInfoComp)
    {
        return basicInfoComp->timeStamp;
    }
    return -1;
}
