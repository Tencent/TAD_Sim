// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PLineActor.generated.h"

UCLASS()
class AUTOROAD_API APLineActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    APLineActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // 厚仟可嵎tilling
    UFUNCTION(BlueprintImplementableEvent, Category = "Road | Object")
    void FuncUpdateScaleImplementable(float xScale, float yScale, float mapScale = 100);
};
