// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TP_VehicleAdvWheelFront.h"
// #include "Display.h"
#include "TireConfig.h"
#include "UObject/ConstructorHelpers.h"

UTP_VehicleAdvWheelFront::UTP_VehicleAdvWheelFront()
{
    ShapeRadius = shape_radius;
    ShapeWidth = shape_width;
    bAffectedByHandbrake = b_affected_by_handbrake;
    SteerAngle = steer_angle;
    // LatStiffValue = lat_stiff_value;
    // LongStiffValue = long_stiff_value;
    // LatStiffMaxLoad = lat_stiff_max_load;
    //  Setup suspension forces
    SuspensionForceOffset = suspension_force_offset;
    SuspensionMaxRaise = suspension_max_raise;
    SuspensionMaxDrop = suspension_max_drop;
    SuspensionNaturalFrequency = suspension_natural_frequency;
    SuspensionDampingRatio = suspension_damping_ratio;

    // Find the tire object and set the data for it
    static ConstructorHelpers::FObjectFinder<UTireConfig> TireData(
        TEXT("/Game/VehicleAdv/Vehicle/WheelData/Vehicle_FrontTireConfig.Vehicle_FrontTireConfig"));
    TireConfig = TireData.Object;
    // TireConfig->SetFrictionScale(friction_scale);
}
