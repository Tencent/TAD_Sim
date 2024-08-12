// Fill out your copyright notice in the Description page of Project Settings.

#include "EnvManager.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "XmlFile.h"
#include "Framework/DisplayPlayerController.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/WindDirectionalSource.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/WindDirectionalSourceComponent.h"
#include "Objects/StreetLights/StreetLight.h"
#include "Env/EnvDepthTrace.h"
#include "Framework/DisplayGameStateBase.h"
#include "scene.pb.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogEnvManager, Log, All);

TMap<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float> AEnvManager::CloudLevelDensityMapping = {
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_UNKNOWN,
        0),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_OTHER,
        0),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_ZERO_OKTAS,
        0),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_ONE_OKTAS,
        1.5),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_TWO_OKTAS,
        1.5),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_THREE_OKTAS,
        1.5),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_FOUR_OKTAS,
        4.3),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_FIVE_OKTAS,
        4.3),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_SIX_OKTAS,
        6),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_SEVEN_OKTAS,
        6),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_EIGHT_OKTAS,
        6),
    TPair<sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover, float>(
        sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover::
            EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_SKY_OBSCURED,
        6)};

void FSimEnvData::GetEnvOriginInfo(FSimEnvOriginInfo& OriginInfo)
{
    OriginInfo.Hour = date.GetHour();
    OriginInfo.Visibility = visibility * 1000.f;

    OriginInfo.WindSpeed = windSpeed;

    OriginInfo.RainFall = rainFall / 255.f;

    OriginInfo.SnowFall = snowFall / 10.f;

    OriginInfo.CloudyDensity = 1.f;

    OriginInfo.Temperature = temperature;

    OriginInfo.Unix_timestamp = date.ToUnixTimestamp() * 1000;

    for (auto Elem : AEnvManager::CloudLevelDensityMapping)
    {
        if (Elem.Value >= cloudDensity)
        {
            OriginInfo.CloudyDensity = Elem.Key;
        }
    }
}

AEnvManager::AEnvManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    // MaterialParameterCollection'/Game/Weather/Assets/Materials/Weather_adjustment.Weather_adjustment'
    EnvParamsCollection = LoadObject<UMaterialParameterCollection>(
        NULL, TEXT("MaterialParameterCollection'/Game/Art/Public/SH/EnvironmentParams.EnvironmentParams'"));
    if (!EnvParamsCollection)
    {
        UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: Cant load EnvironmentParams!"));
    }
    // Load skybox BP.
    skyBoxClass = LoadClass<ASkyActor>(
        NULL, TEXT("Blueprint'/Game/UltraDynamicSky/Blueprints/Ultra_Dynamic_Sky_BP.Ultra_Dynamic_Sky_BP_C'"));
    if (!skyBoxClass)
    {
        UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: Cant load skyBoxBP!"));
    }

    // Load emitters
    particle_Rain =
        LoadObject<UParticleSystem>(NULL, TEXT("ParticleSystem'/Game/Weather/VFX_Weather/VFX_Weather_Particles/"
                                               "VFX_Weather_Rain_Heavy_Local.VFX_Weather_Rain_Heavy_Local'"));
    particle_Snow =
        LoadObject<UParticleSystem>(NULL, TEXT("ParticleSystem'/Game/Weather/VFX_Weather/VFX_Weather_Particles/"
                                               "VFX_Weather_Snow_Heavy_Local.VFX_Weather_Snow_Heavy_Local'"));
    if (!particle_Rain)
    {
        UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: Cant load particle_Rain!"));
    }
    if (!particle_Snow)
    {
        UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: Cant load particle_Snow!"));
    }

    // CloudLevelDensityMapping init sort
    AEnvManager::CloudLevelDensityMapping.KeySort([](float A, float B) { return A < B; });
}

void AEnvManager::BeginPlay()
{
    Super::BeginPlay();

    check(GetWorld());
    check(GetWorld()->GetFirstPlayerController());
    PC = Cast<ADisplayPlayerController>(GetWorld()->GetFirstPlayerController());

    if (PC)
    {
        PC->OnEgoViewChange.AddUObject(this, &AEnvManager::UpdateEnvEffectLocation);
    }

    // Get EnvParamsCollectionInstance.
    if (EnvParamsCollection)
    {
        EnvParamsCollectionInstance = GetWorld()->GetParameterCollectionInstance(EnvParamsCollection);
        if (!EnvParamsCollectionInstance)
        {
            UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: Cant get EnvParamsCollectionInstance!"));
        }
    }

    // Get skyBox instance.
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyActor::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0 && FoundActors[0])
        {
            skyBox = Cast<ASkyActor>(FoundActors[0]);
        }
        if (skyBox)
        {
            skyBox->envManager = this;
        }
        else
        {
            UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: skyBox is NULL!"));
        }
    }

    // Get Height Fog
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0 && FoundActors[0])
        {
            heightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
        if (!heightFog)
        {
            UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: heightFog is NULL!"));
        }
    }

    // Get Wind
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWindDirectionalSource::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0 && FoundActors[0])
        {
            windSource = Cast<AWindDirectionalSource>(FoundActors[0]);
        }
        if (!windSource)
        {
            UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: windSource is NULL!"));
        }
    }

    // Get StreetLight
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStreetLight::StaticClass(), FoundActors);
        for (auto Elem : FoundActors)
        {
            streetLightArry.Add(Cast<AStreetLight>(Elem));
        }
        UE_LOG(SimLogEnvManager, Warning, TEXT("Get StreetLight`s num: %d"), streetLightArry.Num());
    }

    {
        EnvDepthTrace = GetWorld()->SpawnActor<AEnvDepthTrace>();
    }

    bIsInited = true;
}

