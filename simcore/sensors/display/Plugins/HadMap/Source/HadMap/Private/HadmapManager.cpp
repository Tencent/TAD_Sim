// Fill out your copyright notice in the Description page of Project Settings.

#include "HadmapManager.h"
// hadmap header
#include "common/coord_trans.h"
#include "structs/base_struct.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
// hadmap header

THIRD_PARTY_INCLUDES_START
#include "kdtree_flann.h"
THIRD_PARTY_INCLUDES_END

#include "Misc/Paths.h"

using namespace hadmapue4;

// HadmapManager* HadmapManager::instance = NULL;

HadmapManager::HadmapManager()
{
    bInitSuccess = false;
}

HadmapManager::~HadmapManager()
{
    CloseHadmapHandle();
    delete m_gKdtree;
}

bool hadmapue4::HadmapManager::CreateHadmapHandle()
{
    CloseHadmapHandle();

    bool bCreateSuccess = false;

    // Init hadmap by mode
    switch (mapMode)
    {
        case hadmapue4::ROUTINGMAP:
        {
            // routingmap
            if (routingmapHandle)
            {
                delete routingmapHandle;
            }
            routingmapHandle = NULL;
            routingmapInterface = NULL;
            routingmapHandle = new hadmap::RoutingMap(
                hadmap::CoordType::COORD_WGS84, std::string(TCHAR_TO_UTF8(*mapPath_SQL)).c_str());
            // hadmap::txRoute Route;
            // hadmap::PointVec StartEnd;
            // StartEnd.push_back(hadmap::txPoint(startLon, startLat, mapOriginAlt));
            // StartEnd.push_back(hadmap::txPoint(endLon, endLat, mapOriginAlt));
            if (routingmapHandle /*&& routingmapHandle->routingSync(StartEnd, Route)*/)
            {
                routingmapInterface = routingmapHandle->getMapInterface();
            }
            // Detect init state
            if (routingmapHandle && routingmapInterface)
            {
                bCreateSuccess = true;
            }
            break;
        }
        case hadmapue4::MAPENGINE:
        {
            // mapengine
            if (mapengineHandle)
            {
                hadmap::hadmapClose(&mapengineHandle);
                mapengineHandle = NULL;
                mapEngineStat = TX_HADMAP_HANDLE_ERROR;
            }
            hadmap::MAP_DATA_TYPE MapDataType = GetMapDataType(mapPath_SQL);
#if PLATFORM_WINDOWS
            mapEngineStat =
                hadmap::hadmapConnect(std::string(TCHAR_TO_UTF8(*mapPath_SQL)).c_str(), MapDataType, &mapengineHandle, false);
#else
            mapEngineStat =
                hadmap::hadmapConnect(std::string(TCHAR_TO_UTF8(*mapPath_SQL)).c_str(), MapDataType, &mapengineHandle);
#endif
            // mapEngineStat =
            // hadmap::hadmapConnect("C:/Users/jiangyulin/AppData/Roaming/TADSim/scenario/hadmap/1004-1-002-181121.sqlite",
            // hadmap::SQLITE, &mapengineHandle);
            // C:/Users/jiangyulin/AppData/Roaming/TADSim/scenario/hadmap/1004-1-002-181121.sqlite
            // UE_LOG(LogTemp, Warning, TEXT("MapPath is: %s"),
            // *mapPath_SQL);//C:\Users\jiangyulin\AppData\Roaming\TADSim\scenario\scene../hadmap/1004-1-002-181121.sqlite
            UE_LOG(LogTemp, Warning, TEXT("hadmap mapengine state is: %d"), mapEngineStat);
            // Detect init state
            if (mapengineHandle && mapEngineStat == TX_HADMAP_HANDLE_OK)
            {
                bCreateSuccess = true;
            }
            break;
        }
        default:
        {
            UE_LOG(LogTemp, Warning, TEXT("Create HadmapHandle failed, cannot detect mapmode!"));
            break;
        }
    }

    return bCreateSuccess;
}

void hadmapue4::HadmapManager::CloseHadmapHandle()
{
    switch (mapMode)
    {
        case hadmapue4::ROUTINGMAP:
        {
            if (routingmapHandle)
            {
                delete routingmapHandle;
                routingmapHandle = NULL;
            }
            break;
        }
        case hadmapue4::MAPENGINE:
        {
            if (mapengineHandle)
            {
                hadmap::hadmapClose(&mapengineHandle);
                mapengineHandle = NULL;
                mapEngineStat = TX_HADMAP_HANDLE_ERROR;
            }
            break;
        }
        default:
            UE_LOG(LogTemp, Warning, TEXT("Close HadmapHandle failed, cannot detect mapmode!"));
            break;
    }
    // bInitSuccess = false;
}

