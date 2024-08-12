// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Sensors/SensorActor.h"
#include "Networking.h"
#include <map>
#include "UltrasonicSensor.generated.h"

UENUM(BlueprintType)
enum class EAttachmentTypeEnum : uint8
{
    AT_None UMETA(DisplayName = "None"),
    AT_Water UMETA(DisplayName = "Water"),
    AT_Mud UMETA(DisplayName = "Mud")
};

USTRUCT(BlueprintType)
struct FUltrasonicConfig : public FSensorConfig
{
    GENERATED_BODY();
    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float fc = 40000.0;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float dB_min = -6;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float radius = 0.01;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float azimuth_fov = 60;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float elevation_fov = 40;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float ray_division_ele = 5.0;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float ray_division_hor = 5.0;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float scan_distance = 4.0;

    // 0~10
    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float noise_factor = 5;
    // 1~10
    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float noise_std = 5;

    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    EAttachmentTypeEnum attachment_type = EAttachmentTypeEnum::AT_None;
    // 0~1
    UPROPERTY(BlueprintReadOnly, Category = "UltrasonicConfig")
    float attachment_range = 0.5;

    bool openIndirectDistance = false;
    float timePulse0 = 0;
    float timePulsePeriod = 100;
    TArray<FUltrasonicConfig> all_ultrasonicArry;
    FString pollTurn;
    FString cfgDir = "";

    bool bSaveData = false;

    bool debug_frustum = false;
    bool debug_rescan = false;
    bool debug_scan = false;
};
USTRUCT()
struct FUltrasonicInput : public FSensorInput
{
    GENERATED_BODY()
public:
};

struct FDetectLine
{
    FRotator lineNor;
    float length = 999999.9f;
};

USTRUCT()
struct FUltrasonicOutput : public FSensorOutput
{
    GENERATED_BODY()
public:
    UPROPERTY()
    float timeStamp;
    UPROPERTY()
    float distance;
    UPROPERTY()
    float distance2;
};
/**
 *
 */
UCLASS()
class DISPLAY_API AUltrasonicSensor : public ASensorActor
{
    GENERATED_BODY()
public:
    virtual bool Init(const FSensorConfig& _Config);

    virtual void Update(const FSensorInput& _Input, FSensorOutput& _Output);

    virtual bool Save();

public:
    // Sets default values for this actor's properties
    AUltrasonicSensor(const FObjectInitializer& ObjectInitializer);
    /** Called whenever this actor is being removed from a level */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditDefaultsOnly)
    class USceneComponent* root;

    // virtual bool SetSensorConfig(struct FSensorConfigStruct &_NewConfig);
    // virtual void Update(double _TimeStamp, struct FSensorDataStruct &_NewData);
    // virtual bool SaveData();

private:
    UPROPERTY()
    FUltrasonicConfig config;
    UPROPERTY()
    TArray<FUltrasonicOutput> dataArry;

    // 0    关闭
    // 1    正常工作，能发能收
    // 2    send只发声
    // 3    receive只接收
    TMap<int, int> rs10;    //

    class AFovDepthBuffer* depthBuffer;

    TArray<FDetectLine> detectRays;
    FRotator rangePt[4];
    bool public_msg = false;

    const float sonic_v = 343.3f;
    class AEnvManager* envManager = NULL;
    float timeStamp = -1;

    std::map<int, std::pair<uint32, uint32>> stencilMap;
    // reflection members
    struct DeltaCoe
    {
        float a, b, c;
    };
    std::map<size_t, DeltaCoe> deltaCoe;

    float CalIndirectDistance(const FDetectLine& domain, double timestamp);
    bool LoadStencilMap(const FString& dir);
    bool loadCeoData(const FString& dir);
    bool GetCeoData(unsigned int tag_c, unsigned int tag_t, float& a, float& b, float& c);
    bool GetCeoData(int tag, float& a, float& b, float& c);
    float DeltaMaterial(unsigned int tag_c, unsigned int tag_t, float d);
};
