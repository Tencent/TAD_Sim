// Fill out your copyright notice in the Description page of Project Settings.

#include "TransportPawn.h"
#include "Display/Components/BasicInfoComp.h"
#include "Components/SimMoveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/LightMasterComp.h"
#include "Components/CameraMasterComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Framework/DisplayPlayerController.h"
#include "Utils/DataFunctionLibrary.h"
#include "Components/MannedControlComponent.h"
#include "TP_VehicleAdv/TP_VehicleAdvPawn.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/TrajectoryComponent.h"
#include "Components/SplineComponent.h"

// Sets default values
ATransportPawn::ATransportPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    basicInfoComp = CreateDefaultSubobject<UBasicInfoComp>(FName(TEXT("BasicInfo")));
    RootComponent = basicInfoComp;

    simMoveComponent = CreateDefaultSubobject<USimMoveComponent>(FName(TEXT("SimMove")));

    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("Mesh")));
    meshComp->SetupAttachment(RootComponent);
    meshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    // meshComp->SetAnimInstanceClass();
    meshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    meshComp->SetCollisionProfileName(FName(TEXT("SimVehicle")));

    lightMasterComp = CreateDefaultSubobject<ULightMasterComp>(FName(TEXT("LightMaster")));

    cameraMasterComp = CreateDefaultSubobject<UCameraMasterComponent>(FName(TEXT("CameraMaster")));

    springArm_Bird = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("SpringArm_Bird")));
    springArm_Bird->SetupAttachment(meshComp, TEXT("FreeView"));
    springArm_Bird->SetRelativeRotation(FRotator(-89.f, 0.f, 0.f));
    springArm_Bird->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
    springArm_Bird->TargetArmLength = 2300;
    springArm_Bird->bDoCollisionTest = false;
    springArm_Bird->ProbeChannel = ECollisionChannel::ECC_GameTraceChannel2;

    camera_BirdView = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Camera_BirdView")));
    camera_BirdView->SetupAttachment(springArm_Bird);

    camera_Driver = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Camera_Driver")));
    camera_Driver->SetupAttachment(meshComp, TEXT("DriverView"));

    camera_Roof = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Camera_Roof")));
    camera_Roof->SetupAttachment(meshComp, TEXT("RoofView"));

    springArm_Free = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("SpringArm_Free")));
    // FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,
    // EAttachmentRule::KeepWorld, false); springArm_Free->AttachToComponent(meshComp,
    // AttachRules)/*->SetupAttachment(meshComp, TEXT("FreeView"))*/;
    springArm_Free->SetupAttachment(meshComp, TEXT("FreeView"));
    springArm_Free->TargetArmLength = 500;
    springArm_Free->bDoCollisionTest = false;
    springArm_Free->ProbeChannel = ECollisionChannel::ECC_GameTraceChannel2;

    camera_Free = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Camera_Free")));
    camera_Free->SetupAttachment(springArm_Free);

    mannedControlComponent = CreateDefaultSubobject<UMannedControlComponent>(FName(TEXT("MannedControl")));

    trajectoryComponent = CreateDefaultSubobject<UTrajectoryComponent>(FName(TEXT("Trajectory")));

    SetReplicates(false);
}

void ATransportPawn::ApplyCatalogOffset(const FVector OffSet)
{
    if (bDisableCatalog)
    {
        return;
    }

    FVector LocalOffset = OffSet;
    LocalOffset.Y = -LocalOffset.Y;
    meshComp->SetRelativeLocation(LocalOffset * 100.f);
}

FVector ATransportPawn::BP_GetVelocity() const
{
    return basicInfoComp ? basicInfoComp->velocity : FVector();
}

float ATransportPawn::BP_GetTimeStamp() const
{
    return basicInfoComp ? basicInfoComp->timeStamp : 0;
}

const FSimActorConfig* ATransportPawn::GetConfig() const
{
    check(basicInfoComp);
    return basicInfoComp->config.Get();
}

