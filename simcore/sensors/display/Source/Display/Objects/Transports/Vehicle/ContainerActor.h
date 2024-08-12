// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ContainerActor.generated.h"

UCLASS()
class DISPLAY_API AContainerActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AContainerActor();

    // Sim move component
    class USimMoveComponent* simMoveComponent = NULL;

    // Skeleton mesh component
    UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* meshComp = NULL;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void SimInit(const FString& _TypeName, double _Timestamp, const FVector& _Loc, const FRotator& _Rot, float _Speed);
    void SimUpdate(double _Timestamp, const FVector& _Loc, const FRotator& _Rot, float _Speed);
};
