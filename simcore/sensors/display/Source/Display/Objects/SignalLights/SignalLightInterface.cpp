// Fill out your copyright notice in the Description page of Project Settings.

#include "SignalLightInterface.h"

// Add default functionality here for any ITrafficLightInterface functions that are not pure virtual.

FColor ISignalLightInterface::GetColor() const
{
    return FColor::Black;
}

bool ISignalLightInterface::SetColor(const FColor& _Color)
{
    return false;
}

bool ISignalLightInterface::SwitchLight(bool _IsOn)
{
    return false;
}

bool ISignalLightInterface::IsSwitchOn() const
{
    return false;
}
