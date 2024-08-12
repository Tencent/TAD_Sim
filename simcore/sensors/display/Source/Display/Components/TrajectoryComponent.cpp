// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryComponent.h"
#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"

// Sets default values for this component's properties
UTrajectoryComponent::UTrajectoryComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;
    splineMesh =
        LoadObject<UStaticMesh>(NULL, TEXT("StaticMesh'/Game/Blueprints/Meshes/SM_Cylinder_X_1M.SM_Cylinder_X_1M'"));
    splineMeshMat =
        LoadObject<UMaterialInterface>(NULL, TEXT("Material'/Game/Blueprints/Material/Mat_Trajectory.Mat_Trajectory'"));
}

// Called when the game starts
void UTrajectoryComponent::BeginPlay()
{
    Super::BeginPlay();

    Setup();
}

void UTrajectoryComponent::Setup()
{
    spline = NewObject<USplineComponent>(this->GetOwner(), FName(TEXT("TrajectorySpline")));
    spline->RegisterComponent();
    if (spline)
    {
        spline->SetDrawDebug(true);
    }

    if (render_model == ERenderModel::SPLINEMESH)
    {
        SetupSplineMesh();
    }
    else if (render_model == ERenderModel::PROCEDURALMESH)
    {
        SetupProceduralMesh();
    }
}

void UTrajectoryComponent::UpdateMesh()
{
    if (render_model == ERenderModel::SPLINEMESH)
    {
        UpdateSplineMesh();
    }
    else if (render_model == ERenderModel::PROCEDURALMESH)
    {
        UpdateProceduralMesh();
    }
}

void UTrajectoryComponent::SetupProceduralMesh()
{
    proceduralMesh = NewObject<UProceduralMeshComponent>(this->GetOwner(), FName(TEXT("TrajectoryMesh")));
    proceduralMesh->RegisterComponent();

    if (proceduralMesh)
    {
        TArray<FVector> vertices;
        vertices.Add(FVector(0, 0, 0));
        vertices.Add(FVector(0, 100, 0));
        vertices.Add(FVector(0, 0, 100));

        TArray<int32> Triangles;
        Triangles.Add(0);
        Triangles.Add(1);
        Triangles.Add(2);

        TArray<FVector> normals;
        normals.Add(FVector(1, 0, 0));
        normals.Add(FVector(1, 0, 0));
        normals.Add(FVector(1, 0, 0));

        TArray<FVector2D> UV0;
        UV0.Add(FVector2D(0, 0));
        UV0.Add(FVector2D(10, 0));
        UV0.Add(FVector2D(0, 10));

        TArray<FProcMeshTangent> tangents;
        tangents.Add(FProcMeshTangent(0, 1, 0));
        tangents.Add(FProcMeshTangent(0, 1, 0));
        tangents.Add(FProcMeshTangent(0, 1, 0));

        TArray<FLinearColor> vertexColors;
        vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
        vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
        vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

        proceduralMesh->CreateMeshSection_LinearColor(
            0, vertices, Triangles, normals, UV0, vertexColors, tangents, false);
    }
}

void UTrajectoryComponent::UpdateProceduralMesh()
{
}

void UTrajectoryComponent::SetupSplineMesh()
{
    for (size_t i = 0; i < splineMeh_num; i++)
    {
        FName Name = *(FString(TEXT("TrajectorySplineMesh_")) + FString::FromInt(i));
        USplineMeshComponent* NewSplineMesh = NewObject<USplineMeshComponent>(this->GetOwner(), Name);
        if (NewSplineMesh)
        {
            NewSplineMesh->RegisterComponent();
            NewSplineMesh->SetMobility(EComponentMobility::Type::Movable);
            NewSplineMesh->SetStaticMesh(splineMesh);
            NewSplineMesh->SetMaterial(0, splineMeshMat);
            NewSplineMesh->SetRelativeScale3D(splineMesh_scale);
            NewSplineMesh->SetStartAndEnd(FVector(0), FVector(0), FVector(0), FVector(0));
            splineMesh_arry.Add(NewSplineMesh);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("can not create spline mesh!"));
        }
    }
}

void UTrajectoryComponent::UpdateSplineMesh()
{
    for (size_t i = 0; i < splineMeh_num && i < splineMesh_arry.Num(); i++)
    {
        float SumDis = i * step_distance;
        float NextSumDis = (i + 1) * step_distance;

        if (SumDis < start_loc)
        {
            SumDis = start_loc;
        }
        if (NextSumDis < start_loc)
        {
            NextSumDis = start_loc;
        }

        FVector StartLoc = spline->GetLocationAtDistanceAlongSpline(SumDis, ESplineCoordinateSpace::Type::World);
        FVector StartTangent = spline->GetTangentAtDistanceAlongSpline(SumDis, ESplineCoordinateSpace::Type::World);
        FVector EndLoc = spline->GetLocationAtDistanceAlongSpline(NextSumDis, ESplineCoordinateSpace::Type::World);
        FVector EndTangent = spline->GetTangentAtDistanceAlongSpline(NextSumDis, ESplineCoordinateSpace::Type::World);

        splineMesh_arry[i]->SetStartAndEnd(StartLoc, StartTangent, EndLoc, EndTangent);
    }
}

// Called every frame
void UTrajectoryComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UTrajectoryComponent::UpdateSpline(TArray<struct FSplinePoint> _Points)
{
    if (!spline || !spline->IsValidLowLevel())
    {
        return;
    }

    FOccluderVertexArray Points;
    for (size_t i = 0; i < _Points.Num(); i++)
    {
        FVector NewLoc = _Points[i].Position;
        Points.Add(NewLoc);
    }
    spline->SetSplinePoints(Points, ESplineCoordinateSpace::Type::World);
    // for (size_t i = 0; i < Points.Num(); i++)
    //{
    //     FVector Tangent;
    //     spline->SetTangentAtSplinePoint(i, Tangent, ESplineCoordinateSpace::Type::Local);
    // }

    UpdateMesh();
}

void UTrajectoryComponent::SetRenderActive(bool _IsActive)
{
    if (render_model == ERenderModel::SPLINEMESH)
    {
        for (size_t i = 0; i < splineMesh_arry.Num(); i++)
        {
            splineMesh_arry[i]->SetVisibility(_IsActive);
        }
    }
    else if (render_model == ERenderModel::PROCEDURALMESH)
    {
        // ..
    }
}
