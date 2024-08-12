// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ObjInterface.generated.h"

UENUM()
enum class EReturnType : uint8
{
    RT_COPY,
    RT_CALCULATION
};

USTRUCT()
struct FObjInit
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FObjUpdate
{
    GENERATED_BODY()
public:
    double timeStamp = 0.f;
};

USTRUCT()
struct FObjReturn
{
    GENERATED_BODY()
public:
    EReturnType type = EReturnType::RT_COPY;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UObjInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class DISPLAY_API IObjInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    /* Local Interface */

    /**
     * Called by object`s manager, init object, and send data.
     */
    virtual void ObjInit(const FObjInit& _InitData) = 0;

    /**
     * Called by object`s manager, begin update, and send data.
     */
    virtual void ObjUpdate(const FObjUpdate& _UpdateData) = 0;

    /**
     * Called by object, tell object`s manager this update is done, and return data.
     */
    virtual void ObjReturn(const FObjReturn& _ReturnData, const IObjInterface* _Parent) = 0;

    /**
     * Called by object or manager, destroy object.
     */
    virtual void ObjDestroy() = 0;
};
