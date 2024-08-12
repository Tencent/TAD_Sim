#pragma once
#include "CoreMinimal.h"

class Polygon
{
public:
    void CreateAndPlace(const FString& fname);

protected:
    bool LoadData(const FString& fname);
    bool LoadConfig();

private:
    struct Ply
    {
        FString type;
        FString id;
        TArray<FVector> plypos;
    };
    TArray<Ply> plys;

    struct Config
    {
        enum UV_TO
        {
            FOLLOW_LINE,
            FOLLOW_SKELETON
        };

        struct UVconfig
        {
            UV_TO uvto;
            double uvlen;
            FString asset;
        };

        TArray<TPair<double, UVconfig>> shrink;
        UVconfig hole;
    };
    TMap<FString, Config> config;

    struct Ply_hole
    {
        TArray<FVector> outline;
        TArray<TArray<FVector>> holes;
    };
    struct RefLine
    {
        TArray<FVector> line;
    };
    TArray<FVector> ShrinkPolygon(const TArray<FVector>& ply, double slen);
    TArray<FVector> PolygonSkeleton(const TArray<FVector>& ply);
    TArray<FVector> SmoothPolygon(const TArray<FVector>& ply);
    RefLine BuildRefline(const TArray<FVector>& line);
};
