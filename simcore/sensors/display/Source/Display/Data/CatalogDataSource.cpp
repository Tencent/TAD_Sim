// Fill out your copyright notice in the Description page of Project Settings.

#include "CatalogDataSource.h"
#include "XmlParser.h"
#include "scene.pb.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogCatalog, Log, All);

TMap<int32, FString> UCatalogDataSource::VehicleMap_Name_Type = {{-1, "suv"}, {-2, "truck"}, {0, "suv"}, {1, "Sedan"},
    {2, "SUV"}, {3, "Bus"}, {4, "Truck"}, {5, "Truck_1"}, {6, "Sedan_001"}, {7, "Sedan_002"}, {8, "Sedan_003"},
    {9, "Sedan_004"}, {10, "SUV_001"}, {11, "SUV_002"}, {12, "SUV_003"}, {13, "SUV_004"}, {14, "Truck_002"},
    {15, "Truck_003"}, {16, "Bus_001"}, {17, "Semi_Trailer_Truck_001"}, {18, "SUV_005"}, {19, "SUV_006"},
    {20, "Bus_004"}, {21, "Ambulance_001"}, {22, "HongqiHS5"}, {23, "Bus_003"}, {24, "Sedan_005"}, {25, "Bus_005"},
    {27, "LandRover_RangeRover_2010"}, {28, "BMW_MINI_CooperS_2010"}, {29, "Opel_Corsa_2009"},
    {30, "Lexus_IS_2009"}, {31, "Suzuki_Celerio_2011"}, {32, "Lancia_DeltaIntegrale_Evo1"},
    {33, "Chevrolet_ExpressVan1500_2001"}, {34, "MercedesBenz_SL_2005"}, {35, "Dodge_Charger_2006"},
    {36, "Porsche_911_TurboS_2020"}, {37, "Nissan_GTR_2009"}, {38, "Opel_Ampera_2009"},
    {39, "Dodge_ViperMambaEdition_RCH680"}, {40, "Mitsubishi_iMiEV_2018"}, {41, "Audi_A8_2015"},
    {42, "Toyota_Tacoma_2017"}, {43, "Lamborghini_Gallardo_2011"}, {44, "MercedesBenz_SLSAMG_2014"},
    {45, "Scania_R620_2012"}, {46, "MAN_LionsCoach_2003"}, {47, "Cadillac_XTSLimousine_2018"},
    {48, "MercedesBenz_Arocs_3240"}, {49, "BMW_X6_2014"},

    {501, "Ambulance"}, {502, "Ambulance_002"}, {530, "FireEngine_01"}, {601, "AdminVehicle_01"},
    {602, "AdminVehicle_02"}, {650, "RelativeObstacle"}, {503, "AIV_FullLoad_001"}, {504, "AIV_Empty_001"},
    {505, "MPV_001"}, {506, "AIV_FullLoad_002"}, {507, "AIV_Empty_002"}, {508, "MIFA_01"}, {509, "Truck_with_cargobox"},
    {510, "Truck_without_cargobox"}, {511, "MPV_002"}, {512, "MiningDumpTruck_001"}, {513, "AIV_V5_FullLoad_001"},
    {514, "AIV_V5_Empty_001"}};

