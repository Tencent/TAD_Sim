// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Materials/Material.h"
#include "UObject/NoExportTypes.h"
#include "OpenDriveBaseStruct.generated.h"

// 各物体材质表
USTRUCT(BlueprintType)
struct FRoadObjectMaterialConfig : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    // Style|ArtConfig
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadObjectMaterialConfig")
    UMaterialInterface* UsingMaterial;
};

// static mesh材质表
USTRUCT(BlueprintType)
struct FRoadObjectStaticMeshConfig : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    // Style|ArtConfig
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadObjectStaticMeshConfig")
    UStaticMesh* UsingMesh;
};

// 车道线颜色配置表
USTRUCT(BlueprintType)
struct FRoadMarkColorConfig : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    // Style|ArtConfig
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadMarkColorConfig")
    FLinearColor UsingColor;
};
