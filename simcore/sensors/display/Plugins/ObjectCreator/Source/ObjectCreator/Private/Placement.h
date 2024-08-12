#pragma once
#include "CoreMinimal.h"
#include <cmath>
#include <Map.h>
class FPlacement
{
public:
    FString WorldDir = TEXT("MsLevel");
    void CreateAndPlace(const TArray<FString>& path);
    void SetPole(class Pole* p)
    {
        pole = p;
    }

protected:
    class Pole* pole = nullptr;
    bool LoadData(const TArray<FString>& fnames);
    bool LoadConfig();

private:
    struct SPosData
    {
        int id;
        FString type;

        FVector pos = FVector(0);
        FRotator rot = FRotator(0);
        FVector size = FVector(1.f);
        FString pid;
        TMap<FString, FString> userdata;
        TMap<FString, FString> deviceinfo;
    };
    struct SLineData
    {
        int id;
        FString type;
        FVector size = FVector(1.f);
        TArray<FVector> line;
    };

    struct SPolygonData
    {
        int id;
        FString type;
        FVector size = FVector(1.f);
        TArray<FVector> outline;
        TArray<TArray<FVector>> holes;
    };

    enum SType
    {
        T_POINT,
        T_LINE,
        T_POLYGON
    };
    struct Config
    {
        bool PinTerrain = true;
        double PinRadius = 100;
        double Turnover = 0;
        double RandAngle = 0;
        double RandRot = 0;
        double RandSize = 0;
        double LineLen = 0;
        bool PosTop = false;
        double pinRoad = 0;
        double PlyDensity = 1;
        bool Foliaged = true;
        double offset[3]{0, 0, 0};
        double scale[3]{1, 1, 1};
        bool detect_building = false;
        TArray<FString> AssetPath;
    };

    TMap<FString, Config> AssetConfigs;
    TMap<FString, TArray<SPosData>> PosArray;
    TMap<FString, TArray<SLineData>> LineArray;
    TMap<FString, TArray<SPolygonData>> PolygonArray;
    bool SpawnAssets();
    bool Line2Pos(const Config& cfg, const SLineData& line, TArray<SPosData>& pos, float Al);
    bool Polgyon2Pos(const Config& cfg, const SPolygonData& ply, TArray<SPosData>& pos, const TArray<float>& area);
    bool SpawnFoliageAsset(const TArray<FAssetData>& InAssets, const Config& cfg, const TArray<SPosData>& poss);
    bool SpawnActorAsset(const FString& Folder, const FString& Name, const TArray<FAssetData>& InAssets,
        const Config& cfg, const TArray<SPosData>& poss, const TMap<FString, AActor*>& existNames, bool* cancel = NULL);

    FVector2D Box0, Box1;
    bool isInBox(const FVector& p);
};
