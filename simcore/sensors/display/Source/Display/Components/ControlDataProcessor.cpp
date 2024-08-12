// Fill out your copyright notice in the Description page of Project Settings.

#include "ControlDataProcessor.h"

// Sets default values for this component's properties
UControlDataProcessor::UControlDataProcessor()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

// Called when the game starts
void UControlDataProcessor::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

void UControlDataProcessor::RecieveData()
{
    if (device_name == TEXT("REALCAR"))
    {
    }
}

// Called every frame
void UControlDataProcessor::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    RecieveData();
}

void UControlDataProcessor::SimInit(const FVehicleConfig& _InData)
{
}

void UControlDataProcessor::UpdateVehicleInfo(const FDataOutput& _Info)
{
}

void UControlDataProcessor::UpdateSimData(const FVehicleIn& _InData)
{
    if (device_name == TEXT("REALCAR"))
    {
    }
}
