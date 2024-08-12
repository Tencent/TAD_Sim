// Copyright (c) 2018 Tencent. All rights reserved.

#include "NetworkModule.h"
// #include "Display.h"
// #ifdef _MSC_VER
// #include "AllowWindowsPlatformTypes.h"
// #endif
// #include "traffic.pb.h"
// #include "location.pb.h"
// #include "planOutput.pb.h"
// #include "trajectory.pb.h"
// #include "grading.pb.h"
// #include "controlSim.pb.h"
// #include "planStatus.pb.h"
//
// #ifdef _MSC_VER
// #include "HideWindowsPlatformTypes.h"
// #endif

// JiangyuLin
// #include "EgoVehicle.h"
#include "DisplayGameInstance.h"
#include "Engine/DataTable.h"
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>

DEFINE_LOG_CATEGORY_STATIC(SimLogNet, Log, All);
TXSIM_MODULE(NetworkModule)
// bool GetValueFromSimMessage(const sim_msg::SimMessage& msg, std::string key, std::string& value);
NetworkModule::NetworkModule()
{
    threadSuspendedEvent = FPlatformProcess::GetSynchEventFromPool();
}

NetworkModule::~NetworkModule()
{
    FPlatformProcess::ReturnSynchEventToPool(threadSuspendedEvent);
    threadSuspendedEvent = nullptr;
}

void NetworkModule::Init(/*const sim_msg::InitParams& params, sim_msg::TopicList& topics*/ tx_sim::InitHelper& helper)
{
    std::chrono::time_point<std::chrono::system_clock> Start, End;
    Start = std::chrono::system_clock::now();
    RunScript(TEXT("SimInit"));

    GConfig->GetString(TEXT("MessageTopic"), TEXT("Traffic"), TrafficTopic, GGameIni);
    GConfig->GetString(TEXT("MessageTopic"), TEXT("Location"), LocationTopic, GGameIni);
    // UE_LOG(SimLogNet, Log, TEXT("costTime init: %d seconds"), costTime);
    helper.Subscribe(TCHAR_TO_ANSI(*LocationTopic));
    helper.Subscribe("LOCATION_TRAILER");
    helper.Subscribe("TRAJECTORY");
    helper.Subscribe("PLAN_OUTPUT");
    helper.Subscribe(TCHAR_TO_ANSI(*TrafficTopic));
    helper.Subscribe("GRADING");
    helper.Subscribe("CONTROL");
    helper.Subscribe("PLAN_STATUS");
    helper.Subscribe("TX_Manual_Vehicle");
    helper.Subscribe("ENVIRONMENTAL");

    FString UnionTrafficTopic = UnionPrefixStr + LocationTopic;
    FString UnionTrafficTRAILERTopic = UnionPrefixStr + FString(TEXT("LOCATION_TRAILER"));
    helper.Subscribe(TCHAR_TO_ANSI(*UnionTrafficTopic));
    helper.Subscribe(TCHAR_TO_ANSI(*UnionTrafficTRAILERTopic));

    if (!helper.GetParameter("time0").empty())
    {
        time0 = std::atof(helper.GetParameter("time0").c_str());
        UE_LOG(SimLogNet, Log, TEXT("Display init time0 = %f"), time0);
    }
    if (!helper.GetParameter("step_size").empty())
    {
        realstep = std::atof(helper.GetParameter("step_size").c_str());
        UE_LOG(SimLogNet, Log, TEXT("Display init realstep = %f"), realstep);
    }

    if (myGameInstance->GetFlag_SendVilMsg())
    {
        helper.Publish("TX_Manual_Vehicle");
    }
    FString device;
    if (!FParse::Value(FCommandLine::Get(), TEXT("-topicId="), device))
    {
        if (!FParse::Value(FCommandLine::Get(), TEXT("-device="), device))
        {
            device = TEXT("0");
        }
    }
    SensorTopic = TEXT("DISPLAYSENSOR_") + device;
    PoseTopic = TEXT("DISPLAYPOSE_") + device;

    helper.Publish(std::string(TCHAR_TO_ANSI(*SensorTopic)));
    helper.Publish(std::string(TCHAR_TO_ANSI(*PoseTopic)));
    UE_LOG(SimLogNet, Log, TEXT("Display publish topic is %s"), *SensorTopic);

    // helper.Publish("ME_SENSOR0");
    // helper.Publish("ME_SENSOR1");
    // helper.Publish("ME_SENSOR2");
    // helper.Publish("ME_SENSOR3");
    // helper.Publish("ME_SENSOR4");
    // helper.Publish("ME_SENSOR5");
    // helper.Publish("ME_SENSOR6");
    // helper.Publish("ME_SENSOR7");
    // topics.add_topic(sim_msg::Topic::LOCATION);
    // topics.add_topic(sim_msg::Topic::TRAJECTORY);
    // topics.add_topic(sim_msg::Topic::PLAN_OUTPUT);
    // topics.add_topic(sim_msg::Topic::TRAFFIC);
    // topics.add_topic(sim_msg::Topic::GRADING);
    // topics.add_topic(sim_msg::Topic::CONTROL);
    // topics.add_topic(sim_msg::Topic::PLAN_STATUS);

    {
        FScopeLock ScopeLock(&mutex_Input);

        TSharedPtr<FSimInitIn> NewInPtr = MakeShared<FSimInitIn>();
        NewInPtr->clientNum = 1;
        NewInPtr->name = TEXT("INIT");
        NewInPtr->timeStamp = -1;

        myGameInstance->simInDataArry.Add(NewInPtr);
        myGameInstance->bSimInDataRefreshed = true;

        // simUpdateIn.frameID = 0;
        // myGameInstance->simState = ESimState::SA_INIT;
        ////myGameInstance->SimInput(true);
        // myGameInstance->bIsSimTrigger = true;
    }

    myGameInstance->threadSuspendedEvent->Trigger();

    // threadSuspendedEvent->Wait();
    // TODO: Get config value.

    End = std::chrono::system_clock::now();
    std::chrono::duration<double> CostTime = End - Start;
    UE_LOG(SimLogNet, Log, TEXT("Display Init Cost Time: %f seconds"), CostTime.count());
}

