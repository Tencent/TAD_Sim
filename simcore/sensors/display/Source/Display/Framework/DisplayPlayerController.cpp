// Fill out your copyright notice in the Description page of Project Settings.

#include "DisplayPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "DisplayGameModeBase.h"
#include "DisplayPlayerState.h"
#include "DisplayGameStateBase.h"
#include "UnrealNetwork.h"
#include "Managers/TransportManager.h"
#include "DisplayGameInstance.h"
#include "UI/OutlineWidget.h"
#include "DisplayPawn.h"
#include "GodPawn.h"
#include "UI/DrivingWidget.h"
#include "Objects/Equipment/Gate.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Components/OcculutionTraceComponent.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogPlayerController, Log, All);

ADisplayPlayerController::ADisplayPlayerController()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bShowMouseCursor = bShowMouseConfig;
    OcculutionTraceComp = CreateDefaultSubobject<UOcculutionTraceComponent>(FName(TEXT("OcculutionTrace")));
}

void ADisplayPlayerController::OnSimResetInput()
{
    // GetWorld()->GetTimerManager().ClearTimer(onSimTriggerTimer);

    //// Create pawn
    // ghostPawn = GetWorld()->SpawnActor<ADisplayPawn>();
    // godPawn = GetWorld()->SpawnActor<AGodPawn>();

    // FLocalResetOut NewResetOut;

    // if (SpawnAndInitAllManagers())
    //{
    //     NewResetOut.message = TEXT("SUCCESS");
    // }
    // else
    //{
    //     NewResetOut.message = TEXT("FAIL");
    // }

    //// Reset call back
    // if (Role == ROLE_Authority)
    //{
    //     // Listen server
    //     GetGameInstance<UDisplayGameInstance>()->SimResetOut(NewResetOut,
    //     GetPlayerState<ADisplayPlayerState>()->UniqueId);
    // }
    // else
    //{
    //     // Client
    //     Server_SimResetOutput(NewResetOut);
    // }
}

void ADisplayPlayerController::OnSimUpdateInput()
{
    ////UE_LOG(SimLogPlayerController, Log, TEXT("OnSimUpdateInput!"));
    // GetWorld()->GetTimerManager().ClearTimer(onSimTriggerTimer);

    // UpdateAllManagers();

    //// Update call back
    // if (Role == ROLE_Authority)
    //{
    //     //UE_LOG(SimLogPlayerController, Log, TEXT("SimUpdateOutput!"));
    //     GetGameInstance<UDisplayGameInstance>()->SimUpdateOut(updateOut,
    //     GetPlayerState<ADisplayPlayerState>()->UniqueId);
    // }
    // else
    //{
    //     UE_LOG(SimLogPlayerController, Log, TEXT("Begin Server_SimUpdateOutput!"));
    //     Server_SimUpdateOutput(updateOut);
    //     UE_LOG(SimLogPlayerController, Log, TEXT("End Server_SimUpdateOutput!"));
    // }

    ////bSimTriggerFlag = false;
    ////GetWorld()->GetTimerManager().UnPauseTimer(onSimTriggerTimer_Test);
}

FString ADisplayPlayerController::ConsoleCommand(const FString& Command, bool bWriteToLog)
{
#if WITH_EDITOR
    return Super::ConsoleCommand(Command, bWriteToLog);
#else
    if (Command == TEXT("EnableConsoleCommand"))
    {
        bEnableConsole = true;
    }
    if (bEnableConsole)
    {
        return Super::ConsoleCommand(Command, bWriteToLog);
    }
    return FString();
#endif
}

void ADisplayPlayerController::OnSimUpdateInput_Deferred()
{
    GetWorld()->GetTimerManager().ClearTimer(onSimTriggerTimer);

    updateOut = FLocalUpdateOut();
    check(SensorManager);
    SensorManager->Update(updateIn.sensorManager, updateOut.sensorManager);
    updateOut.message = TEXT("SUCCESS");

    // Update call back
    if (GetLocalRole() == ROLE_Authority)
    {
        // UE_LOG(SimLogPlayerController, Log, TEXT("SimUpdateOutput!"));
        GetGameInstance<UDisplayGameInstance>()->SimUpdateOut_Deferred(
            updateOut, GetPlayerState<ADisplayPlayerState>()->UniqueId);
    }
    else
    {
        UE_LOG(SimLogPlayerController, Log, TEXT("Begin Server_SimUpdateOutput!"));
        // Server_SimUpdateOutput(updateOut);
        UE_LOG(SimLogPlayerController, Log, TEXT("End Server_SimUpdateOutput!"));
    }
}