void ATransportPawn::Init(const FSimActorConfig& _Config)
{
    const FTransportConfig* TConfig = Cast_Sim<const FTransportConfig>(_Config);
    check(TConfig);

    isEgoSnap = TConfig->isEgoSnap;
    trafficType = TConfig->trafficType;
    FTransform NewTransform;
    if (isEgoSnap)
    {
        if (simMoveComponent)
        {
            simMoveComponent->GetSnapGroundTransform(
                NewTransform, TConfig->startLocation, TConfig->startRotation, true);
        }
    }
    else
    {
        NewTransform = FTransform(TConfig->startRotation, TConfig->startLocation);
    }
    SetActorTransform(NewTransform);
    UE_LOG(LogTemp, Warning, TEXT("Transport init Loc: %s(Name: %s)"), *this->GetActorLocation().ToString(),
        *TConfig->typeName);

    basicInfoComp->Init(*TConfig);

    if (true /*TConfig->trafficType == ETrafficType::ST_Ego*/)
    {
        // SetDefaultCamera(defaultCameraName);
    }

    trajectoryComponent->SetRenderActive(bRenderTrajectory);
}

void ATransportPawn::Update(const FSimActorInput& _Input, FSimActorOutput& _Output)
{
    const FTransportIn* TransportIn = Cast_Sim<const FTransportIn>(_Input);
    FTransportOut* TransportOut = Cast_Sim<FTransportOut>(_Output);
    check(TransportIn);

    // Drive mode
    if (GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->id_controlled == GetConfig()->id &&
        GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->GetDrivingMode() == 1)
    {
        if (mannedControlComponent && mannedControlComponent->GetMode() == EDriveMode::Auto)
        {
            // mannedControlComponent->SwitchToManned(this);
            //  begin communication
            // vehsimComponent->StartCommunication();
            SwitchDrivingMode(EDrivingMode::DM_MANNED);
        }
    }
    else
    {
        if (mannedControlComponent && mannedControlComponent->GetMode() == EDriveMode::Manned)
        {
            // mannedControlComponent->SwitchToAuto();
            //  close communication
            // vehsimComponent->EndCommunication();
            SwitchDrivingMode(EDrivingMode::DM_AUTOPILOT);
        }

        FTransform NewTransform;
        if (isEgoSnap)
        {
            FVector RaycastStart = TransportIn->location;
            RaycastStart.Z = this->GetActorLocation().Z;
            simMoveComponent->GetSnapGroundTransform(NewTransform, RaycastStart, TransportIn->rotation);
        }
        else
        {
            NewTransform = FTransform(TransportIn->rotation, TransportIn->location);
        }

        SetActorTransform(NewTransform);
    }

    if (bRenderTrajectory)
    {
        TArray<FSplinePoint> SplinePoints;
        for (size_t i = 0; i < TransportIn->trajectory.data.Num(); i++)
        {
            FSplinePoint Point;

            FTransform NewTransform;
            FVector RaycastStartLoc = TransportIn->trajectory.data[i].location;
            simMoveComponent->GetSnapGroundTransform_NotAverage(
                NewTransform, RaycastStartLoc, TransportIn->trajectory.data[i].rotator, true);
            Point.Position = NewTransform.GetTranslation();

            Point.Rotation = TransportIn->trajectory.data[i].rotator;
            Point.Scale = FVector(1);
            // Point.ArriveTangent;
            // Point.LeaveTangent;
            Point.Type = ESplinePointType::Type::Curve;
            SplinePoints.Add(Point);
        }
        trajectoryComponent->UpdateSpline(SplinePoints);
    }

    basicInfoComp->Update(_Input);
}

void ATransportPawn::Destroy()
{
    Super::Destroy();
}

double ATransportPawn::GetTimeStamp() const
{
    if (basicInfoComp)
    {
        return basicInfoComp->timeStamp;
    }
    return -1;
}

