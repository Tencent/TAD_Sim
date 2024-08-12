// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveDataThread.h"
// #include "Display.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformProcess.h"
#include "Runtime/Core/Public/HAL/Event.h"
#include <string>
#include <iostream>
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Engine/Public/HighResScreenshot.h"

// SaveDataThread* SaveDataThread::instance = NULL;
#define PACK_SIZE 40960    // udp pack size; note that OSX limits < 8100 bytes

SaveDataThread::SaveDataThread()
{
    m_Kill = false;
    m_Pause = false;

    // Initialize FEvent (as a cross platform (Confirmed Mac/Windows))
    m_semaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);

    Thread = FRunnableThread::Create(this, TEXT("SaveDataThread"), 0, EThreadPriority::TPri_Normal);
}

SaveDataThread::~SaveDataThread()
{
    if (m_semaphore)
    {
        // Cleanup the FEvent
        FGenericPlatformProcess::ReturnSynchEventToPool(m_semaphore);
        m_semaphore = nullptr;
    }

    if (Thread)
    {
        // Cleanup the worker thread
        delete Thread;
        Thread = nullptr;
    }

    // if (instance)
    //{
    //     delete instance;
    //     instance = NULL;
    // }
    UE_LOG(LogTemp, Log, TEXT("SaveDataThread Has Delete!"));
}

bool SaveDataThread::Init()
{
    // imageWrapperModule = &FModuleManager::LoadModuleChecked< IImageWrapperModule >("ImageWrapper");
    int32 SendSize = PACK_SIZE;
    SenderSocket = FUdpSocketBuilder(TEXT("SensorBuf"))
                       .AsReusable()
                       .WithBroadcast()    /////////////
                       .WithSendBufferSize(SendSize)
        //.BoundToEndpoint(Endpoint)
        ;
    if (SenderSocket)
    {
        // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

        // Set Send Buffer Size
        SenderSocket->SetSendBufferSize(SendSize, SendSize);
        SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
        UE_LOG(LogTemp, Log, TEXT("SenderSocket SendSize is %d"), SenderSocket);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SenderSocket Cannot be Created!"));
    }

    return true;
}

uint32 SaveDataThread::Run()
{
    // Initial wait before starting
    FPlatformProcess::Sleep(0.03);

    while (!m_Kill)
    {
        if (m_Pause)
        {
            // FEvent->Wait(); will "sleep" the thread until it will get a signal "Trigger()"
            m_semaphore->Wait();

            if (m_Kill)
            {
                return 0;
            }
        }
        else
        {
            SaveData();
            // if (DoHaveWork())
            //{
            //     //A little sleep between the chunks (So CPU will rest a bit -- (may be omitted))
            //     FPlatformProcess::Sleep(0.1);
            // }
            // else
            //{
            //
            // }
        }
    }
    return 0;
}

void SaveDataThread::Stop()
{
    m_Kill = true;    // Thread kill condition "while (!m_Kill){...}"
    m_Pause = false;

    if (m_semaphore)
    {
        // We shall signal "Trigger" the FEvent (in case the Thread is sleeping it shall wake up!!)
        m_semaphore->Trigger();
    }
}

void SaveDataThread::EnsureCompletion()
{
    Stop();
    if (Thread)
    {
        Thread->WaitForCompletion();
    }
}

void SaveDataThread::PauseThread()
{
    m_Pause = true;
}

void SaveDataThread::ContinueThread()
{
    m_Pause = false;

    if (m_semaphore)
    {
        // Here is a FEvent signal "Trigger()" -> it will wake up the thread.
        m_semaphore->Trigger();
    }
}

bool SaveDataThread::IsThreadPaused()
{
    return (bool) m_Pause;
}

bool SaveDataThread::SaveString(const FString& _String, const FString _Path)
{
    // if (m_mutex.TryLock())
    //{
    //     FStringData& NewData = stringDataArry.Emplace_GetRef();
    //     NewData.data = _MessageArry;
    //     NewData.path = _Path;
    //     m_mutex.Unlock();
    //     return true;
    // }
    // return false;
    m_mutex.Lock();
    FStringData& NewData = stringDataArry.Emplace_GetRef();
    NewData.data = _String;
    NewData.path = _Path;
    m_mutex.Unlock();
    return true;
}

bool SaveDataThread::SaveJPG(const TArray64<uint8>& _DataArry, const FString _Path)
{
    m_mutex.Lock();
    FJPGData& NewData = jpgDataArry.Emplace_GetRef();
    NewData.data = _DataArry;
    NewData.path = _Path;
    m_mutex.Unlock();
    return true;
}

