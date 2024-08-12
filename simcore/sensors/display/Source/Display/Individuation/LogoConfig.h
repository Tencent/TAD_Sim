// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "LogoConfig.generated.h"

UCLASS()
class DISPLAY_API ALogoConfig : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALogoConfig();

protected:
    virtual void PostLoad() override;

    class UMaterialInterface* Mat;
};
