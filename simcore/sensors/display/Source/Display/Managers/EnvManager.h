// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "Objects/Weather/SkyActor.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "SunPosition/Public/SunPosition.h"
#include <string>
#include "environment.pb.h"
#include "EnvManager.generated.h"

UENUM(BlueprintType)
enum class EWeatherStateEnum : uint8
{
    WS_Snow UMETA(DisplayName = "Snowy Day"),
    WS_Rain UMETA(DisplayName = "Rainy Day"),
    WS_Sun UMETA(DisplayName = "Sunny Day"),
    WS_Fog UMETA(DisplayName = "Foggy Day"),
    WS_Cloud UMETA(DisplayName = "Cloudy Day")
};

struct FSimEnvOriginInfo
{
    float Hour;

    float Visibility;

    float WindSpeed;

    float RainFall;

    float SnowFall;

    int32 CloudyDensity;

    float Temperature;

    int64 Unix_timestamp;
};

USTRUCT()
struct FSimEnvData
{
    GENERATED_BODY();

public:
    // TimeStamp
    UPROPERTY()
    double timeStamp = 0;

    // Date time
    UPROPERTY()
    FDateTime date = FDateTime(2020, 5, 23, 14, 30, 0, 0);

    // Location
    UPROPERTY()
    double location[3] = {121.1638975600, 31.2806051100, 13.2};

    // Temperature -50-50
    UPROPERTY()
    double temperature = 0;

    // RainFall 0-250
    UPROPERTY()
    double rainFall = 0;

    // SnowFall 0-10
    UPROPERTY()
    double snowFall = 0;

    // Visibility 0-30
    UPROPERTY()
    double visibility = 30;

    // Cloud density 0-10
    UPROPERTY()
    double cloudDensity = 0;

    // Cloud thickness 0-1
    UPROPERTY()
    double cloudThickness = 0;

    // Wind speed 0-61.2
    UPROPERTY()
    double windSpeed = 0;

    // Wind direction
    UPROPERTY()
    FRotator windRotation = FRotator(0, 45, 0);

    // Humidity 0-1
    UPROPERTY()
    double humidity = 0;

    // sunStrength 0-200000
    UPROPERTY()
    double sunStrength = 200000;

    // sunRotation
    UPROPERTY()
    FRotator sunRotation = FRotator(0, 45, 0);

    // moonStrength 0-1
    UPROPERTY()
    double moonStrength = 0;

    // moonRotation
    UPROPERTY()
    FRotator moonRotation = FRotator(0, 45, 0);

    void GetEnvOriginInfo(FSimEnvOriginInfo& OriginInfo);

    // Computing env data;
    bool Compute()
    {
        // Clamp
        rainFall = FMath::Clamp<double>(rainFall, 0.f, 250.f);
        snowFall = FMath::Clamp<double>(snowFall, 0.f, 10.f);
        visibility = FMath::Clamp<double>(visibility, 0.f, 30.f);
        cloudDensity = FMath::Clamp<double>(cloudDensity, 0.f, 10.f);
        windSpeed = FMath::Clamp<double>(windSpeed, 0.f, 61.2);

        // Calcu location , get sun direction and moon direction
        FSunPositionData sunPosData;
        USunPositionFunctionLibrary::GetSunPosition(location[1], location[0], 8.f, false, date.GetYear(),
            date.GetMonth(), date.GetDay(), date.GetHour(), date.GetMinute(), date.GetSecond(), sunPosData);
        sunRotation = FRotator(sunPosData.CorrectedElevation, sunPosData.Azimuth + 0.f, 0.f);
        // Set moon`s rotation as the reverse direction of sun`s rotation
        moonRotation = (sunRotation.Quaternion().GetForwardVector() * (-1)).ToOrientationQuat().Rotator();

        // Compute params
        double SnowFall_Max = (1 - (rainFall / 250.f)) * 10.f;
        snowFall = snowFall < SnowFall_Max ? snowFall : SnowFall_Max;
        double CloudDensity_Min = ((rainFall / 250.f) + (snowFall / 10.f)) * 10;
        cloudDensity = cloudDensity > CloudDensity_Min ? cloudDensity : CloudDensity_Min;
        // double Visibility_Min = (1.f - ((rainFall / 250.f) + (snowFall / 10.f))) * 30;
        // visibility = FMath::Clamp<double>(visibility, 0, Visibility_Min);

        sunStrength = (1.2 - (cloudDensity / 10.f)) * 200000.f;
        sunStrength = FMath::Clamp<double>(sunStrength, 0, 200000.f);
        // moonStrength + sunStrength = 200000
        moonStrength = 200000.f - sunStrength;
        moonStrength = FMath::Clamp<double>(moonStrength, 0, 200000.f);

        return true;
    };

    static bool EarlierData(const FSimEnvData& _A, const FSimEnvData& _B)
    {
        return _A.timeStamp < _B.timeStamp;
    };
};

USTRUCT()
struct FSimEnvTimeline
{
    GENERATED_BODY();

public:
    UPROPERTY()
    FSimEnvData initData;

