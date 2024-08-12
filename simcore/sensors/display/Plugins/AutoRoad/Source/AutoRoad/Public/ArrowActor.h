// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArrowActor.generated.h"

UCLASS()
class AUTOROAD_API AArrowActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AArrowActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // 更新箭头贴图
    UFUNCTION(BlueprintImplementableEvent, Category = "Road | Object")
    void FuncUpdateArrowType(const FString& arrowType);

    // UI显示
    UFUNCTION(BlueprintImplementableEvent, Category = "Road | Object")
    void FuncSetVisibleArrowType(const bool isShow);
};
