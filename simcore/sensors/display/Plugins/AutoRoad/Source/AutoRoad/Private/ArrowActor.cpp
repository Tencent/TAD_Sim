// Fill out your copyright notice in the Description page of Project Settings.

#include "ArrowActor.h"

// Sets default values
AArrowActor::AArrowActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AArrowActor::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AArrowActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