// Called every frame
void AEnvManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PC && ParticleRain.IsValid() && ParticleSnow.IsValid())
    {
        FVector Location = PC->PlayerCameraManager->GetCameraLocation() + FVector(0, 0, 1000);
        ParticleRain->SetActorLocation(Location);
        ParticleSnow->SetActorLocation(Location);
        if (EnvDepthTrace.IsValid())
        {
            EnvDepthTrace->SetActorLocation(Location);
            // UE_LOG(SimLogEnvManager, Warning, TEXT("EnvDepthTrace %f, %f, %f"), Location.X, Location.Y, Location.Z);
        }
    }

    // UpdateWeather(DeltaTime);
}

bool AEnvManager::ReadConfigFile(const FString& _Path, FSimEnvTimeline& _Data)
{
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*_Path))
    {
        UE_LOG(SimLogEnvManager, Warning, TEXT("Can`t Read EnvManager Config File! Directory Don`t Exists!"));
        return false;
    }

    FXmlFile xmlfile(_Path);
    if (xmlfile.IsValid())
    {
        FXmlNode* Evn = xmlfile.GetRootNode();
        if (Evn)
        {
            FXmlNode* Weather = Evn->FindChildNode(TEXT("Weather"));
            FXmlNode* Date = Evn->FindChildNode(TEXT("Date"));
            FXmlNode* Time = Evn->FindChildNode(TEXT("Time"));
            FXmlNode* TimeLine = Evn->FindChildNode(TEXT("TimeLine"));

            // Get Init data
            {
                if (Weather)
                {
                    _Data.initData.rainFall = FCString::Atod(*Weather->GetAttribute(TEXT("RainFall")));
                    _Data.initData.snowFall = FCString::Atod(*Weather->GetAttribute(TEXT("SnowFall")));
                    _Data.initData.windSpeed = FCString::Atod(*Weather->GetAttribute(TEXT("WindSpeed")));
                    _Data.initData.cloudDensity = FCString::Atod(*Weather->GetAttribute(TEXT("CloudDensity")));
                    _Data.initData.visibility = FCString::Atod(*Weather->GetAttribute(TEXT("Visibility")));
                }
                else
                {
                    UE_LOG(SimLogEnvManager, Warning, TEXT("Can Not Get Value: Weather"));
                }

                int32 Year = 2013;
                int32 Month = 1;
                int32 Day = 1;
                int32 Hour = 12;
                int32 Minute = 0;
                int32 Second = 0;
                int32 Millisecond = 0;

                if (Date)
                {
                    Year = FCString::Atoi(*Date->GetAttribute(TEXT("Year")));
                    Month = FCString::Atoi(*Date->GetAttribute(TEXT("Month")));
                    Day = FCString::Atoi(*Date->GetAttribute(TEXT("Day")));
                }
                else
                {
                    UE_LOG(SimLogEnvManager, Warning, TEXT("Can Not Get Value: Date"));
                }

                if (Time)
                {
                    Hour = FCString::Atoi(*Time->GetAttribute(TEXT("Hour")));
                    Minute = FCString::Atoi(*Time->GetAttribute(TEXT("Minute")));
                    Second = FCString::Atoi(*Time->GetAttribute(TEXT("Second")));
                    Millisecond = FCString::Atoi(*Time->GetAttribute(TEXT("Millisecond")));
                }
                else
                {
                    UE_LOG(SimLogEnvManager, Warning, TEXT("Can Not Get Value: Time"));
                }

                _Data.initData.date = FDateTime(Year, Month, Day, Hour, Minute, Second, Millisecond);

                _Data.initData.Compute();
            }

            // Get Timeline keys
            if (TimeLine)
            {
                TArray<FXmlNode*> KeyNodes = TimeLine->GetChildrenNodes();
                for (size_t i = 0; i < KeyNodes.Num(); i++)
                {
                    bool IsInKeys = false;
                    // timestamp is in keys?
                    for (size_t j = 0; j < _Data.Keys.Num(); j++)
                    {
                        if (FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("TimeStamp"))) == _Data.Keys[j].timeStamp)
                        {
                            FSimEnvData NewEnvData = _Data.Keys[j];
                            if (KeyNodes[i]->GetTag() == TEXT("Weather"))
                            {
                                NewEnvData.timeStamp = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("TimeStamp")));
                                NewEnvData.visibility = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("Visibility")));
                                NewEnvData.rainFall = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("RainFall")));
                                NewEnvData.snowFall = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("SnowFall")));
                                NewEnvData.windSpeed = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("WindSpeed")));
                                NewEnvData.cloudDensity =
                                    FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("CloudDensity")));
                                NewEnvData.Compute();
                                _Data.Keys[j] = NewEnvData;
                            }
                            else if (KeyNodes[i]->GetTag() == TEXT("Time"))
                            {
                                NewEnvData.timeStamp = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("TimeStamp")));
                                int32 Hour = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Hour")));
                                int32 Minute = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Minute")));
                                int32 Second = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Second")));
                                int32 Millisecond = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Millisecond")));
                                NewEnvData.date = FDateTime(NewEnvData.date.GetYear(), NewEnvData.date.GetMonth(),
                                    NewEnvData.date.GetDay(), Hour, Minute, Second, Millisecond);
                                NewEnvData.Compute();
                                _Data.Keys[j] = NewEnvData;
                            }
                            else if (KeyNodes[i]->GetTag() == TEXT("Date"))
                            {
                                NewEnvData.timeStamp = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("TimeStamp")));
                                int32 Year = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Year")));
                                int32 Month = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Month")));
                                int32 Day = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Day")));
                                NewEnvData.date =
                                    FDateTime(Year, Month, Day, NewEnvData.date.GetHour(), NewEnvData.date.GetMinute(),
                                        NewEnvData.date.GetSecond(), NewEnvData.date.GetMillisecond());
                                NewEnvData.Compute();
                                _Data.Keys[j] = NewEnvData;
                            }
                            IsInKeys = true;
                            break;
                        }
                    }
                    // If not in keys
                    if (!IsInKeys)
                    {
                        FSimEnvData NewEnvData = _Data.initData;
                        if (KeyNodes[i]->GetTag() == TEXT("Weather"))
                        {
                            NewEnvData.timeStamp = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("TimeStamp")));
                            NewEnvData.visibility = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("Visibility")));
                            NewEnvData.rainFall = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("RainFall")));
                            NewEnvData.snowFall = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("SnowFall")));
                            NewEnvData.windSpeed = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("WindSpeed")));
                            NewEnvData.cloudDensity = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("CloudDensity")));
                            NewEnvData.Compute();
                            _Data.Keys.Add(NewEnvData);
                        }
                        else if (KeyNodes[i]->GetTag() == TEXT("Time"))
                        {
                            NewEnvData.timeStamp = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("TimeStamp")));
                            int32 Hour = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Hour")));
                            int32 Minute = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Minute")));
                            int32 Second = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Second")));
                            int32 Millisecond = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Millisecond")));
                            NewEnvData.date = FDateTime(NewEnvData.date.GetYear(), NewEnvData.date.GetMonth(),
                                NewEnvData.date.GetDay(), Hour, Minute, Second, Millisecond);
                            NewEnvData.Compute();
                            _Data.Keys.Add(NewEnvData);
                        }
                        else if (KeyNodes[i]->GetTag() == TEXT("Date"))
                        {
                            NewEnvData.timeStamp = FCString::Atod(*KeyNodes[i]->GetAttribute(TEXT("TimeStamp")));
                            int32 Year = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Year")));
                            int32 Month = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Month")));
                            int32 Day = FCString::Atoi(*KeyNodes[i]->GetAttribute(TEXT("Day")));
                            NewEnvData.date =
                                FDateTime(Year, Month, Day, NewEnvData.date.GetHour(), NewEnvData.date.GetMinute(),
                                    NewEnvData.date.GetSecond(), NewEnvData.date.GetMillisecond());
                            NewEnvData.Compute();
                            _Data.Keys.Add(NewEnvData);
                        }
                    }
                }
            }
            else
            {
                UE_LOG(SimLogEnvManager, Warning, TEXT("Can Not Get Value: TimeLine"));
            }

            // Order the keys
            _Data.Keys.Sort(
                [](const FSimEnvData& LHS, const FSimEnvData& RHS) { return LHS.timeStamp < RHS.timeStamp; });
        }
    }

    return true;
}

