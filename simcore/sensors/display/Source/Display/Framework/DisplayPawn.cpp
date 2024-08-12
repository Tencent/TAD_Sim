// Fill out your copyright notice in the Description page of Project Settings.

#include "DisplayPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerInput.h"

FName ADisplayPawn::MovementComponentName(TEXT("MovementComponent0"));
FName ADisplayPawn::CollisionComponentName(TEXT("CollisionComponent0"));
FName ADisplayPawn::MeshComponentName(TEXT("MeshComponent0"));

//// Sets default values
// ADisplayPawn::ADisplayPawn()
//{
//      // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//     PrimaryActorTick.bCanEverTick = true;
//
// }

//// Called when the game starts or when spawned
// void ADisplayPawn::BeginPlay()
//{
//     Super::BeginPlay();
//
// }
//
//// Called every frame
// void ADisplayPawn::Tick(float DeltaTime)
//{
//     Super::Tick(DeltaTime);
//
// }
//
//// Called to bind functionality to input
// void ADisplayPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//     Super::SetupPlayerInputComponent(PlayerInputComponent);
//
// }

ADisplayPawn::ADisplayPawn()
{
    SetCanBeDamaged(true);
    SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
    bReplicates = true;
    NetPriority = 3.0f;

    BaseEyeHeight = 0.0f;
    bCollideWhenPlacing = false;
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(ADisplayPawn::CollisionComponentName);
    CollisionComponent->InitSphereRadius(35.0f);
    CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

    CollisionComponent->CanCharacterStepUpOn = ECB_No;
    CollisionComponent->SetShouldUpdatePhysicsVolume(true);
    CollisionComponent->SetCanEverAffectNavigation(false);
    CollisionComponent->bDynamicObstacle = true;

    RootComponent = CollisionComponent;

    MovementComponent =
        CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>(ADisplayPawn::MovementComponentName);
    MovementComponent->UpdatedComponent = CollisionComponent;

    // Structure to hold one-time initialization
    struct FConstructorStatics
    {
        ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh;
        FConstructorStatics() : SphereMesh(TEXT("/Engine/EngineMeshes/Sphere"))
        {
        }
    };

    static FConstructorStatics ConstructorStatics;

    MeshComponent = CreateOptionalDefaultSubobject<UStaticMeshComponent>(ADisplayPawn::MeshComponentName);
    if (MeshComponent)
    {
        MeshComponent->SetStaticMesh(ConstructorStatics.SphereMesh.Object);
        MeshComponent->AlwaysLoadOnClient = true;
        MeshComponent->AlwaysLoadOnServer = true;
        MeshComponent->bOwnerNoSee = true;
        MeshComponent->bCastDynamicShadow = true;
        MeshComponent->bAffectDynamicIndirectLighting = false;
        MeshComponent->bAffectDistanceFieldLighting = false;
        MeshComponent->PrimaryComponentTick.TickGroup = TG_PrePhysics;
        MeshComponent->SetupAttachment(RootComponent);
        MeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
        const float Scale =
            CollisionComponent->GetUnscaledSphereRadius() /
            160.f;    // @TODO: hardcoding known size of EngineMeshes.Sphere. Should use a unit sphere instead.
        MeshComponent->SetRelativeScale3D(FVector(Scale));
        MeshComponent->SetGenerateOverlapEvents(false);
        MeshComponent->SetCanEverAffectNavigation(false);

        MeshComponent->SetHiddenInGame(true);
    }

    // This is the default pawn class, we want to have it be able to move out of the box.
    bAddDefaultMovementBindings = true;

    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;
}

