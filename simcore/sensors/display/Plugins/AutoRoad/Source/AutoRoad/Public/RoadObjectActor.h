// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadObjectActor.generated.h"

UCLASS()
class AUTOROAD_API ARoadObjectActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ARoadObjectActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // 更新材质tilling
    UFUNCTION(BlueprintImplementableEvent, Category = "Road | Object")
    void UpdateTexture(const FString& type, const FString& name);

    // UI显示
    UFUNCTION(BlueprintImplementableEvent, Category = "Road | Object")
    void SetVisibleObjectContent(const bool isShow);
};
