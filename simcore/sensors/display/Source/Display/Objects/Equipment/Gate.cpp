// Fill out your copyright notice in the Description page of Project Settings.

#include "Gate.h"

#include "Managers/TransportManager.h"
#include "Framework/DisplayGameStateBase.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"
// Sets default values

AGate::AGate()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AGate::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bEnter)
    {
        if (bOpen)
        {
            CheckEgoLeave();
        }
    }
    else
    {
        if (!bOpen)
        {
            CheckEgoComming();
        }
    }
}

void AGate::CheckEgoComming()
{
    AVehiclePawn* EgoPawn = nullptr;
    TWeakObjectPtr<ATransportManager> TM =
        GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->transportManager;
    if (TM.IsValid() && TM->vehicleManager)
    {
        ISimActorInterface* SimActor = TM->vehicleManager->GetVehicle(ETrafficType::ST_Ego, 0);
        if (SimActor)
        {
            EgoPawn = Cast<AVehiclePawn>(SimActor);
        }
    }

    if (!EgoPawn)
    {
        EgoPawn = DubugActor;
    }

    if (EgoPawn)
    {
        if ((EgoPawn->GetActorLocation() - GetActorLocation()).SizeSquared() > (SafeDistance * SafeDistance))
        {
            SetActorTickInterval(1);    // Reduce detection frequency at long distances
            return;
        }
        else
        {
            SetActorTickInterval(0);
        }

        float VehicleDistance_X = GetActorForwardVector() | (EgoPawn->GetActorLocation() - GetActorLocation());
        VehicleDistance_X -= EgoPawn->GetMeshHead();
        if (VehicleDistance_X > 0.f && VehicleDistance_X < EnterDistance_X)
        {
            float VehicleDistance_Y = GetActorRightVector() | (EgoPawn->GetActorLocation() - GetActorLocation());
            if (bReverseY)
            {
                VehicleDistance_Y *= -1.f;
            }
            if (VehicleDistance_Y > 0.f && VehicleDistance_Y < EnterDistance_Y)
            {
                bEnter = true;
                UE_LOG(LogTemp, Log, TEXT("bEnter"));
                GetWorld()->GetTimerManager().SetTimer(onTriggerOpenTimer, this, &AGate::OnOpen, DelayOpen, false);
            }
        }
    }
}

void AGate::CheckEgoLeave()
{
    AVehiclePawn* EgoPawn = nullptr;
    TWeakObjectPtr<ATransportManager> TM =
        GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->transportManager;
    if (TM.IsValid() && TM->vehicleManager)
    {
        ISimActorInterface* SimActor = TM->vehicleManager->GetVehicle(ETrafficType::ST_Ego, 0);
        if (SimActor)
        {
            EgoPawn = Cast<AVehiclePawn>(SimActor);
        }
    }

    if (!EgoPawn)
    {
        EgoPawn = DubugActor;
    }

    if (EgoPawn)
    {
        float CheckDistance = LeaveDistance;
        float VehicleDistance = (GetActorForwardVector() * -1.f) | (EgoPawn->GetActorLocation() - GetActorLocation());
        VehicleDistance -= EgoPawn->GetMeshEnd();
        if (VehicleDistance > 0.f && VehicleDistance > CheckDistance)
        {
            FVector VehicleDir = (EgoPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            float angle = acos(VehicleDir | GetActorForwardVector()) * 180 / PI;
            if (angle > LeaveAngle)
            {
                bEnter = false;
                UE_LOG(LogTemp, Log, TEXT("OnClose"));
                OnClose();
            }
        }
    }
}

void AGate::OnOpen()
{
    Receive_OnOpen();
}

void AGate::OnClose()
{
    Receive_OnClose();
}