void ATransportPawn::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ATransportPawn::BeginPlay()
{
    Super::BeginPlay();

    camera_Driver->SetConstraintAspectRatio(bConstrainAspectRatio_CameraDriver);
    camera_Driver->SetAspectRatio(aspectRatio_CameraDriver);
    camera_Driver->AddRelativeLocation(offset_CameraDriver);
    camera_Driver->SetFieldOfView(fov_CameraDriver);

    springArm_Free->TargetArmLength = spring_length;
    springArm_Free->SetRelativeRotation(spring_rotator);
}

void ATransportPawn::BeginRotateView()
{
    if (camera_Free && cameraMasterComp->GetCurrentCamera() == camera_Free)
    {
        bActiveRotateView = true;
    }
}

void ATransportPawn::EndRotateView()
{
    if (camera_Free && cameraMasterComp->GetCurrentCamera() == camera_Free)
    {
        bActiveRotateView = false;
    }
}

void ATransportPawn::ZoomIn()
{
    if (camera_Free && cameraMasterComp->GetCurrentCamera() == camera_Free)
    {
        springArm_Free->TargetArmLength -= 10;
    }
    else if (camera_BirdView && cameraMasterComp->GetCurrentCamera() == camera_BirdView)
    {
        springArm_Bird->TargetArmLength -= 30;
    }
}

void ATransportPawn::ZoomOut()
{
    if (camera_Free && cameraMasterComp->GetCurrentCamera() == camera_Free)
    {
        springArm_Free->TargetArmLength += 10;
    }
    else if (camera_BirdView && cameraMasterComp->GetCurrentCamera() == camera_BirdView)
    {
        springArm_Bird->TargetArmLength += 30;
    }
}

void ATransportPawn::SetupCameras()
{
    // Set camera transform
    if (camera_BirdView)
    {
        // camera_BirdView->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        // TEXT("BirdView"));
        cameraMasterComp->RegisterCamera(TEXT("Camera_BirdView"), camera_BirdView);
    }
    if (camera_Driver)
    {
        // camera_Driver->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        // TEXT("DriverView"));
        cameraMasterComp->RegisterCamera(TEXT("Camera_Driver"), camera_Driver);
    }
    if (camera_Roof)
    {
        // camera_Roof->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        // TEXT("RoofView"));
        cameraMasterComp->RegisterCamera(TEXT("Camera_Roof"), camera_Roof);
    }
    if (camera_Free && springArm_Free)
    {
        // springArm_Free->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        // TEXT("FreeView")); camera_Free->AttachToComponent(springArm_Free,
        // FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        cameraMasterComp->RegisterCamera(TEXT("Camera_Free"), camera_Free);
    }
}

void ATransportPawn::SetupLights()
{
}

// Called every frame
void ATransportPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update camera with spring arm
    if (bActiveRotateView)
    {
        FRotator SpringArmRot = springArm_Free->GetRelativeTransform().Rotator();
        FVector2D MouseInputDelta;
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        PC->GetInputMouseDelta(MouseInputDelta.X, MouseInputDelta.Y);
        SpringArmRot += FRotator(MouseInputDelta.Y, MouseInputDelta.X, 0);
        SpringArmRot.Pitch = FMath::Clamp(SpringArmRot.Pitch, -89.f, 89.f);
        springArm_Free->SetRelativeRotation(SpringArmRot);
    }

    // Update lights
    int32 Hour =
        GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->envManager->GetCurrentEnvData().date.GetHour();
    if (Hour < 6 || Hour > 18)
    {
        bBeingNight = true;
        if (bBeingDay)
        {
            // Night, Switch on light
            lightMasterComp->SwitchLight(TEXT("Lamp_HighBeam"), true);
            lightMasterComp->SwitchLight(TEXT("Lamp_Clearance"), true);
            bBeingDay = false;
            // UE_LOG(LogTemp, Log, TEXT("Vehicle`s lights on."));
        }
    }
    else
    {
        bBeingDay = true;
        if (bBeingNight)
        {
            // Day, Switch off light
            lightMasterComp->SwitchLight(TEXT("Lamp_HighBeam"), false);
            lightMasterComp->SwitchLight(TEXT("Lamp_Clearance"), true);
            bBeingNight = false;
            // UE_LOG(SimLogEnvManager, Log, TEXT("Street`s lights off."));
        }
    }
}

