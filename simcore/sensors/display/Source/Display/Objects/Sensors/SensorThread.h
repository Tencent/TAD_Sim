// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runnable.h"
#include "SensorInterface.h"

/**
 *
 */
class DISPLAY_API SensorThread : public FRunnable
{
public:
    SensorThread();
    ~SensorThread();

    bool triggerFlag = false;

protected:
    ISensorInterface* sensor;

public:
    // Begin FRunnable interface.
    virtual bool Init();
    virtual uint32 Run();
    virtual void Stop();
    // End FRunnable interface
};