bool HadmapManager::Init(MapMode _Mode, FString _DBPath, double _OriginLon, double _OriginLat, double _OriginAlt,
    const FString& _GPSFilePath)
{
    // Set sql file path
    mapPath_SQL = _DBPath;

    bool IsSuccess = false;
    IsSuccess = Init(_OriginLon, _OriginLat, _OriginAlt, _GPSFilePath);

    //// Set origin coordinate
    // mapOriginLon = _OriginLon;
    // mapOriginLat = _OriginLat;
    // mapOriginAlt = _OriginAlt;
    //// Clean handle
    // CloseHadmapHandle();

    // Set new mode
    mapMode = _Mode;
    // Create hadmap handle
    bMapDataLoaded = CreateHadmapHandle();
    IsSuccess = IsSuccess && bMapDataLoaded;

    bInitSuccess = IsSuccess;
    return bInitSuccess;
}

bool HadmapManager::Init(double _OriginLon, double _OriginLat, double _OriginAlt, const FString& _GPSFilePath)
{
    bool IsSuccess = false;
    // Set origin coordinate
    mapOriginLon = _OriginLon;
    mapOriginLat = _OriginLat;
    mapOriginAlt = _OriginAlt;
    // Clean handle
    CloseHadmapHandle();
    if (_GPSFilePath.IsEmpty())
    {
        IsSuccess = true;
        bUseDecrypt = false;
    }
    else
    {
        bUseDecrypt = SetDecryptMode(_GPSFilePath);
        IsSuccess = bUseDecrypt;
    }

    bInitSuccess = IsSuccess;
    return bInitSuccess;
}

MapMode hadmapue4::HadmapManager::GetMapMode() const
{
    return mapMode;
}

void hadmapue4::HadmapManager::UpdateRoutingmap(double _EgoLon, double _EgoLat, double _EgoAlt)
{
    if (routingmapHandle)
    {
        hadmap::txPoint EgoPos = hadmap::txPoint(_EgoLon, _EgoLat, _EgoAlt);
        routingmapHandle->updateLocation(EgoPos);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Update routingmap failed, routingmapHandle is null!"));
    }
}

hadmap::txMapInterfacePtr HadmapManager::GetMapInterface() const
{
    return routingmapInterface;
}

bool HadmapManager::GetLane(double _Lon, double _Lat, double _Alt, hadmap::txLanePtr& _Lane, double _RadiusInMeter)
{
    if (routingmapInterface)
    {
        hadmap::txLanes Lanes = routingmapInterface->getLanes(hadmap::txPoint(_Lon, _Lat, _Alt), _RadiusInMeter);
        if (!Lanes.empty())
        {
            // TODO: filter data
            _Lane = Lanes[0];
            return true;
        }
    }
    _Lane = NULL;
    return false;
}

bool HadmapManager::GetLanes(double _Lon, double _Lat, double _Alt, hadmap::txLanes& _Lanes, double _RadiusInMeter)
{
    if (routingmapInterface)
    {
        _Lanes = routingmapInterface->getLanes(hadmap::txPoint(_Lon, _Lat, _Alt), _RadiusInMeter);
        return true;
    }
    _Lanes.clear();
    return false;
}

bool HadmapManager::GetRightLane(const hadmap::txLanePtr _Lane, hadmap::txLanePtr& _RightLane)
{
    if (routingmapInterface)
    {
        _RightLane = routingmapInterface->getRightLane(_Lane);
        return true;
    }
    _RightLane = NULL;
    return false;
}

bool HadmapManager::GetLeftLane(const hadmap::txLanePtr _Lane, hadmap::txLanePtr& _LeftLane)
{
    if (routingmapInterface)
    {
        _LeftLane = routingmapInterface->getLeftLane(_Lane);
        return true;
    }
    _LeftLane = NULL;
    return false;
}

bool HadmapManager::GetNextLanes(const hadmap::txLanePtr _Lane, hadmap::txLanes& _NextLanes)
{
    if (routingmapInterface && _Lane)
    {
        _NextLanes = routingmapInterface->getNextLanes(_Lane);
        return true;
    }
    return false;
}