// Called to bind functionality to input
void ATransportPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAction("SwitchCamera", IE_Released, this, &ATransportPawn::SwitchCamera);
    PlayerInputComponent->BindAction("RotateView", IE_Pressed, this, &ATransportPawn::BeginRotateView);
    PlayerInputComponent->BindAction("RotateView", IE_Released, this, &ATransportPawn::EndRotateView);
    PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &ATransportPawn::ZoomIn);
    PlayerInputComponent->BindAction("ZoomOut", IE_Released, this, &ATransportPawn::ZoomOut);
    // PlayerInputComponent->BindAction("SwitchCamera", IE_Released, this->cameraMasterComp,
    // &UCameraMasterComponent::SwitchCamera);
}
void ATransportPawn::PostLoad()
{
    Super::PostLoad();
}
#if WITH_EDITOR
void ATransportPawn::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif    // WITH_EDITOR

void ATransportPawn::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    // Setup lights of transport
    SetupLights();
    // Setup cameras of transport
    SetupCameras();

    // if (lightMasterComp->CreateLampWithLightComponent<USpotLightComponent>(TEXT("Lamp_HighBeam_L"),
    // TEXT("Mat_carlight"), TEXT("Lamp_HighBeam"), TEXT("Lamp_HighBeam_L"), meshComp))
    //{
    //     TTuple<int, FString, class ULocalLightComponent*, bool>* Lamp = lightMasterComp->GetLamp(meshComp,
    //     TEXT("Lamp_HighBeam_L")); if (Lamp)
    //     {
    //         Lamp->Get<2>()->SetIntensity(100000);
    //         Lamp->Get<2>()->SetAttenuationRadius(2000);
    //         Lamp->Get<2>()->SetMobility(EComponentMobility::Type::Movable);
    //         Lamp->Get<2>()->SetCastShadows(false);
    //         Cast<USpotLightComponent>(Lamp->Get<2>())->SetInnerConeAngle(50);
    //         Cast<USpotLightComponent>(Lamp->Get<2>())->SetOuterConeAngle(80);
    //         Cast<USpotLightComponent>(Lamp->Get<2>())->SetSourceRadius(300);
    //     }
    // }
    // if (lightMasterComp->CreateLampWithLightComponent<USpotLightComponent>(TEXT("Lamp_HighBeam_R"),
    // TEXT("Mat_carlight"), TEXT("Lamp_HighBeam"), TEXT("Lamp_HighBeam_R"), meshComp))
    //{
    //     TTuple<int, FString, class ULocalLightComponent*, bool>* Lamp = lightMasterComp->GetLamp(meshComp,
    //     TEXT("Lamp_HighBeam_R")); if (Lamp)
    //     {
    //         Lamp->Get<2>()->SetIntensity(100000);
    //         Lamp->Get<2>()->SetAttenuationRadius(2000);
    //         Lamp->Get<2>()->SetMobility(EComponentMobility::Type::Movable);
    //         Lamp->Get<2>()->SetCastShadows(false);
    //         Cast<USpotLightComponent>(Lamp->Get<2>())->SetInnerConeAngle(50);
    //         Cast<USpotLightComponent>(Lamp->Get<2>())->SetOuterConeAngle(80);
    //         Cast<USpotLightComponent>(Lamp->Get<2>())->SetSourceRadius(300);
    //     }
    // }
    // TArray<FString> HighBeamGroup = { TEXT("Lamp_HighBeam_L") ,TEXT("Lamp_HighBeam_R") };
    // lightMasterComp->GroupLamp(TEXT("Lamp_HighBeam"), HighBeamGroup);
    ////lightMasterComp->CreateLampWithLightComponent<USpotLightComponent>(TEXT("Lamp_LowBeam_R"), TEXT("Mat_carlight"),
    ///TEXT("Lamp_LowBeam"), TEXT("Lamp_HighBeam_R"), meshComp);
    // lightMasterComp->CreateLamp(TEXT("Lamp_Stop"), TEXT("Mat_carlight"), TEXT("Lamp_Stop"), meshComp);
    // lightMasterComp->CreateLamp(TEXT("Lamp_Backup"), TEXT("Mat_carlight"), TEXT("Lamp_Backup"), meshComp);
    // lightMasterComp->CreateLamp(TEXT("Lamp_Clearance"), TEXT("Mat_carlight"), TEXT("Lamp_Clearance"), meshComp);
}

