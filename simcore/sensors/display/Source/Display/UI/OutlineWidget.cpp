// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlineWidget.h"
#include "Framework/DisplayPlayerController.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"

void UOutlineWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UOutlineWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bHasEnvEditPermission && envEManager && envEManager->IsValidLowLevel() && envEManager->IsInited())
    {
        FUIEnvData NewUIEnvData = BPI_GetUIEnvData();
        UIEnvDataAddToSimEnvData(NewUIEnvData, envData_Sim);
        envData_Sim.Compute();
        envEManager->ExternalInputEnvData(envData_Sim);
    }

    /*if (GetWorld())
    {
        ADisplayPlayerController* PC = GetWorld()->GetFirstPlayerController<ADisplayPlayerController>();
        if (PC)
        {
            if (PC && PC->transportManager &&PC->transportManager->vehicleManager)
            {
                egoArry = PC->transportManager->vehicleManager->egoArry;
            }
            else
            {
                egoArry.Empty();
            }

            if (PC && PC->transportManager &&PC->transportManager->vehicleManager)
            {
                trafficArry = PC->transportManager->vehicleManager->trafficArry;
            }
            else
            {
                trafficArry.Empty();
            }

            if (PC && PC->creatureManager &&PC->creatureManager->pedestrianManager)
            {
                pedestrianArry = PC->creatureManager->pedestrianManager->pedestrianArry;
            }
            else
            {
                pedestrianArry.Empty();
            }

            if (PC && PC->obstacleManager &&PC->obstacleManager)
            {
                obstacleArry = PC->obstacleManager->obstacleArry;
            }
            else
            {
                obstacleArry.Empty();
            }
        }

    }*/
}

void UOutlineWidget::BeginDestroy()
{
    Super::BeginDestroy();
    this->RemoveFromViewport();
}

TArray<FVehicleInfo> UOutlineWidget::GetVehicleInfo()
{
    TArray<FVehicleInfo> VehilceInfoArry;
    for (auto& Elem : egoArry)
    {
        FVehicleInfo NewInfo;
        AVehiclePawn* VP = Cast<AVehiclePawn>(Elem);
        NewInfo.typeName = VP->GetConfig()->typeName;
        NewInfo.id = VP->GetConfig()->id;
        NewInfo.location = VP->GetActorLocation();
        NewInfo.rotation = VP->GetActorRotation();
        VehilceInfoArry.Add(NewInfo);
    }
    for (auto& Elem : trafficArry)
    {
        FVehicleInfo NewInfo;
        AVehiclePawn* VP = Cast<AVehiclePawn>(Elem);
        NewInfo.typeName = VP->GetConfig()->typeName;
        NewInfo.id = VP->GetConfig()->id;
        NewInfo.location = VP->GetActorLocation();
        NewInfo.rotation = VP->GetActorRotation();
        VehilceInfoArry.Add(NewInfo);
    }
    return VehilceInfoArry;
}

TArray<FSimActorInfo> UOutlineWidget::GetAllInfo()
{
    TArray<FSimActorInfo> SimActorInfoArry;
    for (auto& Elem : egoArry)
    {
        FSimActorInfo NewInfo;
        AVehiclePawn* VP = Cast<AVehiclePawn>(Elem);
        NewInfo.typeName = VP->GetConfig()->typeName;
        NewInfo.id = VP->GetConfig()->id;
        NewInfo.location = VP->GetActorLocation();
        NewInfo.rotation = VP->GetActorRotation();
        SimActorInfoArry.Add(NewInfo);
    }
    for (auto& Elem : trafficArry)
    {
        FSimActorInfo NewInfo;
        AVehiclePawn* VP = Cast<AVehiclePawn>(Elem);
        NewInfo.typeName = VP->GetConfig()->typeName;
        NewInfo.id = VP->GetConfig()->id;
        NewInfo.location = VP->GetActorLocation();
        NewInfo.rotation = VP->GetActorRotation();
        SimActorInfoArry.Add(NewInfo);
    }
    for (auto& Elem : pedestrianArry)
    {
        FSimActorInfo NewInfo;
        APedestrianCharacter* VP = Cast<APedestrianCharacter>(Elem);
        if (VP)
        {
            NewInfo.typeName = VP->GetConfig()->typeName;
            NewInfo.id = VP->GetConfig()->id;
            NewInfo.location = VP->GetActorLocation();
            NewInfo.rotation = VP->GetActorRotation();
            SimActorInfoArry.Add(NewInfo);
        }
    }
    for (auto& Elem : obstacleArry)
    {
        FSimActorInfo NewInfo;
        AObstacleActor* VP = Cast<AObstacleActor>(Elem);
        NewInfo.typeName = VP->GetConfig()->typeName;
        NewInfo.id = VP->GetConfig()->id;
        NewInfo.location = VP->GetActorLocation();
        NewInfo.rotation = VP->GetActorRotation();
        SimActorInfoArry.Add(NewInfo);
    }
    return SimActorInfoArry;
}

