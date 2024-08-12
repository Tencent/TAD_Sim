// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraSensor.h"
#include "SemanticCamera.generated.h"

/**
 *
 */
UCLASS()
class DISPLAY_API ASemanticCamera : public ACameraSensor
{
    GENERATED_BODY()
public:
    ASemanticCamera();
    ~ASemanticCamera();

    virtual bool Save();
};