bool AEnvManager::ParseEnvString(const std::string& Buffer, FSimEnvTimeline& _Data)
{
    if (Buffer.empty())
    {
        return false;
    }
    sim_msg::Scene scene;
    if (!scene.ParseFromString(Buffer))
    {
        UE_LOG(SimLogEnvManager, Warning, TEXT("ParseFromString faild."));

        return false;
    }

    TMap<int64, FSimEnvData> EnvDataMap;
    for (auto it = scene.environment().begin(); it != scene.environment().end(); it++)
    {
        FSimEnvData EnvDataFrame;
        sim_msg::EnvironmentalConditions EnvCondition = it->second;

        if (EnvCondition.precipitation().type() == sim_msg::EnvironmentalConditions_Precipitation_Type_RAIN)
        {
            EnvDataFrame.rainFall = EnvCondition.precipitation().intensity() * 255.f;
        }
        else if (EnvCondition.precipitation().type() == sim_msg::EnvironmentalConditions_Precipitation_Type_SNOW)
        {
            EnvDataFrame.snowFall = EnvCondition.precipitation().intensity() * 10.f;
        }
        EnvDataFrame.windSpeed = EnvCondition.wind().speed();
        float* CloudValue = AEnvManager::CloudLevelDensityMapping.Find(EnvCondition.clouds().fractional_cloud_cover());
        EnvDataFrame.cloudDensity = CloudValue ? *CloudValue : 0.f;

        EnvDataFrame.visibility = EnvCondition.fog().visibility() / 1000.f;
        EnvDataFrame.date = FDateTime::FromUnixTimestamp(EnvCondition.unix_timestamp() / 1000);
        FString DateStr = EnvDataFrame.date.ToString();
        EnvDataFrame.timeStamp = it->first;
        EnvDataFrame.Compute();
        EnvDataMap.Add(it->first, EnvDataFrame);
        EnvDataMap.KeySort(TLess<int32>());
    }

    bool bFirstEnv = true;
    for (auto FrameData : EnvDataMap)
    {
        if (bFirstEnv)
        {
            if (0 == FrameData.Key)
            {
                _Data.initData = FrameData.Value;
            }
            else    // if first timestamp != 0,then copy first frame data to timestamp 0
            {
                FSimEnvData InitData = FrameData.Value;
                InitData.timeStamp = 0;
                _Data.initData = InitData;
                _Data.Keys.Add(FrameData.Value);
            }
            bFirstEnv = false;
        }
        else
        {
            _Data.Keys.Add(FrameData.Value);
        }
    }

    return true;
}

