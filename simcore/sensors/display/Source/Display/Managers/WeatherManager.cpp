// Fill out your copyright notice in the Description page of Project Settings.

#include "WeatherManager.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogWeatherManager, Log, All);

AWeatherManager::AWeatherManager()
{
    // PrimaryActorTick.bCanEverTick = true;
    ////MaterialParameterCollection'/Game/Weather/Assets/Materials/Weather_adjustment.Weather_adjustment'
    // weatherParamCollection = LoadObject<UMaterialParameterCollection>(NULL,
    // TEXT("MaterialParameterCollection'/Game/Weather/Assets/Materials/Weather_adjustment.Weather_adjustment'")); if
    // (!weatherParamCollection)
    //{
    //     UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: Cant load weatherParamCollection!"));
    // }
    ////Load skybox BP.
    // skyBoxClass = LoadClass<ASkyBoxBase>(NULL,
    // TEXT("Blueprint'/Game/UltraDynamicSky/Blueprints/Ultra_Dynamic_Sky_BP.Ultra_Dynamic_Sky_BP_C'")); if
    // (!skyBoxClass)
    //{
    //     UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: Cant load skyBoxBP!"));
    // }
}

void AWeatherManager::Init(const FManagerConfig& Config)
{
    const FWeatherManagerConfig* WeatherConfig = Cast_Data<const FWeatherManagerConfig>(Config);
    if (!WeatherConfig)
    {
        UE_LOG(SimLogWeatherManager, Error, TEXT("Can Not Get WeatherConfig!"));
        return;
    }
    check(WeatherConfig);
    weatherManagerConfig = *WeatherConfig;

    UpdateWeather(0);
}

void AWeatherManager::ReadFile(const FString& _Path)
{
}

void AWeatherManager::UpdateWeather(double _TimeStamp)
{
}