void NetworkModule::Reset(/*const sim_msg::ResetParams& scenarioUrl*/ tx_sim::ResetHelper& helper)
{
    UE_LOG(SimLogNet, Log, TEXT("Reset begin"));
    std::chrono::time_point<std::chrono::system_clock> Start, End;
    Start = std::chrono::system_clock::now();
    RunScript(TEXT("SimReset"));

    {
        FScopeLock ScopeLock(&mutex_Input);

        TSharedPtr<FSimResetIn> NewInPtr = MakeShared<FSimResetIn>();
        NewInPtr->configFilePath = UTF8_TO_TCHAR(helper.scenario_file_path().c_str());
        NewInPtr->configFilePath = NewInPtr->configFilePath.Replace(TEXT("\\"), TEXT("/"));

        sim_msg::Location StartLoc;
        StartLoc.ParseFromString(helper.ego_start_location().c_str());

        NewInPtr->startLon = StartLoc.position().x();
        NewInPtr->startLat = StartLoc.position().y();
        NewInPtr->startAlt = StartLoc.position().z();
        NewInPtr->startSpeed =
            FVector(StartLoc.velocity().x(), StartLoc.velocity().y(), StartLoc.velocity().z()).Size();
        NewInPtr->startTheta = StartLoc.rpy().z();
        NewInPtr->mapDataBasePath = UTF8_TO_TCHAR(helper.map_file_path().c_str());
        NewInPtr->mapDataBasePath = NewInPtr->mapDataBasePath.Replace(TEXT("\\"), TEXT("/"));
        NewInPtr->mapDataBaseName = FPaths::GetCleanFilename(NewInPtr->mapDataBasePath);
        NewInPtr->sceneBuffer = helper.scene_pb();

        FString tadsim_path;
        bool bUseLocalScenarioDir = false;
        GConfig->GetBool(TEXT("Sensor"), TEXT("bUseLocalScenarioDir"), bUseLocalScenarioDir, GGameIni);
        if (bUseLocalScenarioDir)
        {
            if (GConfig->GetString(TEXT("Sensor"), TEXT("TadsimConfigPath"), tadsim_path, GGameIni))
            {
            }
        }
        if (tadsim_path.IsEmpty())
        {
            if (FParse::Value(FCommandLine::Get(), TEXT("-tadsim_dir="), tadsim_path))
            {
            }
        }
        NewInPtr->tadsimPath =
            FPaths::GetPath(FPaths::GetPath(FPaths::GetPath(FPaths::GetPath(NewInPtr->configFilePath))));
        if (!tadsim_path.IsEmpty())
        {
            tadsim_path = tadsim_path.Replace(TEXT("\\"), TEXT("/"));

            UE_LOG(SimLogNet, Log, TEXT("tadsim dir is %s"), *tadsim_path);
            FString cfgStr =
                NewInPtr->configFilePath.Right(NewInPtr->configFilePath.Len() - NewInPtr->tadsimPath.Len() - 1);
            NewInPtr->configFilePath = FPaths::Combine(tadsim_path, cfgStr);

            cfgStr = NewInPtr->mapDataBasePath.Right(NewInPtr->mapDataBasePath.Len() - NewInPtr->tadsimPath.Len() - 1);
            NewInPtr->mapDataBasePath = FPaths::Combine(tadsim_path, cfgStr);

            NewInPtr->tadsimPath = tadsim_path;
        }

        NewInPtr->name = TEXT("RESET");
        NewInPtr->timeStamp = 0;
        myGameInstance->simInDataArry.Add(NewInPtr);
        asynchronousMode = myGameInstance->GetGameConfig(TEXT("Mode"), TEXT("Asynchronous")) == TEXT("true");
        myGameInstance->SetAsynchronousMode(asynchronousMode);
        myGameInstance->bSimInDataRefreshed = true;

        myGameInstance->ModuleGroupName = UTF8_TO_TCHAR(helper.group_name().c_str());

        // simUpdateIn.frameID = 0;
        // myGameInstance->simRestIn->configFilePath =
        // UTF8_TO_TCHAR(helper.scenario_file_path().c_str()/*scenarioUrl.scenariopath().c_str()*/);
        // myGameInstance->simState = ESimState::SA_RESET;
        ////myGameInstance->SimInput(true);
        // myGameInstance->bIsSimTrigger = true;
    }

    myGameInstance->threadSuspendedEvent->Trigger();

    // if (myGameInstance->IsLockStep())
    //{
    //     UE_LOG(SimLogNet, Log, TEXT("Reset thread wait"));
    //     threadSuspendedEvent->Wait();
    // }

    threadSuspendedEvent->Wait();

    End = std::chrono::system_clock::now();
    std::chrono::duration<double> CostTime = End - Start;
    UE_LOG(SimLogNet, Log, TEXT("Display Reset Cost Time: %f seconds"), CostTime.count());

    if (!myGameInstance->ResetFaildStr.IsEmpty())
    {
        throw std::runtime_error(TCHAR_TO_ANSI(*myGameInstance->ResetFaildStr));
    }
}

