// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VehicleWheel.h"
#include "TP_VehicleAdvWheelFront.generated.h"

UCLASS(config = game)
class UTP_VehicleAdvWheelFront : public UVehicleWheel
{
    GENERATED_BODY()

public:
    UTP_VehicleAdvWheelFront();

    UPROPERTY(config)
    float friction_scale = 1.f;

    UPROPERTY(config)
    float shape_radius = 18.f;
    UPROPERTY(config)
    float shape_width = 13.f;
    UPROPERTY(config)
    bool b_affected_by_handbrake = false;
    UPROPERTY(config)
    float steer_angle = 40.f;
    UPROPERTY(config)
    float lat_stiff_value = 100.f;
    UPROPERTY(config)
    float long_stiff_value = 3000.f;
    UPROPERTY(config)
    float lat_stiff_max_load = 2.f;

    UPROPERTY(config)
    float suspension_force_offset = -2.f;
    UPROPERTY(config)
    float suspension_max_raise = 4.f;
    UPROPERTY(config)
    float suspension_max_drop = 3.f;
    UPROPERTY(config)
    float suspension_natural_frequency = 6.f;
    UPROPERTY(config)
    float suspension_damping_ratio = 1.05f;
};
