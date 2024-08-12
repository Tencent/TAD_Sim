// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrajectoryComponent.generated.h"

UENUM()
enum class ERenderModel : uint8
{
    SPLINEMESH,
    PROCEDURALMESH
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Config = Game)
class DISPLAY_API UTrajectoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UTrajectoryComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    UPROPERTY(config)
    ERenderModel render_model = ERenderModel::SPLINEMESH;

    class USplineComponent* spline = nullptr;
    class UProceduralMeshComponent* proceduralMesh = nullptr;
    class UStaticMesh* splineMesh = nullptr;
    class UMaterialInterface* splineMeshMat = nullptr;
    UPROPERTY(config)
    uint8 splineMeh_num = 20;
    UPROPERTY(config)
    float step_distance = 100;
    UPROPERTY(config)
    FVector splineMesh_scale = FVector(1.f, 1.f, 1.f);
    UPROPERTY(config)
    float start_loc = 0.f;

    TArray<class USplineMeshComponent*> splineMesh_arry;

    void Setup();
    void UpdateMesh();

    // ~ProceduralMesh
    void SetupProceduralMesh();
    void UpdateProceduralMesh();
    // ~ProceduralMesh

    // ~SplineMesh
    void SetupSplineMesh();
    void UpdateSplineMesh();
    // ~SplineMesh
public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void UpdateSpline(TArray<struct FSplinePoint> _Points);
    void SetRenderActive(bool _IsActive);
};