void ATransportPawn::SwitchCamera(FString _Name)
{
    if (cameraMasterComp)
    {
        cameraMasterComp->SwitchCameraByName(_Name);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Switch Camera Failed! cameraMasterComp Is Null!"))
    }
}

void ATransportPawn::SwitchCamera()
{
    SwitchCamera(FString());
}

FVector ATransportPawn::GetVelocity() const
{
    return basicInfoComp ? basicInfoComp->velocity : FVector();
}

bool ATransportPawn::SwitchDrivingMode(EDrivingMode _DrivingMode)
{
    if (!mannedControlComponent)
        return false;
    if (_DrivingMode == EDrivingMode::DM_MANNED)
    {
        mannedControlComponent->SwitchToManned(this);
        return true;
    }
    else if (_DrivingMode == EDrivingMode::DM_AUTOPILOT)
    {
        mannedControlComponent->SwitchToAuto();
        return true;
    }
    return false;
}

bool ATransportPawn::InstallCamera(const FString& _Name, class UCameraComponent* _Camera)
{
    // TODO: check camera in this actor.
    if (_Camera)
    {
        if (cameraMasterComp)
        {
            bool bIsSuccess = cameraMasterComp->RegisterCamera(_Name, _Camera);
            if (bIsSuccess)
            {
                // if (_Name == defaultCameraName)
                //{
                //     SwitchCamera(_Name);
                // }
                // SetDefaultCamera(defaultCameraName);
            }
            return bIsSuccess;
        }
    }
    return false;
}

bool ATransportPawn::UninstallCamera(const FString& _Name)
{
    // TODO: Unregister camera in cameraMasterComp
    return false;
}

bool ATransportPawn::SetDefaultCamera(const FString& _Name)
{
    defaultCameraName = _Name;
    return true;
    return cameraMasterComp->SetCurrentCamera(_Name);
}

void ATransportPawn::ToggleDriveMode()
{
    if (modeDrive == 1)
    {
        modeDrive = 0;
    }
    else if (modeDrive == 0)
    {
        modeDrive = 1;
    }
    // return modeDrive;
}

void ATransportPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

// void ATransportPawn::SwitchLight(bool _TurnOn, FString _Name)
//{
//     TTuple< int32, class ULocalLightComponent* >* Lamp = lampMap.Find(_Name);
//     if (Lamp)
//     {
//         UMaterialInterface* MatInterface = meshComp->GetMaterial(Lamp->Get<0>());
//         if (MatInterface)
//         {
//             UMaterialInstanceDynamic* MatInsDynamic = meshComp->CreateDynamicMaterialInstance(Lamp->Get<0>(),
//             MatInterface);
//             //UMaterialInstanceDynamic* MatInsDynamic = UMaterialInstanceDynamic::Create(MatInterface, this);
//             if (MatInsDynamic)
//             {
//                 MatInsDynamic->SetScalarParameterValue(*_Name, _TurnOn ? 1 : 0);
//                 //meshComp->SetMaterial(Lamp->Get<0>(), MatInsDynamic);
//             }
//         }
//         if (Lamp->Get<1>())
//         {
//             Lamp->Get<1>()->SetVisibility(_TurnOn);
//         }
//     }
// }

