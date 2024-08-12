// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrivingWidget.generated.h"

/**
 *
 */
UCLASS(config = game)
class DISPLAY_API UDrivingWidget : public UUserWidget
{
    GENERATED_BODY()
protected:
    UPROPERTY(BlueprintReadWrite)
    float speed = 0.f;
    UPROPERTY(BlueprintReadWrite)
    float rpm = 0.f;

    UPROPERTY(BlueprintReadWrite)
    class UTextureRenderTarget2D* render_target_right = nullptr;
    UPROPERTY(BlueprintReadWrite)
    class UTextureRenderTarget2D* render_target_left = nullptr;
    UPROPERTY(config)
    float scale_speed = 1;
    UPROPERTY(config)
    float scale_rpm = 1;

public:
    virtual void NativeConstruct();

    void SetRightRearViewRT(class UTextureRenderTarget2D* _RenderTarget);
    void SetLeftRearViewRT(class UTextureRenderTarget2D* _RenderTarget);

    void UpdateSpeed(float _Speed)
    {
        speed = _Speed * scale_speed;
    }
    void UpdateRPM(float _RPM)
    {
        rpm = _RPM * scale_rpm;
    }
};
