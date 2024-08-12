// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <string>
#include <map>
#include "VehicleSimUDPComponent.generated.h"

USTRUCT()
struct FVehDataGram
{
    GENERATED_USTRUCT_BODY()
public:
    // UPROPERTY()
    char szFlag[8];
    // UPROPERTY()
    short nID;
    // UPROPERTY()
    short nCmd;
    // UPROPERTY()
    char status[16];
    // UPROPERTY()
    float values[12];

    // FORCEINLINE FArchive& operator<<(FVehDataGram& TheStruct)
    //{
    //
    // }

    void DataUnSerialize(const std::string& _Data)
    {
        if (_Data.size() != 76)
        {
            return;
        }
        std::string MarkStr = _Data.substr(0, 8);
        std::string SysStr = _Data.substr(8, 2);
        std::string CmdStr = _Data.substr(10, 2);
        std::string StatusStr = _Data.substr(12, 16);
        std::string ValueStr = _Data.substr(28, 48);

        for (size_t i = 0; i < 8; i++)
        {
            szFlag[i] = MarkStr[i];
        }
        memcpy(&nID, SysStr.c_str(), 2);
        memcpy(&nCmd, CmdStr.c_str(), 2);
        for (size_t i = 0; i < 16; i++)
        {
            status[i] = StatusStr[i];
        }
        for (size_t i = 0; i < 12; i++)
        {
            std::string rvalue = ValueStr.substr(i * 4, 4);
            memcpy(&values[i], rvalue.c_str(), 4);
        }
    }

    std::string DataSerialize()
    {
        std::string DataStr;
        DataStr.resize(76);

        std::string MarkStr = "";
        for (size_t i = 0; i < 8; i++)
        {
            MarkStr = MarkStr + szFlag[i];
        }

        std::string SysStr = "";
        char SysChar[2];
        memcpy(&SysChar, &nID, 2);
        for (size_t i = 0; i < 2; i++)
        {
            SysStr = SysStr + SysChar[i];
        }

        std::string CmdStr = "";
        char CmdChar[2];
        memcpy(&CmdChar, &nCmd, 2);
        for (size_t i = 0; i < 2; i++)
        {
            CmdStr = CmdStr + CmdChar[i];
        }

        std::string StatusStr = "";
        for (size_t i = 0; i < 16; i++)
        {
            StatusStr = StatusStr + status[i];
        }

        std::string ValueStr = "";
        for (size_t i = 0; i < 12; i++)
        {
            char StrChar[4];
            memcpy(&StrChar, &values[i], 4);
            for (size_t j = 0; j < 4; j++)
            {
                ValueStr = ValueStr + StrChar[j];
            }
        }

        DataStr = MarkStr + SysStr + CmdStr + StatusStr + ValueStr;
        return DataStr;
    }
};

USTRUCT()
struct FVehDataGramRecv : public FVehDataGram
{
    GENERATED_USTRUCT_BODY()
private:
    std::map<std::string, size_t> mapStatus = {{"gear", 0}, {"handbrake", 1}, {"seatbelt", 2}, {"speaker", 3},
        {"turnleft", 4}, {"turnright", 5}, {"doubleflash", 6}, {"smalllamp", 7}, {"headlamp", 8}, {"highbeam", 9},
        {"foglamp", 10}, {"keyleft", 11}, {"keyright", 12}, {"keyconfirm", 13}, {"electrickey", 14}};

    std::map<std::string, size_t> mapValues = {{"brake", 0}, {"throttle", 1}, {"clutch", 2}, {"steeringwheel", 3}};

public:
    FVehDataGramRecv()
    {
        std::string MarkStr = "HZLH-IO\0";
        for (size_t i = 0; i < 8; i++)
        {
            szFlag[i] = MarkStr[i];
        }
        nID = 1;
        nCmd = 0;
    }

    void SetStatus(const std::string& _Name, short _Value)
    {
        // Find status name
        std::map<std::string, size_t>::const_iterator It_Statu;
        It_Statu = mapStatus.find(_Name);
        if (It_Statu != mapStatus.end())
        {
            status[It_Statu->second] = _Value;
        }
    }

