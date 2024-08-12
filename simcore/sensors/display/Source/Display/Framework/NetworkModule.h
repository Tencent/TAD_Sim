// Copyright (c) 2018 Tencent. All rights reserved.

#pragma once
#include "CoreMinimal.h"

// #ifdef _MSC_VER
// #include "AllowWindowsPlatformTypes.h"
// #endif
////Protobuf
// #include "traffic.pb.h"
// #include "location.pb.h"
// #include "planOutput.pb.h"
// #include "trajectory.pb.h"
// #include "grading.pb.h"
// #include "controlSim.pb.h"
// #include "planStatus.pb.h"
// #ifdef _MSC_VER
// #include "HideWindowsPlatformTypes.h"
// #endif

#include "txsim_module_service.h"
#include "DisplayGameInstance.h"
// #include "Containers/LockFreeList.h"
// #include "NetworkModule.generated.h"

// struct FSimUpdateIn;
// struct FSimUpdateOut;

/**
 *
 */
class DISPLAY_API NetworkModule : public tx_sim::SimModule
{
public:
    NetworkModule();
    ~NetworkModule();

    // void init(const sim_msg::InitParams& params, sim_msg::TopicList& topics);
    // void reset(const sim_msg::ResetParams& scenarioUrl);
    // bool step(const sim_msg::SimMessage& input, sim_msg::SimMessage& output);
    virtual void Init(tx_sim::InitHelper& helper);
    virtual void Reset(tx_sim::ResetHelper& helper);
    virtual void Step(tx_sim::StepHelper& helper);
    virtual void Stop(tx_sim::StopHelper& helper);
    // void stop(google::protobuf::Map<std::string, std::string>& feedbacks);
    FCriticalSection mutex_Input;
    FCriticalSection mutex_Output;
    FEvent* getThreadSuspendedEvent();
    class UDisplayGameInstance* myGameInstance = nullptr;

private:
    FEvent* threadSuspendedEvent = nullptr;
    FSimUpdateIn simUpdateIn;
    bool asynchronousMode = false;
    // Spy cycle time, in seconds.
    // double costTime = 0.f;

    FString TrafficTopic = TEXT("TRAFFIC");
    FString LocationTopic = TEXT("LOCATION");
    FString SensorTopic = TEXT("DISPLAYSENSOR_");
    FString PoseTopic = TEXT("DISPLAYPOSE_");

    FString UnionPrefixStr = TEXT("EgoUnion/");

    double time0 = 0;
    double realstep = 0;

    void PublicUpdateMessage(tx_sim::StepHelper& helper);

    // TArray<FSimData*> dataArry;
    // TLockFreePointerListLIFO<FSimData> dataList;

    void RunScript(const FString& Name);
};
