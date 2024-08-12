// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Objects/SimActorInterface.h"
#include "Objects/Transports/TransportInterface.h"
#include <string>
#include "SensorInterface.generated.h"

USTRUCT()
struct FSensorConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FString typeName;
    UPROPERTY()
    int32 id = -1;
    UPROPERTY()
    FVector installLocation = FVector(0);
    UPROPERTY()
    FRotator installRotation = FRotator(0);
    UPROPERTY()
    ETrafficType targetType;
    UPROPERTY()
    int32 targetId = 0;
    UPROPERTY()
    double frequency = 0;
    UPROPERTY()
    FString savePath;
    UPROPERTY()
    FString device;
    UPROPERTY()
    FString installSlot;

    static FString GetType(FString _TypeName)
    {
        FString Left;
        FString Right;
        _TypeName.Split(TEXT("/"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
        return Right;
    };

    FString GetType() const
    {
        return GetType(typeName);
    };
};

USTRUCT()
struct FSensorInput
{
    GENERATED_BODY()
public:
    UPROPERTY()
    double timeStamp;
    double timeStamp_ego = 0.;
    double timeStamp_tail = 0.;
};

USTRUCT()
struct FSensorOutput
{
    GENERATED_BODY()
public:
    double timeStamp = 0;
    int id = 0;
    FString type;
    std::string serialize_string;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USensorInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API ISensorInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual bool Init(const FSensorConfig& _Config) = 0;

    virtual void Update(const FSensorInput& _Input, FSensorOutput& _Output) = 0;

    virtual void Destroy(FString _Reason) = 0;

    FSensorConfig configBase;
    // virtual void Send() = 0;
};
