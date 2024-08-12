// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraSensor.h"
#include "DepthSensor.generated.h"

/**
 *
 */
UCLASS()
class DISPLAY_API ADepthSensor : public ACameraSensor
{
    GENERATED_BODY()
public:
    ADepthSensor();
    ~ADepthSensor();

    virtual bool Init(const FSensorConfig& _Config);
    virtual bool Save();
};
