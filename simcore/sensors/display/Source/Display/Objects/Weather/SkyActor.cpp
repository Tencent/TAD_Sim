// Fill out your copyright notice in the Description page of Project Settings.

#include "SkyActor.h"

// Sets default values
ASkyActor::ASkyActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASkyActor::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ASkyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
