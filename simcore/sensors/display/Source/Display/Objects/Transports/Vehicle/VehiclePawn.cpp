// Fill out your copyright notice in the Description page of Project Settings.

#include "VehiclePawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "Display/Components/BasicInfoComp.h"
#include "Managers/Manager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimBlueprint.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/LightMasterComp.h"
#include "Utils/DataFunctionLibrary.h"
#include "Components/SpotLightComponent.h"
#include "Components/SimMoveComponent.h"
#include "Components/ControlDataProcessor.h"
#include "Components/MannedControlComponent.h"
#include "TP_VehicleAdv/TP_VehicleAdvPawn.h"
#include "Framework/DisplayPlayerController.h"
#include "Components/DriveWidget.h"
#include "WheeledVehicleMovementComponent4W.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogVehicle, Log, All);

AVehiclePawn::AVehiclePawn()
{
    // TODO: Set collision channel

    // auto TestCamera = CreateDefaultSubobject<UCameraComponent>("TestCamera");
    // TestCamera->SetupAttachment(RootComponent);
    // TestCamera->SetRelativeLocation(FVector(0, 0, 300));

    containerSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("ContainerComponent"));
    containerSceneComp->SetupAttachment(RootComponent);

    containerMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Container"));
    containerMeshComp->SetupAttachment(containerSceneComp);
    containerMeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    // meshComp->SetAnimInstanceClass();
    containerMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    containerMeshComp->SetCollisionProfileName(FName(TEXT("SimVehicle")));

    //
    RuntimeMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CustomMesh"));
    RuntimeMeshComp->SetupAttachment(meshComp);
    RuntimeMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    RuntimeMeshComp->SetCollisionProfileName(FName(TEXT("SimVehicle")));
    //

    // Create light for lamp
    lamp_HighBeam_L = CreateDefaultSubobject<USpotLightComponent>(TEXT("Lamp_HighBeam_L"));
    lamp_HighBeam_L->SetupAttachment(meshComp, TEXT("Lamp_HighBeam_L"));
    // TODO: Load config
    lamp_HighBeam_L->SetIntensity(100000);
    lamp_HighBeam_L->SetAttenuationRadius(2000);
    lamp_HighBeam_L->SetMobility(EComponentMobility::Type::Movable);
    lamp_HighBeam_L->SetCastShadows(false);
    lamp_HighBeam_L->SetInnerConeAngle(50);
    lamp_HighBeam_L->SetOuterConeAngle(80);
    lamp_HighBeam_L->SetSourceRadius(300);
    lamp_HighBeam_L->SetVisibility(false);

    lamp_HighBeam_R = CreateDefaultSubobject<USpotLightComponent>(TEXT("Lamp_HighBeam_R"));
    lamp_HighBeam_R->SetupAttachment(meshComp, TEXT("Lamp_HighBeam_R"));
    // TODO: Load config
    lamp_HighBeam_R->SetIntensity(100000);
    lamp_HighBeam_R->SetAttenuationRadius(2000);
    lamp_HighBeam_R->SetMobility(EComponentMobility::Type::Movable);
    lamp_HighBeam_R->SetCastShadows(false);
    lamp_HighBeam_R->SetInnerConeAngle(50);
    lamp_HighBeam_R->SetOuterConeAngle(80);
    lamp_HighBeam_R->SetSourceRadius(300);
    lamp_HighBeam_R->SetVisibility(false);

    driving_ui = CreateDefaultSubobject<UDriveWidget>(TEXT("DrivingUI"));

    // SaveConfig();
}

void AVehiclePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AVehiclePawn::Init(const FSimActorConfig& _Config)
{
    ATransportPawn::Init(_Config);

    const FVehicleConfig* VehicleConfig = Cast_Sim<const FVehicleConfig>(_Config);
    check(VehicleConfig);
    vehicleConfig = *VehicleConfig;

    UE_LOG(LogTemp, Warning, TEXT("Name: %s, uuid: %d"), *VehicleConfig->typeName, uuid);

    // Generate ui for driving
    if (driving_ui && bActiveDrivingUI)
    {
        FVector RightRearLoc = meshComp->GetSocketLocation(TEXT("RearView_R"));
        FRotator RightRearRot = meshComp->GetSocketRotation(TEXT("RearView_R"));

        FVector LeftRearLoc = meshComp->GetSocketLocation(TEXT("git"));
        FRotator LeftRearRot = meshComp->GetSocketRotation(TEXT("RearView_L"));

        driving_ui->Init(RightRearLoc, RightRearRot, LeftRearLoc, LeftRearRot);
    }

    if (VehicleConfig->CustomMesh)
    {
        RuntimeMeshComp->SetStaticMesh(VehicleConfig->CustomMesh);

        lamp_HighBeam_L->AttachToComponent(
            RuntimeMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("SOCKET_Lamp_HighBeam_L"));
        lamp_HighBeam_R->AttachToComponent(
            RuntimeMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("SOCKET_Lamp_HighBeam_R"));

        camera_Driver->AttachToComponent(RuntimeMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("SOCKET_Camera_DriverView"));
        camera_Roof->AttachToComponent(
            RuntimeMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("SOCKET_Camera_RoofView"));
        springArm_Free->AttachToComponent(
            RuntimeMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("SOCKET_Camera_FreeView"));
        springArm_Bird->AttachToComponent(
            RuntimeMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT(""));    // žļšÎÖÐÐÄĩã
        springArm_Bird->SetRelativeRotation(FRotator(-89.f, 0.f, 0.f));
        meshComp->SetVisibility(false);
    }
    // check(basicInfoComp);
    // basicInfoComp->SetConfig(*VehicleConfig);
    // basicInfoComp->id = VehicleConfig->id;
    // basicInfoComp->typeName = VehicleConfig->typeName;

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
    //    }
    //    else
    //    {
    //        UE_LOG(SimLogVehicle, Error, TEXT("Can not load mesh: %s"), *MeshPath);
    //    }
    //    if (AnimBPClass)
    //    {
    //        meshComp->SetAnimInstanceClass(AnimBPClass);
    //    }
    //    else
    //    {
    //        UE_LOG(SimLogVehicle, Error, TEXT("Can not load AnimBP: %s"), *AnimPath);
    //    }
    //}
    // else
    //{
    //    UE_LOG(SimLogVehicle, Error, TEXT("The type is not in list: %s"), *basicInfoComp->typeName);
    //}

    // SetupCameras();

    // SetupLights();

    //// Set camera transform
    // if (camera_BirdView)
    //{
    //     camera_BirdView->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
    //     TEXT("BirdView"));
    // }
    // if (camera_Driver)
    //{
    //     camera_Driver->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
    //     TEXT("DriverView"));
    // }
    // if (camera_Roof)
    //{
    //     camera_Roof->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
    //     TEXT("RoofView"));
    // }
    // if (camera_Free && springArm_Free)
    //{
    //     springArm_Free->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
    //     TEXT("FreeView"));
    // }

    // SetActorLocationAndRotation(VehicleConfig->startLocation, VehicleConfig->startRotation);

    // Register SimActor
    AManager::RegisterSimActor(this);

    UE_LOG(SimLogVehicle, Warning, TEXT("Vehicle Pawn Inited, Id: %d"), basicInfoComp->id);
    // TODO: Set wheels pose.
}

