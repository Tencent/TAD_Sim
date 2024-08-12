// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Engine/DataTable.h"
#include "hadmap.h"
#include "mapengine/hadmap_engine.h"
#include "mapengine/hadmap_codes.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "types/map_types.h"
#include "MapGeneratedActor.generated.h"

struct FMapObjInfo
{
public:
    TWeakObjectPtr<AActor> Actor;

    hadmap::OBJECT_TYPE ObjType;

    hadmap::OBJECT_SUB_TYPE ObjSubType;
};

USTRUCT()
struct FProperties
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int32 TL_ID;
    UPROPERTY()
    float YAW;
};

USTRUCT()
struct FGeoVector
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<float> Value;
};

USTRUCT()
struct FGeometryData
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FString type;
    UPROPERTY()
    TArray<float> coordinates;
};

USTRUCT()
struct FTrafficLightInfo
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FString type;

    UPROPERTY()
    FGeometryData geometry;

    // UPROPERTY()
    // FProperties properties;
};

USTRUCT()
struct FGeo
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FString type;
    UPROPERTY()
    TArray<FTrafficLightInfo> features;
};

struct FRoadMarkData
{
    // 车道线顶点数据 | 车道线索引数据 | 车道线法线数据 | 车道线UV数据
    TArray<FVector> RoadMarkVertices;
    TArray<int32> RoadMarkTriangles;
    TArray<FVector> RoadMarkNormals;
    TArray<FVector2D> RoadMarkUV0s;
    int32 LineNum = 0;
    int32 RocordIndex = 0;
};

struct VertexInfo
{
    int ID;
    FVector InstanceNormal;
    FVector2D InstanceUV;    // 此三角形中顶点的UV
    VertexInfo(int InID, FVector InInstanceNormal, FVector2D InInstanceUV)
        : ID(InID), InstanceNormal(InInstanceNormal), InstanceUV(InInstanceUV)
    {
    }
};

struct LaneBoundaryInfo
{
    hadmap::txLaneBoundaryPtr BoundaryPtr;
    hadmap::txLanePtr LanePtr;
    hadmap::PointVec Points;
    size_t RoadId;
    size_t SectionId;
};

struct SceneFaceData
{
    std::vector<std::vector<FVector>> veritics;
    std::vector<std::vector<FVector>> normals;
    std::vector<FVector2D> uvs;
    std::vector<FVector> normal_left;
    std::vector<std::vector<size_t>> faces;
    std::string matrix;
    std::string name;

    size_t lane_num = 0;
    size_t lane_vnum = 0;
};

UCLASS(config = game)
class AUTOROAD_API AMapGeneratedActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AMapGeneratedActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // 蓝图调用函数
public:
    void DrawMap(FString pathIput, FString DecryptFilePath, bool bArtLevel,
        const TMap<FString, TPair<FString, FVector>>& ModelData, const FString& ModelRootPath);

    UFUNCTION(BlueprintCallable)
    void DebugGetLane(FVector Pos);

    void DrawRoadDetails(TArray<LaneBoundaryInfo>& lane_bdids);
    void DrawMarkDetails(TArray<LaneBoundaryInfo>& lane_bdids);
    void DrawJunctionDetails();
    void DrawObjDetails();

    ///
    void DrawLongitudinalDecelerationItem(const FVector& StartPoint, TArray<FVector>& VertexPoints,
        TArray<int32>& VertexTranigles, TArray<FVector>& VertexNormals, TArray<FVector2D>& UVs, const FVector& RoadDir,
        const FVector& CenterDir, const float Width, const float Length, int32& VertexIndex);

    UFUNCTION(BlueprintCallable)
    void DrawLateralDecelerationLine(const TArray<FVector>& LeftLinePoints, const TArray<FVector>& RightLinePoints);

    UFUNCTION(BlueprintCallable)
    void DrawLongitudinalDecelerationLine(
        const TArray<FVector>& LeftLinePoints, const TArray<FVector>& RightLinePoints);

    UFUNCTION(BlueprintCallable)
    void DrawSemicircleLine(const TArray<FVector>& LeftLinePoints, const TArray<FVector>& RightLinePoints);

    UFUNCTION(BlueprintCallable)
    void DrawNonAreaLine(TArray<FVector>& VertexPoints, TArray<int32>& VertexTranigles, TArray<FVector>& VertexNormals,
        TArray<FVector2D>& UVs, FVector StartPoint, const FVector& LineDir, const float MaxLength);

    UFUNCTION(BlueprintCallable)
    void DrawNonArea(FVector Location, FRotator Rotation, float Width, float Length);

    // UFUNCTION(BlueprintCallable)
    void DrawDirectionLane(const TArray<FVector>& _leftPoints, const TArray<FVector>& _rightPoints, bool bVariable);

    UFUNCTION(BlueprintCallable)
    void DrawGuideLine(const FVector& V1, const FVector& V2, const FVector& V3, const FVector& V4);
    ///

