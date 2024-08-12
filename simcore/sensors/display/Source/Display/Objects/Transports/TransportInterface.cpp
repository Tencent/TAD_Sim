// Fill out your copyright notice in the Description page of Project Settings.

#include "TransportInterface.h"

// Add default functionality here for any ITransportInterface functions that are not pure virtual.

void ITransportInterface::SwitchLight(bool _TurnOn, FString _Name)
{
}

void ITransportInterface::SwitchCamera(FString _Name)
{
}

FTransform ITransportInterface::GetSnapGroundTransform(const FVector& _Location, const FRotator& _Rotation)
{
    return FTransform();
}

// FVector ITransportInterface::GetVelocity()
//{
//     return FVector();
// }

bool ITransportInterface::SwitchDrivingMode(EDrivingMode _DrivingMode)
{
    return false;
}