bool HadmapManager::GetPreLanes(const hadmap::txLanePtr _Lane, hadmap::txLanes& _PreLanes)
{
    if (routingmapInterface && _Lane)
    {
        _PreLanes = routingmapInterface->getPreLanes(_Lane);
        return true;
    }
    return false;
}

bool HadmapManager::GetLeftBoundary(const hadmap::txLanePtr _Lane, hadmap::txLaneBoundaryPtr& _Boundary)
{
    if (_Lane)
    {
        _Boundary = _Lane->getLeftBoundary();
        return true;
    }
    _Boundary = NULL;
    return false;
}

bool HadmapManager::GetRightBoundary(const hadmap::txLanePtr _Lane, hadmap::txLaneBoundaryPtr& _Boundary)
{
    if (_Lane)
    {
        _Boundary = _Lane->getRightBoundary();
        return true;
    }
    _Boundary = NULL;
    return false;
}

bool HadmapManager::IsJunction(const hadmap::txLanePtr _Lane)
{
    if (_Lane)
    {
        return _Lane->getSection()->getRoad()->isJunction();
    }
    return false;
}

bool hadmapue4::HadmapManager::SampleCurveToLonLat(
    const hadmap::txCurve* _Curve, hadmap::PointVec& PointArry, double _DeltaDistance, double _StartPos, double _EndPos)
{
    if (!_Curve)
    {
        return false;
    }
    for (size_t i = 0; (double) i * _DeltaDistance + _StartPos <= _EndPos; i++)
    {
        PointArry.push_back(_Curve->getPoint((double) i * _DeltaDistance + _StartPos));
    }
    return true;
}

bool hadmapue4::HadmapManager::SampleCurveToLoc(
    const hadmap::txCurve* _Curve, TArray<FVector>& PointArry, double _DeltaDistance, double _StartPos, double _EndPos)
{
    if (!_Curve)
    {
        return false;
    }
    hadmap::txPoint LonLatPoint;
    for (size_t i = 0; (double) i * _DeltaDistance + _StartPos <= _EndPos; i++)
    {
        PointArry.Emplace();
        LonLatToLocal(_Curve->getPoint((double) i * _DeltaDistance + _StartPos).x,
            _Curve->getPoint((double) i * _DeltaDistance + _StartPos).y,
            _Curve->getPoint((double) i * _DeltaDistance + _StartPos).z, PointArry.Last());
    }
    return true;
}

bool hadmapue4::HadmapManager::GetFrontLanesByLength(
    const hadmap::txLanePtr _StartLane, double _Lon, double _Lat, double _Alt, TArray<SubLane>& _Lanes, double _Length)
{
    // if (_StartLane)
    //{
    //     //TODO: Implement it.
    //     double Length = 0;

    //    hadmap::txLanes NextLanes;
    //    GetNextLanes(_StartLane, NextLanes);
    //
    //    hadmap::txPoint VehicleLonLat = hadmap::txPoint(_Lon, _Lat, _Alt);
    //    FVector VehicleLoc;
    //    LonLatToLocal(VehicleLonLat.x, VehicleLonLat.y, VehicleLonLat.z, VehicleLoc);

    //    hadmap::txPoint PointLonLat = _StartLane->getGeometry()->getStart();
    //    FVector PointLoc;
    //    LonLatToLocal(PointLonLat.x, PointLonLat.y, PointLonLat.z, PointLoc);

    //
    //}
    return false;
}

bool hadmapue4::HadmapManager::GetBackLanesByLength(
    const hadmap::txLanePtr _StartLane, double _Lon, double _Lat, double _Alt, TArray<SubLane>& _Lanes, double _Length)
{
    // TODO: Implement it.
    return false;
}

hadmap::txMapHandle* HadmapManager::GetMapHandle() const
{
    return mapengineHandle;
}

void hadmapue4::HadmapManager::LocalToLonLat(double& _X, double& _Y, double& _Z)
{
    coord_trans_api::local2global(_X, _Y, _Z, mapOriginLon, mapOriginLat, mapOriginAlt);
    coord_trans_api::global2lonlat(_X, _Y, _Z);
}

void hadmapue4::HadmapManager::LocalToLonLat(const FVector& _Loc, double& _X, double& _Y, double& _Z)
{
    double X, Y, Z = 0.0;
    X = _Loc.X / 100.f;
    Y = -(_Loc.Y / 100.f);
    Z = _Loc.Z / 100.f;
    LocalToLonLat(X, Y, Z);
    _X = X;
    _Y = Y;
    _Z = Z;
}

