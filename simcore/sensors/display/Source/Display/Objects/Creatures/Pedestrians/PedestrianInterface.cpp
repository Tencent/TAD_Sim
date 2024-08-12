// Fill out your copyright notice in the Description page of Project Settings.

#include "PedestrianInterface.h"

// Add default functionality here for any IPedestrianInterface functions that are not pure virtual.
FTransform IPedestrianInterface::GetSnapGroundTransform(const FVector& _Location, const FRotator& _Rotation)
{
    return FTransform();
}