void NetworkModule::Step(/*const sim_msg::SimMessage& input, sim_msg::SimMessage& output*/ tx_sim::StepHelper& helper)
{
    double timestamp = helper.timestamp();
    double bei = realstep > 0 ? ((timestamp - time0) / realstep) : 1.0;
    if (bei < 0 || FMath::Modf(bei, &bei) > 1e-4)
    {
        // UE_LOG(SimLogNet, Log, TEXT("Display step jump: %f"), timestamp);
        return;
    }
    /*
    std::chrono::time_point<std::chrono::system_clock> Start, End;
    std::chrono::duration<double> CostTime;
    Start = std::chrono::system_clock::now();//*/

    // Lock input data
    {
        auto hilpos = myGameInstance->nHILpos;
        if (hilpos.X >= 0 && hilpos.Y >= 0)
        {
            timestamp = helper.current_timestamp() * 1000.0;
            double utc_offset = 0;
            GConfig->GetDouble(TEXT("Sensor"), TEXT("LidarUtcOffect"), utc_offset, GGameIni);
            timestamp += utc_offset * 1000.0;
        }

        FScopeLock ScopeLock(&mutex_Input);

        // simUpdateIn->frameID = input.frameid();
        simUpdateIn.timeStamp = timestamp;

        // sim_msg::Location egoLocation;
        std::string strLocation;
        helper.GetSubscribedMessage(TCHAR_TO_ANSI(*LocationTopic), strLocation);
        // if (!strLocation.empty())
        //{
        //     simUpdateIn.egoData.ParseFromString(strLocation);
        // }

        std::string strUnionLocation;
        helper.GetSubscribedMessage(TCHAR_TO_ANSI(*(UnionPrefixStr + LocationTopic)), strUnionLocation);

        // sim_msg::Location egoLocation;
        std::string strLocation_Trailer;
        helper.GetSubscribedMessage("LOCATION_TRAILER", strLocation_Trailer);
        // if (!strLocation_Trailer.empty())
        //{
        //     simUpdateIn.egoContainerData.ParseFromString(strLocation_Trailer);
        // }

        std::string strUnionLocation_Trailer;
        helper.GetSubscribedMessage(TCHAR_TO_ANSI(*(UnionPrefixStr + "LOCATION_TRAILER")), strUnionLocation_Trailer);

        // sim_msg::Trajectory trajectoryData;
        std::string strTrajectory;
        helper.GetSubscribedMessage("TRAJECTORY", strTrajectory);
        // if (!strTrajectory.empty())
        //{
        //     simUpdateIn.trajectoryData.ParseFromString(strTrajectory);
        // }

        // sim_msg::PlanOutput planOutputData;
        std::string strPlanOutput;
        helper.GetSubscribedMessage("PLAN_OUTPUT", strPlanOutput);
        // if (!strPlanOutput.empty())
        //{
        //     simUpdateIn.planOutputData.ParseFromString(strPlanOutput);
        // }

        // sim_msg::Traffic trafficData;
        std::string strTraffic;
        helper.GetSubscribedMessage(TCHAR_TO_ANSI(*TrafficTopic), strTraffic);
        // if (!strTraffic.empty())
        //{
        //     simUpdateIn.trafficData.ParseFromString(strTraffic);
        // }

        // Control
        // sim_msg::ControlSim controlSimData;
        std::string strControlSim;
        helper.GetSubscribedMessage("CONTROL", strControlSim);
        // if (!strControlSim.empty())
        //{
        //     simUpdateIn.controlSimData.ParseFromString(strControlSim);
        // }
        //  Plan_Status
        // sim_msg::PlanStatus PlanStatusData;
        std::string StrPlanStatus;
        helper.GetSubscribedMessage("PLAN_STATUS", StrPlanStatus);
        // if (!StrPlanStatus.empty())
        //{
        //     simUpdateIn.planStatusData.ParseFromString(StrPlanStatus);
        // }
        std::string StrEnviroment;
        helper.GetSubscribedMessage("ENVIRONMENTAL", StrEnviroment);

        // std::string StrMannedTraffic;
        // helper.GetSubscribedMessage("TX_Manual_Vehicle", StrMannedTraffic);
        // sim_msg::ManualVehicleControlInfoArray DebugInfo;
        // DebugInfo.ParseFromString(StrMannedTraffic);

        //{
        //    //TArray<FSimUpdateOut> simOuts;
        //    //myGameInstance->StepUpdateImmediately(simUpdateIn, simOuts);
        //    //PublicUpdateMessage(helper, simOuts);
        //}
        // if (asynchronousMode && myGameInstance->simState == ESimState::SA_UPDATE)
        //{
        //    //ÒìČœ·œÊœ
        //    return;
        //}

        //*myGameInstance->simUpdateIn = simUpdateIn;
        // myGameInstance->simState = ESimState::SA_UPDATE;
        ////myGameInstance->SimInput(true);
        // myGameInstance->bIsSimTrigger = true;
        // simUpdateIn.frameID += 1;

        TSharedPtr<FSimUpdateIn> NewInPtr = MakeShared<FSimUpdateIn>();
        NewInPtr->name = TEXT("UPDATE");
        NewInPtr->timeStamp = timestamp;

        sim_msg::Union UnionLocation;
        UnionLocation.ParseFromString(strUnionLocation);

        NewInPtr->egoData.Empty(UnionLocation.messages_size());
        for (int32 i = 0; i < UnionLocation.messages_size(); ++i)
        {
            const auto& msg = UnionLocation.messages(i);
            std::string groupname = msg.groupname();
            std::string content = msg.content();
            sim_msg::Location locationMsg;
            if (locationMsg.ParseFromString(content))
            {
                NewInPtr->egoData.Emplace(UTF8_TO_TCHAR(groupname.c_str()), locationMsg);
            }
        }

        sim_msg::Location ModuleLocation_Trailer;
        ModuleLocation_Trailer.ParseFromString(strLocation_Trailer);

        sim_msg::Union UnionLocation_Trailer;
        UnionLocation_Trailer.ParseFromString(strUnionLocation_Trailer);

        NewInPtr->egoContainerData.Empty(UnionLocation_Trailer.messages_size() + 1);
        NewInPtr->egoContainerData.Emplace(myGameInstance->ModuleGroupName, ModuleLocation_Trailer);

        for (int32 i = 0; i < UnionLocation_Trailer.messages_size(); ++i)
        {
            const auto& msg = UnionLocation_Trailer.messages(i);
            std::string groupname = msg.groupname();
            std::string content = msg.content();
            sim_msg::Location locationMsg_Trailer;
            if (locationMsg_Trailer.ParseFromString(content))
            {
                NewInPtr->egoContainerData.Emplace(UTF8_TO_TCHAR(groupname.c_str()), locationMsg_Trailer);
            }
        }

        NewInPtr->trajectoryData.ParseFromString(strTrajectory);
        NewInPtr->planOutputData.ParseFromString(strPlanOutput);
        NewInPtr->trafficData.ParseFromString(strTraffic);
        NewInPtr->controlSimData.ParseFromString(strControlSim);
        NewInPtr->planStatusData.ParseFromString(StrPlanStatus);
        NewInPtr->environmentData.ParseFromString(StrEnviroment);

        // delete myGameInstance->simInDataArry.Pop();
        myGameInstance->simInDataArry.Add(NewInPtr);
        myGameInstance->bSimInDataRefreshed = true;
    }
    // Unlock input data

    // Trigger gameinstance
    // myGameInstance->threadSuspendedEvent->Trigger();
    if (myGameInstance->bIsFrameSync)
    {
        // UE_LOG(SimLogNet, Log, TEXT("step trigger"));
        myGameInstance->threadSuspendedEvent->Trigger();
    }
    // End = std::chrono::system_clock::now();
    // CostTime = End - Start;
    // UE_LOG(SimLogNet, Log, TEXT("Display Update sync1 Cost Time: %f seconds"), CostTime.count());//*/
    //  Wait for gameinstance complete
    if (!asynchronousMode)
    {
        // UE_LOG(SimLogNet, Log, TEXT("step waiting"));
        threadSuspendedEvent->Wait();
        // UE_LOG(SimLogNet, Log, TEXT("step waiting ok"));
    }
    //*
    // Lock data
    PublicUpdateMessage(helper);
    // Unlock data

    /**/

    //// Sent message to output.
    //::google::protobuf::Map<::std::string, ::std::string>* outMessage = output.mutable_messages();
    //// Control send data.
    // sim_msg::ControlSim ControlSendData;
    //*ControlSendData.mutable_sim2car() = myGameInstance->GetControlSendData();
    // std::string ControlDataString;
    // ControlSendData.SerializeToString(&ControlDataString);
    //(*outMessage)[sim_msg::Topic_Name(sim_msg::Topic::CONTROL)] = ControlDataString;

    // if (myGameInstance->IsLockStep()) // Manned
    //{
    //     //override location if user in control
    //     sim_msg::Location overrideLocation;
    //     //threadSuspendedEvent->Wait();
    //     overrideLocation = myGameInstance->getOverrideLocation();
    //     //UE_LOG(LogTemp, Log, TEXT("Sent overrideLocation is: %s"),
    //     overrideLocation.velocity().DebugString().c_str()); std::string strOverrideLocation;
    //     overrideLocation.SerializeToString(&strOverrideLocation);
    //     (*outMessage)[sim_msg::Topic_Name(sim_msg::Topic::LOCATION)] = strOverrideLocation;
    //     if (myGameInstance->drivingMode == EDrivingMode::DM_MANNED)
    //     {
    //         //std::string ModeString("manned");
    //         //(*outMessage)[sim_msg::Topic_Name(sim_msg::Topic::DRIVING_MODE)] = ModeString;
    //         sim_msg::PlanStatus PlanStatus;
    //         sim_msg::PlanMode PlanMode = sim_msg::PlanMode::PLAN_MODE_MANUAL;
    //         std::string StrPlanStatus;
    //         PlanStatus.set_mode(PlanMode);
    //         PlanStatus.SerializeToString(&StrPlanStatus);
    //         (*outMessage)[sim_msg::Topic_Name(sim_msg::Topic::PLAN_STATUS)] = StrPlanStatus;
    //     }
    //     if (myGameInstance->drivingMode == EDrivingMode::DM_AUTOPILOT)
    //     {
    //         //std::string ModeString("auto");
    //         //(*outMessage)[sim_msg::Topic_Name(sim_msg::Topic::DRIVING_MODE)] = ModeString;
    //         sim_msg::PlanStatus PlanStatus;
    //         sim_msg::PlanMode PlanMode = sim_msg::PlanMode::PLAN_MODE_CHANGE_LANE;
    //         std::string StrPlanStatus;
    //         PlanStatus.set_mode(PlanMode);
    //         PlanStatus.SerializeToString(&StrPlanStatus);
    //         (*outMessage)[sim_msg::Topic_Name(sim_msg::Topic::PLAN_STATUS)] = StrPlanStatus;
    //     }

    //    //UE_LOG(LogTemp, Log, TEXT("Sent date done,timestamp is: %f"), (float)myGameInstance->getTimeStamp());
    //}
    // else // Autopilot
    //{
    //}

    /*
    End = std::chrono::system_clock::now();
    CostTime = End - Start;
    UE_LOG(SimLogNet, Log, TEXT("Display Update out Cost Time: %f seconds"), CostTime.count());//*/
    // return false;
}