void ADisplayPlayerController::Server_SimResetOutput_Implementation(FLocalResetOut _ReturnData)
{
    if (GetGameInstance<UDisplayGameInstance>()->ModuleGroupName.IsEmpty())    // Display配置在全局算法，则创建选择主车UI
    {
        if (!SwitchEgoWidget)
        {
            UClass* MyWidgetClass =
                LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/WBP_SwitchEgo.WBP_SwitchEgo_C'"));
            if (MyWidgetClass)
            {
                SwitchEgoWidget = CreateWidget<UUserWidget>(this, MyWidgetClass);
                if (SwitchEgoWidget)
                {
                    SwitchEgoWidget->AddToViewport(1);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("DisplayPlayerController: Cant load /Game/UI/WBP_SwitchEgo UWidgetBlueprint asset!"));
            }
        }
    }
    PossessVehicleExec(GetGameInstance<UDisplayGameInstance>()->GetEgoIDByGroupName());

    GetWorld()->GetAuthGameMode<ADisplayGameModeBase>()->SimOutput(
        _ReturnData, GetPlayerState<ADisplayPlayerState>()->UniqueId);
    // GetGameInstance<UDisplayGameInstance>()->SimResetOut(_ReturnData,
    // GetPlayerState<ADisplayPlayerState>()->UniqueId);
}

bool ADisplayPlayerController::Server_SimResetOutput_Validate(FLocalResetOut _ReturnData)
{
    return true;
}

void ADisplayPlayerController::Server_SimUpdateOutput_Implementation(FLocalUpdateOut _OutData)
{
    // UE_LOG(SimLogPlayerController, Log, TEXT("Server_SimUpdateOutput!"));
    GetWorld()->GetAuthGameMode<ADisplayGameModeBase>()->SimOutput(
        _OutData, GetPlayerState<ADisplayPlayerState>()->UniqueId);
    // GetGameInstance<UDisplayGameInstance>()->SimUpdateOut(_OutData, GetPlayerState<ADisplayPlayerState>()->UniqueId);
}

bool ADisplayPlayerController::Server_SimUpdateOutput_Validate(FLocalUpdateOut _OutData)
{
    return true;
}

UDrivingWidget* ADisplayPlayerController::GetDrivingWidget()
{
    if (!driving_widget)
    {
        /* Driving UI */
        UClass* DrivingWidgetClass =
            LoadClass<UDrivingWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/WBP_RearView.WBP_RearView_C'"));
        if (DrivingWidgetClass)
        {
            driving_widget = CreateWidget<UDrivingWidget>(this, DrivingWidgetClass);
            if (driving_widget)
            {
                driving_widget->AddToViewport(1);
                driving_widget->SetVisibility(ESlateVisibility::Hidden);
                // UE_LOG(LogTemp, Error, TEXT("DisplayPlayerController: Create
                // WidgetBlueprint'/Game/UI/WBP_RearView.WBP_RearView'!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("DisplayPlayerController: Cant load WidgetBlueprint'/Game/UI/WBP_RearView.WBP_RearView'!"));
        }
    }
    return driving_widget;
}

void ADisplayPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (outlineWidget)
    {
        outlineWidget->SetVisibility(ESlateVisibility::Collapsed);
        outlineWidget->RemoveFromViewport();
        outlineWidget = nullptr;
    }
    if (driving_widget)
    {
        driving_widget->SetVisibility(ESlateVisibility::Collapsed);
        driving_widget->RemoveFromViewport();
        driving_widget = nullptr;
    }
    if (SwitchEgoWidget)
    {
        SwitchEgoWidget->SetVisibility(ESlateVisibility::Collapsed);
        SwitchEgoWidget->RemoveFromViewport();
        SwitchEgoWidget = nullptr;
    }
    // CollectGarbage(EObjectFlags::RF_NoFlags);
}

void ADisplayPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // if (bSimTriggerFlag)
    //{
    //     bSimTriggerFlag = false;
    //     OnSimUpdateInput();
    // }
}

void ADisplayPlayerController::PossessActor(AActor* _ActorToPossess)
{
    if (!_ActorToPossess)
    {
        UE_LOG(SimLogPlayerController, Warning, TEXT("The Actor To Possess Is Null!"));
        return;
    }
    APawn* Pawn = Cast<APawn>(_ActorToPossess);
    if (Pawn)
    {
        OnPossess(Pawn);
    }
    else
    {
        UE_LOG(SimLogPlayerController, Warning, TEXT("The Actor To Possess Is Not A Pawn!"));
    }
}

void ADisplayPlayerController::SetPossessEgoName(const FString& VehicleName)
{
    PossessVehicleExec(GetGameInstance<UDisplayGameInstance>()->GetEgoIDByGroupName(VehicleName));
}

void ADisplayPlayerController::PossessVehicleExec(int64 _Id)
{
    TWeakObjectPtr<ATransportManager> TM =
        GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->transportManager;
    if (TM.IsValid() && TM->vehicleManager)
    {
        ISimActorInterface* SimActor = TM->vehicleManager->GetVehicle(ETrafficType::ST_Ego, _Id);
        if (SimActor)
        {
            id_controlled = _Id;
            OnPossess(Cast<APawn>(SimActor));
            if (AVehiclePawn* EgoVehicle = Cast<AVehiclePawn>(SimActor))
            {
                EgoVehicle->SwitchCamera(EgoVehicle->GetDefaultCameraName());
            }
            UE_LOG(SimLogPlayerController, Log, TEXT("possess pawn(name:%s, id:%ld)"),
                *(Cast<APawn>(SimActor)->GetName()), _Id);
            return;
        }
        else
        {
            SimActor = TM->vehicleManager->GetVehicle(ETrafficType::ST_TRAFFIC, _Id);
            if (SimActor)
            {
                id_controlled = _Id;
                OnPossess(Cast<APawn>(SimActor));
                UE_LOG(SimLogPlayerController, Log, TEXT("possess pawn(name:%s, id:%ld)"),
                    *(Cast<APawn>(SimActor)->GetName()), _Id);
                return;
            }
        }

        UE_LOG(SimLogPlayerController, Log, TEXT("Cant possess target vehicle!(id: %ld)"), _Id);

        // for (auto &Elem : TM->vehicleManager->egoArry)
        //{
        //     if (Elem->GetConfig()->id == _Id)
        //     {
        //         OnPossess(Cast<APawn>(Elem));
        //     }
        // }

        // for (auto &Elem : TM->vehicleManager->trafficArry)
        //{
        //     if (Elem->GetConfig()->id == _Id)
        //     {
        //         OnPossess(Cast<APawn>(Elem));
        //     }
        // }
    }
}

void ADisplayPlayerController::DebugGate(bool bEnable)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGate::StaticClass(), FoundActors);
    for (auto Elem : FoundActors)
    {
        if (AGate* Gate = Cast<AGate>(Elem))
        {
            Gate->ShowDebug(bEnable);
        }
    }
}

void ADisplayPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!outlineWidget || !outlineWidget->IsValidLowLevel())
    {
        /* UI */
        UClass* MyWidgetClass =
            LoadClass<UOutlineWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/WidgetBP_Outline.WidgetBP_Outline_C'"));
        if (MyWidgetClass)
        {
            outlineWidget = CreateWidget<UOutlineWidget>(this, MyWidgetClass);
            if (outlineWidget)
            {
                outlineWidget->AddToViewport(0);
                outlineWidget->Init(this);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("DisplayPlayerController: Cant load /Game/UI/WidgetBP_Outline_C UWidgetBlueprint asset!"));
        }
    }

    /* Driving UI */
    if (!driving_widget || !driving_widget->IsValidLowLevel())
    {
        UClass* DrivingWidgetClass =
            LoadClass<UDrivingWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/WBP_RearView.WBP_RearView_C'"));
        if (DrivingWidgetClass)
        {
            driving_widget = CreateWidget<UDrivingWidget>(this, DrivingWidgetClass);
            if (driving_widget)
            {
                driving_widget->AddToViewport(1);
                driving_widget->SetVisibility(ESlateVisibility::Hidden);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("DisplayPlayerController: Cant load WidgetBlueprint'/Game/UI/WBP_RearView.WBP_RearView'!"));
        }
    }

    // Create pawn
    ghostPawn = GetWorld()->SpawnActor<ADisplayPawn>();
    godPawn = GetWorld()->SpawnActor<AGodPawn>();

    FString LevelName = GetWorld()->GetCurrentLevel()->GetPathName();

    // OnPossess(ghostPawn);

    // SwitchPawnToEgo();

    // PossessVehicleExec(0);
}

void ADisplayPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    check(InputComponent);
    // InputComponent->BindAction(TEXT(""))

    InputComponent->BindAction(
        TEXT("ToggleUIVisibility"), EInputEvent::IE_Released, this, &ADisplayPlayerController::ToggleUIVisibility);
    InputComponent->BindAction(
        TEXT("Ghost"), EInputEvent::IE_Released, this, &ADisplayPlayerController::SwitchPawnToEgoOrGhost);
    InputComponent->BindAction(TEXT("God"), EInputEvent::IE_Released, this, &ADisplayPlayerController::SwitchPawnToGod);
    InputComponent->BindAction(TEXT("Ego"), EInputEvent::IE_Released, this, &ADisplayPlayerController::SwitchPawnToEgo);
    InputComponent->BindAction(
        TEXT("SwitchDrivingMode"), IE_Released, this, &ADisplayPlayerController::ToggleDriveMode);

    // InputComponent->BindAxis("MoveForward", this, &ADisplayPlayerController::MoveForward);
    // InputComponent->BindAxis("MoveRight", this, &ADisplayPlayerController::MoveRight);
    // InputComponent->BindAction("Handbrake", IE_Pressed, this, &ADisplayPlayerController::OnHandbrakePressed);
    // InputComponent->BindAction("Handbrake", IE_Released, this, &ADisplayPlayerController::OnHandbrakeReleased);
    // InputComponent->BindAction("SwitchCamera", IE_Pressed, this, &ADisplayPlayerController::OnToggleCamera);
}