bool UOutlineWidget::GetEnvEditPermission()
{
    if (envEManager && envEManager->GetEnvDataEditPermission())
    {
        envEManager->SetEnvDataEditPermission(false);
        bHasEnvEditPermission = true;
        return true;
    }
    bHasEnvEditPermission = false;
    return false;
}

void UOutlineWidget::ReturnEnvEditPermission()
{
    if (envEManager && !envEManager->GetEnvDataEditPermission())
    {
        envEManager->SetEnvDataEditPermission(true);
        bHasEnvEditPermission = false;
    }
    bHasEnvEditPermission = false;
}

void UOutlineWidget::Init(ADisplayPlayerController* _PC)
{
    if (_PC)
    {
        envEManager = GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->envManager;
    }

    GConfig->GetBool(TEXT("UI"), TEXT("bAllowModifyEnvData"), bDefaultAllowModifyEnvData, GGameIni);
    GConfig->GetBool(TEXT("UI"), TEXT("bDefaultOpenUIPanel"), bDefaultOpenUIPanel, GGameIni);

    if (bDefaultAllowModifyEnvData)
    {
        if (GetEnvEditPermission())
        {
            envData_Sim = envEManager->GetCurrentEnvData();
            envData_UI = SimEnvDataToUIEnvData(envData_Sim);
            BPI_SetUIEnvData(envData_UI);
        }
    }

    BPI_OnInit();

    if (bDefaultOpenUIPanel)
    {
        this->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        this->SetVisibility(ESlateVisibility::Collapsed);
    }
}

FUIEnvData UOutlineWidget::SimEnvDataToUIEnvData(const FSimEnvData& _SimEnvData)
{
    FUIEnvData NewUIEnvData;
    NewUIEnvData.date = _SimEnvData.date;
    NewUIEnvData.cloudDensity = _SimEnvData.cloudDensity;
    NewUIEnvData.cloudThickness = _SimEnvData.cloudThickness;
    NewUIEnvData.humidity = _SimEnvData.humidity;
    NewUIEnvData.moonStrength = _SimEnvData.moonStrength;
    NewUIEnvData.rainFall = _SimEnvData.rainFall;
    NewUIEnvData.snowFall = _SimEnvData.snowFall;
    NewUIEnvData.sunStrength = _SimEnvData.sunStrength;
    NewUIEnvData.temperature = _SimEnvData.temperature;
    NewUIEnvData.visibility = _SimEnvData.visibility;
    NewUIEnvData.windRotation = _SimEnvData.windRotation;
    NewUIEnvData.windSpeed = _SimEnvData.windSpeed;

    return NewUIEnvData;
}

void UOutlineWidget::UIEnvDataAddToSimEnvData(const FUIEnvData& _UIEnvData, FSimEnvData& _SimEnvData)
{
    _SimEnvData.date = _UIEnvData.date;
    _SimEnvData.cloudDensity = _UIEnvData.cloudDensity;
    _SimEnvData.cloudThickness = _UIEnvData.cloudThickness;
    _SimEnvData.humidity = _UIEnvData.humidity;
    _SimEnvData.moonStrength = _UIEnvData.moonStrength;
    _SimEnvData.rainFall = _UIEnvData.rainFall;
    _SimEnvData.snowFall = _UIEnvData.snowFall;
    _SimEnvData.sunStrength = _UIEnvData.sunStrength;
    _SimEnvData.temperature = _UIEnvData.temperature;
    _SimEnvData.visibility = _UIEnvData.visibility;
    _SimEnvData.windRotation = _UIEnvData.windRotation;
    _SimEnvData.windSpeed = _UIEnvData.windSpeed;
}

// void UOutlineWidget::UpdateData_Implementation()
//{
// }
