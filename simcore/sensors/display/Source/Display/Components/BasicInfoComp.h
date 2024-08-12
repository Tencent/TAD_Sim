// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "../Objects/SimActorInterface.h"
#include "BasicInfoComp.generated.h"

/**
 * Storing object`s basic information.
 * For example, location, rotation, lonlat, bounding box and so on.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API UBasicInfoComp : public USceneComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UBasicInfoComp();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    /* Basic Information */

    // Object id
    int32 id = -1;

    // Type name
    FString typeName;

    // Time stamp
    double timeStamp = 0.f;

    // LonLat location
    double lon = 0.f;
    double lat = 0.f;
    double alt = 0.f;

    // Last Time Transform
    FTransform transform_Last = FTransform();

    // Velocity
    FVector velocity = FVector(0);

    // Bounding box
    FVector boundingOrigin;
    FVector boundingBoxExtend;

    TSharedPtr<FSimActorConfig> config;

public:
    // Called every frame
    // virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
    // override;

    // Init
    bool Init(const FSimActorConfig& _Config);

    // Update
    void Update(const FSimActorInput& _InData);

    template <class T = FSimActorConfig>
    void SetConfig(const T& _Config)
    {
        config = MakeShared<T>();
        *config = _Config;
    }
};