#if WITH_EDITOR
    UFUNCTION(BlueprintCallable)
    void GenerateGeo();
#endif
    FTransform GetRoadTrnasfrom(FVector Pos, FRotator Rot, FVector Box);

    void GetObjRangeGeoms(hadmap::txObjectPtr Obj, TArray<FVector>& LeftPoints, TArray<FVector>& RightPoints);

    void LonLatToLocal(double& _Lon, double& _Lat, double& _Alt);
    // Instance可修改参数
public:
    UPROPERTY()
    class UMapModelComponent* MapModelComponent;

    UPROPERTY(EditAnywhere)
    double RefX;
    UPROPERTY(EditAnywhere)
    double RefY;
    UPROPERTY(EditAnywhere)
    double RefZ;
    UPROPERTY(EditAnywhere)
    bool bRemovePoints = true;

    // 取样点剔除的相差角度
    UPROPERTY(EditAnywhere, Config)
    float AngleDeltaToRemovePoint = 0.01;

    UPROPERTY(EditAnywhere, Config)
    int32 MaxRenderLaneNum = 100;

    UPROPERTY(EditAnywhere, Config)
    int32 MaxRenderLaneVertices = 30000;

    UPROPERTY(EditAnywhere, Config)
    int32 MaxRenderLineNum = 20;

    UPROPERTY(EditAnywhere, Config)
    int32 MaxRenderLineVertices = 30000;

    UPROPERTY(EditAnywhere)
    bool ShowObjectOnly = false;

    UPROPERTY(Config)
    bool ShowObjectOnArtLevel = false;

    UPROPERTY(Config)
    bool ShowObjectOnAutoRoad = true;

    UPROPERTY(Config)
    bool EnableModifyMapObjScale = false;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
    float CurbHeight = 0.1;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
    float CurbWidth = 0.2;

    UPROPERTY(EditAnywhere)
    bool GrassHide = true;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
    float GrassZOff = 0.1;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
    float RoadSideGrassWidth = 1;

    UPROPERTY(EditAnywhere)
    bool UnknowObjectHide = true;

    UPROPERTY(EditAnywhere)
    bool reverse;

    UPROPERTY(Config)
    bool bGenearateRoadOnXY = false;

    UPROPERTY(EditAnywhere)
    FString GeoPath;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> GeoClass;

    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> pillar_points;

    UPROPERTY(Config)
    int32 pillar_interval = 50;

    UFUNCTION(BlueprintImplementableEvent)
    void RenderRoadSplines(const TArray<FHitResult>& HitResList);

    // 获取材质 Mat_Cylinder 参数
    UFUNCTION(BlueprintCallable)
    float BPGetBlendHeight();

    UFUNCTION(BlueprintCallable)
    AActor* GeneareteCrossWalkLine(FVector Location, FRotator Rotation, float Width, float Length, bool bDrawArrow);

    UFUNCTION(BlueprintCallable)
    void DrawCrossWalkLine(
        FVector Location, FRotator Rotation, float Width, float Length, bool bDrawArrow, bool bDouble);

    class UStaticMesh* GeneareteCrossWalkLineArrow(float Length, float Width);

