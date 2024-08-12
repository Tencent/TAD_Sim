// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerActor.h"
// #include "Components/SkeletalMeshComponent.h"

// Sets default values
AContainerActor::AContainerActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("Mesh")));
    // RootComponent = meshComp;
    // meshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    ////meshComp->SetAnimInstanceClass();
    // meshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    // meshComp->SetCollisionProfileName(FName(TEXT("SimVehicle")));
}

// Called when the game starts or when spawned
void AContainerActor::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AContainerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AContainerActor::SimInit(
    const FString& _TypeName, double _Timestamp, const FVector& _Loc, const FRotator& _Rot, float _Speed)
{
}

void AContainerActor::SimUpdate(double _Timestamp, const FVector& _Loc, const FRotator& _Rot, float _Speed)
{
}