// bool ADisplayPlayerController::SpawnAndInitAllManagers()
//{
//
//     bool AllManagerReady = true;
//
//     /* Map */
//
//     /* Transports */
//     transportManager = GetWorld()->SpawnActor<ATransportManager>();
//     if (transportManager)
//     {
//         transportManager->Init(resetIn.transportManager);
//     }
//     else
//     {
//         AllManagerReady = false;
//         UE_LOG(SimLogPlayerController, Error, TEXT("Can not spawn TransportManager!"));
//     }
//
//     /* Creatures */
//     creatureManager = GetWorld()->SpawnActor<ACreatureManager>();
//     if (creatureManager)
//     {
//         creatureManager->Init(resetIn.creatureManager);
//     }
//     else
//     {
//         AllManagerReady = false;
//         UE_LOG(SimLogPlayerController, Error, TEXT("Can not spawn CreatureManager!"));
//     }
//
//     /* Obstacles */
//     obstacleManager = GetWorld()->SpawnActor<AObstacleManager>();
//     if (obstacleManager)
//     {
//         obstacleManager->Init(resetIn.obstacleManager);
//     }
//     else
//     {
//         AllManagerReady = false;
//         UE_LOG(SimLogPlayerController, Error, TEXT("Can not spawn ObstacleManager!"));
//     }
//
//     /* SignalLights */
//     signalligthManager = GetWorld()->SpawnActor<ASignalLightManager>();
//     if (signalligthManager)
//     {
//         signalligthManager->Init(resetIn.signallightManager);
//     }
//     else
//     {
//         AllManagerReady = false;
//         UE_LOG(SimLogPlayerController, Error, TEXT("Can not spawn signalligthManager!"));
//     }
//
//     /* Sensors */
//     SensorManager = GetWorld()->SpawnActor<ASensorManager>();
//     if (SensorManager)
//     {
//         SensorManager->Init(resetIn.sensorManager);
//     }
//     else
//     {
//         AllManagerReady = false;
//         UE_LOG(SimLogPlayerController, Error, TEXT("Can not spawn SensorManager!"));
//     }
//
//     /* Environment */
//     envManager = GetWorld()->SpawnActor<AEnvManager>();
//     if (envManager)
//     {
//         envManager->Init(resetIn.envManager);
//     }
//     else
//     {
//         AllManagerReady = false;
//         UE_LOG(SimLogPlayerController, Error, TEXT("Can not spawn EnvManager!"));
//     }
//
//     /* Terrain */
//     terrainManager = GetWorld()->SpawnActor<ATerrainManager>();
//     if (terrainManager)
//     {
//         terrainManager->Init(FManagerConfig());
//     }
//     else
//     {
//         AllManagerReady = false;
//         UE_LOG(SimLogPlayerController, Error, TEXT("Can not spawn TerrainManager!"));
//     }
//
//     /* UI */
//     UClass* MyWidgetClass = LoadClass<UOutlineWidget>(NULL,
//     TEXT("WidgetBlueprint'/Game/UI/WidgetBP_Outline.WidgetBP_Outline_C'")); if (MyWidgetClass)
//     {
//         outlineWidget = CreateWidget<UOutlineWidget>(this, MyWidgetClass);
//         if (outlineWidget)
//         {
//             outlineWidget->AddToViewport(1);
//             outlineWidget->Init(this);
//         }
//     }
//     else
//     {
//         UE_LOG(LogTemp, Warning, TEXT("DisplayPlayerController: Cant load /Game/Blueprints/UMG_ControlPad
//         UWidgetBlueprint asset!"));
//     }
//
//     OnAllManagersInit();
//     return AllManagerReady;
// }
//
// bool ADisplayPlayerController::UpdateAllManagers()
//{
//     updateOut = FLocalUpdateOut();
//
//     check(transportManager);
//     transportManager->Update(updateIn.transportManager, updateOut.transportManager);
//     creatureManager->Update(updateIn.creatureManager, updateOut.creatureManager);
//     obstacleManager->Update(updateIn.obstacleManager, updateOut.obstacleManager);
//     signalligthManager->Update(updateIn.signallightManager, updateOut.signallightManager);
//     envManager->Update(updateIn.envManager, updateOut.envManager);
//     //SensorManager->Update(updateIn.sensorManager, updateOut.sensorManager);
//
//     updateOut.message = TEXT("SUCCESS");
//
//     OnAllManagersUpdate();
//     return true;
// }
//
// void ADisplayPlayerController::OnAllManagersInit()
//{
//     // Possess to ego pawn
//     if (transportManager && transportManager->vehicleManager)
//     {
//         if (transportManager->vehicleManager->egoArry.Num() > 0)
//         {
//             ATransportPawn* Ego = Cast<ATransportPawn>(transportManager->vehicleManager->egoArry[0]);
//             if (Ego)
//             {
//                 this->Possess(Ego);
//                 Ego->SwitchCamera(defaultCameraName);
//             }
//         }
//     }
//
// }
//
// void ADisplayPlayerController::OnAllManagersUpdate()
//{
//
// }

void ADisplayPlayerController::SwitchPawnToEgo()
{
    TWeakObjectPtr<ATransportManager> TM =
        GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->transportManager;
    if (TM.IsValid() && TM->vehicleManager)
    {
        ISimActorInterface* SimActor = TM->vehicleManager->GetVehicle(ETrafficType::ST_Ego, 0);
        if (SimActor)
        {
            AVehiclePawn* EgoPawn = Cast<AVehiclePawn>(SimActor);
            if (EgoPawn)
            {
                this->Possess(EgoPawn);
                EgoPawn->SwitchCamera(EgoPawn->GetDefaultCameraName());
                // if (this->GetPawn() != EgoPawn)
                //{
                //     this->Possess(EgoPawn);
                //     EgoPawn->SwitchCamera(EgoPawn->GetDefaultCameraName());
                // }
                // else
                //{
                //     UE_LOG(SimLogPlayerController, Log, TEXT("Switch Pawn Cancel, Current Pawn Is Ego!"));
                // }
                return;
            }
        }
    }
    UE_LOG(SimLogPlayerController, Warning, TEXT("Switch Pawn To Ego Failed!"));
}