TMap<int32, FString> UCatalogDataSource::CreatureMap_Name_Type = {{0, "human"}, {1, "child"}, {2, "oldman"},
    {3, "woman"}, {4, "girl"}, {5, "Woman002"}, {6, "Woman003"}, {7, "Woman004"}, {8, "Woman005"}, {9, "Woman006"},
    {10, "Woman007"}, {11, "Girl002"}, {12, "Man002"}, {13, "Man003"}, {14, "Man004"}, {15, "Man005"}, {16, "Man006"},
    {17, "Oldman002"}, {18, "Oldman003"},

    {100, "cat"}, {101, "dog"}, {201, "bike_001"}, {202, "elecBike_001"}, {203, "tricycle_001"}, {204, "tricycle_002"},
    {205, "tricycle_003"}, {206, "Mobike_Classic2"}, {301, "moto_001"}, {302, "moto_002"},

    {303, "Honda_CreaScoopy_AF55"}, {304, "HarleyDavidson_VRod_2003"}, {305, "Aprilia_SR50R_Euro4"},
    {306, "Suzuki_GSXR1000_2010"}, {307, "Honda_Monkey125_2017"}, {308, "Giant_EscapeR3_2018"},
    {309, "Giant_SCR2_2017"}, {310, "Maruishi_Bicycle_26Inch"}, {311, "Xidesheng_AD350_2020"},

    {401, "vendingCar_001"}, {515, "Unloaded_Trailer"}, {516, "Half_loaded_Trailer"}, {517, "Full_loaded_Trailer"},
    {601, "Port_Crane_001"}, {602, "Port_Crane_002"}, {603, "Port_Crane_002_0_0"}, {604, "Port_Crane_002_0_2"},
    {605, "Port_Crane_002_0_5"}, {606, "Port_Crane_002_1_0"}, {607, "Port_Crane_002_5_0"}, {608, "Port_Crane_003"},
    {609, "Port_Crane_004"}};

TMap<int32, FString> UCatalogDataSource::Obstacle_Name_Type = {{0, "Sedan"}, {1, "Box"}, {2, "Person"}, {3, "Cone"},
    {4, "Sand"}, {5, "Box_001"}, {101, "Stob_001"}, {102, "Stob_002"}, {103, "Stob_003"}, {104, "Stob_004"},
    {105, "Stob_005"}, {106, "Stob_006"}, {201, "Stob_tran_001"}, {301, "Trash_001"}, {302, "Trash_002"},
    {401, "Wheel_001"}, {402, "Wheel_002"}, {403, "Stone_001"}, {501, "Warning_001"}, {601, "Port_Crane_001"},
    {602, "Port_Crane_002"}, {603, "Port_Container_001"}, {604, "Port_Container_002"}};

FVector UCatalogDataSource::GetOffset(const FString& TypeName)
{
    if (TrafficCatalogData.Contains(ECatalogType::CT_EgoVehicle) &&
        TrafficCatalogData[ECatalogType::CT_EgoVehicle].Contains(TypeName))
    {
        return TrafficCatalogData[ECatalogType::CT_EgoVehicle][TypeName]->CenterOffset;
    }
    return FVector(0.f);
}

FTruckCatalogData UCatalogDataSource::GetTruckOffset(const FString& TypeName)    // for ego truck catalog
{
    if (TruckOffsetData.Contains(TypeName))
    {
        return *TruckOffsetData[TypeName];
    }
    return FTruckCatalogData();
}

FVector UCatalogDataSource::GetOffset(ECatalogType CatalogType, int32 Type)
{
    FString TypeName = "";
    switch (CatalogType)
    {
        case ECatalogType::CT_TrafficVehicle:
            if (VehicleMap_Name_Type.Contains(Type))
                TypeName = VehicleMap_Name_Type[Type];
            else
                UE_LOG(LogTemp, Log, TEXT("vehicle catalog type not find ,type : %d"), Type);
            break;
        case ECatalogType::CT_Creature:
            if (CreatureMap_Name_Type.Contains(Type))
            {
                TypeName = CreatureMap_Name_Type[Type];
            }
            break;
        case ECatalogType::CT_Obstacle:
            if (Obstacle_Name_Type.Contains(Type))
            {
                TypeName = Obstacle_Name_Type[Type];
            }
            break;
    }
    if (TrafficCatalogData.Contains(CatalogType) && TrafficCatalogData[CatalogType].Contains(TypeName))
    {
        return TrafficCatalogData[CatalogType][TypeName]->CenterOffset;
    }
    else
        UE_LOG(LogTemp, Log, TEXT("catalog  not find ,typeName : %s"), *TypeName);
    return FVector(0.f);
}

