#pragma once
#include "CoreMinimal.h"

class Pole
{
public:
    FString WorldDir = TEXT("MsLevel");
    void CreateAndPlace(const TArray<FString>& path);

    bool Pin(FVector& p, float yaw, const FString& id, const FVector& r = FVector(50, 50, 50), float moveto = 0);
    bool Has(const FString& id);

private:
    struct Pos
    {
        FString id;
        FVector pos{0};
        float yaw{0};
        float z_adjust = 0;
    };
    struct Para
    {
        FString gid;    // group
        FString type;
        float width{0};
        float height{500};

        TArray<Pos> pos;
        bool operator==(const FString& _id) const
        {
            return gid == _id;
        }
    };
    TArray<Para> poles;

    struct Component
    {
        struct Object
        {
            double p{0};
            double yaw{0};
            double ox{0};
            double oy{0};
            double oz{0};
            double sx{1};
            double sy{1};
            double sz{1};
            FString path;
        };

        struct Vertical
        {
            double len{0};
            bool repeat{false};
            double ra = 0, rb = 0;
            double ox = 0, oy = 0;
            double sx = 1, sy = 1;
            double sol = 0;    // scale of lengh
            FString path;
            TArray<Object> objects;
        };
        struct Horizontal
        {
            double len{0};
            bool repeat{false};
            double z{0};
            double yaw;
            double oy = 0;
            double sy = 1, sz = 1;
            double sol = 0;    // scale of lengh
            double ra = 0, rb = 0;
            FString path;
            TArray<Object> objects;
        };
        int stencil{0};
        FString name;
        bool gantry{false};
        FString pluginmode;
        double vert_rounding = 0;
        double hori_rounding = 0;
        TArray<Vertical> vertical;
        TArray<Horizontal> horizontal;
        bool pin_height = false;
    };
    TMap<FString, Component> poleComponent;

    bool LoadConfig(bool sub_category = false);
    bool LoadData(const TArray<FString>& fname);
    UObject* CreatePole(const Component& comp, float width, float height, const FString& NewPackageName);

    FVector2D Box0, Box1;
    bool isInBox(const FVector& p);
};
