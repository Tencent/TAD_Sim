// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "WeatherManager.generated.h"

class ASkyActor;

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    WS_SNOW UMETA(DisplayName = "Snowy Day"),
    WS_RAIN UMETA(DisplayName = "Rainy Day"),
    WS_SUN UMETA(DisplayName = "Sunny Day"),
    WS_FOG UMETA(DisplayName = "Foggy Day"),
    WS_CLOUD UMETA(DisplayName = "Cloudy Day")
};

UENUM(BlueprintType)
enum class ESeasonState : uint8
{
    SS_SPRING UMETA(DisplayName = "Spring Season"),
    SS_SUMMER UMETA(DisplayName = "Summer Season"),
    SS_AUTUMN UMETA(DisplayName = "Autumn Season"),
    SS_WINTER UMETA(DisplayName = "Winter Season")
};

USTRUCT()
struct FWeatherData
{
    GENERATED_BODY()
public:
    float temperature = 20.f;
    float timeOfYear = 0.f;
    FVector directionOfSun = FVector(0);
    FVector directionOfMoon = FVector(0);
    FVector forceOfWind = FVector(0);

    // float sun;
    //  Sun
    //  Moon
    //  Cloud
    //  Wind
    //  Emitter
};

USTRUCT()
struct FWeatherAttribute
{
    GENERATED_BODY()
public:
    FString name;
    FString value;
};

USTRUCT()
struct FWeatherCustomState
{
    GENERATED_BODY()
public:
    FString name;
    TArray<FWeatherAttribute> attributeArray;
};

USTRUCT()
struct FWeatherAttributeKey
{
    GENERATED_BODY()
public:
    FWeatherAttribute attribute;
    double time;
};

USTRUCT()
struct FWeatherStateKey
{
    GENERATED_BODY()
public:
    FString name;
    double time;
};

USTRUCT()
struct FWeatherModeKey
{
    GENERATED_BODY()
public:
    FString name;
    FString param;
};

USTRUCT()
struct FWeatherManagerConfig : public FManagerConfig
{
    GENERATED_BODY();

public:
    TArray<FWeatherCustomState> customStateArry;
    TArray<FWeatherAttributeKey> attributeKeyArry;
    TArray<FWeatherStateKey> stateKeyArry;
    TArray<FWeatherModeKey> modeKeyArry;
};

/**
 *
 */
UCLASS()
class DISPLAY_API AWeatherManager : public AManager
{
    GENERATED_BODY()
public:
    AWeatherManager();

public:
    virtual void Init(const FManagerConfig& Config);

protected:
    FWeatherManagerConfig weatherManagerConfig;
    FWeatherData weatherData;
    ASkyActor* sky = NULL;
    UMaterialParameterCollection* weatherParamCollection;
    UMaterialParameterCollectionInstance* weatherParamCollectionInstance;

    EWeatherState weatherState = EWeatherState::WS_SUN;

    void ReadFile(const FString& _Path);
    void UpdateWeather(double _TimeStamp);
    // bool ParameterChangeTo_BP(float _From, float _To, float _DeltaTime, float _Speed, float &_OutPut);
};