    UPROPERTY()
    TArray<FSimEnvData> Keys;
};

USTRUCT()
struct FEnvManagerConfig : public FManagerConfig
{
    GENERATED_BODY();

public:
    FSimEnvTimeline timeline;
};

USTRUCT()
struct FEnvManagerIn : public FManagerIn
{
    GENERATED_BODY();

public:
    float visibility;

    sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover cloudLevel;

    float windSpeed;

    float rainFall;

    float snowFall;

    int32 Type;

    float temperature;

    int64 unix_timestamp;

    sim_msg::EnvironmentalConditions environment;
};

USTRUCT()
struct FEnvManagerOut : public FManagerOut
{
    GENERATED_BODY();

public:
};

/**
 *
 */
UCLASS()
class DISPLAY_API AEnvManager : public AManager
{
    GENERATED_BODY()

public:
    AEnvManager();

    /* Manager Interface */
    virtual void Init(const FManagerConfig& Config);
    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaSeconds) override;

    static bool ReadConfigFile(const FString& _Path, FSimEnvTimeline& _Data);
    static bool ParseEnvString(const std::string& Buffer, FSimEnvTimeline& _Data);

    const sim_msg::EnvironmentalConditions& GetEnvPB()
    {
        return EnvironmentInfo;
    }

private:
    // float timeOfDay;
    // EWeatherStateEnum weatherState = EWeatherStateEnum::WS_Sun;
    // EWeatherStateEnum weatherState_Destin = EWeatherStateEnum::WS_Sun;

    bool bHasEnvEditPermission = true;

    double timeStamp_Pre = 0;
    double timeStamp = 0;

    FSimEnvTimeline timeline;
    FSimEnvData envData_Current;
    sim_msg::EnvironmentalConditions EnvironmentInfo;

    void UpdateTimeline(double _TimeStamp);
    FSimEnvData Interpolation(const FSimEnvData& _From, const FSimEnvData& _To, double _TimeStamp);
    void ApplyEnvData(const FSimEnvData& _Data);

    bool ParameterChangeTo(float& _From, float _To, float _DeltaTime);

    // float Scale_Sun;
    // float Scale_Cloud;
    // float Scale_Fog;
    // float Scale_Rain;
    // float Scale_Snow;

    class ADisplayPlayerController* PC = NULL;
    class AVehiclePawn* egoVehicle = NULL;
    class UParticleSystem* particle_Rain = NULL;
    class UParticleSystem* particle_Snow = NULL;
    TWeakObjectPtr<class AEmitter> ParticleRain = NULL;
    TWeakObjectPtr<class AEmitter> ParticleSnow = NULL;
    class UParticleSystemComponent* particleComponent_Rain = NULL;
    class UParticleSystemComponent* particleComponent_Snow = NULL;
    class AExponentialHeightFog* heightFog = NULL;
    class AWindDirectionalSource* windSource = NULL;
    TSubclassOf<ASkyActor> skyBoxClass = NULL;
    ASkyActor* skyBox = NULL;

    // StreetLights
    TArray<class AStreetLight*> streetLightArry;

    TWeakObjectPtr<class AEnvDepthTrace> EnvDepthTrace;

    bool bBeingDay = false;
    bool bBeingNight = false;
    float sunriseTime = 6;
    float sunsetTime = 18;

public:
    // UFUNCTION(BlueprintCallable, Category = "DayNightSystem")
    //     void SetTimeOfDay(float _NewTime);

    // UFUNCTION(BlueprintCallable, Category = "DayNightSystem")
    //     float GetTimeOfDay()const;

    // UFUNCTION(BlueprintCallable, Category = "WeatherSystem")
    //     void SetWeatherState(EWeatherStateEnum _NewState);

    // UFUNCTION(BlueprintCallable, Category = "WeatherSystem")
    //     EWeatherStateEnum GetWeatherState()const;

    UMaterialParameterCollection* EnvParamsCollection;

    UMaterialParameterCollectionInstance* EnvParamsCollectionInstance;

    UPROPERTY(config)
    bool bOutputEnvLog = false;

    bool bFirstOutputLog = true;

    UPROPERTY(EditAnyWhere, Category = "WeatherSystem")
    float changeSpeed = 0.2;

    UFUNCTION(BlueprintCallable, Category = "WeatherSystem")
    bool ParameterChangeTo_BP(float _From, float _To, float _DeltaTime, float _Speed, float& _OutPut);

    FSimEnvData GetCurrentEnvData() const
    {
        return envData_Current;
    }
    void SetEnvDataEditPermission(bool _IsActive);
    bool GetEnvDataEditPermission() const
    {
        return bHasEnvEditPermission;
    }
    void ExternalInputEnvData(const FSimEnvData& _Data);

    void UpdateEnvEffectLocation(const FName& CameraName);

public:
    // static FEnvManagerConfig ReadFile(const FString& _Path);
    FORCEINLINE bool IsInited() const
    {
        return bIsInited;
    }

    static TMap<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float> CloudLevelDensityMapping;

protected:
    bool bIsInited = false;

    bool bReceivedPB = false;
};