void hadmapue4::HadmapManager::LonLatToLocal(double& _Lon, double& _Lat, double& _Alt)
{
    if (bUseDecrypt)
    {
        Decrypt(_Lon, _Lat);
    }
    // coord_trans_api::lonlat2enu(_Lon, _Lat, _Alt, mapOriginLon, mapOriginLat, mapOriginAlt);
    //_Lon = _Lon * 100.f;
    //_Lat = -(_Lat * 100.f);
    //_Alt = _Alt * 100.f;
    // return;

    coord_trans_api::lonlat2local(_Lon, _Lat, _Alt, mapOriginLon, mapOriginLat, mapOriginAlt);
    _Lon = _Lon * 100.f;
    _Lat = -(_Lat * 100.f);
    _Alt = _Alt * 100.f;
}

void hadmapue4::HadmapManager::LonLatToLocal(double _X, double _Y, double _Z, FVector& _Loc)
{
    double X = _X;
    double Y = _Y;
    double Z = _Z;
    LonLatToLocal(X, Y, Z);
    _Loc = FVector(X, Y, Z);
}

void hadmapue4::HadmapManager::LocalToLonLat(const FVector& _Loc, double& _X, double& _Y, double& _Z,
    double _MapOriginLon, double _MapOriginLat, double _MapOriginAlt)
{
    double X, Y, Z = 0.0;
    X = _Loc.X;
    Y = _Loc.Y;
    Z = _Loc.Z;
    coord_trans_api::local2global(X, Y, Z, _MapOriginLon, _MapOriginLat, _MapOriginAlt);
    coord_trans_api::global2lonlat(X, Y, Z);
    _X = X;
    _Y = Y;
    _Z = Z;
}

void hadmapue4::HadmapManager::LonLatToLocal(
    double _X, double _Y, double _Z, FVector& _Loc, double _MapOriginLon, double _MapOriginLat, double _MapOriginAlt)
{
    double X = _X;
    double Y = _Y;
    double Z = _Z;
    coord_trans_api::lonlat2local(X, Y, Z, _MapOriginLon, _MapOriginLat, _MapOriginAlt);
    X = X * 100.f;
    Y = -(Y * 100.f);
    Z = 4.f;
    _Loc = FVector(X, Y, Z);
}

bool hadmapue4::HadmapManager::SetDecryptMode(const FString& gpsfile)
{
    if (gpsfile.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Do not need to read gpsfile:%s"), *gpsfile);
        return false;
    }

    TArray<uint8> rawBuf;
    if (!FFileHelper::LoadFileToArray(rawBuf, *gpsfile))
    {
        UE_LOG(LogTemp, Error, TEXT("CANNOT read gpsfile:%s"), *gpsfile);
        return false;
    }

    double* pt = (double*) rawBuf.GetData();
    size_t pn = rawBuf.Num() / (sizeof(double) * 4);
    if (pn == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("read gpsfile empty:%s"), *gpsfile);
        return false;
    }
    if (!m_gKdtree)
    {
        m_gKdtree = new kdtree_flann<double, std::pair<double, double>>;
    }
    m_gKdtree->Clear();
    for (size_t i = 0; i < pn; i++)
    {
        double p[3] = {pt[0], pt[1], 0};
        m_gKdtree->Add(p, std::make_pair(pt[2], pt[3]));
        pt = &pt[4];
    }

    if (!m_gKdtree->Build())
    {
        UE_LOG(LogTemp, Error, TEXT("gpsfile build faild:%s"), *gpsfile);
    }

    return true;
}

bool hadmapue4::HadmapManager::Decrypt(double& _Lon, double& _Lat)
{
    if (!m_gKdtree || !m_gKdtree->Valid())
        return false;
    double p[3] = {_Lon, _Lat, 0};
    std::pair<double, double> dt;
    double distance;
    if (m_gKdtree->SearchNeighbour(p, dt, distance))
    {
        _Lon += dt.first;
        _Lat += dt.second;

        if (distance > 1e-6)
        {
            // UE_LOG(LogTemp, Warning, TEXT("gps ts too far:%f,%f"), _Lon, _Lat);
        }

        return true;
    }
    return false;
}

hadmap::txObjects hadmapue4::HadmapManager::GetObjects(
    const hadmap::PointVec& _Envelope, const std::vector<hadmap::OBJECT_TYPE>& _ObjTypes)
{
    if (routingmapInterface)
    {
        return routingmapInterface->getObjects(_Envelope, _ObjTypes);
    }
    return hadmap::txObjects();
}