void AEnvManager::Init(const FManagerConfig& Config)
{
    const FEnvManagerConfig* EnvConfig = Cast_Data<const FEnvManagerConfig>(Config);
    if (!EnvConfig)
    {
        UE_LOG(SimLogEnvManager, Error, TEXT("Can Not Get EnvConfig!"));
        return;
    }
    // Get config
    timeline = EnvConfig->timeline;
    envData_Current = timeline.initData;

    if (particle_Rain)
    {
        ParticleRain = GetWorld()->SpawnActor<AEmitter>();
        ParticleRain->SetTemplate(particle_Rain);
        // particleComponent_Rain = UGameplayStatics::SpawnEmitterAtLocation(this, particle_Rain, FVector(),
        // FRotator());
        if (!ParticleRain.IsValid())
        {
            UE_LOG(SimLogEnvManager, Warning, TEXT("particleComponent_Rain is Null!"));
        }
        else
        {
            ParticleRain->GetParticleSystemComponent()->Activate(true);
            ParticleRain->GetParticleSystemComponent()->bAutoActivate = true;
            ParticleRain->GetParticleSystemComponent()->SetMobility(EComponentMobility::Type::Movable);
        }
    }
    if (particle_Snow)
    {
        ParticleSnow = GetWorld()->SpawnActor<AEmitter>();
        ParticleSnow->SetTemplate(particle_Snow);
        // particleComponent_Snow = UGameplayStatics::SpawnEmitterAtLocation(this, particle_Snow, FVector(),
        // FRotator());
        if (!ParticleSnow.IsValid())
        {
            UE_LOG(SimLogEnvManager, Warning, TEXT("particleComponent_Snow is Null!"));
        }
        else
        {
            ParticleSnow->GetParticleSystemComponent()->Activate(true);
            ParticleSnow->GetParticleSystemComponent()->bAutoActivate = true;
            ParticleSnow->GetParticleSystemComponent()->SetMobility(EComponentMobility::Type::Movable);
        }
    }

    ApplyEnvData(envData_Current);
    // TODO: Init emitters

    // if (!egoVehicle || !egoVehicle->Emitter_Rain || !egoVehicle->Emitter_Snow || !egoVehicle->Emitter_Fog)
    //{
    //     UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: Cant get egoVehicle or egoVehicle`s
    //     Emitter_Weather!")); return;
    // }
    // managerState = EManagerStateEnum::MANAGERSTATE_INITIALIZED;

    UE_LOG(SimLogEnvManager, Warning, TEXT("rainFall: %f"), envData_Current.rainFall);
    UE_LOG(SimLogEnvManager, Warning, TEXT("cloudDensity: %f"), envData_Current.cloudDensity);
    UE_LOG(SimLogEnvManager, Warning, TEXT("cloudThickness: %f"), envData_Current.cloudThickness);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("date: %f"), envData_Current.date);
    UE_LOG(SimLogEnvManager, Warning, TEXT("humidity: %f"), envData_Current.humidity);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("rainFall: %f"), envData_Current.location);
    UE_LOG(SimLogEnvManager, Warning, TEXT("moonRotation: %s"), *envData_Current.moonRotation.ToString());
    UE_LOG(SimLogEnvManager, Warning, TEXT("moonStrength: %f"), envData_Current.moonStrength);
    UE_LOG(SimLogEnvManager, Warning, TEXT("snowFall: %f"), envData_Current.snowFall);
    UE_LOG(SimLogEnvManager, Warning, TEXT("sunRotation: %s"), *envData_Current.sunRotation.ToString());
    UE_LOG(SimLogEnvManager, Warning, TEXT("sunStrength: %f"), envData_Current.sunStrength);
    UE_LOG(SimLogEnvManager, Warning, TEXT("temperature: %f"), envData_Current.temperature);
    UE_LOG(SimLogEnvManager, Warning, TEXT("timeStamp: %f"), envData_Current.timeStamp);
    UE_LOG(SimLogEnvManager, Warning, TEXT("visibility: %f"), envData_Current.visibility);
    UE_LOG(SimLogEnvManager, Warning, TEXT("windRotation: %s"), *envData_Current.windRotation.ToString());
    UE_LOG(SimLogEnvManager, Warning, TEXT("windSpeed: %f"), envData_Current.windSpeed);
    UE_LOG(SimLogEnvManager, Warning, TEXT("Year: %d"), envData_Current.date.GetYear());
    UE_LOG(SimLogEnvManager, Warning, TEXT("Month: %d"), envData_Current.date.GetMonth());
    UE_LOG(SimLogEnvManager, Warning, TEXT("Day: %d"), envData_Current.date.GetDay());
    UE_LOG(SimLogEnvManager, Warning, TEXT("Hour: %d"), envData_Current.date.GetHour());
    UE_LOG(SimLogEnvManager, Warning, TEXT("Minute: %d"), envData_Current.date.GetMinute());
    UE_LOG(SimLogEnvManager, Warning, TEXT("Second: %d"), envData_Current.date.GetSecond());
    UE_LOG(SimLogEnvManager, Warning, TEXT("Lon: %f"), envData_Current.location[0]);
    UE_LOG(SimLogEnvManager, Warning, TEXT("lat: %f"), envData_Current.location[1]);
    UE_LOG(SimLogEnvManager, Warning, TEXT("Alt: %f"), envData_Current.location[2]);
}

void AEnvManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    const FEnvManagerIn* EnvManagerIn = Cast_Data<const FEnvManagerIn>(Input);

    check(EnvManagerIn);

    if (EnvManagerIn->unix_timestamp != 0 && bHasEnvEditPermission)
    {
        bReceivedPB = true;

        EnvironmentInfo = EnvManagerIn->environment;
        envData_Current.visibility = EnvManagerIn->visibility / 1000.f;

        float* CloudValue = AEnvManager::CloudLevelDensityMapping.Find(EnvManagerIn->cloudLevel);
        envData_Current.cloudDensity = CloudValue ? *CloudValue : 0.f;
        envData_Current.windSpeed = EnvManagerIn->windSpeed;
        envData_Current.rainFall = EnvManagerIn->rainFall * 250.f;
        envData_Current.snowFall = EnvManagerIn->snowFall * 10.f;
        envData_Current.temperature = EnvManagerIn->temperature;
        envData_Current.date = FDateTime::FromUnixTimestamp(EnvManagerIn->unix_timestamp / 1000);

        if (bOutputEnvLog || bFirstOutputLog)
        {
            UE_LOG(SimLogEnvManager, Log, TEXT("SimEnv visibility: %f"), envData_Current.visibility);
            UE_LOG(SimLogEnvManager, Log, TEXT("SimEnvcloudDensity : %f"), envData_Current.cloudDensity);
            UE_LOG(SimLogEnvManager, Log, TEXT("SimEnvwindSpeed : %f"), envData_Current.windSpeed);
            UE_LOG(SimLogEnvManager, Log, TEXT("SimEnvrainFall: %f"), envData_Current.rainFall)
            UE_LOG(SimLogEnvManager, Log, TEXT("SimEnvsnowFall: %f"), envData_Current.snowFall);
            UE_LOG(SimLogEnvManager, Log, TEXT("SimEnvtemperature: %f"), envData_Current.temperature);
            UE_LOG(SimLogEnvManager, Log, TEXT("SimEnvdate: %lld"), EnvManagerIn->unix_timestamp);
            UE_LOG(SimLogEnvManager, Log, TEXT("Sim Date: %s"), *envData_Current.date.ToString());
            UE_LOG(SimLogEnvManager, Log, TEXT("Sim Hour: %d"), envData_Current.date.GetHour());

            bFirstOutputLog = false;
        }
        envData_Current.Compute();

        ApplyEnvData(envData_Current);

        return;
    }

    if (!bReceivedPB)
    {
        UpdateTimeline(Input.timeStamp);
    }
    // UE_LOG(SimLogEnvManager, Warning, TEXT("Distance P2E: %f"), (particleComponent_Snow->GetComponentLocation() -
    // egoVehicle->GetActorLocation()).Size());
}

// void AEnvManager::SetTimeOfDay(float _NewTime)
//{
//     timeOfDay = _NewTime;
// }
//
// float AEnvManager::GetTimeOfDay()const
//{
//     return timeOfDay;
// }
//
// void AEnvManager::SetWeatherState(EWeatherStateEnum _NewState)
//{
//     weatherState = _NewState;
// }
//
// EWeatherStateEnum AEnvManager::GetWeatherState() const
//{
//     return weatherState;
// }

