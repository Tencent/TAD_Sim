// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "Engine/World.h"
#include "Framework/DisplayPlayerController.h"
#include "Components/MapElemGenComponent.h"
#include "HadMap/Public/HadmapManager.h"

ATerrainManager::ATerrainManager()
{
    PrimaryActorTick.bCanEverTick = true;
    // hadmapVisualComponent = CreateDefaultSubobject<UMapDrawComponent>(TEXT("HadMapVisualizer"));
    mapLineGenComponent = CreateDefaultSubobject<UMapElemGenComponent>(TEXT("MapLineGen"));
}

void ATerrainManager::Init(const FManagerConfig& Config)
{
    return;

    const FTerrainManagerConfig* TerrainConfig = Cast_Data<const FTerrainManagerConfig>(Config);

    // Get hadmaphandle
    hadmapHandle = hadmapue4::HadmapManager::Get();
    // Get transport manager

    // First load map data
    hadmap::txPoint Origin = hadmap::txPoint(0, 0, 0);
    if (hadmapHandle && hadmapHandle->IsMapReady() && hadmapHandle->IsMapDataLoaded() &&
        hadmapHandle->GetMapMode() == hadmapue4::MapMode::ROUTINGMAP)
    {
        hadmapHandle->LocalToLonLat(Origin.x, Origin.y, Origin.z);
        hadmapHandle->UpdateRoutingmap(Origin.x, Origin.y, Origin.z);
    }

    // Generate Object Base On HadMap
    GConfig->GetBool(TEXT("MapSettings"), TEXT("bGenerateHadMapObject"), bGenerateHadMapObject, GGameIni);
    if (bGenerateHadMapObject)
    {
        TArray<FString> TypeNameArry;
        GConfig->GetArray(TEXT("MapSettings"), TEXT("HadMapObjectType"), TypeNameArry, GGameIni);
        for (auto& Elem : TypeNameArry)
        {
            objectTypeArry_HadMap.push_back(hadmapue4::HadmapManager::StringToObjectType(Elem));
        }
        float LineWidth = 10;
        float LineThickness = 2;
        GConfig->GetFloat(TEXT("MapSettings"), TEXT("LineWidth"), LineWidth, GGameIni);
        GConfig->GetFloat(TEXT("MapSettings"), TEXT("LineThickness"), LineThickness, GGameIni);

        if (hadmapHandle && hadmapHandle->IsMapReady() && hadmapHandle->IsMapDataLoaded())
        {
            hadmap::PointVec Envelope;
            hadmap::txPoint Min = hadmap::txPoint(Origin.x - 500000000, Origin.y - 500000000, Origin.z);
            hadmap::txPoint Max = hadmap::txPoint(Origin.x + 500000000, Origin.y + 500000000, Origin.z);
            Envelope.push_back(Min);
            Envelope.push_back(Max); /*
             std::vector<hadmap::OBJECT_TYPE> ObjTypeArry;
             for (auto &Elem : objectTypeArry_HadMap)
             {
                 ObjTypeArry.push_back(Elem);
             }*/
            hadmap::txObjects ObjectData = hadmapHandle->GetObjects(Envelope, objectTypeArry_HadMap);

            // TArray<FVector> Points;
            for (size_t i = 0; i < ObjectData.size(); i++)
            {
                TArray<FVector> NewPoints;
                hadmapHandle->GetPointsFromObj(*ObjectData[i], NewPoints);
                if (NewPoints.Num() > 2)
                {
                    FVector FrontPoint = NewPoints[0];
                    NewPoints.Add(FrontPoint);
                }
                TArray<FMapLineDataBase> LinesData;
                for (size_t j = 0; j + 1 < NewPoints.Num(); j++)
                {
                    FMapLineDataBase NewData;
                    NewData.begin = NewPoints[j];
                    NewData.end = NewPoints[j + 1];
                    LinesData.Add(NewData);
                }
                FMapLineParam Params;
                Params.shape = EMapLineShape::LS_Solid;
                Params.width = LineWidth;
                Params.thickness = LineThickness;
                Params.color = FColor::White;
                mapLineGenComponent->GenerateLinesOnMap(LinesData, Params, ECollisionChannel::ECC_GameTraceChannel2);
            }
        }
    }

    SpawnItems();
}

