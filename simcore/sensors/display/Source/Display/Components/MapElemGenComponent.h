// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MapElemGenComponent.generated.h"

UENUM(BlueprintType)
enum class EMapLineShape : uint8
{
    LS_Solid,
    LS_Dotted
};

USTRUCT(BlueprintType)
struct FMapLineParam
{
    GENERATED_USTRUCT_BODY()
public:
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    EMapLineShape shape;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    FColor color;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    float width;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    float thickness;
};

USTRUCT(BlueprintType)
struct FMapLineDataBase
{
    GENERATED_USTRUCT_BODY()
public:
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    FVector begin = FVector(0);
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    FVector end = FVector(0);
};

USTRUCT(BlueprintType)
struct FMapLineData : public FMapLineDataBase
{
    GENERATED_USTRUCT_BODY()
public:
    FMapLineParam params;
    FInterpCurveVector curve;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API UMapElemGenComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UMapElemGenComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    TMap<EMapLineShape, class UInstancedStaticMeshComponent*> instancedMap;

    UStaticMesh* LoadMesh(EMapLineShape _Shape);

    class UInstancedStaticMeshComponent* CreateInstance(EMapLineShape _Shape);

    class UInstancedStaticMeshComponent* SetupLineShape(EMapLineShape _Shape);

    void SetupLineColor(FColor _Color);

    // Use LineCasting to get points that snaped ground
    void GetProjectedLines(const TArray<FMapLineDataBase>& _LineBaseArry, TArray<FMapLineData>& _LineArry,
        ECollisionChannel _Channel, float SampleInterval = 10);

    FTransform GetLineTransform(const FVector& _Begin, const FVector& _End, float _Width, float _Thickness);

    void GenerateLine(const FMapLineData& _LineData);

    void GenerateLines(TArray<FMapLineData>& _LineDataArry);

public:
    UFUNCTION(BlueprintCallable)
    void GenerateLinesOnMap(
        const TArray<FMapLineDataBase>& _LineDataArry, const FMapLineParam& Params, ECollisionChannel _Channel);
    UFUNCTION(BlueprintCallable)
    void CleanLines();

    UPROPERTY(EditAnyWhere)
    float sampleInterval = 10;

    UPROPERTY(EditAnyWhere)
    FVector2D tracingRange = FVector2D(-1000000, 1000000);
};