// void AEnvManager::UpdateWeather(float _DeltaTime)
//{
//     //if (!weatherParamCollection || !weatherParamCollectionInstance)
//     //{
//     //    return;
//     //}
//     //if (managerState != EManagerStateEnum::MANAGERSTATE_INITIALIZED)
//     //{
//     //    return;
//     //}
//
//     ////Update weatherParamCollection
//     //weatherParamCollectionInstance->GetScalarParameterValue(FName(TEXT("Sun_Strength")), Scale_Sun);
//     //weatherParamCollectionInstance->GetScalarParameterValue(FName(TEXT("Cloud_Strength")), Scale_Cloud);
//     //weatherParamCollectionInstance->GetScalarParameterValue(FName(TEXT("Fog_Strength")), Scale_Fog);
//     //weatherParamCollectionInstance->GetScalarParameterValue(FName(TEXT("Rain_Strength")), Scale_Rain);
//     //weatherParamCollectionInstance->GetScalarParameterValue(FName(TEXT("Snow_Strength")), Scale_Snow);
//
//     //switch (weatherState)
//     //{
//     //case EWeatherStateEnum::WS_Snow:
//     //    ParameterChangeTo(Scale_Sun, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Cloud, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Fog, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Rain, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Snow, 1, _DeltaTime);
//     //    break;
//     //case EWeatherStateEnum::WS_Rain:
//     //    ParameterChangeTo(Scale_Sun, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Cloud, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Fog, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Rain, 1, _DeltaTime);
//     //    ParameterChangeTo(Scale_Snow, 0, _DeltaTime);
//     //    break;
//     //case EWeatherStateEnum::WS_Sun:
//     //    ParameterChangeTo(Scale_Sun, 1, _DeltaTime);
//     //    ParameterChangeTo(Scale_Cloud, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Fog, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Rain, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Snow, 0, _DeltaTime);
//     //    break;
//     //case EWeatherStateEnum::WS_Fog:
//     //    ParameterChangeTo(Scale_Sun, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Cloud, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Fog, 1, _DeltaTime);
//     //    ParameterChangeTo(Scale_Rain, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Snow, 0, _DeltaTime);
//     //    break;
//     //case EWeatherStateEnum::WS_Cloud:
//     //    ParameterChangeTo(Scale_Sun, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Cloud, 1, _DeltaTime);
//     //    ParameterChangeTo(Scale_Fog, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Rain, 0, _DeltaTime);
//     //    ParameterChangeTo(Scale_Snow, 0, _DeltaTime);
//     //    break;
//     //default:
//     //    break;
//     //}
//     //weatherParamCollectionInstance->SetScalarParameterValue(FName(TEXT("Sun_Strength")), Scale_Sun);
//     //weatherParamCollectionInstance->SetScalarParameterValue(FName(TEXT("Cloud_Strength")), Scale_Cloud);
//     //weatherParamCollectionInstance->SetScalarParameterValue(FName(TEXT("Fog_Strength")), Scale_Fog);
//     //weatherParamCollectionInstance->SetScalarParameterValue(FName(TEXT("Rain_Strength")), Scale_Rain);
//     //weatherParamCollectionInstance->SetScalarParameterValue(FName(TEXT("Snow_Strength")), Scale_Snow);
//
//     //if (!egoVehicle || !egoVehicle->Emitter_Rain || !egoVehicle->Emitter_Snow || !egoVehicle->Emitter_Fog)
//     //{
//     //    //UE_LOG(SimLogEnvManager, Warning, TEXT("EnvironmentManager: Cant get egoVehicle or egoVehicle`s
//     Emitter_Weather!"));
//     //    return;
//     //}
//
//     ////Update weatherParticle.
//     //switch (weatherState)
//     //{
//     //case EWeatherStateEnum::WS_Snow:
//     //    egoVehicle->Emitter_Snow->SetActive(true);
//     //    egoVehicle->Emitter_Rain->SetActive(false);
//     //    egoVehicle->Emitter_Fog->SetActive(false);
//     //    break;
//     //case EWeatherStateEnum::WS_Rain:
//     //    egoVehicle->Emitter_Snow->SetActive(false);
//     //    egoVehicle->Emitter_Rain->SetActive(true);
//     //    egoVehicle->Emitter_Fog->SetActive(false);
//     //    break;
//     //case EWeatherStateEnum::WS_Sun:
//     //    egoVehicle->Emitter_Snow->SetActive(false);
//     //    egoVehicle->Emitter_Rain->SetActive(false);
//     //    egoVehicle->Emitter_Fog->SetActive(false);
//     //    break;
//     //case EWeatherStateEnum::WS_Fog:
//     //    egoVehicle->Emitter_Snow->SetActive(false);
//     //    egoVehicle->Emitter_Rain->SetActive(false);
//     //    egoVehicle->Emitter_Fog->SetActive(true);
//     //    break;
//     //case EWeatherStateEnum::WS_Cloud:
//     //    egoVehicle->Emitter_Snow->SetActive(false);
//     //    egoVehicle->Emitter_Rain->SetActive(false);
//     //    egoVehicle->Emitter_Fog->SetActive(false);
//     //    break;
//     //default:
//     //    break;
//     //}
// }

// void AEnvManager::SetEgoVehicle(AEgoVehicle * _NewEgo)
//{
//     egoVehicle = _NewEgo;
// }
//
// AEgoVehicle * AEnvManager::GetEgoVehicle() const
//{
//     return egoVehicle;
// }

bool AEnvManager::ParameterChangeTo_BP(float _From, float _To, float _DeltaTime, float _Speed, float& _OutPut)
{
    if (_From < _To)
    {
        float NextScale = _From;
        NextScale += _Speed * _DeltaTime;
        if (NextScale >= _To)
        {
            _OutPut = _To;
            return true;
        }
        else
        {
            _OutPut = NextScale;
            return false;
        }
    }
    else if (_From > _To)
    {
        float NextScale = _From;
        NextScale -= _Speed * _DeltaTime;
        if (NextScale <= _To)
        {
            _OutPut = _To;
            return true;
        }
        else
        {
            _OutPut = NextScale;
            return false;
        }
    }
    else
    {
        return true;
    }
}

void AEnvManager::SetEnvDataEditPermission(bool _IsActive)
{
    bHasEnvEditPermission = _IsActive;
}

void AEnvManager::ExternalInputEnvData(const FSimEnvData& _Data)
{
    if (bHasEnvEditPermission)
    {
        return;
    }
    envData_Current = _Data;
    ApplyEnvData(envData_Current);
}