FString UCatalogDataSource::GetModelPathByType(ECatalogType CatalogType, const int32 Type)
{
    FString TypeName = "";
    switch (CatalogType)
    {
        case ECatalogType::CT_TrafficVehicle:
            if (VehicleMap_Name_Type.Contains(Type))
            {
                TypeName = VehicleMap_Name_Type[Type];
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("GetModelPathByType Vehicle catalog type not find ,type : %d"), Type);
            }

            break;
        case ECatalogType::CT_Creature:
            if (CreatureMap_Name_Type.Contains(Type))
            {
                TypeName = CreatureMap_Name_Type[Type];
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("GetModelPathByType Creature catalog type not find ,type : %d"), Type);
            }

            break;
        case ECatalogType::CT_Obstacle:
            if (Obstacle_Name_Type.Contains(Type))
            {
                TypeName = Obstacle_Name_Type[Type];
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("GetModelPathByType Obstacle catalog type not find ,type : %d"), Type);
            }

            break;
    }

    if (TypeName.IsEmpty())
    {
        return FString();
    }
    return GetModelPathByTypeName(CatalogType, TypeName);
}

FString UCatalogDataSource::GetModelPathByTypeName(ECatalogType CatalogType, const FString& TypeName)
{
    if (TrafficCatalogData.Contains(CatalogType))
    {
        for (auto& Elem : TrafficCatalogData[CatalogType])
        {
            if (Elem.Value->TypeName.Equals(TypeName))
            {
                return Elem.Value->GetModelPath();
            }
        }
    }

    return FString();
}

TArray<FString> UCatalogDataSource::GetTypeNameListWithModelPath(ECatalogType CatalogType, const FString& ModelPath)
{
    TArray<FString> Result;
    if (TrafficCatalogData.Contains(CatalogType))
    {
        // TMap<FString, TSharedPtr<FCommonCatalogData>> FindCalalogs = TrafficCatalogData[CatalogType];
        for (auto& Elem : TrafficCatalogData[CatalogType])
        {
            if (Elem.Value->GetModelPath().Equals(ModelPath))
            {
                Result.Add(Elem.Value->TypeName);
            }
        }
    }
    return Result;
}