private:
    // HadMap_SDK
    void GenerateRoad(hadmap::txMapHandle* pHandle, const std::string& filename, double x_reference, double y_reference,
        double z_reference, bool gcj02 = false, bool fix_connect = false, double road_expand_right = 0,
        double road_expand_left = 0, double minLen = 0);
    // 创建道路物件
    void GenerateObject(hadmap::txMapHandle* pHandle, const std::string RoadId, const std::string LaneId,
        const FName& MaterialName, const FVector Location, const double Length, const double Width,
        const FVector RelativeLocation, int interval = 1);

    // 道路物件引用SplineStaticMesh
    UPROPERTY(EditAnywhere)
    class UStaticMesh* SplineStaticMesh;

    // 根据材质类型创建动态材质
    UMaterialInstanceDynamic* CreateMaterial(const FName& MaterialName);

    UMaterialInterface* CreateStaticMaterial(const FName& MaterialName);

    // 根据object名字获得StaticMesh
    UStaticMesh* GetStaticMesh(const FName& StaticMeshName);

    TSubclassOf<AActor> GetActor(const FString& ActorName);
    // 根据colorname 获得LinearColor
    FLinearColor GetLinerColor(const FName& ColorName);

    UStaticMesh* CreateStaticMesh(const TArray<FVector>& Vertices, const TArray<int32>& Triangles,
        const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, UMaterialInterface* UsingMaterial,
        bool CastShadow, uint8 bCastDynamicShadow, uint8 bCastStaticShadow, float RelativeZ,
        bool bCreateCollision = true, bool bLod = false, bool bNewActor = false);

    // 创建ProcedualMesh
    void CreateProcedualMesh(const TArray<FVector>& Vertices, const TArray<int32>& Triangles,
        const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, UMaterialInterface* UsingMaterial,
        bool CastShadow, uint8 bCastDynamicShadow, uint8 bCastStaticShadow, float RelativeZ,
        bool bCreateCollision = true);

    void ConvertParkRenderData(const FVector& V1, const FVector& V2, const FVector& V3, const FVector& V4,
        TArray<FVector>& Park_Vertices, TArray<int32>& Park_Triangles, TArray<FVector>& Park_Normals,
        TArray<FVector2D>& Park_UV0, float Width, bool bBroken);

    bool SetGeomsFromRoad(const hadmap::txLanePtr lanePtr, hadmap::txObjectPtr obj, TArray<FVector>& LeftBoundaryPoints,
        TArray<FVector>& RightBoundaryPoints);

    static void InitTypeStrMap();

private:
    // 材质配置表
    UPROPERTY()
    UDataTable* RoadObjectMaterialConfig;

    // TODO:避免HardCode，材质Key值(后续可按配置表或xml文件等修改)
    UPROPERTY(Config)
    FName MatRoad = TEXT("Road");
    UPROPERTY(Config)
    FName MatRoadCurb = TEXT("RoadBoundary");
    UPROPERTY(Config)
    FName MatRoadObject = TEXT("Road");
    UPROPERTY(Config)
    FName MatGrass = TEXT("Grass");
    UPROPERTY(Config)
    FName MatRoadParking = TEXT("road_parking");
    UPROPERTY(Config)
    FName MatGround = TEXT("Ground");
    UPROPERTY(Config)
    FName MatTunnel = TEXT("Tunnel");
    UPROPERTY(Config)
    FName MatYellow = TEXT("Line_Yellow");
    UPROPERTY(Config)
    FName MatWhite = TEXT("Line_White");
    // 材质/模型默认Index
    const int32 ElementIndex = 0;

    // 承载道路物体的Mesh数组
    TArray<UProceduralMeshComponent*> RoadObjectMeshes;

    TArray<hadmap::roadpkid> HasGeneratedTunnelRoadID;

    // 车道线颜色配置表
    UPROPERTY()
    UDataTable* RoadObjectStaticMeshConfig;

    // 道路物件表
    UPROPERTY()
    UDataTable* RoadMarkColorConfig;

    bool bOldVersion = false;

    bool bShowObject = true;

    TMap<FName, FMapObjInfo> MapObjs;

    static TMap<hadmap::OBJECT_TYPE, FString> ObjectTypeStrMap;
    static TMap<hadmap::OBJECT_SUB_TYPE, FString> ObjectSubTypeStrMap;

private:
    void PointDilution(hadmap::PointVec& points);
    void GenearateTunnel(hadmap::txObjectPtr Obj);
    /// 求两个向量交点算法
    int dblcmp(double a, double b);
    double dot(double x1, double y1, double x2, double y2);
    int point_on_line(FVector a, FVector b, FVector c);
    double cross(double x1, double y1, double x2, double y2);
    double ab_cross_ac(FVector a, FVector b, FVector c);
    int ab_cross_cd(FVector a, FVector b, FVector c, FVector d);

    hadmap::txMapHandle* Handle;
    /// 求两个向量交点END

    void CreateSpline(hadmap::txMapHandle* pHandle, double x_reference, double y_reference, double z_reference);

    std::vector<SceneFaceData> scene;
    TArray<LaneBoundaryInfo> lane_bdids;
    std::set<hadmap::laneboundarypkid> bdl_bdldsL;
    std::set<hadmap::laneboundarypkid> bdl_bdldsR;
    std::vector<SceneFaceData> scene_curb;
    std::map<std::pair<size_t, size_t>, size_t> section_map_curb;

    UPROPERTY()
    TMap<FName, UMaterialInstanceDynamic*> MID_Map;
};
