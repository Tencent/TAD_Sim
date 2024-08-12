// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraSensor.h"
#include "RingSensor.generated.h"

/**
 *
 */
UCLASS()
class DISPLAY_API ARingSensor : public ACameraSensor
{
    GENERATED_BODY()
public:
    ARingSensor();

    bool Init(const FSensorConfig& _Config);
};