void ATerrainManager::Update(const FManagerIn& Input, FManagerOut& Output)
{
    // if (hadmapHandle && hadmapHandle->IsMapReady() && hadmapHandle->GetMapMode() == hadmapue4::MapMode::ROUTINGMAP &&
    // bNeedToLoadHadmap)
    //{
    //     hadmapHandle->UpdateRoutingmap(simUpdateIn->egoData.position().x(), simUpdateIn->egoData.position().y(),
    //     simUpdateIn->egoData.position().z());
    // }
}

void ATerrainManager::BeginPlay()
{
    Super::BeginPlay();
    // GetWorld()->GetFirstPlayerController<ADisplayPlayerController>();

    // bool IsVisualHadmapData = false;
    // GConfig->GetBool(TEXT("MapSettings"), TEXT("bDrawObject"), IsVisualHadmapData, GGameIni);
    // if (hadmapVisualComponent && IsVisualHadmapData)
    //{
    //     ISimActorInterface* SimActor =
    //     GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->
    //     transportManager->vehicleManager->GetVehicle(ETrafficType::ST_Ego,
    //     0); FVector DrawCenter = Cast<AActor>(SimActor)->GetActorLocation();
    //     hadmapVisualComponent->SetDrawRange(FVector2D(10000000, 10000000));
    //     TArray<hadmap::OBJECT_TYPE> TypeArry;
    //     TypeArry.Add(hadmap::OBJECT_TYPE::OBJECT_TYPE_ParkingSpace);
    //     hadmapVisualComponent->DrawObjects(DrawCenter, TypeArry);
    // }
}

void ATerrainManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATerrainManager::SpawnItems()
{
    // Get map data
    hadmapHandle = hadmapue4::HadmapManager::Get();

    std::vector<hadmap::OBJECT_TYPE> types;
    hadmap::txObjects objects;
    // TODO: add type
    // types.push_back(hadmap::OBJECT_TYPE_Pole);
    // types.push_back(hadmap::OBJECT_TYPE_TrafficSign);
    // types.push_back(hadmap::OBJECT_TYPE_TrafficLights);

    if (hadmapHandle->GetMapHandle())
    {
        hadmap::getObjects(hadmapHandle->GetMapHandle(), std::vector<hadmap::txLaneId>(), types, objects);
    }

    for (auto& Elem : objects)
    {
        int32 Id = Elem->getId();
        FString Name = Elem->getName().c_str();
        double Lon, Lat, Alt;
        double Roll, Pitch, Yaw;
        double Length, Width, Height;
        std::string Type, SubType;
        Elem->getLWH(Length, Width, Height);
        Lon = Elem->getGeom()->getGeometry()->getStart().x;
        Lat = Elem->getGeom()->getGeometry()->getStart().y;
        Alt = Elem->getGeom()->getGeometry()->getStart().z;
        Elem->getRPY(Roll, Pitch, Yaw);
        Elem->getRawTypeString(Type, SubType);

        // cast transform coordinator to local
        FVector Loc;
        hadmapHandle->LonLatToLocal(Lon, Lat, Alt, Loc);
        FRotator Rot(ForceInit);
        Rot.Roll = (float) (Roll * 180 / PI);
        Rot.Pitch = (float) (-Pitch * 180 / PI);
        Rot.Yaw = (float) (-Yaw * 180 / PI - 90);

        // TODO: project to typname
        bool NeedToSpawn = false;
        FString CombinedName;
        if (Type == "pole")
        {
            CombinedName = (FString) ((Type + "_").c_str()) + Name;
            NeedToSpawn = true;
        }
        else if (Type == "trafficSign" || Type == "trafficLight")
        {
            CombinedName = (Type + "_" + SubType).c_str();
            NeedToSpawn = true;
        }

        if (NeedToSpawn)
        {
            TSubclassOf<AActor> Class;    // = GetBPResource<AActor>(CombinedName, itemClassMap);
            if (Class.Get())
            {
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                AActor* NewItem = GetWorld()->SpawnActor<AActor>(Class.Get(), Loc, Rot, Params);
                itemArray.Add(NewItem);
            }
            else
            {
                UE_LOG(
                    LogTemp, Error, TEXT("Can not find resource, item name: %s, type name: %s"), *Name, *CombinedName);
            }
        }
    }
}

void ATerrainManager::GenerateRoad()
{
}
