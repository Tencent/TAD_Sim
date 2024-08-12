// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TP_VehicleAdvPawn.h"
// #include "Display.h"
#include "TP_VehicleAdvWheelFront.h"
#include "TP_VehicleAdvWheelRear.h"
// #include "TP_VehicleAdvHud.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#define HMD_MODULE_INCLUDED 0
// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#endif    // HMD_MODULE_INCLUDED

const FName ATP_VehicleAdvPawn::LookUpBinding("LookUp");
const FName ATP_VehicleAdvPawn::LookRightBinding("LookRight");
const FName ATP_VehicleAdvPawn::EngineAudioRPM("RPM");

#define LOCTEXT_NAMESPACE "VehiclePawn"

ATP_VehicleAdvPawn::ATP_VehicleAdvPawn()
{
    // Car mesh
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(
        TEXT("/Game/VehicleAdv/Vehicle/Vehicle_SkelMesh.Vehicle_SkelMesh"));
    GetMesh()->SetSkeletalMesh(CarMesh.Object);
    GetMesh()->SetRelativeScale3D(FVector(weightScale));

    static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(
        TEXT("/Game/VehicleAdv/Vehicle/VehicleAnimationBlueprint"));
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);

    // Setup friction materials
    static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> SlipperyMat(
        TEXT("/Game/VehicleAdv/PhysicsMaterials/Slippery.Slippery"));
    SlipperyMaterial = SlipperyMat.Object;

    static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> NonSlipperyMat(
        TEXT("/Game/VehicleAdv/PhysicsMaterials/NonSlippery.NonSlippery"));
    NonSlipperyMaterial = NonSlipperyMat.Object;

    UWheeledVehicleMovementComponent4W* Vehicle4W =
        CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

    check(Vehicle4W->WheelSetups.Num() == 4);

    // Wheels/Tyres
    // Setup the wheels
    Vehicle4W->WheelSetups[0].WheelClass = UTP_VehicleAdvWheelFront::StaticClass();
    Vehicle4W->WheelSetups[0].BoneName = FName("PhysWheel_FL");
    Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -8.f, 0.f);

    Vehicle4W->WheelSetups[1].WheelClass = UTP_VehicleAdvWheelFront::StaticClass();
    Vehicle4W->WheelSetups[1].BoneName = FName("PhysWheel_FR");
    Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 8.f, 0.f);

    Vehicle4W->WheelSetups[2].WheelClass = UTP_VehicleAdvWheelRear::StaticClass();
    Vehicle4W->WheelSetups[2].BoneName = FName("PhysWheel_BL");
    Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -8.f, 0.f);

    Vehicle4W->WheelSetups[3].WheelClass = UTP_VehicleAdvWheelRear::StaticClass();
    Vehicle4W->WheelSetups[3].BoneName = FName("PhysWheel_BR");
    Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 8.f, 0.f);

    // Adjust the tire loading
    Vehicle4W->MinNormalizedTireLoad = 0.0f;
    Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
    Vehicle4W->MaxNormalizedTireLoad = 2.0f;
    Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

    // Engine
    // Torque setup
    Vehicle4W->MaxEngineRPM = 5700.0f;
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);

    // Adjust the steering
    Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

    // Transmission
    // We want 4wd
    Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;

    // Drive the front wheels a little more than the rear
    Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65;

    // Automatic gearbox
    Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;
    Vehicle4W->TransmissionSetup.GearSwitchTime = 0.15f;
    Vehicle4W->TransmissionSetup.GearAutoBoxLatency = 1.0f;

    // Physics settings
    // Adjust the center of mass - the buggy is quite low
    UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
    if (UpdatedPrimitive)
    {
        UpdatedPrimitive->BodyInstance.COMNudge = FVector(8.0f, 0.0f, 0.0f);
    }

    // Set the inertia scale. This controls how the mass of the vehicle is distributed.
    Vehicle4W->InertiaTensorScale = FVector(1.0f, 1.333f, 1.2f);

    // Create a spring arm component for our chase camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 34.0f));
    SpringArm->SetWorldRotation(FRotator(-20.0f, 0.0f, 0.0f));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 125.0f;
    SpringArm->bEnableCameraLag = false;
    SpringArm->bEnableCameraRotationLag = false;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritRoll = true;

    // Create the chase camera component
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->SetRelativeLocation(FVector(-125.0, 0.0f, 0.0f));
    Camera->SetRelativeRotation(FRotator(10.0f, 0.0f, 0.0f));
    Camera->bUsePawnControlRotation = false;
    Camera->FieldOfView = 90.f;

    // Create In-Car camera component
    InternalCameraOrigin = FVector(-34.0f, -10.0f, 50.0f);
    InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
    InternalCameraBase->SetRelativeLocation(InternalCameraOrigin);
    InternalCameraBase->SetupAttachment(GetMesh());

    InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
    InternalCamera->bUsePawnControlRotation = false;
    InternalCamera->FieldOfView = 90.f;
    InternalCamera->SetupAttachment(InternalCameraBase);

    // In car HUD
    // Create text render component for in car speed display
    InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
    InCarSpeed->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
    InCarSpeed->SetRelativeLocation(FVector(35.0f, -6.0f, 20.0f));
    InCarSpeed->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    InCarSpeed->SetupAttachment(GetMesh());

    // Create text render component for in car gear display
    InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
    InCarGear->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
    InCarGear->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
    InCarGear->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    InCarGear->SetupAttachment(GetMesh());

    // Setup the audio component and allocate it a sound cue
    static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(
        TEXT("/Game/VehicleAdv/Sound/Engine_Loop_Cue.Engine_Loop_Cue"));
    EngineSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
    EngineSoundComponent->SetSound(SoundCue.Object);
    EngineSoundComponent->SetupAttachment(GetMesh());

    // Colors for the in-car gear display. One for normal one for reverse
    GearDisplayReverseColor = FColor(255, 0, 0, 255);
    GearDisplayColor = FColor(255, 255, 255, 255);

    bIsLowFriction = false;
    bInReverseGear = false;

    //// Create udp component
    // vehsimUDP = CreateDefaultSubobject<UVehicleSimUDPComponent>(FName(TEXT("VehSim")));
}

