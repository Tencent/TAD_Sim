// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"
#include "Objects/Creatures/Pedestrians/PedestrianCharacter.h"
#include "Objects/Obstacles/ObstacleActor.h"
#include "Managers/EnvManager.h"
#include "SunPosition/Public/SunPosition.h"
#include "OutlineWidget.generated.h"

USTRUCT(BlueprintType)
struct FUIEnvData
{
    GENERATED_BODY()
public:
    // Date time
    UPROPERTY(BlueprintReadWrite)
    FDateTime date = FDateTime(2020, 5, 23, 14, 30, 0, 0);

    //// Location
    // UPROPERTY(BlueprintReadWrite)
    //     FVector location = FVector(121.1638975600, 31.2806051100, 13.2);

    // Temperature -50-50
    UPROPERTY(BlueprintReadWrite)
    float temperature;

    // RainFall 0-250
    UPROPERTY(BlueprintReadWrite)
    float rainFall;

    // SnowFall 0-10
    UPROPERTY(BlueprintReadWrite)
    float snowFall;

    // Visibility 0-30
    UPROPERTY(BlueprintReadWrite)
    float visibility;

    // Cloud density 0-10
    UPROPERTY(BlueprintReadWrite)
    float cloudDensity;

    // Cloud thickness 0-1
    UPROPERTY(BlueprintReadWrite)
    float cloudThickness;

    // Wind speed 0-61.2
    UPROPERTY(BlueprintReadWrite)
    float windSpeed;

    // Wind direction
    UPROPERTY(BlueprintReadWrite)
    FRotator windRotation = FRotator(0, 45, 0);

    // Humidity 0-1
    UPROPERTY(BlueprintReadWrite)
    float humidity;

    // sunStrength 0-200000
    UPROPERTY(BlueprintReadWrite)
    float sunStrength = 200000;

    //// sunRotation
    // UPROPERTY(BlueprintReadWrite)
    //     FRotator sunRotation = FRotator(0, 45, 0);

    // moonStrength 0-1
    UPROPERTY(BlueprintReadWrite)
    float moonStrength = 0;

    //// moonRotation
    // UPROPERTY(BlueprintReadWrite)
    //     FRotator moonRotation = FRotator(0, 45, 0);
};

// USTRUCT(BlueprintType)
// struct FModifierAttribute
//{
//     GENERATED_BODY()
// public:
//     UPROPERTY(BlueprintReadOnly)
//         FString name;
//
//     UPROPERTY(BlueprintReadOnly)
//         float min = 0.f;
//
//     UPROPERTY(BlueprintReadOnly)
//         float max = 1.f;
//
//     UPROPERTY(BlueprintReadOnly)
//         float default = 0.f;
//
//     UPROPERTY(BlueprintReadOnly)
//         float value = 0.f;
// };

USTRUCT(BlueprintType)
struct FSimActorInfo
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly)
    FString typeName;

    UPROPERTY(BlueprintReadOnly)
    int32 id = -1;

    UPROPERTY(BlueprintReadOnly)
    float timeStamp = 0.f;

    UPROPERTY(BlueprintReadOnly)
    FVector location = FVector(0);

    UPROPERTY(BlueprintReadOnly)
    FRotator rotation = FRotator(0);
};

USTRUCT(BlueprintType)
struct FVehicleInfo : public FSimActorInfo
{
    GENERATED_BODY()
public:
};

USTRUCT(BlueprintType)
struct FPedestrianInfo : public FSimActorInfo
{
    GENERATED_BODY()
public:
};

USTRUCT(BlueprintType)
struct FObstacleInfo : public FSimActorInfo
{
    GENERATED_BODY()
public:
};

/**
 *
 */
UCLASS()
class DISPLAY_API UOutlineWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct();
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
    virtual void BeginDestroy() override;

    // UPROPERTY(BlueprintReadOnly)
    //     TArray<FVehicleInfo> vehicleInfoArry;

    UFUNCTION(BlueprintCallable)
    TArray<FVehicleInfo> GetVehicleInfo();

    UFUNCTION(BlueprintCallable)
    TArray<FSimActorInfo> GetAllInfo();

    // UFUNCTION(BlueprintNativeEvent)
    //     void UpdateData();

    // void SetupEnvAttributesModifier(const TArray<FModifierAttribute>& _AttributeArry);
    // UFUNCTION(BlueprintNativeEvent)
    //     void BPE_SetupEnvAttributesModifier(const TArray<FModifierAttribute>& _AttributeArry);
    UFUNCTION(BlueprintImplementableEvent)
    void BPI_OpenEnvDataModifyPanel();

    UFUNCTION(BlueprintImplementableEvent)
    void BPI_CloseEnvDataModifyPanel();

    UFUNCTION(BlueprintImplementableEvent)
    FUIEnvData BPI_GetUIEnvData();

    UFUNCTION(BlueprintImplementableEvent)
    void BPI_SetUIEnvData(const FUIEnvData& _UIEnvData);

    UFUNCTION(BlueprintCallable)
    bool GetEnvEditPermission();

    UFUNCTION(BlueprintCallable)
    void ReturnEnvEditPermission();

    UPROPERTY(BlueprintReadOnly)
    bool bDefaultAllowModifyEnvData = false;

    UPROPERTY(BlueprintReadOnly)
    bool bHasEnvEditPermission = false;

    UPROPERTY(BlueprintReadOnly)
    bool bDefaultOpenUIPanel = false;

    UFUNCTION(BlueprintImplementableEvent)
    void BPI_OnInit();

    void Init(class ADisplayPlayerController* _PC);

protected:
    TArray<ISimActorInterface*> egoArry;
    TArray<ISimActorInterface*> trafficArry;
    TArray<AObstacleActor*> obstacleArry;
    TArray<APedestrianCharacter*> pedestrianArry;

    // FTimerHandle updateDataTimerHandle;

    AEnvManager* envEManager = NULL;
    FUIEnvData envData_UI;
    FSimEnvData envData_Sim;
    // TArray<FModifierAttribute> attributeArry_Env;

    FUIEnvData SimEnvDataToUIEnvData(const FSimEnvData& _SimEnvData);
    void UIEnvDataAddToSimEnvData(const FUIEnvData& _UIEnvData, FSimEnvData& _SimEnvData);
};
