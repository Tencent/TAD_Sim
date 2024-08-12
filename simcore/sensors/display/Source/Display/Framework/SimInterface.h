// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "SimInterface.generated.h"

UENUM(BlueprintType)
enum class ESimState : uint8
{
    SA_DONE,
    SA_INIT,
    SA_RESET,
    SA_UPDATE
};

// USTRUCT()
struct FSimData
{
    // GENERATED_BODY()
public:
    virtual ~FSimData()
    {
    }
    int datatype = 0;

    double timeStamp = 0.f;
    ESimState state = ESimState::SA_DONE;
    FString name;
    int bIsConsumed = 0;
    double timeStamp_ego = 0.f;
    double timeStamp_tail = 0.f;
};

USTRUCT()
struct FLocalData
{
    GENERATED_BODY()
public:
    // bool bIsSuccess = false;
    double timeStamp = 0.f;
    ESimState state = ESimState::SA_DONE;
    FString name;
    double timeStamp_ego = 0.f;
    double timeStamp_tail = 0.f;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USimInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API ISimInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    /**
     * Sim interface
     * Trigger simulator action to GameInstance from SimModule.
     */
    virtual void SimInput(bool _bNeedReturn = true);

    /**
     * Sim interface
     * Trigger simulator action to GameInstance from SimModule.
     */
    virtual void SimInput(const FSimData& _Input, bool _bNeedReturn = true);

    /**
     * Sim interface
     * Trigger simulator action to GameInstance from SimModule.
     */
    virtual void SimInput(const FLocalData& _Input);

    virtual void SimCacul(FLocalData& _Output);

    virtual bool IsComplete();

    // virtual void ChildrenReturn(const FLocalData& _ChildrenInput, const ISimInterface* _ChildNode);

    /**
     * Sim interface
     * Receive action`s result to GameInstance from GameMode.
     */
    virtual void SimOutput(const FLocalData& _Output);
};