void ATP_VehicleAdvPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // set up gameplay key bindings
    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ATP_VehicleAdvPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ATP_VehicleAdvPawn::MoveRight);
    PlayerInputComponent->BindAxis(LookUpBinding);
    PlayerInputComponent->BindAxis(LookRightBinding);

    PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &ATP_VehicleAdvPawn::OnHandbrakePressed);
    PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &ATP_VehicleAdvPawn::OnHandbrakeReleased);
    PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &ATP_VehicleAdvPawn::OnToggleCamera);

    PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ATP_VehicleAdvPawn::OnResetVR);
}

void ATP_VehicleAdvPawn::MoveForward(float Val)
{
    if (Val > 0.f)
    {
        GetVehicleMovementComponent()->SetThrottleInput(Val * weightThrottle);
        // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MoveForward Input: %f"), Val *
        // weightThrottle));
    }
    else
    {
        GetVehicleMovementComponent()->SetThrottleInput(Val * weightBrake);
        // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MoveForward Input: %f"), Val *
        // weightBrake));
    }
    // UE_LOG(LogTemp, Warning, TEXT("ATP_VehicleAdvPawn::MoveForward is: %f"), Val);
}

void ATP_VehicleAdvPawn::MoveRight(float Val)
{
    GetVehicleMovementComponent()->SetSteeringInput(Val * weightSteeringWheel);
    // UE_LOG(LogTemp, Warning, TEXT("ATP_VehicleAdvPawn::MoveRight is: %f"), Val);
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("MoveRight Input: %f"), Val *
    // weightSteeringWheel));
}

