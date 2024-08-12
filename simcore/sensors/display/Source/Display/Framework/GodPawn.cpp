// Fill out your copyright notice in the Description page of Project Settings.

#include "GodPawn.h"
#include "Misc/ConfigCacheIni.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AGodPawn::AGodPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    SetCanBeDamaged(true);

    SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
    bReplicates = true;
    NetPriority = 3.0f;

    BaseEyeHeight = 0.0f;
    bCollideWhenPlacing = false;
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(FName(TEXT("Sphere")));
    CollisionComponent->InitSphereRadius(35.0f);
    CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

    CollisionComponent->CanCharacterStepUpOn = ECB_No;
    CollisionComponent->SetShouldUpdatePhysicsVolume(true);
    CollisionComponent->SetCanEverAffectNavigation(false);
    CollisionComponent->bDynamicObstacle = true;

    RootComponent = CollisionComponent;

    MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>(FName(TEXT("Movement")));
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

    MeshComponent = CreateOptionalDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Mesh")));
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

// Called when the game starts or when spawned
void AGodPawn::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AGodPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGodPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AGodPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AGodPawn::MoveRight);
    // PlayerInputComponent->BindAxis("DefaultPawn_MoveUp", this, &AGodPawn::MoveUp_World);
    PlayerInputComponent->BindAxis("Turn", this, &AGodPawn::AddControllerYawInput);
    // PlayerInputComponent->BindAxis("DefaultPawn_TurnRate", this, &AGodPawn::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &AGodPawn::AddControllerPitchInput);
    // PlayerInputComponent->BindAxis("DefaultPawn_LookUpRate", this, &AGodPawn::LookUpAtRate);
}

void AGodPawn::MoveRight(float Val)
{
    if (Val != 0.f)
    {
        if (Controller)
        {
            FRotator const ControlSpaceRot = Controller->GetControlRotation();

            // transform to world space and add it
            // AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Y), Val);
            FVector DeltaMove =
                FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Y) * FVector(1, 1, 0) * Val * moveSpeed;
            this->AddActorWorldOffset(DeltaMove);
        }
    }
}

void AGodPawn::MoveForward(float Val)
{
    if (Val != 0.f)
    {
        if (Controller)
        {
            FRotator const ControlSpaceRot = Controller->GetControlRotation();

            //// transform to world space and add it
            // AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X), Val);
            FVector DeltaMove =
                FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X) * FVector(1, 1, 0) * Val * moveSpeed;
            this->AddActorWorldOffset(DeltaMove);
        }
    }
}

void AGodPawn::MoveUp_World(float Val)
{
    if (Val != 0.f)
    {
        AddMovementInput(FVector::UpVector, Val);
    }
}

void AGodPawn::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AGodPawn::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}