// hadmap::txObjects hadmapue4::HadmapManager::GetObjects(const FVector & _MinPoint, const FVector & _MaxPoint, const
// TArray<hadmap::OBJECT_TYPE>& _ObjTypes)
//{
//     hadmap::txPoint Min = hadmap::txPoint(_MinPoint.X, _MinPoint.Y, _MinPoint.Z);
//     hadmap::txPoint Max = hadmap::txPoint(_MaxPoint.X, _MaxPoint.Y, _MaxPoint.Z);
//     hadmap::PointVec Envelope = { Min ,Max };
//     std::vector<hadmap::OBJECT_TYPE> ObjTypes;
//     for (auto &Elem : _ObjTypes)
//     {
//         ObjTypes.push_back(Elem);
//     }
//     return GetObjects(Envelope, ObjTypes);
// }

void hadmapue4::HadmapManager::GetPointsFromObj(const hadmap::txObject& _Object, hadmap::PointVec& _Points)
{
    for (size_t i = 0; i < _Object.getGeomSize(); i++)
    {
        hadmap::txObjGeomPtr Geom = _Object.getGeom(i);
        dynamic_cast<const hadmap::txLineCurve*>(Geom->getGeometry())->getPoints(_Points);
    }
}

void hadmapue4::HadmapManager::GetPointsFromObj(const hadmap::txObject& _Object, TArray<FVector>& _Points)
{
    //_Points.Empty();
    hadmap::PointVec PointVec;
    GetPointsFromObj(_Object, PointVec);
    for (auto& Elem : PointVec)
    {
        FVector& NewPoint = _Points.Emplace_GetRef();
        LonLatToLocal(Elem.x, Elem.y, Elem.z, NewPoint);
    }
}

hadmap::OBJECT_TYPE hadmapue4::HadmapManager::StringToObjectType(const FString& _String)
{
    if (_String == TEXT("ParkingSpace"))
    {
        return hadmap::OBJECT_TYPE::OBJECT_TYPE_ParkingSpace;
    }
    return hadmap::OBJECT_TYPE::OBJECT_TYPE_None;
}

hadmap::MAP_DATA_TYPE hadmapue4::HadmapManager::GetMapDataType(const FString& _FilePath) const
{
    hadmap::MAP_DATA_TYPE MapDataType = hadmap::MAP_DATA_TYPE::SQLITE;
    FString Extension = FPaths::GetExtension(_FilePath);
    if (Extension == TEXT("sql"))
    {
        MapDataType = hadmap::MAP_DATA_TYPE::SQLITE;
    }
    if (Extension == TEXT("xodr"))
    {
        MapDataType = hadmap::MAP_DATA_TYPE::OPENDRIVE;
    }

    return MapDataType;
}

void HadmapManager::GetObjects(const std::vector<hadmap::txLaneId>& _Envelope,
    const std::vector<hadmap::OBJECT_TYPE>& _ObjTypes, hadmap::txObjects& objects)
{
    hadmap::getObjects(mapengineHandle, _Envelope, _ObjTypes, objects);
}

int HadmapManager::getJunctions(hadmap::txJunctions& junctions)
{
    return hadmap::getJunctions(mapengineHandle, junctions);
}

int HadmapManager::getLaneLink(const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr)
{
    return hadmap::getLaneLink(mapengineHandle, lanelinkId, lanelinkPtr);
}

int HadmapManager::getRoad(const hadmap::roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr)
{
    return hadmap::getRoad(mapengineHandle, roadId, wholeData, roadPtr);
}

hadmap::txPoint HadmapManager::getTxObjectPos(const hadmap::txObjects::value_type object)
{
    // TODO： 过滤linux报错 - 缺库
    // return hadmap::txPoint();
    return object.get()->getPos();
}

void HadmapManager::getTxObjectUserData(
    const hadmap::txObjects::value_type object, std::map<std::string, std::string>& map)
{
    // TODO： 过滤linux报错 - 缺库
    // return;
    object.get()->getUserData(map);
}

void HadmapManager::getTxJunctionLanLink(
    const hadmap::txJunctions::value_type object, std::vector<hadmap::lanelinkpkid>& map)
{
    // TODO： 过滤linux报错 - 缺库
    // return;
    return object.get()->getLaneLink(map);
}