// void ATransportPawn::SetupLights()
//{
//     if (!meshComp)
//     {
//         UE_LOG(LogTemp, Error, TEXT("Vehicle`s mesh is null! Can`t setup lights!(ActorName: %s)"), *this->GetName());
//         return;
//     }
//
//     if (CreateLampWithLightComponent<USpotLightComponent>(TEXT("Lamp_HighBeam"), meshComp, lampMap))
//     {
//         if (lampMap.Find(TEXT("Lamp_HighBeam")))
//         {
//             // TODO: Set light parameters
//         }
//     }
//     if (CreateLampWithLightComponent<USpotLightComponent>(TEXT("Lamp_LowBeam"), meshComp, lampMap))
//     {
//         if (lampMap.Find(TEXT("Lamp_LowBeam")))
//         {
//             // TODO: Set light parameters
//         }
//     }
//     CreateLamp(TEXT("Lamp_Stop"), meshComp, lampMap);
//     CreateLamp(TEXT("Lamp_Fog"), meshComp, lampMap);
//     CreateLamp(TEXT("Lamp_LeftTurningSignal"), meshComp, lampMap);
//     CreateLamp(TEXT("Lamp_RightTurningSignal"), meshComp, lampMap);
//     CreateLamp(TEXT("Lamp_Backup"), meshComp, lampMap);
//     CreateLamp(TEXT("Lamp_DoubleJump"), meshComp, lampMap);
//     CreateLamp(TEXT("Lamp_Clearance"), meshComp, lampMap);
//
// #if WITH_EDITOR
//     for (auto &Elem : lampMap)
//     {
//         bool IsExist = false;
//         for (auto &T : lampInfoArray)
//         {
//             if (T.name == *Elem.Key)
//             {
//                 IsExist = true;
//             }
//         }
//         if (!IsExist)
//         {
//             FLampInfo NewLampInfo;
//             NewLampInfo.name = *Elem.Key;
//             NewLampInfo.bActive = false;
//             lampInfoArray.Add(NewLampInfo);
//         }
//     }
// #endif //WITH_EDITOR
// }

// bool ATransportPawn::CreateLamp(const FString& _LampName, USkeletalMeshComponent* _TransportMesh, TMap< FString,
// TTuple< int32, class ULocalLightComponent* > >& _LampMap)
//{
//     if (!_TransportMesh)
//     {
//         return false;
//     }
//     TTuple<int32, ULocalLightComponent*> NewLamp;
//     //const USkeletalMeshSocket* Socket = meshComp->GetSocketByName(*_LampName);
//     int32 MatIndex = meshComp->GetMaterialIndex(TEXT("Mat_carlight"));
//     if (MatIndex != INDEX_NONE)
//     {
//         UMaterialInterface* MatInterface = meshComp->GetMaterial(MatIndex);
//         ULocalLightComponent* Light = NULL;
//         if (MatInterface)
//         {
//             UMaterialInstanceDynamic* MatInsDynamic = _TransportMesh->CreateDynamicMaterialInstance(MatIndex,
//             MatInterface); float DefaultValue = 0; if (MatInsDynamic &&
//             MatInsDynamic->GetScalarParameterValue(*_LampName, DefaultValue))
//             {
//                 MatInsDynamic->SetScalarParameterValue(*_LampName, 0.f);
//             }
//             else
//             {
//                 UE_LOG(LogTemp, Error, TEXT("Can`t get lamp material parameter!(ActorName: %s, LampName: %s)"),
//                 *this->GetName(), *_LampName); return false;
//             }
//             NewLamp.Get<0>() = MatIndex;
//             NewLamp.Get<1>() = NULL;
//             TTuple<int32, ULocalLightComponent*>* RepeatLamp = _LampMap.Find(_LampName);
//             if (RepeatLamp)
//             {
//                 *RepeatLamp = NewLamp;
//             }
//             else
//             {
//                 _LampMap.Add(_LampName, NewLamp);
//             }
//         }
//         else
//         {
//             UE_LOG(LogTemp, Error, TEXT("Can`t get lamp material!(ActorName: %s, LampName: %s)"), *this->GetName(),
//             *_LampName); return false;
//         }
//     }
//     return true;
// }