void ADisplayPlayerController::SwitchPawnToGhost()
{
    if (bEnableGhost && ghostPawn)
    {
        if (this->GetPawn() != ghostPawn)
        {
            // Snap to current camera view
            float Yaw = GetPawn()->GetActorRotation().Yaw;
            float Pitch = GetPawn()->GetActorRotation().Pitch;
            ghostPawn->SetActorLocation(this->PlayerCameraManager->GetCameraLocation());
            ghostPawn->SetActorRotation(FRotator(Pitch, Yaw, 0.f));
            ghostPawn->SetActorEnableCollision(false);
            this->Possess(ghostPawn);
        }
        else
        {
            UE_LOG(SimLogPlayerController, Log, TEXT("Switch Pawn Cancel, Current Pawn Is Ghost!"));
        }
        return;
    }
    UE_LOG(SimLogPlayerController, Warning, TEXT("Switch Pawn To Ghost Failed!"));
}

void ADisplayPlayerController::SwitchPawnToGod()
{
    if (bEnableGod && godPawn)
    {
        if (this->GetPawn() != godPawn)
        {
            // Move godpawn to above egovehicle
            TWeakObjectPtr<ATransportManager> TM =
                GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->transportManager;
            if (TM.IsValid() && TM->vehicleManager)
            {
                ISimActorInterface* SimActor = TM->vehicleManager->GetVehicle(ETrafficType::ST_Ego, 0);
                if (SimActor)
                {
                    APawn* EgoPawn = Cast<APawn>(SimActor);
                    if (EgoPawn)
                    {
                        godPawn->SetActorLocationAndRotation(
                            EgoPawn->GetActorLocation() + FVector(0, 0, 100000), FRotator(-90, 0, 0));
                    }
                }
            }
            this->Possess(godPawn);
        }
        else
        {
            UE_LOG(SimLogPlayerController, Log, TEXT("Switch Pawn Cancel, Current Pawn Is God!"));
        }
        return;
    }
    UE_LOG(SimLogPlayerController, Warning, TEXT("Switch Pawn To God Failed!"));
}

void ADisplayPlayerController::SwitchPawnToEgoOrGhost()
{
    if (GetGameInstance<UDisplayGameInstance>()->bAllowSync)
    {
        UClass* WanderInfoClass =
            LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/WBP_WanderInfo.WBP_WanderInfo_C'"));
        if (WanderInfoClass)
        {
            if (WanderInfoWidget)
            {
                WanderInfoWidget->RemoveFromParent();
                WanderInfoWidget = nullptr;
            }

            WanderInfoWidget = CreateWidget<UUserWidget>(this, WanderInfoClass);
            if (WanderInfoWidget)
            {
                WanderInfoWidget->AddToViewport(1);
            }
        }
        return;
    }

    if (this->GetPawn() != ghostPawn)
    {
        SwitchPawnToGhost();
    }
    else
    {
        SwitchPawnToEgo();
    }
}

void ADisplayPlayerController::ToggleDriveMode()
{
    if (!bAllowSwitchDriveMode)
    {
        UE_LOG(SimLogPlayerController, Log, TEXT("Switch drive mode feature has been disabled"));
        return;
    }

    if (modeDrive == 1)
    {
        modeDrive = 0;
        UE_LOG(SimLogPlayerController, Log, TEXT("drive mode is auto"));
    }
    else if (modeDrive == 0)
    {
        modeDrive = 1;
        UE_LOG(SimLogPlayerController, Log, TEXT("drive mode is manned"));
    }
}

void ADisplayPlayerController::ToggleUIVisibility()
{
    if (outlineWidget)
    {
        if (outlineWidget->IsVisible())
        {
            outlineWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            outlineWidget->SetVisibility(ESlateVisibility::Visible);
        }
    }
}