// void InitializeDefaultPawnInputBindings()
//{
//     static bool bBindingsAdded = false;
//     if (!bBindingsAdded)
//     {
//         bBindingsAdded = true;
//
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::W, 1.f));
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::S, -1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Up, 1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Down,
//         -1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward",
//         EKeys::Gamepad_LeftY, 1.f));
//
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::A, -1.f));
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::D, 1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight",
//         EKeys::Gamepad_LeftX, 1.f));
//
//         // HACK: Android controller bindings in ini files seem to not work
//         //  Direct overrides here some to work
// #if !PLATFORM_ANDROID
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp",
//         EKeys::Gamepad_LeftThumbstick, 1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp",
//         EKeys::Gamepad_RightThumbstick, -1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp",
//         EKeys::Gamepad_FaceButton_Bottom, 1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::LeftControl,
//         -1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp",
//         EKeys::SpaceBar, 1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::C, -1.f));
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::E, 1.f));
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Q, -1.f));
// #else
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp",
//         EKeys::Gamepad_LeftTriggerAxis, -0.5f));
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp",
//         EKeys::Gamepad_RightTriggerAxis, 0.5f));
// #endif
//
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate",
//         EKeys::Gamepad_RightX, 1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Left, -1.f));
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Right, 1.f));
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_Turn", EKeys::MouseX, 1.f));
//
//         //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_LookUpRate",
//         EKeys::Gamepad_RightY, 1.f));
//         UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_LookUp", EKeys::MouseY, -1.f));
//     }
// }

void ADisplayPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    if (bAddDefaultMovementBindings)
    {
        // InitializeDefaultPawnInputBindings();

        PlayerInputComponent->BindAxis("MoveForward", this, &ADisplayPawn::MoveForward);
        PlayerInputComponent->BindAxis("MoveRight", this, &ADisplayPawn::MoveRight);
        PlayerInputComponent->BindAxis("MoveUp", this, &ADisplayPawn::MoveUp_World);
        PlayerInputComponent->BindAxis("Turn", this, &ADisplayPawn::AddControllerYawInput);
        // PlayerInputComponent->BindAxis("DefaultPawn_TurnRate", this, &ADisplayPawn::TurnAtRate);
        PlayerInputComponent->BindAxis("LookUp", this, &ADisplayPawn::AddControllerPitchInput);
        // PlayerInputComponent->BindAxis("DefaultPawn_LookUpRate", this, &ADisplayPawn::LookUpAtRate);
        PlayerInputComponent->BindAction(
            TEXT("IncreaseSpeed"), EInputEvent::IE_Released, this, &ADisplayPawn::increaseSpeed);
        PlayerInputComponent->BindAction(
            TEXT("ReduceSpeed"), EInputEvent::IE_Released, this, &ADisplayPawn::reduceSpeed);
    }
}

void ADisplayPawn::UpdateNavigationRelevance()
{
    if (CollisionComponent)
    {
        CollisionComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
    }
}

void ADisplayPawn::MoveRight(float Val)
{
    if (Val != 0.f)
    {
        if (Controller)
        {
            FRotator const ControlSpaceRot = Controller->GetControlRotation();

            // transform to world space and add it
            AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Y), Val * speedScale);
        }
    }
}

void ADisplayPawn::MoveForward(float Val)
{
    if (Val != 0.f)
    {
        if (Controller)
        {
            FRotator const ControlSpaceRot = Controller->GetControlRotation();

            // transform to world space and add it
            AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X), Val * speedScale);
        }
    }
}

void ADisplayPawn::MoveUp_World(float Val)
{
    if (Val != 0.f)
    {
        AddMovementInput(FVector::UpVector, Val * speedScale);
    }
}

void ADisplayPawn::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void ADisplayPawn::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void ADisplayPawn::increaseSpeed()
{
    speedScale += 0.5f;
}

void ADisplayPawn::reduceSpeed()
{
    if (speedScale > 0.5f)
    {
        speedScale -= 0.5f;
    }
}

void ADisplayPawn::BeginPlay()
{
    Super::BeginPlay();
    AddActorWorldOffset(start_location_offset);
    AddActorWorldRotation(start_rotation_offset);
}

UPawnMovementComponent* ADisplayPawn::GetMovementComponent() const
{
    return MovementComponent;
}