void NetworkModule::Stop(/*google::protobuf::Map<std::string, std::string>& feedbacks*/ tx_sim::StopHelper& helper)
{
    UE_LOG(SimLogNet, Log, TEXT("Stop begin"));

    RunScript(TEXT("SimStop"));

    {
        FScopeLock ScopeLock(&mutex_Input);

        TSharedPtr<FSimStopIn> NewInPtr = MakeShared<FSimStopIn>();
        NewInPtr->name = TEXT("STOP");
        NewInPtr->timeStamp = -1;
        myGameInstance->simInDataArry.Add(NewInPtr);
    }

    simUpdateIn.frameID = 0;
    if (myGameInstance->bIsFrameSync)
    {
        UE_LOG(SimLogNet, Log, TEXT("stop trigger"));
        myGameInstance->threadSuspendedEvent->Trigger();
    }

    UE_LOG(SimLogNet, Log, TEXT("stop ok"));

    ////JiangyuLin
    // if (myGameInstance->egoVehicle && myGameInstance->egoVehicle->gradingManager)
    //{
    //     //myGameInstance->egoVehicle->gradingManager->Statistics();
    //     int Scount_1 = 0;
    //     int Scount_2 = 0;
    //     int Scount_3 = 0;

    //    int Acount_1 = 0;
    //    int Acount_2 = 0;
    //    int Acount_3 = 0;

    //    for (FFrameDataTable& Elem : myGameInstance->egoVehicle->gradingManager->frameTables)
    //    {
    //        //speed
    //        if (Elem.speed<6.5f&&Elem.speed>=4.5f)
    //        {
    //            Scount_3++;
    //        }
    //        else if (Elem.speed<4.5f&&Elem.speed >= 2.5f)
    //        {
    //            Scount_2++;
    //        }
    //        else
    //        {
    //            Scount_1++;
    //        }

    //        //acceleration
    //        if (Elem.accelerate<-1.f)
    //        {
    //            Acount_1++;
    //        }
    //        else if (Elem.accelerate>=-1&& Elem.accelerate<1)
    //        {
    //            Acount_2++;
    //        }
    //        else
    //        {
    //            Acount_3++;
    //        }
    //    }
    //    TArray<float> SRange = { 0,2.5,4.5,6.5 };
    //    TArray<float> ARange = { -10,-1,1,10 };
    //    TArray<int> Scount = { Scount_1,Scount_2,Scount_3 };
    //    TArray<int> Acount = { Acount_1,Acount_2,Acount_3 };

    //    std::string SR;
    //    std::string SC;
    //    std::string AR;
    //    std::string AC;
    //    for (size_t i = 0; i < SRange.Num()-1; i++)
    //    {

    //        std::string A2 = std::to_string(SRange[i]);
    //        std::string A3 = "-";
    //        std::string A4 = std::to_string(SRange[i+1]);
    //        std::string A5 = "";
    //        if (i!= SRange.Num() - 2)
    //        {
    //            A5 = ",";
    //        }
    //        SR += A2 + A3 + A4 + A5;
    //    }
    //    for (size_t i = 0; i < Scount.Num(); i++)
    //    {
    //        std::string A2 = std::to_string(Scount[i]);
    //        std::string A5 = "";
    //        if (i != Scount.Num() - 1)
    //        {
    //            A5 = ",";
    //        }
    //        SC += A2 + A5;
    //    }
    //    for (size_t i = 0; i < ARange.Num() - 1; i++)
    //    {

    //        std::string A2 = std::to_string(ARange[i]);
    //        std::string A3 = "-";
    //        std::string A4 = std::to_string(ARange[i + 1]);
    //        std::string A5 = "";
    //        if (i != ARange.Num() - 2)
    //        {
    //            A5 = ",";
    //        }
    //        AR += A2 + A3 + A4 + A5;
    //    }
    //    for (size_t i = 0; i < Acount.Num(); i++)
    //    {
    //        std::string A2 = std::to_string(Acount[i]);
    //        std::string A5 = "";
    //        if (i != Acount.Num() - 1)
    //        {
    //            A5 = ",";
    //        }
    //        AC += A2 + A5;
    //    }
    //    std::string A1 = "{x:[";
    //    std::string A2 = "],y:[";
    //    std::string A3 = "]}";

    //    std::string Result_Speed = A1 + SR + A2 + SC + A3;
    //    std::string Result_Acceleration = A1 + AR + A2 + AC + A3;

    //    feedbacks["Speed"] = Result_Speed;
    //    feedbacks["Acceleration"] = Result_Acceleration;

    //    std::cout << Result_Speed <<"/n"<< Result_Acceleration;
    //}
    ////Save file
    // if (myGameInstance && myGameInstance->egoVehicle && myGameInstance->egoVehicle->gradingManager)
    //{
    //     myGameInstance->egoVehicle->gradingManager->Statistics();
    //     myGameInstance->egoVehicle->gradingManager->Grading();
    //     myGameInstance->egoVehicle->gradingManager->SaveFile();
    // }
}

