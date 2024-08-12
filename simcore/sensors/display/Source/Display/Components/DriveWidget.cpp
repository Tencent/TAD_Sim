// Fill out your copyright notice in the Description page of Project Settings.

#include "DriveWidget.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UI/DrivingWidget.h"
#include "Framework/DisplayPlayerController.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UDriveWidget::UDriveWidget()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
    // UObject* RT_Right_Obj = StaticLoadObject(UTextureRenderTarget2D::StaticClass(), NULL,
    // TEXT("TextureRenderTarget2D'/Game/Blueprints/Vehicle/rearview/RT_RearView_Right.RT_RearView_Right'")); UObject*
    // RT_Left_Obj = StaticLoadObject(UTextureRenderTarget2D::StaticClass(), NULL,
    // TEXT("TextureRenderTarget2D'/Game/Blueprints/Vehicle/rearview/RT_ssight.RT_RearView_Left'")); if (RT_Right_Obj)
    //{
    //     render_target_right = Cast<UTextureRenderTarget2D>(RT_Right_Obj);
    // }
    // if (RT_Left_Obj)
    //{
    //     render_target_left = Cast<UTextureRenderTarget2D>(RT_Left_Obj);
    // }
    render_target_right = LoadObject<UTextureRenderTarget2D>(
        nullptr, TEXT("TextureRenderTarget2D'/Game/Blueprints/Vehicle/rearview/RT_RearView_Right.RT_RearView_Right'"));
    render_target_left = LoadObject<UTextureRenderTarget2D>(
        nullptr, TEXT("TextureRenderTarget2D'/Game/Blueprints/Vehicle/rearview/RT_RearView_Left.RT_RearView_Left'"));
    // ...
}

// Called when the game starts
void UDriveWidget::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

// Called every frame
void UDriveWidget::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

UTextureRenderTarget2D* UDriveWidget::GetRightRT2D()
{
    return render_target_right;
}

UTextureRenderTarget2D* UDriveWidget::GetLeftRT2D()
{
    return render_target_left;
}

void UDriveWidget::SetupSceneCapture(USceneCaptureComponent2D* _SceneCapture)
{
    _SceneCapture->bCaptureEveryFrame = false;
    _SceneCapture->bCaptureOnMovement = false;
    _SceneCapture->bAutoActivate = false;
    //_SceneCapture->Activate();
    _SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    _SceneCapture->ProjectionType = ECameraProjectionMode::Perspective;
    //_SceneCapture->FOVAngle = fmaxf(NewFov_H, NewFov_V);
}

void UDriveWidget::Init(const FVector& _Loc_R, const FRotator& _Rot_R, const FVector& _Loc_L, const FRotator& _Rot_L)
{
    rear_view_right = NewObject<USceneCaptureComponent2D>(this->GetOwner(), FName(TEXT("RearViewRight")));
    rear_view_left = NewObject<USceneCaptureComponent2D>(this->GetOwner(), FName(TEXT("RearViewLeft")));

    rear_view_right->RegisterComponent();
    rear_view_left->RegisterComponent();

    rear_view_right->AttachToComponent(
        this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    rear_view_left->AttachToComponent(
        this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    rear_view_right->SetWorldLocationAndRotation(_Loc_R, _Rot_R);
    rear_view_left->SetWorldLocationAndRotation(_Loc_L, _Rot_L);

    rear_view_right->TextureTarget = render_target_right;
    rear_view_left->TextureTarget = render_target_left;

    SetupSceneCapture(rear_view_right);
    SetupSceneCapture(rear_view_left);

    main_widget = GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->GetDrivingWidget();
    if (main_widget && main_widget->IsInViewport())
    {
        main_widget->SetRightRearViewRT(render_target_right);
        main_widget->SetLeftRearViewRT(render_target_left);
    }

    Close();

    bInited = true;
}

void UDriveWidget::Enable()
{
    if (!bInited)
    {
        return;
    }

    if (rear_view_right && rear_view_right->IsValidLowLevel())
    {
        rear_view_right->Activate();
    }

    if (rear_view_left && rear_view_left->IsValidLowLevel())
    {
        rear_view_left->Activate();
    }

    if (main_widget && main_widget->IsInViewport())
    {
        main_widget->SetVisibility(ESlateVisibility::Visible);
    }
}

void UDriveWidget::Close()
{
    if (!bInited)
    {
        return;
    }

    if (rear_view_right && rear_view_right->IsValidLowLevel())
    {
        rear_view_right->Deactivate();
    }

    if (rear_view_left && rear_view_left->IsValidLowLevel())
    {
        rear_view_left->Deactivate();
    }

    if (main_widget)
    {
        main_widget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UDriveWidget::UpdateSpeed(float _Speed)
{
    if (!bInited)
    {
        return;
    }

    if (main_widget)
    {
        // GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Yellow, *FString::Printf(TEXT("Vehicle speed : %f"),
        // _Speed));
        main_widget->UpdateSpeed(_Speed);
    }
}

void UDriveWidget::UpdateRPM(float _RPM)
{
    if (!bInited)
    {
        return;
    }

    if (main_widget)
    {
        // GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Yellow, *FString::Printf(TEXT("Vehicle RPM : %f"), _RPM));
        main_widget->UpdateRPM(_RPM);
    }
}
