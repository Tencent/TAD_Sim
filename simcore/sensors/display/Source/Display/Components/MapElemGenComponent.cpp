// Fill out your copyright notice in the Description page of Project Settings.

#include "MapElemGenComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/ConfigCacheIni.h"
#ifdef UE_BUILD_DEBUG
#include "DrawDebugHelpers.h"
#endif    //  UE_BUILD_DEBUG

#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UMapElemGenComponent::UMapElemGenComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

// Called when the game starts
void UMapElemGenComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

// Called every frame
void UMapElemGenComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

UStaticMesh* UMapElemGenComponent::LoadMesh(EMapLineShape _Shape)
{
    FString StaticMeshPath;
    FString ShapeName = UEnum::GetValueAsString<EMapLineShape>(_Shape);
    if (GConfig->GetString(TEXT("MapElemGenerateSettings"), *ShapeName, StaticMeshPath, GGameIni))
    {
        UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(NULL, *StaticMeshPath);
        return StaticMesh;
    }

    return nullptr;
}

UInstancedStaticMeshComponent* UMapElemGenComponent::SetupLineShape(EMapLineShape _Shape)
{
    UInstancedStaticMeshComponent** InstancePtr = instancedMap.Find(_Shape);
    if (InstancePtr)
    {
        return *InstancePtr;
    }
    else
    {
        UInstancedStaticMeshComponent* NewInstance = CreateInstance(_Shape);
        if (NewInstance)
        {
            instancedMap.Add(_Shape, NewInstance);
            return NewInstance;
        }
    }
    return NULL;
}

void UMapElemGenComponent::SetupLineColor(FColor _Color)
{
}

UInstancedStaticMeshComponent* UMapElemGenComponent::CreateInstance(EMapLineShape _Shape)
{
    UStaticMesh* Mesh = LoadMesh(_Shape);
    if (Mesh)
    {
        UInstancedStaticMeshComponent* NewInstance =
            NewObject<UInstancedStaticMeshComponent>(GetOwner(), TEXT("MapElemGen_Instance"));
        if (NewInstance)
        {
            NewInstance->RegisterComponent();
            NewInstance->SetStaticMesh(Mesh);
            return NewInstance;
        }
        UE_LOG(LogTemp, Error, TEXT("Create Instance Failed!"));
    }
    return NULL;
}

void UMapElemGenComponent::GetProjectedLines(const TArray<FMapLineDataBase>& _LineBaseArry,
    TArray<FMapLineData>& _LineArry, ECollisionChannel _Channel, float SampleInterval)
{
    for (auto& Elem : _LineBaseArry)
    {
        FMapLineData NewData;
        NewData.begin = Elem.begin;
        NewData.end = Elem.end;

        FVector Dir = (Elem.end - Elem.begin).GetSafeNormal();
        float Length = (Elem.end - Elem.begin).Size();
        if (Length > 0.001)
        {
            float SamplePos = 0;
            while (SamplePos < Length)
            {
                FHitResult Result;
                FVector SamplePoint = Elem.begin + Dir * SamplePos;
                if (GetWorld()->LineTraceSingleByChannel(Result, SamplePoint + FVector(0, 0, tracingRange.Y),
                        SamplePoint + FVector(0, 0, tracingRange.X), _Channel))
                {
                    NewData.curve.AddPoint(SamplePos / Length, Result.ImpactPoint);
                }
#ifdef UE_BUILD_DEBUG
                if (Result.bBlockingHit)
                {
                    DrawDebugLine(GetWorld(), SamplePoint + FVector(0, 0, tracingRange.Y), Result.ImpactPoint,
                        FColor::Green, false, 5);
                }
                else
                {
                    DrawDebugLine(GetWorld(), SamplePoint + FVector(0, 0, tracingRange.Y),
                        SamplePoint + FVector(0, 0, tracingRange.X), FColor::Red, false, 5);
                }
#endif
                if (SampleInterval > 0)
                {
                    SamplePos += SampleInterval;
                }
                else
                {
                    SamplePos = Length;
                }
            }
            FHitResult Result;
            FVector SamplePoint = Elem.end;
            if (GetWorld()->LineTraceSingleByChannel(
                    Result, SamplePoint + FVector(0, 0, 100), SamplePoint + FVector(0, 0, -100), _Channel))
            {
                NewData.curve.AddPoint(1, Result.ImpactPoint);
            }
        }

        _LineArry.Add(NewData);
    }
}

FTransform UMapElemGenComponent::GetLineTransform(
    const FVector& _Begin, const FVector& _End, float _Width, float _Thickness)
{
    FTransform NewTransform;
    FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(_Begin, _End);
    float Length = (_End - _Begin).Size();
    FVector Scale = FVector(Length, _Width, _Thickness);
    NewTransform.SetLocation(_Begin);
    NewTransform.SetRotation(Rotation.Quaternion());
    NewTransform.SetScale3D(Scale);
    return NewTransform;
}

void UMapElemGenComponent::GenerateLine(const FMapLineData& _LineData)
{
    UInstancedStaticMeshComponent* Instance = SetupLineShape(_LineData.params.shape);
    if (Instance)
    {
        // TODO: SetupColor
        for (size_t i = 0; i + 1 < _LineData.curve.Points.Num(); i++)
        {
            FTransform NewTransform = GetLineTransform(_LineData.curve.Points[i].OutVal,
                _LineData.curve.Points[i + 1].OutVal, _LineData.params.width, _LineData.params.thickness);
            Instance->AddInstanceWorldSpace(NewTransform);
        }
    }
}

void UMapElemGenComponent::GenerateLines(TArray<FMapLineData>& _LineDataArry)
{
    for (auto& Elem : _LineDataArry)
    {
        GenerateLine(Elem);
    }
}

void UMapElemGenComponent::GenerateLinesOnMap(
    const TArray<FMapLineDataBase>& _LineDataArry, const FMapLineParam& Params, ECollisionChannel _Channel)
{
    TArray<FMapLineData> LineObjArry;
    GetProjectedLines(_LineDataArry, LineObjArry, _Channel, sampleInterval);

    for (auto& Elem : LineObjArry)
    {
        Elem.params = Params;
    }

    GenerateLines(LineObjArry);
}

void UMapElemGenComponent::CleanLines()
{
    for (auto& Elem : instancedMap)
    {
        Elem.Value->ClearInstances();
    }
}