    void SetValues(const std::string& _Name, float _Value)
    {
        // Find value name
        std::map<std::string, size_t>::const_iterator It_Value;
        It_Value = mapValues.find(_Name);
        if (It_Value != mapValues.end())
        {
            values[It_Value->second] = _Value;
        }
    }

    short GetStatus(const std::string& _Name)
    {
        // Find status name
        std::map<std::string, size_t>::const_iterator It_Statu;
        It_Statu = mapStatus.find(_Name);
        if (It_Statu != mapStatus.end())
        {
            return status[It_Statu->second];
        }
        return -1;
    }

    float GetValues(const std::string& _Name)
    {
        // Find value name
        std::map<std::string, size_t>::const_iterator It_Value;
        It_Value = mapValues.find(_Name);
        if (It_Value != mapValues.end())
        {
            return values[It_Value->second];
        }
        return -1;
    }
};

USTRUCT()
struct FVehDataGramSend : public FVehDataGram
{
    GENERATED_USTRUCT_BODY()
private:
    std::map<std::string, size_t> mapStatus = {
        {"type", 0}, {"panellight", 1}, {"leftturnlamp", 2}, {"rightturnlamp", 3}, {"smalllamp", 4}, {"headlamp", 5},
        {"highbeam", 6}, {"foglamp", 7}, {"seatbeltsign", 8}, {"batterysign", 9}, {"enginefaultsign", 10},
        {"enginestatus", 11} /*,
          {"Undefined",11},
          {"Undefined",12},
          {"Undefined",13},
          {"Undefined",14},
          {"Undefined",13}*/
    };

    std::map<std::string, size_t> mapValues = {
        {"rpm", 0}, {"speed", 1}, {"x", 2}, {"y", 3}, {"z", 4}, {"yaw", 5}, {"pitch", 6}, {"roll", 7}, {"ax", 8},
        {"ay", 9}, {"az", 10} /*,
                     {"Undefined",13}*/
    };

public:
    FVehDataGramSend()
    {
        std::string MarkStr = "HZLH-IO\0";
        for (size_t i = 0; i < 8; i++)
        {
            szFlag[i] = MarkStr[i];
        }
        nID = 0;
        nCmd = 1;
        status[0] = 2;
    }

    void SetStatus(const std::string& _Name, short _Value)
    {
        // Find status name
        std::map<std::string, size_t>::const_iterator It_Statu;
        It_Statu = mapStatus.find(_Name);
        if (It_Statu != mapStatus.end())
        {
            status[It_Statu->second] = _Value;
        }
    }

    void SetValues(const std::string& _Name, float _Value)
    {
        // Find value name
        std::map<std::string, size_t>::const_iterator It_Value;
        It_Value = mapValues.find(_Name);
        if (It_Value != mapValues.end())
        {
            values[It_Value->second] = _Value;
        }
    }
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API UVehicleSimUDPComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UVehicleSimUDPComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    bool bCommunicationIsActive = false;

    FVehDataGramRecv dataReceive;
    FVehDataGramSend dataSend;

    FString defaultIp_Recv;
    int32 defaultPort_Recv;
    FString defaultIp_Send;
    int32 defaultPort_Send;

public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    void StartCommunication(const FString& _Ip_Recv, int32 _Port_Recv, const FString& _Ip_Send, int32 _Port_Send);
    void StartCommunication();
    void EndCommunication();

    void ReceiveData();
    void SendData();

    bool GetCommunicationActive() const
    {
        return bCommunicationIsActive;
    }

    float GetValue_SteeringWheel();
    float GetValue_Brake();
    float GetValue_Throttle();
    int32 GetValue_Gear();

    int32 GetValue_KeyConfirm();
    int32 GetValue_KeyLeft();

    void SetValue_Speed(float _Value);
    void SetValue_RotationSpeed(float _Value);

    void SetValue_RPY(const FRotator& _Value);
    void SetValue_Accel(const FVector& _Value);
};
