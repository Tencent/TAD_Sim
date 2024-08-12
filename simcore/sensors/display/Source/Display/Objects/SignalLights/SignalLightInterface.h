// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Display/Objects/SimActorInterface.h"
#include "SignalLightInterface.generated.h"

USTRUCT()
struct FSignalLightConfig : public FSimActorConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FColor color = FColor::Black;
};

USTRUCT()
struct FSignalLightIn : public FSimActorInput
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FColor color = FColor::Black;
};

USTRUCT()
struct FSignalLightOut : public FSimActorOutput
{
    GENERATED_BODY()
public:
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USignalLightInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API ISignalLightInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual FColor GetColor() const;
    virtual bool SetColor(const FColor& _Color);
    virtual bool SwitchLight(bool _IsOn);
    virtual bool IsSwitchOn() const;
};