bool SaveDataThread::SendBmp(const TArray<FColor>& dataArry, int32 w, int32 h, int32 port)
{
    if (!ImageWrapper)
    {
        IImageWrapperModule& ImageWrapperModule =
            FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
        ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
    }

    m_mutex.Lock();
    FBmpData& NewData = bmpDataArry.Emplace_GetRef();
    NewData.data = dataArry;
    NewData.port = port;
    NewData.height = h;
    NewData.width = w;
    m_mutex.Unlock();
    return true;
}

void SaveDataThread::SaveData()
{
    TArray<FStringData> ReadyToSave_String;
    TArray<FJPGData> ReadyToSave_JPG;
    TArray<FBmpData> ReadyToSave_Bmp;
    m_mutex.Lock();
    if (stringDataArry.Num() > 0)
    {
        ReadyToSave_String = stringDataArry;
        stringDataArry.Empty();
    }
    if (jpgDataArry.Num() > 0)
    {
        ReadyToSave_JPG = jpgDataArry;
        jpgDataArry.Empty();
    }
    if (bmpDataArry.Num() > 0)
    {
        ReadyToSave_Bmp = bmpDataArry;
        bmpDataArry.Empty();
    }
    m_mutex.Unlock();
    for (auto& Elem : ReadyToSave_String)
    {
        FFileHelper::SaveStringToFile(Elem.data, *Elem.path, FFileHelper::EEncodingOptions::AutoDetect,
            &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
    }
    for (auto& Elem : ReadyToSave_JPG)
    {
        FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();

        FFileHelper::SaveArrayToFile(Elem.data, *Elem.path);
    }
    if (ReadyToSave_Bmp.Num() && ImageWrapper)
    {
        for (auto& Elem : ReadyToSave_Bmp)
        {
            if (Elem.data.Num() == 0)
            {
                continue;
            }
            if (!ImageWrapper->SetRaw(Elem.data.GetData(), sizeof(FColor) * Elem.data.Num(), Elem.width, Elem.height,
                    ERGBFormat::BGRA, 8))
            {
                UE_LOG(LogTemp, Warning, TEXT("SaveDataThread: ImageWrapper faild"));
                continue;
            }
            auto buf = ImageWrapper->GetCompressed(18);

            auto* ia = remoteAddr.Find(Elem.port);
            if (!ia)
            {
                ia = &remoteAddr.Add(Elem.port);
                ia->frame = 0;
                ia->addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
                bool bIsValid = false;
                ia->addr->SetIp(TEXT("127.0.0.1"), bIsValid);
                ia->addr->SetPlatformPort(Elem.port);
            }
            if (SenderSocket)
            {
                int pkgs = PACK_SIZE - sizeof(int) * 3;

                int total_pack = 1 + (buf.Num() - 1) / pkgs;
                TArray<uint8> abuf;
                abuf.SetNum(PACK_SIZE);
                int* ibuf = (int*) abuf.GetData();
                ibuf[0] = ia->frame;
                ibuf[1] = total_pack;
                ibuf[2] = 0;

                int32 BytesSent = 0;
                for (int i = 0; i < total_pack; i++)
                {
                    ibuf[2] = i;
                    memcpy(abuf.GetData() + sizeof(int) * 3, &buf[i * pkgs],
                        FMath::Min(buf.Num() - i * pkgs, (int64) pkgs));
                    // FPlatformProcess::Sleep(0.0001f);
                    SenderSocket->SendTo(abuf.GetData(), PACK_SIZE, BytesSent, *ia->addr);
                }

                ia->frame++;
            }
        }
    }

    // if (m_mutex.TryLock())
    //{
    //     //bool IsSuccessed = false;
    //     //for (auto &Elem : stringDataArry)
    //     //{
    //     //    IsSuccessed = FFileHelper::SaveStringToFile(Elem.data, *Elem.path,
    //     FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
    //     //}
    //     //if (IsSuccessed)
    //     //{
    //     //    stringDataArry.Empty();
    //     //}
    //     TArray<FStringData> ToSaveArry = stringDataArry;
    //     stringDataArry.Empty();
    //     m_mutex.Unlock();
    //     for (auto &Elem : ToSaveArry)
    //     {
    //         FFileHelper::SaveStringToFile(Elem.data, *Elem.path, FFileHelper::EEncodingOptions::AutoDetect,
    //         &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
    //     }
    // }
    // if (mutexImage.TryLock())
    //{
    //     mutexImage.Unlock();
    // }
}

bool SaveDataThread::DoHaveWork()
{
    if (stringDataArry.Num() > 0)
    {
        return true;
    }
    return false;
}
