// Fill out your copyright notice in the Description page of Project Settings.

#include "SensorFactory.h"

// Sets default values
ASensorFactory::ASensorFactory()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASensorFactory::BeginPlay()
{
    Super::BeginPlay();
}
