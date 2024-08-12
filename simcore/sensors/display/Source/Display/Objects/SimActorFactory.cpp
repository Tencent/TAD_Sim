// Fill out your copyright notice in the Description page of Project Settings.

#include "SimActorFactory.h"
#include "Engine/World.h"
#include "SimActorInterface.h"

// Sets default values
ASimActorFactory::ASimActorFactory()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASimActorFactory::BeginPlay()
{
    Super::BeginPlay();
}