void AEnvManager::UpdateTimeline(double _TimeStamp)
{
    // Update time
    timeStamp_Pre = timeStamp;
    timeStamp = _TimeStamp;

    //// Init data
    // if (timeStamp <= 0.001 || timeStamp >= -0.001)
    //{
    //     envData_Current = timeline.initData;
    //     //ApplyEnvData(envData_Current);
    //     //return;
    // }

    if (bHasEnvEditPermission)
    {
        // Interpolation
        if (timeline.Keys.Num() > 0 && timeStamp >= timeline.Keys.Last().timeStamp)
        {
            envData_Current = timeline.Keys.Last();
        }
        else
        {
            for (size_t i = 0; i < timeline.Keys.Num(); i++)
            {
                if (timeStamp <= timeline.Keys[i].timeStamp)
                {
                    if (i > 0)
                    {
                        envData_Current = Interpolation(timeline.Keys[i - 1], timeline.Keys[i], timeStamp);
                    }
                    else
                    {
                        envData_Current = Interpolation(timeline.initData, timeline.Keys[i], timeStamp);
                    }
                    break;
                }
            }
        }

        // Apply data
        ApplyEnvData(envData_Current);
    }

    // UE_LOG(SimLogEnvManager, Warning, TEXT("rainFall: %f"), envData_Current.rainFall);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("cloudDensity: %f"), envData_Current.cloudDensity);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("cloudThickness: %f"), envData_Current.cloudThickness);
    ////UE_LOG(SimLogEnvManager, Warning, TEXT("date: %f"), envData_Current.date);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("humidity: %f"), envData_Current.humidity);
    ////UE_LOG(SimLogEnvManager, Warning, TEXT("rainFall: %f"), envData_Current.location);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("moonRotation: %s"), *envData_Current.moonRotation.ToString());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("moonStrength: %f"), envData_Current.moonStrength);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("snowFall: %f"), envData_Current.snowFall);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("sunRotation: %s"), *envData_Current.sunRotation.ToString());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("sunStrength: %f"), envData_Current.sunStrength);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("temperature: %f"), envData_Current.temperature);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("timeStamp: %f"), envData_Current.timeStamp);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("visibility: %f"), envData_Current.visibility);
    // UE_LOG(SimLogEnvManager, Warning, TEXT("windRotation: %s"), *envData_Current.windRotation.ToString());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("windSpeed: %f"), envData_Current.windSpeed);
}

FSimEnvData AEnvManager::Interpolation(const FSimEnvData& _From, const FSimEnvData& _To, double _TimeStamp)
{
    if (_To.timeStamp == _From.timeStamp)
    {
        return _To;
    }
    // TODO: poccess To<From
    else if (_To.timeStamp < _From.timeStamp)
    {
        return _From;
    }
    if (_TimeStamp <= _From.timeStamp)
    {
        return _From;
    }
    if (_TimeStamp >= _To.timeStamp)
    {
        return _To;
    }

    double Pos = (_TimeStamp - _From.timeStamp) / (_To.timeStamp - _From.timeStamp);

    FSimEnvData NewData;

    NewData.rainFall = _From.rainFall + (_To.rainFall - _From.rainFall) * Pos;
    NewData.snowFall = _From.snowFall + (_To.snowFall - _From.snowFall) * Pos;
    NewData.temperature = _From.temperature + (_To.temperature - _From.temperature) * Pos;
    NewData.visibility = _From.visibility + (_To.visibility - _From.visibility) * Pos;
    NewData.cloudDensity = _From.cloudDensity + (_To.cloudDensity - _From.cloudDensity) * Pos;
    NewData.cloudThickness = _From.cloudThickness + (_To.cloudThickness - _From.cloudThickness) * Pos;
    NewData.windSpeed = _From.windSpeed + (_To.windSpeed - _From.windSpeed) * Pos;
    NewData.humidity = _From.humidity + (_To.humidity - _From.humidity) * Pos;
    NewData.sunStrength = _From.sunStrength + (_To.sunStrength - _From.sunStrength) * Pos;
    NewData.moonStrength = _From.moonStrength + (_To.moonStrength - _From.moonStrength) * Pos;

    NewData.location[0] = _From.location[0] + (_To.location[0] - _From.location[0]) * Pos;
    NewData.location[1] = _From.location[1] + (_To.location[1] - _From.location[1]) * Pos;
    NewData.location[1] = _From.location[2] + (_To.location[2] - _From.location[2]) * Pos;
    NewData.sunRotation = FMath::RInterpTo(_From.sunRotation, _To.sunRotation, Pos, 1);
    NewData.moonRotation = FMath::RInterpTo(_From.moonRotation, _To.moonRotation, Pos, 1);
    NewData.windRotation = FMath::RInterpTo(_From.windRotation, _To.windRotation, Pos, 1);
    NewData.date = _From.date + (_To.date - _From.date) * Pos;

    NewData.timeStamp = _TimeStamp;

    return NewData;
}

