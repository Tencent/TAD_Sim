// Fill out your copyright notice in the Description page of Project Settings.

#include "VehicleManager.h"
#include "Engine/World.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"
#include "Objects/SimActorFactory.h"
#include "Framework/DisplayGameInstance.h"
#include "HadmapManager.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Misc/Paths.h"
#include "Utils/DataFunctionLibrary.h"
#include "Data/CatalogDataSource.h"
#include "LoaderBPFunctionLibrary.h"

// #include "Internationalization/StringTableRegistry.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogVehicleManager, Log, All);

AVehicleManager::AVehicleManager()
{
    // Load vehicle template
    vehicleClass =
        LoadClass<AVehiclePawn>(NULL, TEXT("Blueprint'/Game/Blueprints/Vehicle/BP_VehicleBase.BP_VehicleBase_C'"));
    if (!vehicleClass.Get())
    {
        vehicleClass = AVehiclePawn::StaticClass();
    }

    //// Load vehicle type table
    // vehicleTypeTable = LoadObject<UDataTable>(NULL, TEXT("DataTable'/Game/Data/VehicleTypeTable.VehicleTypeTable'"));
    //// Load text table override
    // if (/*bIsLoadTextTable*/1)
    //{
    //     FString csvFilePath = FPaths::GameContentDir() + TEXT("Data/VehicleTypeTable.csv");
    //     UDataFunctionLibrary::FillDataTableFromCSVFile(vehicleTypeTable, csvFilePath);
    // }

    // LOCTABLE_FROMFILE_GAME("CSVStringTable", "CSVStringTable", "/Data/TypeResourceTable.csv");
    // FText Resource = FText::FromStringTable("/Game/Data/TypeResourceTable",
    // "transport/SUV.havalH7001",EStringTableLoadingPolicy::FindOrLoad); Resource.ToString();

    // TSubclassOf<AVehiclePawn> Class = GetBPResource<AVehiclePawn>(TEXT("transport/SUV.havalH7001"),
    // TEXT("/Game/Data/TypeResourceTable"));
}

void AVehicleManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(SimLogVehicleManager, Warning, TEXT("Spawn VehicleManager!"));
}

void AVehicleManager::Init(const FManagerConfig& Config)
{
    const FVehicleManagerConfig* TConfig = Cast_Data<const FVehicleManagerConfig>(Config);
    check(TConfig);
    UDisplayGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        return;
    }

    for (auto& Elem : TConfig->egoConfigArry)
    {
        TSubclassOf<AVehiclePawn> Class =
            GetBPResource<AVehiclePawn>(Elem.typeName, ECatalogType::CT_EgoVehicle, vehicleClassMap);
        FVehicleConfig VehicleConfig = Elem;

        if (!Class.Get())
        {
            if (GI->GetRuntimeMeshLoader())
            {
                FString ModelPath =
                    GI->GetCatalogDataSource()->GetModelPathByTypeName(ECatalogType::CT_EgoVehicle, Elem.Name);
                VehicleConfig.CustomMesh =
                    GI->GetRuntimeMeshLoader()->LoadStaticMeshFromFBX(ModelPath, true, EMeshType::MT_VEHICLE);

                 UE_LOG(SimLogVehicleManager, Warning, TEXT("VehicleConfig.CustomMesh : %s"), VehicleConfig.CustomMesh);
            }
            Class = vehicleClass;
        }
        AVehiclePawn* NewVehicle = ASimActorFactory::SpawnSimActor<AVehiclePawn>(GetWorld(), Class, VehicleConfig);
        if (NewVehicle)
        {
            if (UDisplayGameInstance* GI = GetGameInstance())
            {
                if (NewVehicle)
                {
                    if (NewVehicle->IsUseContainer())
                    {
                        NewVehicle->ApplyTruckCatalogOffset(
                            GI->GetCatalogDataSource()->GetTruckOffset(VehicleConfig.Name));
                    }
                    else
                    {
                        NewVehicle->ApplyCatalogOffset(GI->GetCatalogDataSource()->GetOffset(VehicleConfig.Name));
                    }
                }
            }

            egoArry.Add(NewVehicle);
        }
        else
        {
            UE_LOG(SimLogVehicleManager, Error, TEXT("Spawn ego vehicle failed, ID: %d"), Elem.id);
        }
    }

    for (auto& Elem : TConfig->trafficConfigArry)
    {
        TSubclassOf<AVehiclePawn> Class =
            GetBPResource<AVehiclePawn>(Elem.typeName, ECatalogType::CT_TrafficVehicle, vehicleClassMap);
        if (!Class.Get())
        {
            Class = vehicleClass;
        }
        AVehiclePawn* NewVehicle = ASimActorFactory::SpawnSimActor<AVehiclePawn>(GetWorld(), Class, Elem);
        if (NewVehicle)
        {
            if (GI->GetCatalogDataSource())
            {
                NewVehicle->ApplyCatalogOffset(GI->GetCatalogDataSource()->GetOffset(Elem.Name));
            }
            trafficArry.Add(NewVehicle);
        }
        else
        {
            UE_LOG(SimLogVehicleManager, Error, TEXT("Spawn traffic vehicle failed, ID: %d"), Elem.id);
        }
    }
}

void AVehicleManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    const FVehicleManagerIn* TInput = Cast_Data<const FVehicleManagerIn>(Input);
    check(TInput);

    FVehicleManagerOut* TOutput = Cast_Data<FVehicleManagerOut>(Output);
    check(TOutput);
    TOutput->trafficOutArry.Empty();
    TOutput->trafficOutArry.AddDefaulted(TInput->trafficVehicleInputArry.Num());
    TOutput->egoOutArry.Empty();
    TOutput->egoOutArry.AddDefaulted(TInput->egoVehicleInputArry.Num());

    /* Update All Traffic Vehicle */
    for (size_t i = 0; i < TInput->trafficVehicleInputArry.Num(); i++)
    {
        bool IsAssigned = false;
        if (i < trafficArry.Num() &&
            TInput->trafficVehicleInputArry[i].id == trafficArry[i]->GetConfig()->id)    // Check
        {
            trafficArry[i]->Update(TInput->trafficVehicleInputArry[i], TOutput->trafficOutArry[i]);
            IsAssigned = true;
        }
        if (!IsAssigned)    // Search
        {
            for (auto& Elem : trafficArry)
            {
                if (Elem->GetConfig()->id == TInput->trafficVehicleInputArry[i].id)
                {
                    Elem->Update(TInput->trafficVehicleInputArry[i], TOutput->trafficOutArry[i]);
                    IsAssigned = true;
                }
            }
        }
        if (!IsAssigned)    // Add
        {
            FVehicleConfig VehicleConfig;
            VehicleConfig.id = TInput->trafficVehicleInputArry[i].id;
            VehicleConfig.startLocation = TInput->trafficVehicleInputArry[i].location;
            VehicleConfig.startRotation = TInput->trafficVehicleInputArry[i].rotation;
            VehicleConfig.typeName = TInput->trafficVehicleInputArry[i].typeName;
            VehicleConfig.timeStamp = TInput->trafficVehicleInputArry[i].timeStamp;
            VehicleConfig.wheelLocationArry = TInput->trafficVehicleInputArry[i].wheelLocationArry;
            VehicleConfig.wheelRotationArry = TInput->trafficVehicleInputArry[i].wheelRotationArry;
            TSubclassOf<AVehiclePawn> Class = GetBPResource<AVehiclePawn>(
                TInput->trafficVehicleInputArry[i].typeName, ECatalogType::CT_TrafficVehicle, vehicleClassMap);
            if (!Class.Get())
            {
                Class = vehicleClass;
                if (UDisplayGameInstance* GI = GetGameInstance())
                {
                    if (GI->GetRuntimeMeshLoader())
                    {
                        FString ModelPath = GI->GetCatalogDataSource()->GetModelPathByType(
                            ECatalogType::CT_TrafficVehicle, TInput->trafficVehicleInputArry[i].type);
                        VehicleConfig.CustomMesh =
                            GI->GetRuntimeMeshLoader()->LoadStaticMeshFromFBX(ModelPath, true, EMeshType::MT_VEHICLE);
                    }
                }
            }
            AVehiclePawn* NewVehicle = ASimActorFactory::SpawnSimActor<AVehiclePawn>(GetWorld(), Class, VehicleConfig);
            if (NewVehicle)
            {
                trafficArry.Add(NewVehicle);
                if (UDisplayGameInstance* GI = GetGameInstance())
                {
                    if (GI->GetCatalogDataSource())
                    {
                        NewVehicle->ApplyCatalogOffset(GI->GetCatalogDataSource()->GetOffset(
                            ECatalogType::CT_TrafficVehicle, TInput->trafficVehicleInputArry[i].type));
                    }
                }
                NewVehicle->Update(TInput->trafficVehicleInputArry[i], TOutput->trafficOutArry[i]);
            }
            else
            {
                UE_LOG(SimLogVehicleManager, Error, TEXT("Spawn vehicle failed, ID: %d"), VehicleConfig.id);
            }
            IsAssigned = true;
        }
    }

    // Find SimActor Need To Destroy
    TArray<int32> ToRemoveTrafficArry;
    for (int32 i = trafficArry.Num() - 1; i >= 0; i--)
    {
        if (trafficArry[i])
        {
            if (trafficArry[i]->GetTimeStamp() != TInput->timeStamp)
            {
                trafficArry[i]->Destroy();
                ToRemoveTrafficArry.Add(i);
            }
        }
        else
        {
            ToRemoveTrafficArry.Add(i);
        }
    }

    // Remove SimActor Destroyed In Array
    for (auto& Elem : ToRemoveTrafficArry)
    {
        trafficArry.RemoveAt(Elem);
    }

    /* Update All Ego Vehicle */
    for (size_t i = 0; i < TInput->egoVehicleInputArry.Num(); i++)
    {
        bool IsAssigned = false;
        if (i < egoArry.Num() && TInput->egoVehicleInputArry[i].id == egoArry[i]->GetConfig()->id)    // Check
        {
            egoArry[i]->Update(TInput->egoVehicleInputArry[i], TOutput->egoOutArry[i]);
            IsAssigned = true;
        }
        if (!IsAssigned)    // Search
        {
            for (auto& Elem : egoArry)
            {
                if (Elem->GetConfig()->id == TInput->egoVehicleInputArry[i].id)
                {
                    Elem->Update(TInput->egoVehicleInputArry[i], TOutput->egoOutArry[i]);
                    IsAssigned = true;
                }
            }
        }
        if (!IsAssigned)    // Add
        {
            FVehicleConfig VehicleConfig;
            VehicleConfig.id = TInput->egoVehicleInputArry[i].id;
            VehicleConfig.startLocation = TInput->egoVehicleInputArry[i].location;
            VehicleConfig.startRotation = TInput->egoVehicleInputArry[i].rotation;
            VehicleConfig.typeName = TInput->egoVehicleInputArry[i].typeName;
            VehicleConfig.timeStamp = TInput->egoVehicleInputArry[i].timeStamp;
            VehicleConfig.wheelLocationArry = TInput->egoVehicleInputArry[i].wheelLocationArry;
            VehicleConfig.wheelRotationArry = TInput->egoVehicleInputArry[i].wheelRotationArry;
            TSubclassOf<AVehiclePawn> Class = GetBPResource<AVehiclePawn>(
                TInput->egoVehicleInputArry[i].typeName, ECatalogType::CT_EgoVehicle, vehicleClassMap);
            if (!Class.Get())
            {
                Class = vehicleClass;
            }
            AVehiclePawn* NewVehicle = ASimActorFactory::SpawnSimActor<AVehiclePawn>(GetWorld(), Class, VehicleConfig);
            if (NewVehicle)
            {
                egoArry.Add(NewVehicle);
                if (UDisplayGameInstance* GI = GetGameInstance())
                {
                    if (GI->GetCatalogDataSource())
                    {
                        NewVehicle->ApplyCatalogOffset(GI->GetCatalogDataSource()->GetOffset(
                            ECatalogType::CT_TrafficVehicle, TInput->egoVehicleInputArry[i].type));
                    }
                }
                NewVehicle->Update(TInput->egoVehicleInputArry[i], TOutput->egoOutArry[i]);
            }
            else
            {
                UE_LOG(SimLogVehicleManager, Error, TEXT("Spawn vehicle failed, ID: %d"), VehicleConfig.id);
            }
            IsAssigned = true;
        }
    }

    // Find SimActor Need To Destroy
    TArray<int32> ToRemoveEgoArry;
    for (int32 i = egoArry.Num() - 1; i >= 0; i--)
    {
        if (egoArry[i])
        {
            if (egoArry[i]->GetTimeStamp() > 0 && egoArry[i]->GetTimeStamp() != TInput->timeStamp)
            {
                egoArry[i]->Destroy();
                ToRemoveEgoArry.Add(i);
            }
        }
        else
        {
            ToRemoveEgoArry.Add(i);
        }
    }

    // Remove SimActor Destroyed In Array
    for (auto& Elem : ToRemoveEgoArry)
    {
        egoArry.RemoveAt(Elem);
    }
}