FEvent* NetworkModule::getThreadSuspendedEvent()
{
    return threadSuspendedEvent;
}

void NetworkModule::PublicUpdateMessage(tx_sim::StepHelper& helper)
{
    FScopeLock ScopeLock(&mutex_Output);
    TArray<FSimUpdateOut> OutputDataArry;
    for (const auto& sout : myGameInstance->simOutDataArry)
    {
        if (sout->datatype == 1)
        {
            const FSimUpdateOut* simOut = StaticCast<const FSimUpdateOut*>(sout.Get());
            std::string payload_;
            if (simOut->topic_egoData.size() && simOut->egoData.SerializeToString(&payload_) && payload_.size())
            {
                helper.PublishMessage(simOut->topic_egoData, payload_);
            }
            payload_.clear();
            if (simOut->topic_overrideEgoLocation.size() && simOut->overrideEgoLocation.SerializeToString(&payload_) &&
                payload_.size())
            {
                helper.PublishMessage(simOut->topic_overrideEgoLocation, payload_);
            }
            payload_.clear();
            if (simOut->topic_trafficData.size() && simOut->trafficData.SerializeToString(&payload_) && payload_.size())
            {
                helper.PublishMessage(simOut->topic_trafficData, payload_);
            }
            payload_.clear();
            if (simOut->topic_trajectoryData.size() && simOut->trajectoryData.SerializeToString(&payload_) &&
                payload_.size())
            {
                helper.PublishMessage(simOut->topic_trajectoryData, payload_);
            }
            payload_.clear();
            if (simOut->topic_planOutputData.size() && simOut->planOutputData.SerializeToString(&payload_) &&
                payload_.size())
            {
                helper.PublishMessage(simOut->topic_planOutputData, payload_);
            }
            payload_.clear();
            if (simOut->topic_controlSimData.size() && simOut->controlSimData.SerializeToString(&payload_) &&
                payload_.size())
            {
                helper.PublishMessage(simOut->topic_controlSimData, payload_);
            }
            payload_.clear();
            if (simOut->topic_planStatusData.size() && simOut->planStatusData.SerializeToString(&payload_) &&
                payload_.size())
            {
                helper.PublishMessage(simOut->topic_planStatusData, payload_);
            }
            payload_.clear();
            if (simOut->topic_manualTrafficData.size() && simOut->manualTrafficData.SerializeToString(&payload_) &&
                payload_.size())
            {
                helper.PublishMessage(simOut->topic_manualTrafficData, payload_);
            }
            payload_.clear();
            if (simOut->trafficPose.SerializeToString(&payload_) && payload_.size())
            {
                // UE_LOG(SimLogNet, Log, TEXT("[%f]: %d ego | %d car | %d static | %d dynamic"),
                // simOut->trafficPose.timestamp(),
                //     simOut->trafficPose.egos_size(), simOut->trafficPose.cars_size(),
                //     simOut->trafficPose.staticobstacles_size(), simOut->trafficPose.dynamicobstacles_size());
                helper.PublishMessage(std::string(TCHAR_TO_ANSI(*PoseTopic)), payload_);
            }
        }
        if (sout->datatype == 2)
        {
            const FSimSensorUpdateOut* simSenOut = StaticCast<const FSimSensorUpdateOut*>(sout.Get());
            std::string payload_;
            if (simSenOut->sensorData.SerializeToString(&payload_) && payload_.size())
            {
                helper.PublishMessage(std::string(TCHAR_TO_ANSI(*SensorTopic)), payload_);
            }
        }
    }
    myGameInstance->simOutDataArry.SetNum(0);
}