void AVehiclePawn::Update(const FSimActorInput& _Input, FSimActorOutput& _Output)
{
    ATransportPawn::Update(_Input, _Output);

    const FVehicleIn* VehicleIn = Cast_Sim<const FVehicleIn>(_Input);
    FVehicleOut* VehicleOut = Cast_Sim<FVehicleOut>(_Output);
    *(FSimActorInput*) VehicleOut = _Input;
    check(VehicleIn);
    // FTransform NewTransform = GetSnapGroundTransform(*VehicleIn);
    // SetActorTransform(NewTransform);
    // basicInfoComp->timeStamp = VehicleIn->timeStamp;

    // driving_processor->UpdateSimData(*VehicleIn);
    // driving_processor->GetInput();
    // driving_processor->UpdateVehicleInfo();

    // Send back location by manned drive
    if (GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->id_controlled == GetConfig()->id &&
        GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->GetDrivingMode() == 1)
    {
        VehicleOut->id = this->GetConfig()->id;
        VehicleOut->locationManned = this->GetActorLocation();
        VehicleOut->rotatorManned = this->GetActorRotation();
        VehicleOut->velocityManned = mannedControlComponent->GetMannedPawn()->velocity_Sim;
        VehicleOut->acceleration = mannedControlComponent->GetMannedPawn()->acceleration_Sim;
        VehicleOut->trafficType = trafficType;
        VehicleOut->bHasMannedControlData = true;

        if (driving_ui && bActiveDrivingUI)
        {
            float KPH =
                FMath::Abs(mannedControlComponent->GetMannedPawn()->GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
            int32 KPH_int = FMath::FloorToInt(KPH);

            float RPM = mannedControlComponent->GetMannedPawn()->GetVehicleMovement()->GetEngineRotationSpeed();

            driving_ui->UpdateSpeed(KPH_int);
            driving_ui->UpdateRPM(RPM);
        }
    }
    else
    {
        FTransform NewTransform;
        if (containerSceneComp && containerMeshComp && containerMeshComp->SkeletalMesh && VehicleIn->hasContainer)
        {
            // TODO: support toe simMoveComponent
            if (isEgoSnap)
            {
                FVector RaycastStart = VehicleIn->locContainer;
                RaycastStart.Z = containerSceneComp->GetComponentLocation().Z;
                simMoveComponent->GetSnapGroundTransform(NewTransform, RaycastStart, VehicleIn->rotContainer);
            }
            else
            {
                NewTransform = FTransform(VehicleIn->rotContainer, VehicleIn->locContainer);
            }
            containerSceneComp->SetWorldTransform(NewTransform);
            containerMeshComp->SetRelativeLocation(ContainerOffset);
        }
    }

    VehicleOut->locPose = this->GetActorLocation();
    VehicleOut->rotPose = this->GetActorRotation();
    VehicleOut->bHasPose = true;
    if (VehicleOut->typeName.IsEmpty())
    {
        VehicleOut->type = vehicleConfig.type;
        VehicleOut->typeName = vehicleConfig.typeName;
    }
    VehicleOut->sizeLWH = GetComponentsBoundingBox().GetSize();
}

void AVehiclePawn::Destroy()
{
    Super::Destroy();
}

void AVehiclePawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    SwitchCamera(GetDefaultCameraName());
}

void AVehiclePawn::ApplyTruckCatalogOffset(const FTruckCatalogData& Data)
{
    ApplyCatalogOffset(Data.TruckHeadOffset);

    if (!bDisableCatalog)
    {
        FVector LocalTruckTrailerOffset = Data.TruckTrailerOffset;
        LocalTruckTrailerOffset.Y = -LocalTruckTrailerOffset.Y;
        LocalTruckTrailerOffset *= 100.f;

        FVector CombinationTrailerOffset = Data.CombinationTrailerOffset;
        CombinationTrailerOffset.Y = CombinationTrailerOffset.Y;
        CombinationTrailerOffset *= 100.f;

        ContainerOffset = LocalTruckTrailerOffset;

        containerMeshComp->SetRelativeLocation(LocalTruckTrailerOffset + CombinationTrailerOffset);
    }
}

bool AVehiclePawn::SwitchDrivingMode(EDrivingMode _DrivingMode)
{
    bool IsSuccess = Super::SwitchDrivingMode(_DrivingMode);

    if (GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->id_controlled == GetConfig()->id)
    {
        if (driving_ui && bActiveDrivingUI)
        {
            if (_DrivingMode == EDrivingMode::DM_MANNED)
            {
                driving_ui->Enable();
            }
            else if (_DrivingMode == EDrivingMode::DM_AUTOPILOT)
            {
                driving_ui->Close();
            }
        }
    }

    return IsSuccess;
}

void AVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

    // Debug sedan vehicle tag is 100.f
    float TagValue = 100.f;
    float Factor = 255.f;
    meshComp->SetDefaultCustomPrimitiveDataFloat(0, TagValue / Factor);

    TArray<USceneComponent*> Children;
    meshComp->GetChildrenComponents(false, Children);
    for (auto* Comp : Children)
    {
        if (UPrimitiveComponent* Child = Cast<UPrimitiveComponent>(Comp))
        {
            Child->SetDefaultCustomPrimitiveDataFloat(0, TagValue / Factor);
        }
    }
    /// Debug end
}

void AVehiclePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->id_controlled == GetConfig()->id &&
        GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->GetDrivingMode() == 1)
    {
        if (physicProxyActor)
        {
            FTransform FixedT = GetActorTransform();
            FixedT.SetLocation(physicProxyActor->GetActorLocation());

            FRotator RTRotation = physicProxyActor->GetActorRotation();

            if (bUseAverageManned)
            {
                sumRpy += RTRotation;
                if (smoothRpyBuf.Num() < smoothN)
                {
                    smoothRpyBuf.Add(RTRotation);
                    curIdx = 0;
                }
                else
                {
                    sumRpy -= smoothRpyBuf[curIdx];
                    smoothRpyBuf[curIdx] = RTRotation;
                    curIdx = (curIdx + 1) % smoothN;
                }

                FRotator SumRpy = FRotator();
                TArray<FRotator> RelativeRpyArry;
                for (size_t i = 0; i < smoothRpyBuf.Num(); i++)
                {
                    FRotator RelativeRpy =
                        FQuat::FindBetween(smoothRpyBuf[i].Vector(), physicProxyActor->GetActorRotation().Vector())
                            .Rotator();
                    SumRpy += RelativeRpy;
                    RelativeRpyArry.Add(RelativeRpy);
                }

                // RTRotation = sumRpy * (1.0f / smoothRpyBuf.Num());
                RTRotation = SumRpy * (1.0f / (float) RelativeRpyArry.Num());

                // RTRotation = (RTRotation.Quaternion()*_Rotation.Quaternion()).Rotator();
                // RTRotation = (RTRotation.Quaternion()).Rotator();
                RTRotation = (RTRotation.Quaternion() * physicProxyActor->GetActorRotation().Quaternion()).Rotator();
            }

            if (bUseFixedManned)
            {
                RTRotation.Pitch = 0;
                RTRotation.Roll = 0;
            }
            FixedT.SetRotation(RTRotation.Quaternion());
            SetActorTransform(FixedT);
        }
    }

    if (LastLocation.Equals(GetActorLocation()))
    {
        StayRecordTime += DeltaTime;
        if (StayRecordTime >= 0.5f)
        {
            basicInfoComp->velocity = FVector(0.f);
            StayRecordTime = 0;
        }
    }
    else
    {
        StayRecordTime = 0.f;
    }
    LastLocation = GetActorLocation();
    // Data translate with vehicle sim and manned control..
}

void AVehiclePawn::SetupLights()
{
    Super::SetupLights();

    {
        FLampInfo HighBeamLamp;
        FLampMaterialData NewLampMatData;
        NewLampMatData.matInstance =
            UDataFunctionLibrary::CreateMaterialDynamicInstance(meshComp, TEXT("Mat_carlight"));
        NewLampMatData.paramName = TEXT("Lamp_HighBeam");
        NewLampMatData.defaultScaleValue = 0.f;
        HighBeamLamp.materialArry.Add(NewLampMatData);
        // USpotLightComponent* Spot_L = NewObject<USpotLightComponent>(this, TEXT("Lamp_HighBeam_L"));
        USpotLightComponent* Spot_L = lamp_HighBeam_L;
        if (Spot_L)
        {
            // Spot_L->SetIntensity(100000);
            // Spot_L->SetAttenuationRadius(2000);
            // Spot_L->SetMobility(EComponentMobility::Type::Movable);
            // Spot_L->SetCastShadows(false);
            // Spot_L->SetInnerConeAngle(50);
            // Spot_L->SetOuterConeAngle(80);
            // Spot_L->SetSourceRadius(300);
            // Spot_L->SetVisibility(false);

            // Spot_L->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            // TEXT("Lamp_HighBeam_L")); Spot_L->RegisterComponent();
            HighBeamLamp.lightArry.Add(Spot_L);
        }
        // USpotLightComponent* Spot_R = NewObject<USpotLightComponent>(this, TEXT("Lamp_HighBeam_R"));
        USpotLightComponent* Spot_R = lamp_HighBeam_R;
        if (Spot_R)
        {
            // Spot_R->SetIntensity(100000);
            // Spot_R->SetAttenuationRadius(2000);
            // Spot_R->SetMobility(EComponentMobility::Type::Movable);
            // Spot_R->SetCastShadows(false);
            // Spot_R->SetInnerConeAngle(50);
            // Spot_R->SetOuterConeAngle(80);
            // Spot_R->SetSourceRadius(300);
            // Spot_R->SetVisibility(false);

            // Spot_R->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            // TEXT("Lamp_HighBeam_R")); Spot_R->RegisterComponent();
            HighBeamLamp.lightArry.Add(Spot_R);
        }
        lightMasterComp->CreateLamp(TEXT("Lamp_HighBeam"), HighBeamLamp);
    }

    {
        FLampInfo StopLamp;
        FLampMaterialData& NewLampMatData = StopLamp.materialArry.AddDefaulted_GetRef();
        NewLampMatData.matInstance =
            UDataFunctionLibrary::CreateMaterialDynamicInstance(meshComp, TEXT("Mat_carlight"));
        NewLampMatData.paramName = TEXT("Lamp_Stop");
        NewLampMatData.defaultScaleValue = 0.f;
        lightMasterComp->CreateLamp(TEXT("Lamp_Stop"), StopLamp);
    }

    {
        FLampInfo NewLamp;
        FLampMaterialData& NewLampMatData = NewLamp.materialArry.AddDefaulted_GetRef();
        NewLampMatData.matInstance =
            UDataFunctionLibrary::CreateMaterialDynamicInstance(meshComp, TEXT("Mat_carlight"));
        NewLampMatData.paramName = TEXT("Lamp_Backup");
        NewLampMatData.defaultScaleValue = 0.f;
        lightMasterComp->CreateLamp(TEXT("Lamp_Backup"), NewLamp);
    }

    {
        FLampInfo NewLamp;
        FLampMaterialData& NewLampMatData = NewLamp.materialArry.AddDefaulted_GetRef();
        NewLampMatData.matInstance =
            UDataFunctionLibrary::CreateMaterialDynamicInstance(meshComp, TEXT("Mat_carlight"));
        NewLampMatData.paramName = TEXT("Lamp_Clearance");
        NewLampMatData.defaultScaleValue = 0.f;
        lightMasterComp->CreateLamp(TEXT("Lamp_Clearance"), NewLamp);
    }
}