ISimActorInterface* AVehicleManager::GetCurrentEgo()
{
    if (UDisplayGameInstance* GI = GetGameInstance())
    {
        int64 EgoID = GI->GetEgoIDByGroupName();
        return GetVehicle(ETrafficType::ST_Ego, EgoID);
    }
    return nullptr;
}

ISimActorInterface* AVehicleManager::GetVehicle(ETrafficType _Type, int64 _Id)
{
    TArray<ISimActorInterface*>* vehicles = 0;
    switch (_Type)
    {
        case ETrafficType::ST_TRAFFIC:
            vehicles = &trafficArry;
            break;
        case ETrafficType::ST_Ego:
            vehicles = &egoArry;
            break;
        default:
            break;
    }

    if (vehicles)
    {
        for (auto& Elem : *vehicles)
        {
            if (Elem->GetConfig()->id == _Id)
            {
                return Elem;
            }
        }
    }

    return nullptr;
}

FVehicleManagerConfig AVehicleManager::TranslateData(FSimResetIn& _Data)
{
    FVehicleManagerConfig Config;

    return Config;
}

const UDataTable* AVehicleManager::GetVehicleTypeTable()
{
    return vehicleTypeTable;
}

UDisplayGameInstance* AVehicleManager::GetGameInstance()
{
    UGameInstance* GI = GetWorld()->GetGameInstance();
    UDisplayGameInstance* DGI = NULL;
    if (GI)
    {
        DGI = Cast<UDisplayGameInstance>(GI);
    }
    return DGI;
}
