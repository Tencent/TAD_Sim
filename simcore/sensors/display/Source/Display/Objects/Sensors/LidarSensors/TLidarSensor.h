// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LidarSensorDef.h"
#include "../SensorActor.h"
#include "lidar/Lidar.h"
#include "lidar/LidarModel.h"
#include "Networking.h"
#include <list>
#include <atomic>
#include <vector>
#include <chrono>
#include "TLidarSensor.generated.h"

/**
 *
 */
UCLASS()
class DISPLAY_API ATLidarSensor : public ASensorActor
{
    GENERATED_BODY()
public:
    virtual bool Init(const FSensorConfig& _Config);

    virtual void Update(const FSensorInput& _Input, FSensorOutput& _Output);

    virtual void Destroy(FString _Reason = "");

public:
    // Sets default values for this actor's properties
    ATLidarSensor(const FObjectInitializer& ObjectInitializer);
    /** Called whenever this actor is being removed from a level */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditDefaultsOnly)
    class USceneComponent* root;

    // virtual bool SetSensorConfig(struct FSensorConfigStruct &_NewConfig);
    // virtual void Update(double _TimeStamp, struct FSensorDataStruct &_NewData);
    // virtual bool SaveData();

private:
    /// Creates a Laser.
    bool CreateLasers();
    bool Save(const lidar::TraditionalLidar::lidar_ptset& data, double timeStamp);
    FString LoadAngleDefinition(const FString& fpath);

    FLidarConfig config;
    // AEnvironmentManager* EnvmM;
    std::shared_ptr<lidar::TraditionalLidar> LidarSensor;
    std::shared_ptr<LidarBufferFun> lidarBuffer;

    class AEnvManager* envManager = NULL;
    FTLidarMeasurement LidarMeasurement;
    LidarModel lidarMd;
    bool public_msg = false;
    FString coordType;
    bool lidarFrameAlign = false;
    double timeStamp_last = -10000;

    class SendDataThread : public FRunnable
    {
    public:
        SendDataThread(ATLidarSensor* ls);
        ~SendDataThread();

        void SetData(const FTLidarMeasurement& measure, TSharedPtr<LidarBuffer> buffer);

        // FRunnable interface.
        virtual bool Init();
        virtual uint32 Run();

        // Use this method to kill the thread!!
        void EnsureCompletion();

        struct FrameData
        {
            bool ok = {false};
            double timestamp0 = 0;    // begin of frame
            double timestamp1 = 0;    // end of frame
            FVector loc0;             // first loc
            FRotator rot0;            // first rot
            FVector loc1;             // last loc
            FRotator rot1;            // last rot
            lidar::TraditionalLidar::lidar_ptset data;
        };

        std::list<FrameData*> lidarFrame;
        FCriticalSection m_lidarFrame;

    private:
        ATLidarSensor* lidarActor;
        struct BufferData
        {
            FTLidarMeasurement measure;
            TSharedPtr<LidarBuffer> buffer;
            TSharedPtr<lidar::TraditionalLidar::lidar_ptset> dataPtr;
            int state = 0;
        };

        std::list<BufferData> lidarBuffers;
        FCriticalSection m_lidarBuffer;

        TArray<FRunnableThread*> Thread;

        FThreadSafeBool m_Kill = false;
        FThreadSafeBool send_key = true;
    };

    friend class SendDataThread;
    TSharedPtr<class SendDataThread> senddataThreadHandle;
};