void ATP_VehicleAdvPawn::OnHandbrakePressed()
{
    GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void ATP_VehicleAdvPawn::OnHandbrakeReleased()
{
    GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void ATP_VehicleAdvPawn::OnToggleCamera()
{
    EnableIncarView(!bInCarCameraActive);
}

void ATP_VehicleAdvPawn::EnableIncarView(const bool bState)
{
    if (bState != bInCarCameraActive)
    {
        bInCarCameraActive = bState;

        if (bState == true)
        {
            OnResetVR();
            Camera->Deactivate();
            InternalCamera->Activate();
        }
        else
        {
            InternalCamera->Deactivate();
            Camera->Activate();
        }

        InCarSpeed->SetVisibility(bInCarCameraActive);
        InCarGear->SetVisibility(bInCarCameraActive);
    }
}

void ATP_VehicleAdvPawn::Tick(float Delta)
{
    Super::Tick(Delta);

    FVector V_Now = (this->GetActorLocation() - lastLoc) / Delta;
    acceleration_Sim = (V_Now - velocity_Sim) / Delta;
    velocity_Sim = V_Now;
    lastLoc = this->GetActorLocation();

    // if (GetVehicleMovement()->CheckSlipThreshold(longSlipThreshold, latSlipThreshold))
    //{
    //     GetVehicleMovement()->StopMovementImmediately();
    // }

    //{
    //    float fVehiclePitch = 0.f;
    //    float fVehicleRoll = 0.f;
    //    FVector VehicleAngularVelocity = FVector::ZeroVector;
    //    FVector VehicleUpVector = GetMesh()->GetUpVector();
    //    FVector VehicleRightVector = GetMesh()->GetRightVector();

    //    UKismetMathLibrary::GetSlopeDegreeAngles(VehicleRightVector, FVector(0, 0, 1),
    //        VehicleUpVector, fVehiclePitch, fVehicleRoll);//求出当前载具在坡面或地面上角度

    //        //判断载具大于一定角度 比如30度，开始固定载具角度，就是载具平衡在一个合适角度
    //    if (FMath::Abs(fVehiclePitch) > 30 || FMath::Abs(fVehicleRoll) > 30)
    //    {

    //        VehicleAngularVelocity.X = FMath::ClampAngle(fVehicleRoll, -30, 30);
    //        VehicleAngularVelocity.Y = -FMath::ClampAngle(fVehiclePitch, -30, 30);
    //        //角速度转到载具模型空间下 然后调用SetPhysicsAngularVelocityInDegrees
    //        VehicleAngularVelocity = GetMesh()->GetComponentRotation().RotateVector(VehicleAngularVelocity);

    //        GetMesh()->SetPhysicsAngularVelocityInDegrees(VehicleAngularVelocity, true);
    //    }
    //}

    //// Send data
    // vehsimUDP->SetValue_Speed(GetVehicleMovement()->GetForwardSpeed() * 0.8 * 3.6 / 100);//KM/H
    // FVector Accel = (velocity_Sim - GetVelocity()) / Delta;
    // velocity_Sim = GetVelocity();
    // vehsimUDP->SetValue_Accel(Accel * 0.6);
    // vehsimUDP->SetValue_RPY(GetActorRotation() * 0.8);
    // vehsimUDP->SetValue_RotationSpeed(GetVehicleMovement()->GetEngineRotationSpeed() * 0.3);//RPM

    //// exchange data between udp componnet
    // float Throttle = vehsimUDP->GetValue_Throttle();
    // float SteeringWheel = vehsimUDP->GetValue_SteeringWheel();
    // float Brake = vehsimUDP->GetValue_Brake();

    // float ForwardWeight = FMath::Abs(Throttle * 1.0) - FMath::Abs(Brake * 1.0);

    // MoveForward(ForwardWeight);
    // MoveRight(SteeringWheel * -0.5 * 1.3);

    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Throttle: %f"), Throttle));
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Throttle: %f"), SteeringWheel));
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Throttle: %f"), Brake));

    // Setup the flag to say we are in reverse gear
    bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;

    // Update phsyics material
    UpdatePhysicsMaterial();

    // Update the strings used in the hud (incar and onscreen)
    UpdateHUDStrings();

    // Set the string in the incar hud
    SetupInCarHUD();

    bool bHMDActive = false;
#if HMD_MODULE_INCLUDED
    if ((GEngine->XRSystem.IsValid() == true) &&
        ((GEngine->XRSystem->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true)))
    {
        bHMDActive = true;
    }
#endif    // HMD_MODULE_INCLUDED
    if (bHMDActive == false)
    {
        if ((InputComponent) && (bInCarCameraActive == true))
        {
            FRotator HeadRotation = InternalCamera->GetRelativeRotation();
            HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
            HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
            InternalCamera->SetRelativeRotation(HeadRotation);
        }
    }

    // Pass the engine RPM to the sound component
    float RPMToAudioScale = 2500.0f / GetVehicleMovement()->GetEngineMaxRotationSpeed();
    EngineSoundComponent->SetFloatParameter(
        EngineAudioRPM, GetVehicleMovement()->GetEngineRotationSpeed() * RPMToAudioScale);
}

void ATP_VehicleAdvPawn::BeginPlay()
{
    Super::BeginPlay();

    bool bWantInCar = false;
    // First disable both speed/gear displays
    bInCarCameraActive = false;
    InCarSpeed->SetVisibility(bInCarCameraActive);
    InCarGear->SetVisibility(bInCarCameraActive);

    // Enable in car view if HMD is attached
#if HMD_MODULE_INCLUDED
    bWantInCar = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#endif    // HMD_MODULE_INCLUDED

    EnableIncarView(bWantInCar);
    // Start an engine sound playing
    EngineSoundComponent->Play();

    //// Start Communication
    // if (bUseRemoteMannedControl)
    //{
    //     vehsimUDP->StartCommunication();
    // }
    lastLoc = this->GetActorLocation();
}

void ATP_VehicleAdvPawn::OnResetVR()
{
#if HMD_MODULE_INCLUDED
    if (GEngine->XRSystem.IsValid())
    {
        GEngine->XRSystem->ResetOrientationAndPosition();
        InternalCamera->SetRelativeLocation(InternalCameraOrigin);
        GetController()->SetControlRotation(FRotator());
    }
#endif    // HMD_MODULE_INCLUDED
}

void ATP_VehicleAdvPawn::UpdateHUDStrings()
{
    float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
    int32 KPH_int = FMath::FloorToInt(KPH);
    int32 Gear = GetVehicleMovement()->GetCurrentGear();

    // Using FText because this is display text that should be localizable
    SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));

    if (bInReverseGear == true)
    {
        GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
    }
    else
    {
        GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
    }
}

void ATP_VehicleAdvPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    //// close communication
    // if (bUseRemoteMannedControl)
    //{
    //     vehsimUDP->EndCommunication();
    // }
}

void ATP_VehicleAdvPawn::SetupInCarHUD()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr))
    {
        // Setup the text render component strings
        InCarSpeed->SetText(SpeedDisplayString);
        InCarGear->SetText(GearDisplayString);

        if (bInReverseGear == false)
        {
            InCarGear->SetTextRenderColor(GearDisplayColor);
        }
        else
        {
            InCarGear->SetTextRenderColor(GearDisplayReverseColor);
        }
    }
}

void ATP_VehicleAdvPawn::UpdatePhysicsMaterial()
{
    if (GetActorUpVector().Z < 0)
    {
        if (bIsLowFriction == true)
        {
            GetMesh()->SetPhysMaterialOverride(NonSlipperyMaterial);
            bIsLowFriction = false;
        }
        else
        {
            GetMesh()->SetPhysMaterialOverride(SlipperyMaterial);
            bIsLowFriction = true;
        }
    }
}

#undef LOCTEXT_NAMESPACE
