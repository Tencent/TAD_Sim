// Fill out your copyright notice in the Description page of Project Settings.

#include "UDPFunctionLibrary.h"

TSharedPtr<FInternetAddr> RemoteAddr;
FSocket* SenderSocket;

FSocket* ListenSocket;
FUdpSocketReceiver* UDPReceiver = nullptr;

// ScreenMsg
void ScreenMsg(const FString& Msg)
{
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Msg);
}
void ScreenMsg(const FString& Msg, const float Value)
{
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %f"), *Msg, Value));
}
void ScreenMsg(const FString& Msg, const FString& Msg2)
{
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %s"), *Msg, *Msg2));
}

void UUDPFunctionLibrary::CloseUDPSender()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

bool UUDPFunctionLibrary::RamaUDPSender_SendString(std::string ToSend)    // 发送消息处理
{
    if (!SenderSocket)
    {
        ScreenMsg("No sender socket");
        return false;
    }
    //~~~~~~~~~~~~~~~~
    // 发送消息
    int32 BytesSent = 0;
    // FString serialized = ToSend;
    // TCHAR *serializedChar = serialized.GetCharArray().GetData();
    // int32 size = FCString::Strlen(serializedChar);
    int32 size = ToSend.size();
    int32 sent = 0;
    // SenderSocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *RemoteAddr);
    // SenderSocket->SendTo((uint8*)TCHAR_TO_UTF8(serializedChar), size, BytesSent, *RemoteAddr);//发送给远端地址
    SenderSocket->SendTo((uint8*) ToSend.c_str(), size, BytesSent, *RemoteAddr);    // 发送给远端地址

    if (BytesSent <= 0)
    {
        const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
        UE_LOG(LogTemp, Error, TEXT("%s"), *Str);
        ScreenMsg(Str);
        return false;
    }

    ScreenMsg("UDP Send Succcess!");

    return true;
}

void UUDPFunctionLibrary::DataRecv(std::string& str, bool& success)
{
    if (!ListenSocket)
    {
        ScreenMsg("No sender socket");
        success = false;
        // return success;
    }
    TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    TArray<uint8> ReceivedData;    // 定义一个接收器
    uint32 Size;
    if (ListenSocket->HasPendingData(Size))
    {
        success = true;
        str = "";
        uint8* Recv = new uint8[Size];
        int32 BytesRead = 0;

        ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
        ListenSocket->RecvFrom(
            ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *targetAddr);    // 创建远程接收地址
        // char ansiiData[1024];
        // memcpy(ansiiData, ReceivedData.GetData(), BytesRead);//拷贝数据到接收器
        // ansiiData[BytesRead] = 0;                            //判断数据结束
        // FString debugData = ANSI_TO_TCHAR(ansiiData);         //字符串转换

        char ansiiData[76];
        memcpy(ansiiData, ReceivedData.GetData(), BytesRead);    // 拷贝数据到接收器
        // ansiiData[BytesRead] = 0;                            //判断数据结束
        str = std::string(ansiiData, 76);
        // const char* DataPtr = DataStr.c_str();
        // FString debugData = FString(76, DataPtr);      //字符串转换
        // str = debugData;
        //  memset(ansiiData,0,1024);//清空
    }
    else
    {
        success = false;
    }
}

void UUDPFunctionLibrary::DataRecvFromSend(std::string& Str, bool& Success)
{
    if (!SenderSocket)
    {
        ScreenMsg("No sender/receive socket");
        Success = false;
        // return success;
    }
    // TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    TArray<uint8> ReceivedData;    // 定义一个接收器
    uint32 Size;
    if (SenderSocket->HasPendingData(Size))
    {
        Success = true;
        Str = "";
        uint8* Recv = new uint8[Size];
        int32 BytesRead = 0;

        ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
        // SenderSocket->RecvFrom(ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *targetAddr);//创建远程接收地址
        SenderSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead, ESocketReceiveFlags::None);
        // char ansiiData[1024];
        // memcpy(ansiiData, ReceivedData.GetData(), BytesRead);//拷贝数据到接收器
        // ansiiData[BytesRead] = 0;                            //判断数据结束
        // FString debugData = ANSI_TO_TCHAR(ansiiData);         //字符串转换

        char ansiiData[76];
        memcpy(ansiiData, ReceivedData.GetData(), BytesRead);    // 拷贝数据到接收器
        // ansiiData[BytesRead] = 0;                            //判断数据结束
        Str = std::string(ansiiData, 76);
        // const char* DataPtr = DataStr.c_str();
        // FString debugData = FString(76, DataPtr);      //字符串转换
        // str = debugData;
        //  memset(ansiiData,0,1024);//清空
    }
    else
    {
        Success = false;
    }
}

bool UUDPFunctionLibrary::StartUDPSender(
    const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort)    /// 初始化远端IP 发送信息前
{
    // FIPv4Endpoint Endpoint(FIPv4Address::Any, 6789);
    // Create Remote Address.
    RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    bool bIsValid;
    RemoteAddr->SetIp(*TheIP, bIsValid);
    RemoteAddr->SetPort(ThePort);

    if (!bIsValid)
    {
        ScreenMsg("Rama UDP Sender>> IP address was not valid!", TheIP);
        return false;
    }

    SenderSocket = FUdpSocketBuilder(*YourChosenSocketName)
                       .AsReusable()
                       .WithBroadcast()    // 广播
                       .WithSendBufferSize(2 * 1024 * 1024)
        //.BoundToEndpoint(Endpoint)
        ;

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    int32 SendSize = 2 * 1024 * 1024;
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    if (bIsValid)
    {
        bIsValid = true;
    }
    return bIsValid;
}

void UUDPFunctionLibrary::CloseUDPReceiver()
{
    delete UDPReceiver;
    UDPReceiver = nullptr;

    // Clear all sockets!
    //       makes sure repeat plays in Editor dont hold on to old sockets!
    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
    }
}

void UUDPFunctionLibrary::StartUDPReceiver(
    const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort, bool& success)
{
    TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    FIPv4Address Addr;
    FIPv4Address::Parse(TheIP, Addr);

    // Create Socket
    FIPv4Endpoint Endpoint(FIPv4Address::Any, ThePort);    // 所有ip地址本地
                                                           // FIPv4Endpoint Endpoint(Addr, ThePort); //指定ip地址
    ListenSocket = FUdpSocketBuilder(*YourChosenSocketName)
                       .AsNonBlocking()
                       .AsReusable()
                       .BoundToEndpoint(Endpoint)
                       .WithReceiveBufferSize(2 * 1024 * 1024)

        ;
    // BUFFER SIZE
    int32 BufferSize = 2 * 1024 * 1024;
    ListenSocket->SetSendBufferSize(BufferSize, BufferSize);
    ListenSocket->SetReceiveBufferSize(BufferSize, BufferSize);

    if (!ListenSocket)
    {
        ScreenMsg("No socket");
        success = false;
    }
    if (ListenSocket)
    {
        ScreenMsg("The receiver is initialized");
        success = true;
    }

    // return true;
}
