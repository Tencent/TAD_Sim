// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ProceduralMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoaderBPFunctionLibrary.generated.h"

class UStaticMesh;

UENUM()
enum class EMeshType : uint8
{
    /* 车辆 */
    MT_VEHICLE,
    /* 行人 */
    MT_CREATURE,
    /* 非机动车 */
    MT_BIKE,
    /* 障碍物 */
    MT_OBSTACLE,
    /* 地图放置物 */
    MT_MAP_OBJ,

    MT_Other
};

USTRUCT(BlueprintType)
struct FMeshMaterialList
{
    GENERATED_USTRUCT_BODY()

    FMeshMaterialList()
    {
    }

    FMeshMaterialList(UMaterialInterface* _Mat_Opaque, UMaterialInterface* _Mat_Translucent)
        : Mat_Opaque(_Mat_Opaque), Mat_Translucent(_Mat_Translucent)
    {
    }

    // 不透明材质
    UPROPERTY()
    UMaterialInterface* Mat_Opaque;

    // 半透明材质
    UPROPERTY()
    UMaterialInterface* Mat_Translucent;
};

USTRUCT(BlueprintType)
struct FMeshInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FVector> Vertices;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    /** Vertices index */
    TArray<int32> Triangles;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FVector> Normals;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FVector2D> UV0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FVector2D> UV1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FVector2D> UV2;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FVector2D> UV3;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FLinearColor> VertexColors;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    TArray<FProcMeshTangent> Tangents;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
    FTransform RelativeTransform;
};

// USTRUCT(BlueprintType)
struct FReturnedData
{
    // GENERATED_USTRUCT_BODY()

public:
    /**/
    bool bSuccess;
    /** Contain Mesh Count  */
    int32 NumMeshes;

    TArray<FMeshInfo> meshInfo;

    TArray<TPair<int32, FString>> BaseColorTexPath;

    TArray<TPair<int32, FString>> ORMTexPath;

    TArray<TPair<int32, FString>> NormalTexPath;

    TMap<FString, FVector> SocketInfo;
};

USTRUCT(BlueprintType)
struct FLoadMeshData
{
    GENERATED_USTRUCT_BODY()

public:
    FString LoadPath;

    UPROPERTY();
    TWeakObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY();
    TMap<int32, UMaterialInterface*> MIDs;

    UPROPERTY();
    TArray<UTexture2D*> Texs;
};

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class RUNTIMEMESHLOADER_API URuntimeMeshLoader : public UObject
{
    GENERATED_BODY()
public:
    static URuntimeMeshLoader* Get()
    {
        return Instance;
    }
    static void SetInstance(URuntimeMeshLoader* NewInstance)
    {
        if (Instance && Instance->IsValidLowLevel())
        {
            Instance->RemoveFromRoot();
        }
        Instance = NewInstance;
    }

    UFUNCTION(BlueprintCallable)
    void Init();

    UFUNCTION(BlueprintCallable)
    UStaticMesh* LoadStaticMeshFromFBX(const FString& FilePath, bool bTexByRef, EMeshType Type = EMeshType::MT_Other);

private:
    FReturnedData LoadMesh(const FString& filepath);

    bool IsOpaqueTexture(UTexture2D* Tex);

private:
    UPROPERTY()
    TMap<FName, FLoadMeshData> LoadMeshCache;

    UPROPERTY()
    TMap<EMeshType, FMeshMaterialList> StandardMaterials;

    static URuntimeMeshLoader* Instance;
};