void NetworkModule::RunScript(const FString& Name)
{
    GLog->Flush();
    FString exepath = TEXT("");
    FString param = TEXT("");
    FString batpath = FPaths::ProjectSavedDir() + TEXT("Script/") + Name;
#if PLATFORM_WINDOWS
    exepath = TEXT("cmd.exe");
    param = TEXT("/C ");
    batpath += TEXT(".bat");
#else
    exepath = TEXT("/bin/bash");
    batpath += TEXT(".sh");
#endif
    if (FPaths::FileExists(batpath))
    {
        FString commandLine = param + FPaths::ConvertRelativePathToFull(batpath);
        UE_LOG(SimLogNet, Log, TEXT("Run script: %s %s"), *exepath, *commandLine);
        auto proc =
            FPlatformProcess::CreateProc(*exepath, *commandLine, false, true, true, nullptr, 0, nullptr, nullptr);
        FPlatformProcess::WaitForProc(proc);
    }
    else
    {
        UE_LOG(SimLogNet, Log, TEXT("Cannot find %s"), *batpath);
    }
}

// bool GetValueFromSimMessage(const sim_msg::SimMessage& msg, std::string key, std::string& value)
//{
//     google::protobuf::Map<std::string, std::string>::const_iterator  iter = msg.messages().find(key);
//     if (iter == msg.messages().end())
//         return false;
//
//     value = iter->second;
//     return true;
// }
