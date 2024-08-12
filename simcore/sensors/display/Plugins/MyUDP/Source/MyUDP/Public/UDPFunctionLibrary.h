// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Engine.h"
#include <string>
#include "UDPFunctionLibrary.generated.h"

/**
 *
 */
UCLASS()
class MYUDP_API UUDPFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(/*BlueprintCallable, Category = "UDP"*/)
    // 新建初始化Receiver函数
    static void StartUDPReceiver(
        const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort, bool& success);
    UFUNCTION(/*BlueprintCallable, Category = "UDP"*/)
    static bool StartUDPSender(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort);
    UFUNCTION(/*BlueprintCallable, Category = "UDP"*/)
    static void CloseUDPReceiver();
    UFUNCTION(/*BlueprintCallable, Category = "UDP"*/)
    static void CloseUDPSender();

    // UFUNCTION(/*BlueprintCallable, Category = "UDP"*/)
    static bool RamaUDPSender_SendString(std::string ToSend);

    // UFUNCTION(/*BlueprintPure, Category = "UDP"*/)
    // DataRecv返回函数
    static void DataRecv(std::string& str, bool& success);

    static void DataRecvFromSend(std::string& Str, bool& Success);
};
