// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "Runtime/Core/Public/HAL/PlatformProcess.h"
#include "Runtime/Core/Public/HAL/ThreadSafeBool.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "Networking.h"

struct FStringData
{
    FString data;
    FString path;
};

struct FJPGData
{
    TArray<uint8> data;
    FString path;
};

struct FBmpData
{
    TArray<FColor> data;
    int32 width;
    int32 height;
    int32 port;
};

/**
 *
 */
class DISPLAY_API SaveDataThread : public FRunnable
{
public:
    SaveDataThread();
    ~SaveDataThread();

    // FRunnable interface.
    virtual bool Init();
    virtual uint32 Run();
    virtual void Stop();

    // Use this method to kill the thread!!
    void EnsureCompletion();
    // Pause the thread
    void PauseThread();
    // Continue/UnPause the thread
    void ContinueThread();
    bool IsThreadPaused();

    // Path with name and type.
    bool SaveString(const FString& _String, const FString _Path);
    bool SaveJPG(const TArray64<uint8>& _DataArry, const FString _Path);
    bool SendBmp(const TArray<FColor>& dataArry, int32 w, int32 h, int32 port);

private:
    // static SaveDataThread* instance;

    // Thread to run the worker FRunnable on
    FRunnableThread* Thread;

    FCriticalSection m_mutex;
    FCriticalSection mutexImage;
    FEvent* m_semaphore;

    // As the name states those members are Thread safe
    FThreadSafeBool m_Kill;
    FThreadSafeBool m_Pause;

    TArray<FStringData> stringDataArry;
    TArray<FJPGData> jpgDataArry;
    TArray<FBmpData> bmpDataArry;

    TSharedPtr<IImageWrapper> ImageWrapper;

    // udp
    FSocket* SenderSocket = nullptr;
    struct InternetAddr
    {
        int frame;
        TSharedPtr<FInternetAddr> addr;
    };
    TMap<int32, InternetAddr> remoteAddr;

    void SaveData();
    bool DoHaveWork();
};