void AEnvManager::ApplyEnvData(const FSimEnvData& _Data)
{
    // Skybox

    // EnvParamsCollection
    if (EnvParamsCollectionInstance && EnvParamsCollectionInstance->IsValidLowLevel())
    {
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("year")), _Data.date.GetYear());
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("month")), _Data.date.GetMonth());
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("day")), _Data.date.GetDay());
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("hour")), _Data.date.GetHour());
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("minute")), _Data.date.GetMinute());
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("second")), _Data.date.GetSecond());
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("millisecond")), _Data.date.GetMillisecond());
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("rainFall")), _Data.rainFall);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("snowFall")), _Data.snowFall);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("visibility")), _Data.visibility);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("cloudDensity")), _Data.cloudDensity);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("cloudThickness")), _Data.cloudThickness);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("temperature")), _Data.temperature);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("windSpeed")), _Data.windSpeed);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("humidity")), _Data.humidity);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("sunStrength")), _Data.sunStrength);
        EnvParamsCollectionInstance->SetScalarParameterValue(FName(TEXT("moonStrength")), _Data.moonStrength);
        EnvParamsCollectionInstance->SetVectorParameterValue(FName(TEXT("sunRotation")),
            FVector(_Data.sunRotation.Roll, _Data.sunRotation.Pitch, _Data.sunRotation.Yaw));
        EnvParamsCollectionInstance->SetVectorParameterValue(FName(TEXT("windRotation")),
            FVector(_Data.windRotation.Roll, _Data.windRotation.Pitch, _Data.windRotation.Yaw));
        EnvParamsCollectionInstance->SetVectorParameterValue(FName(TEXT("moonRotation")),
            FVector(_Data.moonRotation.Roll, _Data.moonRotation.Pitch, _Data.moonRotation.Yaw));
    }

    // Emitters
    if (ParticleRain.IsValid())
    {
        ParticleRain->GetParticleSystemComponent()->SetFloatParameter(
            FName(TEXT("rainFall")), (_Data.rainFall / 250.f) * 10000.f);
        ParticleRain->GetParticleSystemComponent()->SetFloatParameter(
            FName(TEXT("rainFall_GPU")), (_Data.rainFall / 250.f) * 50000);
    }
    if (ParticleSnow.IsValid())
    {
        ParticleSnow->GetParticleSystemComponent()->SetFloatParameter(
            FName(TEXT("snowFall")), (_Data.snowFall / 10.f) * 100000.f);
    }

    // WindSource
    // TODO: Set WindSource param
    if (windSource)
    {
        windSource->GetComponent()->SetSpeed(_Data.windSpeed);
    }

    // HeighFog
    // heightFog->GetComponent()->SetFogDensity((_Data.visibility / 30.f)*0.05);
    // heightFog->GetComponent()->SecondFogData.FogDensity = (_Data.visibility / 30.f)*0.05;

    // StreetLights
    if (_Data.date.GetHour() < sunriseTime || _Data.date.GetHour() > sunsetTime)
    {
        bBeingNight = true;
        if (bBeingDay)
        {
            // Night, Switch on light
            for (auto& Elem : streetLightArry)
            {
                if (Elem)
                {
                    Elem->SetLightOn(true);
                }
            }
            bBeingDay = false;
            UE_LOG(SimLogEnvManager, Log, TEXT("Street`s lights on."));
        }
    }
    else
    {
        bBeingDay = true;
        if (bBeingNight)
        {
            // Day, Switch off light
            for (auto& Elem : streetLightArry)
            {
                if (Elem)
                {
                    Elem->SetLightOn(false);
                }
            }
            bBeingNight = false;
            UE_LOG(SimLogEnvManager, Log, TEXT("Street`s lights off."));
        }
    }

    // UE_LOG(SimLogEnvManager, Warning, TEXT("Year: %d"), _Data.date.GetYear());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("Month: %d"), _Data.date.GetMonth());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("Day: %d"), _Data.date.GetDay());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("Hour: %d"), _Data.date.GetHour());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("Minute: %d"), _Data.date.GetMinute());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("Second: %d"), _Data.date.GetSecond());
    // UE_LOG(SimLogEnvManager, Warning, TEXT("rainFall: %f"), _Data.rainFall);
}

bool AEnvManager::ParameterChangeTo(float& _From, float _To, float _DeltaTime)
{
    if (_From < _To)
    {
        float NextScale = _From;
        NextScale += changeSpeed * _DeltaTime;
        if (NextScale >= _To)
        {
            _From = _To;
            return true;
        }
        else
        {
            _From = NextScale;
            return false;
        }
    }
    else if (_From > _To)
    {
        float NextScale = _From;
        NextScale -= changeSpeed * _DeltaTime;
        if (NextScale <= _To)
        {
            _From = _To;
            return true;
        }
        else
        {
            _From = NextScale;
            return false;
        }
    }
    else
    {
        return true;
    }
}

void AEnvManager::UpdateEnvEffectLocation(const FName& CameraName)
{
    float CameraDistance = 0.f;
    if (CameraName.IsEqual(TEXT("Camera_Driver")))
        CameraDistance = 200.f;

    if (ParticleRain.IsValid())
    {
        UMaterialInterface* RainMat1 = ParticleRain->GetParticleSystemComponent()->GetMaterial(1);
        UMaterialInstanceDynamic* RainMID1 = Cast<UMaterialInstanceDynamic>(RainMat1);
        if (!RainMID1)
        {
            RainMID1 = UMaterialInstanceDynamic::Create(RainMat1, this);
            ParticleRain->GetParticleSystemComponent()->SetMaterial(1, RainMID1);
        }

        UMaterialInterface* RainMat3 = ParticleRain->GetParticleSystemComponent()->GetMaterial(3);
        UMaterialInstanceDynamic* RainMID3 = Cast<UMaterialInstanceDynamic>(RainMat3);
        if (!RainMID3)
        {
            RainMID3 = UMaterialInstanceDynamic::Create(RainMat3, this);
            ParticleRain->GetParticleSystemComponent()->SetMaterial(3, RainMID3);
        }

        RainMID1->SetScalarParameterValue(TEXT("CameraMaskDistance"), CameraDistance);
        RainMID3->SetScalarParameterValue(TEXT("CameraMaskDistance"), CameraDistance);
    }

    UMaterialInterface* SnowMat0 = ParticleSnow->GetParticleSystemComponent()->GetMaterial(0);
    UMaterialInstanceDynamic* SnowMID0 = Cast<UMaterialInstanceDynamic>(SnowMat0);
    if (!SnowMID0)
    {
        SnowMID0 = UMaterialInstanceDynamic::Create(SnowMat0, this);
        ParticleSnow->GetParticleSystemComponent()->SetMaterial(0, SnowMID0);

        SnowMID0->SetScalarParameterValue(TEXT("CameraMaskDistance"), CameraDistance);
    }
}