float AVehiclePawn::GetMeshHead()
{
    const TArray<USceneComponent*> ChildComps = meshComp->GetAttachChildren();
    for (int32 i = 0; i < ChildComps.Num(); i++)
    {
        if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(ChildComps[i]))
        {
            return meshComp->GetRelativeLocation().X + SMC->Bounds.BoxExtent.X;
        }
    }
    return meshComp->GetRelativeLocation().X + meshComp->Bounds.BoxExtent.X;
}

float AVehiclePawn::GetMeshEnd()
{
    if (IsUseContainer())
        return containerMeshComp->Bounds.SphereRadius - containerMeshComp->GetRelativeLocation().X -
               containerSceneComp->GetRelativeLocation().X;
    else
    {
        const TArray<USceneComponent*> ChildComps = meshComp->GetAttachChildren();
        for (int32 i = 0; i < ChildComps.Num(); i++)
        {
            if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(ChildComps[i]))
            {
                return SMC->Bounds.SphereRadius - meshComp->GetRelativeLocation().X;
            }
        }
    }
    return meshComp->Bounds.SphereRadius - meshComp->GetRelativeLocation().X;
}

// void AVehiclePawn::SetupLights()
//{
//     if (meshComp)
//     {
//         TArray<FName>AllSocket = meshComp->GetAllSocketNames();
//         for (auto &Elem : AllSocket)
//         {
//             if (Elem == FName(TEXT("Lamp_StopLamp")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("StopLamp"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_LowBeam")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("LowBeam"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_HighBeam")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("HighBeam"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_FogLamp")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("FogLamp"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_ClearanceLamp")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("ClearanceLamp"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_LeftTurningSignalLamp")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("LeftTurningSignalLamp"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_RightTurningSignalLamp")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("RightTurningSignalLamp"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_BackupLamp")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("BackupLamp"), NULL, false);
//             }
//             if (Elem == FName(TEXT("Lamp_DoubleJumpLamp")))
//             {
//                 lightMasterComp->AddLight(meshComp, Elem.ToString(), 1, TEXT("DoubleJumpLamp"), NULL, false);
//             }
//         }
//     }
// }

// void AVehiclePawn::SwitchStopLamp(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("StopLamp"), _IsOn);
// }
//
// void AVehiclePawn::SwitchLowBeam(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("LowBeam"), _IsOn);
// }
//
// void AVehiclePawn::SwitchHighBeam(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("HighBeam"), _IsOn);
// }
//
// void AVehiclePawn::SwitchFogLamp(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("FogLamp"), _IsOn);
// }
//
// void AVehiclePawn::SwitchClearanceLamp(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("ClearanceLamp"), _IsOn);
// }
//
// void AVehiclePawn::SwitchLeftTurningSignalLamp(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("LeftTurningSignalLamp"), _IsOn);
// }
//
// void AVehiclePawn::SwitchRightTurningSignalLamp(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("RightTurningSignalLamp"), _IsOn);
// }
//
// void AVehiclePawn::SwitchBackupLamp(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("BackupLamp"), _IsOn);
// }
//
// void AVehiclePawn::SwitchDoubleJumpLamp(bool _IsOn)
//{
//     lightMasterComp->SwitchLight(TEXT("DoubleJumpLamp"), _IsOn);
// }
