// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DriveWidget.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API UDriveWidget : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UDriveWidget();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    // TMap<FString ,USceneCaptureComponent2D*> mirror_list;

    UPROPERTY()
    class USceneCaptureComponent2D* rear_view_right = nullptr;
    UPROPERTY()
    class USceneCaptureComponent2D* rear_view_left = nullptr;

    UPROPERTY()
    class UTextureRenderTarget2D* render_target_right = nullptr;
    UPROPERTY()
    class UTextureRenderTarget2D* render_target_left = nullptr;

    UFUNCTION(BlueprintCallable)
    class UTextureRenderTarget2D* GetRightRT2D();
    UFUNCTION(BlueprintCallable)
    class UTextureRenderTarget2D* GetLeftRT2D();

    UPROPERTY()
    class UDrivingWidget* main_widget = nullptr;

    void SetupSceneCapture(class USceneCaptureComponent2D* _SceneCapture);

    bool bInited = false;

public:
    void Init(const FVector& _Loc_R, const FRotator& _Rot_R, const FVector& _Loc_L, const FRotator& _Rot_L);
    void Enable();
    void Close();

    void UpdateSpeed(float _Speed);
    void UpdateRPM(float _RPM);
};
