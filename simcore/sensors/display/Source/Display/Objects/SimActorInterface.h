// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SimActorInterface.generated.h"

USTRUCT()
struct FSimActorConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int32 type;

    UPROPERTY()
    FString typeName;    // 用于在game.ini查询UE资源路径

    UPROPERTY()
    FString Name;    // 场景文件中的name字段

    UPROPERTY()
    int64 id = -1;

    UPROPERTY()
    double timeStamp = 0.f;

    UPROPERTY()
    FVector startLocation = FVector(0);

    UPROPERTY()
    FRotator startRotation = FRotator(0);

    UPROPERTY()
    FVector initVelocity = FVector(0);

    UPROPERTY()
    FString RuntimeMeshPath;
};

USTRUCT()
struct FSimActorInput
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int32 type;

    UPROPERTY()
    FString typeName;

    UPROPERTY()
    int64 id = -1;

    UPROPERTY()
    double timeStamp = 0.f;

    UPROPERTY()
    FVector location = FVector(0);

    UPROPERTY()
    FRotator rotation = FRotator(0);

    UPROPERTY()
    FVector velocity = FVector(0);

    UPROPERTY()
    double timeStamp0 = 0;
    UPROPERTY()
    FVector sizeLWH;
};

USTRUCT()
struct FSimActorOutput : public FSimActorInput
{
    GENERATED_BODY()
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USimActorInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API ISimActorInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    // Get the configuration from SimActor
    virtual const FSimActorConfig* GetConfig() const = 0;

    // Init SimActor
    virtual void Init(const FSimActorConfig& _Config) = 0;

    // Update SimActor
    virtual void Update(const FSimActorInput& _Input, FSimActorOutput& _Output) = 0;

    // Destroy the SimActor
    virtual void Destroy() = 0;

    // Get current timestamp
    virtual double GetTimeStamp() const;
};

template <typename T, typename V>
T* Cast_Sim(V& From)
{
    V* Ptr = &From;
    return static_cast<T*>(Ptr);
}
