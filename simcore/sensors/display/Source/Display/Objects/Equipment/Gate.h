// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Gate.generated.h"

UCLASS()
class DISPLAY_API AGate : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AGate();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnOpen();

    UFUNCTION(BlueprintImplementableEvent)
    void Receive_OnOpen();

    void OnClose();

    UFUNCTION(BlueprintImplementableEvent)
    void Receive_OnClose();

    UFUNCTION(BlueprintImplementableEvent)
    void ShowDebug(bool bEnable);

    UPROPERTY(EditAnyWhere)
    float DelayOpen = 10.f;

    UPROPERTY(EditAnyWhere)
    float EnterDistance_X = 1100.f;

    UPROPERTY(EditAnyWhere)
    float EnterDistance_Y = 500.f;

    UPROPERTY(EditAnyWhere)
    float LeaveDistance = 100.f;

    UPROPERTY(EditAnyWhere)
    float LeaveAngle = 90.f;

    UPROPERTY(EditAnyWhere)
    float SafeDistance = 5000.f;

    UPROPERTY(EditAnyWhere)
    bool bReverseY;

    UPROPERTY(EditAnyWhere)
    class AVehiclePawn* DubugActor;

protected:
    UFUNCTION(BlueprintCallable)
    void CheckEgoComming();

    UFUNCTION(BlueprintCallable)
    void CheckEgoLeave();

    UFUNCTION(BlueprintCallable)
    void SetOpenState(bool State)
    {
        bOpen = State;
    }

private:
    bool bEnter = false;

    bool bOpen = false;

    FTimerHandle onTriggerOpenTimer;
};