// FTransform ATransportPawn::GetSnapGroundTransform(const FVector& _Location, const FRotator& _Rotation)
//{
//     FVector RTLocation;
//     FRotator RTRotation;
//     FVector RTStart = _Location;
//     FVector RTEnd = _Location;
//     RTStart.Z += raycastingDistance;
//     RTEnd.Z += -raycastingDistance;
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

// TTuple< int32, class ULocalLightComponent* >* ATransportPawn::CreateLamp(const FString& _LampName,
// USkeletalMeshComponent* _TransportMesh, const ELampLightCompnentEnum& _LightComponentType)
//{
//     const USkeletalMeshSocket* Socket = meshComp->GetSocketByName(*_LampName);
//     int32 MatIndex = meshComp->GetMaterialIndex(TEXT("Mat_carlight"));
//     if (Socket && MatIndex != INDEX_NONE)
//     {
//         UMaterialInterface* MatInterface = meshComp->GetMaterial(MatIndex);
//         ULocalLightComponent* Light = NULL;
//         if (MatInterface)
//         {
//             TTuple<int32, ULocalLightComponent*> NewLamp;
//
//             UMaterialInstanceDynamic* MatInsDynamic = UMaterialInstanceDynamic::Create(MatInterface, this);
//             if (MatInsDynamic)
//             {
//                 MatInsDynamic->SetScalarParameterValue(TEXT("Switch"), 0.f);
//             }
//             NewLamp.Get<0>() = MatIndex;
//             if (_LightComponentType != ELampLightCompnentEnum::NONE)
//             {
//                 if (_LightComponentType == ELampLightCompnentEnum::POINT)
//                 {
//                     Light = NewObject<UPointLightComponent>(this, *_LampName);
//                 }
//                 if (_LightComponentType == ELampLightCompnentEnum::SPOT)
//                 {
//                     Light = NewObject<USpotLightComponent>(this, *_LampName);
//                 }
//                 if (_LightComponentType == ELampLightCompnentEnum::RECT)
//                 {
//                     Light = NewObject<URectLightComponent>(this, *_LampName);
//                 }
//                 if (Light)
//                 {
//                     Light->RegisterComponent();
//                     Light->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
//                     *_LampName); Light->SetActive(false); NewLamp.Get<1>() = Light;
//                 }
//                 else
//                 {
//                     UE_LOG(LogTemp, Error, TEXT("Create LightComponent failed!(ActorName: %s, LampName: %s)"),
//                     *this->GetName(), *_LampName); return NULL;
//                 }
//             }
//             TTuple<int32, ULocalLightComponent*>* RepeatLamp = lampMap.Find(_LampName);
//             if (RepeatLamp)
//             {
//                 *RepeatLamp = NewLamp;
//             }
//             else
//             {
//                 lampMap.Add(_LampName, NewLamp);
//             }
//         }
//         else
//         {
//             UE_LOG(LogTemp, Error, TEXT("Can`t get lamp material!(ActorName: %s, LampName: %s)"), *this->GetName(),
//             *_LampName); return NULL;
//         }
//     }
//     return lampMap.Find(_LampName);
// }
