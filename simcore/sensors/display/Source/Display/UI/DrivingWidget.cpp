// Fill out your copyright notice in the Description page of Project Settings.

#include "DrivingWidget.h"
#include "Engine/TextureRenderTarget2D.h"

void UDrivingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    render_target_right = Cast<UTextureRenderTarget2D>(StaticLoadObject(UTextureRenderTarget2D::StaticClass(), this,
        TEXT("TextureRenderTarget2D'/Game/Blueprints/Vehicle/rearview/RT_RearView_Right.RT_RearView_Right'")));
    render_target_left = Cast<UTextureRenderTarget2D>(StaticLoadObject(UTextureRenderTarget2D::StaticClass(), this,
        TEXT("TextureRenderTarget2D'/Game/Blueprints/Vehicle/rearview/RT_RearView_Left.RT_RearView_Left'")));
}

void UDrivingWidget::SetRightRearViewRT(UTextureRenderTarget2D* _RenderTarget)
{
    render_target_right = _RenderTarget;
}

void UDrivingWidget::SetLeftRearViewRT(UTextureRenderTarget2D* _RenderTarget)
{
    render_target_left = _RenderTarget;
}