bool UCatalogDataSource::LoadSceneBuffer(const std::string& Buffer)
{
    sim_msg::Scene scene;

    if (!scene.ParseFromString(Buffer))
    {
        UE_LOG(SimLogCatalog, Error, TEXT("scene pb ParseFromString faild."));
        return false;
    }

    std::string DebugStr = scene.DebugString();
    FString ModelRootDir = UTF8_TO_TCHAR(scene.setting().model3d_pathdir().c_str());
    ModelRootDir.Append(TEXT("/"));
    {
        TMap<FString, TSharedPtr<FCommonCatalogData>> EgoData;
        ECatalogType Type = ECatalogType::CT_EgoVehicle;
        for (int32 i = 0; i < scene.egos().size(); i++)
        {
            FString TypeName = UTF8_TO_TCHAR(scene.egos(i).physicles(0).common().model_3d().c_str());

            int32 PhysicleSize = scene.egos(i).physicles().size();
            int32 ID = scene.egos(i).physicles(0).common().model_id();
            if (PhysicleSize > 1)
            {
                TSharedPtr<FTruckCatalogData> TruckData = MakeShared<FTruckCatalogData>();

                float X1 = scene.egos(i).physicles(0).common().bounding_box().center().x();
                float Y1 = scene.egos(i).physicles(0).common().bounding_box().center().y();
                float Z1 = scene.egos(i).physicles(0).common().bounding_box().center().z();

                float X2 = scene.egos(i).physicles(1).common().bounding_box().center().x();
                float Y2 = scene.egos(i).physicles(1).common().bounding_box().center().y();
                float Z2 = scene.egos(i).physicles(1).common().bounding_box().center().z();

                float Tractor_offsetX = scene.egos(i).physicles(0).geometory().vehicle_coord().combination_offset().x();
                float Tractor_offsetY = scene.egos(i).physicles(0).geometory().vehicle_coord().combination_offset().y();
                float Tractor_offsetZ = scene.egos(i).physicles(0).geometory().vehicle_coord().combination_offset().z();

                float Trailer_offsetX = scene.egos(i).physicles(1).geometory().vehicle_coord().combination_offset().x();
                float Trailer_offsetY = scene.egos(i).physicles(1).geometory().vehicle_coord().combination_offset().y();
                float Trailer_offsetZ = scene.egos(i).physicles(1).geometory().vehicle_coord().combination_offset().z();

                TruckData->ModelID = ID;
                TruckData->TruckHeadModelPath =
                    ModelRootDir + UTF8_TO_TCHAR(scene.egos(i).physicles(0).common().model_3d().c_str());
                TruckData->TruckTrailerModelPath =
                    ModelRootDir + UTF8_TO_TCHAR(scene.egos(i).physicles(1).common().model_3d().c_str());

                TruckData->TruckHeadOffset = FVector(X1 + Tractor_offsetX, Y1 + Tractor_offsetY, Z1 + Tractor_offsetZ);
                TruckData->TruckTrailerOffset = FVector(X2, Y2, Z2);
                TruckData->CombinationTrailerOffset = FVector(Trailer_offsetX, Trailer_offsetY, Trailer_offsetZ);
                TruckData->TypeName = TypeName;

                EgoData.Add(TypeName, TruckData);
                TruckOffsetData.Add(TypeName, TruckData);
            }
            else
            {
                TSharedPtr<FCommonCatalogData> CommonData = MakeShared<FCommonCatalogData>();

                float X = scene.egos(i).physicles(0).common().bounding_box().center().x();
                float Y = scene.egos(i).physicles(0).common().bounding_box().center().y();
                float Z = scene.egos(i).physicles(0).common().bounding_box().center().z();

                CommonData->ModelID = ID;
                CommonData->CenterOffset = FVector(X, Y, Z);
                CommonData->ModelPath =
                    ModelRootDir + UTF8_TO_TCHAR(scene.egos(i).physicles(0).common().model_3d().c_str());
                CommonData->TypeName = TypeName;

                EgoData.Add(TypeName, CommonData);
            }
            VehicleMap_Name_Type.Add(ID, TypeName);
        }
        TrafficCatalogData.Add(TPair<ECatalogType, TMap<FString, TSharedPtr<FCommonCatalogData>>>(Type, EgoData));
    }

    {
        TMap<FString, TSharedPtr<FCommonCatalogData>> VehicleData;
        ECatalogType Type = ECatalogType::CT_TrafficVehicle;
        for (int32 i = 0; i < scene.vehicles().size(); i++)
        {
            TSharedPtr<FCommonCatalogData> CommonData = MakeShared<FCommonCatalogData>();

            FString TypeName = UTF8_TO_TCHAR(scene.vehicles(i).objectname().c_str());
            int32 ID = scene.vehicles(i).physicle().common().model_id();
            float X = scene.vehicles(i).physicle().common().bounding_box().center().x();
            float Y = scene.vehicles(i).physicle().common().bounding_box().center().y();
            float Z = scene.vehicles(i).physicle().common().bounding_box().center().z();

            CommonData->ModelID = ID;
            CommonData->CenterOffset = FVector(X, Y, Z);
            CommonData->ModelPath =
                ModelRootDir + UTF8_TO_TCHAR(scene.vehicles(i).physicle().common().model_3d().c_str());
            CommonData->TypeName = TypeName;

            VehicleMap_Name_Type.Add(ID, TypeName);
            VehicleData.Add(TypeName, CommonData);
        }
        TrafficCatalogData.Add(TPair<ECatalogType, TMap<FString, TSharedPtr<FCommonCatalogData>>>(Type, VehicleData));
    }

    {
        TMap<FString, TSharedPtr<FCommonCatalogData>> CreatureData;
        ECatalogType Type = ECatalogType::CT_Creature;
        for (int32 i = 0; i < scene.vrus().size(); i++)
        {
            TSharedPtr<FCommonCatalogData> CommonData = MakeShared<FCommonCatalogData>();

            FString TypeName = UTF8_TO_TCHAR(scene.vrus(i).objectname().c_str());
            int32 ID = scene.vrus(i).physicle().common().model_id();

            float X = scene.vrus(i).physicle().common().bounding_box().center().x();
            float Y = scene.vrus(i).physicle().common().bounding_box().center().y();
            float Z = scene.vrus(i).physicle().common().bounding_box().center().z();

            CommonData->ModelID = ID;
            CommonData->CenterOffset = FVector(X, Y, Z);
            CommonData->ModelPath = ModelRootDir + UTF8_TO_TCHAR(scene.vrus(i).physicle().common().model_3d().c_str());
            CommonData->TypeName = TypeName;

            CreatureMap_Name_Type.Add(ID, TypeName);
            CreatureData.Add(TypeName, CommonData);
        }
        TrafficCatalogData.Add(TPair<ECatalogType, TMap<FString, TSharedPtr<FCommonCatalogData>>>(Type, CreatureData));
    }

    {
        TMap<FString, TSharedPtr<FCommonCatalogData>> ObstacleData;
        ECatalogType Type = ECatalogType::CT_Obstacle;
        for (int32 i = 0; i < scene.miscs().size(); i++)
        {
            TSharedPtr<FCommonCatalogData> CommonData = MakeShared<FCommonCatalogData>();

            FString TypeName = UTF8_TO_TCHAR(scene.miscs(i).objectname().c_str());
            int32 ID = scene.miscs(i).physicle().model_id();
            float X = scene.miscs(i).physicle().bounding_box().center().x();
            float Y = scene.miscs(i).physicle().bounding_box().center().y();
            float Z = scene.miscs(i).physicle().bounding_box().center().z();

            CommonData->ModelID = ID;
            CommonData->CenterOffset = FVector(X, Y, Z);
            CommonData->ModelPath = ModelRootDir + UTF8_TO_TCHAR(scene.miscs(i).physicle().model_3d().c_str());
            CommonData->TypeName = TypeName;

            Obstacle_Name_Type.Add(ID, TypeName);
            ObstacleData.Add(TypeName, CommonData);
        }

        TrafficCatalogData.Add(TPair<ECatalogType, TMap<FString, TSharedPtr<FCommonCatalogData>>>(Type, ObstacleData));
    }

    UE_LOG(SimLogCatalog, Log,
        TEXT("sceneBuffer ego vehicle num : %d, traffic vehicle num : %d, creature num : %d,, Obstacle num : %d"),
        scene.egos().size(), scene.vehicles().size(), scene.vrus().size(), scene.miscs().size());

    {
        for (int32 i = 0; i < scene.map_models().size(); i++)
        {
            FString ModelName = UTF8_TO_TCHAR(scene.map_models(i).name().c_str());
            FString ModelPath = UTF8_TO_TCHAR(scene.map_models(i).common().model_3d().c_str());
            float X = scene.map_models(i).common().bounding_box().center().x();
            float Y = scene.map_models(i).common().bounding_box().center().y();
            float Z = scene.map_models(i).common().bounding_box().center().z();
            MapModelData.Add(TPair<FString, TPair<FString, FVector>>(
                ModelName, TPair<FString, FVector>(ModelPath, FVector(X, Y, Z))));
        }
        UE_LOG(SimLogCatalog, Log, TEXT("sceneBuffer MapModel num : %d,"), scene.map_models().size());
    }
    return true;
}

void UCatalogDataSource::ClearData()
{
    TrafficCatalogData.Empty();
    TruckOffsetData.Empty();
}
