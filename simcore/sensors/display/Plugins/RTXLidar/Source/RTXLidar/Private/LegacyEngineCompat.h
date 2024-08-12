/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#pragma once

#include "Runtime/Launch/Resources/Version.h"

// Definitions to allow some code written for UE5 to work with legacy 4.x engines

#if ENGINE_MAJOR_VERSION < 5
// New type names with dims and precision, required in SHADER_PARAMETER declarations and useful everywhere
using FMatrix44f = FMatrix;
using FQuat4f = FQuat;
using FRotator3f = FRotator;
using FVector3f = FVector;
using FVector4f = FVector4;
using FPlane4f = FPlane;
using FRotationMatrix44f = FRotationMatrix;

// New plugin interface names
using FGlobalIlluminationPluginResources = class FGlobalIlluminationExperimentalPluginResources;
using FGlobalIlluminationPluginDelegates = class FGlobalIlluminationExperimentalPluginDelegates;
#endif
