#pragma once
#include "CoreMinimal.h"

class TrafficSign
{
public:
    void SetPole(class Pole* p)
    {
        pole = p;
    }
    void CreateAndPlace(const TArray<FString>& path);

    FString PackageDir = TEXT("/Game/3DRtest/Auto_out/TrafficSign/");
    FString WorldDir = TEXT("MsLevel");

protected:
    class Pole* pole = nullptr;
    bool LoadData(const TArray<FString>& fdir);
    bool LoadConfig();

private:
    struct Sign
    {
        FString type;
        FString id;
        FVector pos{0};
        FRotator rot{0};
        float width{100};
        float height{100};
        float thickness{10};
        FString pid;
    };
    TMap<FString, TArray<Sign>> signs;
    TMap<FString, FString> signres;
    TMap<FString, int> stencils;

    bool ParseText(const FString& fname, const FString& s, TArray<TArray<FString>>& sdat);
    bool CreateAtlas(const TArray<FString>& fimg, const FString& outFname);
    bool CreateNormal(const FString& fimg, const FString& outFname);
    UObject* CreateMeshBp(const FString& outFname, UObject* mesh, UObject* templateObj);
    UObject* CreateMesh(const FString& outFname, UObject* mat, UObject* templateObj);
    UObject* CreateTexture(const FString& fimg);
    void CreateTextures(const TArray<FString>& fimg);
    UObject* CreateMatrial(const FString& outFname, UObject* img, UObject* nor, UObject* templateObj);
    void SpawnActorAsset(UObject* obj, const Sign& s, const TSet<FString>& existNames, TMap<FString, int>& addNames);
    void SpawnActorAsset(const Sign& s, const TSet<FString>& existNames, TMap<FString, int>& addNames);

    FVector2D Box0, Box1;
    bool isInBox(const FVector& p);
};
