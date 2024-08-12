// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Sensors/SensorInterface.h"
#include "HadmapManager.h"
#include "FovFilter.generated.h"

struct FSimUpdateIn;
struct FSimUpdateOut;
USTRUCT()
struct FovFilterConfig : public FSensorConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    double fov_Horizontal = 90;
    UPROPERTY()
    double fov_Vertical = 60;

    UPROPERTY()
    double dis_range = 20000;
};
/**
 *
 */

class DISPLAY_API Age
{
public:
    void YearEnd();
    int operator()(int id);

private:
    TMap<int, int> _data;
    TMap<int, int> _tmp;
};

class DISPLAY_API FovFilter
{
public:
    bool Init(const FovFilterConfig& config_, hadmapue4::HadmapManager* hadmapHandle_);
    void Update(const FSimUpdateIn& simIn, FSimUpdateOut& simOuts);

protected:
    bool IsInView(const FVector& loc, const FRotator& rrt, const FVector& object);
    FovFilterConfig config;
    hadmapue4::HadmapManager* hadmapHandle;

    Age age[3];
};

class DISPLAY_API FovFilterManager
{
public:
    bool Init(const FString& configFilePath, hadmapue4::HadmapManager* hadmapHandle_);
    void Update(const FSimUpdateIn& simIn, TArray<FSimUpdateOut>& simOuts);

protected:
    TArray<FovFilter> filters;
};
