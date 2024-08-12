// Fill out your copyright notice in the Description page of Project Settings.

#include "VehicleSimUDPComponent.h"
#include "UDPFunctionLibrary.h"
#include <string>
#include "Framework/DisplayPlayerController.h"
#include "Misc/ConfigCacheIni.h"

// Sets default values for this component's properties
UVehicleSimUDPComponent::UVehicleSimUDPComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

// Called when the game starts
void UVehicleSimUDPComponent::BeginPlay()
{
    Super::BeginPlay();
    // StartCommunication();
    //  ...

    // Get default ip
    GConfig->GetString(TEXT("VehicleSimulator"), TEXT("ReceiveIP"), defaultIp_Recv, GGameIni);
    GConfig->GetString(TEXT("VehicleSimulator"), TEXT("SendIP"), defaultIp_Send, GGameIni);
    // Get default port
    GConfig->GetInt(TEXT("VehicleSimulator"), TEXT("ReceivePort"), defaultPort_Recv, GGameIni);
    GConfig->GetInt(TEXT("VehicleSimulator"), TEXT("SendPort"), defaultPort_Send, GGameIni);
}

// Called every frame
void UVehicleSimUDPComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ReceiveData();

    //// turn to manned
    // if (dataReceive.GetStatus("keyconfirm") == 1 &&
    // GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->GetDrivingMode() == 0)
    //{
    //     GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->SetDrivingMode(1);
    // }

    //// turn to auto
    // if (dataReceive.GetStatus("keyleft") == 1 &&
    // GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->GetDrivingMode() == 1)
    //{
    //     GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->SetDrivingMode(0);
    // }

    SendData();

    // ...
}

void UVehicleSimUDPComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    // EndCommunication();
}

void UVehicleSimUDPComponent::StartCommunication()
{
    if (bCommunicationIsActive)
    {
        return;
    }
    bool Success = false;
    UUDPFunctionLibrary::StartUDPReceiver(TEXT("Listen"), defaultIp_Recv, defaultPort_Recv, Success);
    UUDPFunctionLibrary::StartUDPSender(TEXT("Send"), defaultIp_Send, defaultPort_Send);
    bCommunicationIsActive = Success;
}

void UVehicleSimUDPComponent::StartCommunication(
    const FString& _Ip_Recv, int32 _Port_Recv, const FString& _Ip_Send, int32 _Port_Send)
{
    if (bCommunicationIsActive)
    {
        return;
    }
    bool Success = false;
    UUDPFunctionLibrary::StartUDPReceiver(TEXT("Listen"), _Ip_Recv, _Port_Recv, Success);
    UUDPFunctionLibrary::StartUDPSender(TEXT("Send"), _Ip_Send, _Port_Send);
    bCommunicationIsActive = Success;
}

void UVehicleSimUDPComponent::EndCommunication()
{
    if (bCommunicationIsActive)
    {
        return;
    }
    UUDPFunctionLibrary::CloseUDPReceiver();
    UUDPFunctionLibrary::CloseUDPSender();
    bCommunicationIsActive = false;
}

void UVehicleSimUDPComponent::ReceiveData()
{
    if (bCommunicationIsActive)
    {
        std::string ReceiverData;
        bool Success = false;
        // UUDPFunctionLibrary::DataRecv(ReceiverData, Success);
        UUDPFunctionLibrary::DataRecvFromSend(ReceiverData, Success);
        if (Success)
        {
            dataReceive.DataUnSerialize(ReceiverData);
            // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %f"), *Msg, Value));
            //  dataReceive.GetValues(("steeringwheel"));
            // dataReceive.GetValues(("throttle"));
            // dataReceive.GetValues(("brake"));
        }
    }
}

void UVehicleSimUDPComponent::SendData()
{
    if (bCommunicationIsActive)
    {
        //// Test
        // dataSend.SetStatus("gear", 1);
        // dataSend.SetStatus("handbrake", 1);
        // dataSend.SetStatus("seatbelt", 1);
        // dataSend.SetStatus("speaker", 1);
        // dataSend.SetStatus("turnleft", 1);
        // dataSend.SetStatus("turnright", 1);
        // dataSend.SetStatus("doubleflash", 1);
        // dataSend.SetStatus("smalllamp", 1);
        // dataSend.SetStatus("headlamp", 1);
        // dataSend.SetStatus("highbeam", 1);
        // dataSend.SetStatus("foglamp", 1);
        // dataSend.SetStatus("keyleft", 1);
        // dataSend.SetStatus("keyright", 1);
        // dataSend.SetStatus("keyconfirm", 1);
        // dataSend.SetStatus("electrickey", 1);

        // dataSend.SetValues("brake", 0.5);
        // dataSend.SetValues("throttle", 0.5);
        // dataSend.SetValues("clutch", 0.5);
        // dataSend.SetValues("steeringwheel", 0.5);

        std::string SendStr = dataSend.DataSerialize();
        // FString FStr = UTF8_TO_TCHAR(SendStr.c_str());
        UUDPFunctionLibrary::RamaUDPSender_SendString(SendStr);
    }
}

float UVehicleSimUDPComponent::GetValue_SteeringWheel()
{
    return dataReceive.GetValues(("steeringwheel"));
}

float UVehicleSimUDPComponent::GetValue_Brake()
{
    return dataReceive.GetValues(("brake"));
}

float UVehicleSimUDPComponent::GetValue_Throttle()
{
    return dataReceive.GetValues(("throttle"));
}

int32 UVehicleSimUDPComponent::GetValue_Gear()
{
    return dataReceive.GetStatus(("gear"));
}

int32 UVehicleSimUDPComponent::GetValue_KeyConfirm()
{
    return dataReceive.GetStatus("keyconfirm");
}

int32 UVehicleSimUDPComponent::GetValue_KeyLeft()
{
    return dataReceive.GetStatus("keyleft");
}

void UVehicleSimUDPComponent::SetValue_Speed(float _Value)
{
    dataSend.SetValues("speed", _Value);
}

void UVehicleSimUDPComponent::SetValue_RPY(const FRotator& _Value)
{
    dataSend.SetValues("roll", -1 * _Value.Roll * PI / 180);
    dataSend.SetValues("pitch", -1 * _Value.Pitch * PI / 180);
    dataSend.SetValues("yaw", -1 * _Value.Yaw * PI / 180);
}

void UVehicleSimUDPComponent::SetValue_Accel(const FVector& _Value)
{
    dataSend.SetValues("ax", _Value.X);
    dataSend.SetValues("ay", -1 * _Value.Y);
    dataSend.SetValues("az", _Value.Z);
}

void UVehicleSimUDPComponent::SetValue_RotationSpeed(float _Value)
{
    dataSend.SetValues("rpm", _Value);
}
