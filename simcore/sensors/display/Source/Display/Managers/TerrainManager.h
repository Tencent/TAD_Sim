// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include <vector>
#include "HadmapManager.h"
#include "TerrainManager.generated.h"

// namespace hadmapue4
//{
//     class HadmapManager;
// }

USTRUCT()
struct FTerrainManagerConfig : public FManagerConfig
{
    GENERATED_BODY();

public:
};

/**
 *
 */
UCLASS()
class DISPLAY_API ATerrainManager : public AManager
{
    GENERATED_BODY()
public:
    ATerrainManager();

    ///* Manager Interface */
    virtual void Init(const FManagerConfig& Config);
    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // class UMapDrawComponent* hadmapVisualComponent = NULL;
    class UMapElemGenComponent* mapLineGenComponent = NULL;

    hadmapue4::HadmapManager* hadmapHandle = NULL;

    bool bGenerateHadMapObject = false;
    std::vector<hadmap::OBJECT_TYPE> objectTypeArry_HadMap;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

protected:
    void SpawnItems();

    TMap<FString, TSubclassOf<AActor>> itemClassMap;

    TArray<AActor*> itemArray;

    void GenerateRoad();
};
