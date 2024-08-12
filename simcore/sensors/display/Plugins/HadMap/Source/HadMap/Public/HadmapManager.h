// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "hadmap.h"
#include "mapengine/hadmap_engine.h"
#include "mapengine/hadmap_codes.h"
#include "routingmap/routing_map.h"

namespace flann
{
template <class T>
struct L2_Simple;
}

template <typename Scalar, typename idx_type, typename Metric>
class kdtree_flann;
namespace hadmapue4
{
enum MapMode
{
    ROUTINGMAP,
    MAPENGINE
};
enum MapFileType
{
    SQL,
    OPENDRIVE
};
struct SubLane
{
    hadmap::txLanePtr lane;
    double startPos;
    double endPos;
};
/**
 *Hadmap service
 *Singleton model
 *Must init before use
 *MAPENGINE mode is not support right now
 */
class HADMAP_API HadmapManager
{
public:
    ~HadmapManager();
    // origin coodinate
    double mapOriginLon = 0;
    double mapOriginLat = 0;
    double mapOriginAlt = 0;

private:
    // static HadmapManager* instance;

    bool bInitSuccess = false;
    bool bMapDataLoaded = false;
    MapMode mapMode = MapMode::MAPENGINE;
    FString mapPath_SQL;
    // mapengine
    hadmap::txMapHandle* mapengineHandle = NULL;
    int mapEngineStat = TX_HADMAP_HANDLE_ERROR;
    // routingmap
    hadmap::RoutingMap* routingmapHandle = NULL;
    hadmap::txMapInterfacePtr routingmapInterface = NULL;

    HadmapManager();
    bool CreateHadmapHandle();
    void CloseHadmapHandle();

public:
    // Get singleton
    static HadmapManager* Get()
    {
        static HadmapManager handle;
        return &handle;
        // if (!instance)
        //{
        //     instance = new HadmapManager();
        // }
        // return instance;
    };
    // Init hadmap, load hadmap default
    bool Init(MapMode _Mode, FString _DBPath, double _OriginLon, double _OriginLat, double _OriginAlt,
        const FString& _GPSFilePath = TEXT(""));
    // Init hadmap, do not load hadmap default
    bool Init(double _OriginLon, double _OriginLat, double _OriginAlt, const FString& _GPSFilePath = TEXT(""));
    // Get map type
    MapMode GetMapMode() const;
    bool IsMapReady() const
    {
        return bInitSuccess;
    }
    bool IsMapDataLoaded() const
    {
        return bMapDataLoaded;
    }
    //=====================
    // Routingmap Interface
    //=====================

    // Update ego vehicle location
    // Call it every frame
    void UpdateRoutingmap(double _EgoLon, double _EgoLat, double _EgoAlt);
    // Get routing map handle
    hadmap::txMapInterfacePtr GetMapInterface() const;
    bool GetLane(double _Lon, double _Lat, double _Alt, hadmap::txLanePtr& _Lane, double _RadiusInMeter = 2);
    bool GetLanes(double _Lon, double _Lat, double _Alt, hadmap::txLanes& _Lanes, double _RadiusInMeter = 100);
    bool GetRightLane(const hadmap::txLanePtr _Lane, hadmap::txLanePtr& _RightLane);
    bool GetLeftLane(const hadmap::txLanePtr _Lane, hadmap::txLanePtr& _LeftLane);
    bool GetNextLanes(const hadmap::txLanePtr _Lane, hadmap::txLanes& _NextLanes);
    bool GetPreLanes(const hadmap::txLanePtr _Lane, hadmap::txLanes& _PreLanes);
    bool GetLeftBoundary(const hadmap::txLanePtr _Lane, hadmap::txLaneBoundaryPtr& _Boundary);
    bool GetRightBoundary(const hadmap::txLanePtr _Lane, hadmap::txLaneBoundaryPtr& _Boundary);
    bool IsJunction(const hadmap::txLanePtr _Lane);
    //
    bool SampleCurveToLonLat(const hadmap::txCurve* _Curve, hadmap::PointVec& PointArry, double _DeltaDistance,
        double _StartPos, double _EndPos);
    bool SampleCurveToLoc(const hadmap::txCurve* _Curve, TArray<FVector>& PointArry, double _DeltaDistance,
        double _StartPos, double _EndPos);

    /**
     *Get Front Lanes in order By Length
     *@ Param _Length: Along the front lanes, unit is meter.
     */
    bool GetFrontLanesByLength(const hadmap::txLanePtr _StartLane, double _Lon, double _Lat, double _Alt,
        TArray<SubLane>& _Lanes, double _Length = 100);
    /**
     *Get back Lanes in order By Length
     *@ Param _Length: Along the back lanes, unit is meter.
     */
    bool GetBackLanesByLength(const hadmap::txLanePtr _StartLane, double _Lon, double _Lat, double _Alt,
        TArray<SubLane>& _Lanes, double _Length = 100);

    //====================
    // Mapengine Interface
    //====================

    // Get mapengine handle
    hadmap::txMapHandle* GetMapHandle() const;

    //===================
    // Coodinate transform
    //===================

    void LocalToLonLat(double& _X, double& _Y, double& _Z);
    void LocalToLonLat(const FVector& _Loc, double& _X, double& _Y, double& _Z);
    void LonLatToLocal(double& _Lon, double& _Lat, double& _Alt);
    void LonLatToLocal(double _X, double _Y, double _Z, FVector& _Loc);
    static void LocalToLonLat(const FVector& _Loc, double& _X, double& _Y, double& _Z, double _MapOriginLon,
        double _MapOriginLat, double _MapOriginAlt);
    static void LonLatToLocal(double _X, double _Y, double _Z, FVector& _Loc, double _MapOriginLon,
        double _MapOriginLat, double _MapOriginAlt);

    // Decrypt
    bool SetDecryptMode(const FString& gpsfile);
    bool Decrypt(double& _Lon, double& _Lat);

    // Get Objects
    hadmap::txObjects GetObjects(const hadmap::PointVec& _Envelope, const std::vector<hadmap::OBJECT_TYPE>& _ObjTypes);
    // hadmap::txObjects GetObjects(const FVector& _MinPoint, const FVector& _MaxPoint, const
    // TArray<hadmap::OBJECT_TYPE> &_ObjTypes);
    static void GetPointsFromObj(const hadmap::txObject& _Object, hadmap::PointVec& _Points);
    void GetPointsFromObj(const hadmap::txObject& _Object, TArray<FVector>& _Points);

    static hadmap::OBJECT_TYPE StringToObjectType(const FString& _String);

private:
    class kdtree_flann<double, std::pair<double, double>, flann::L2_Simple<double>>* m_gKdtree
    {
        nullptr
    };
    bool bUseDecrypt = false;

public:
    hadmap::MAP_DATA_TYPE GetMapDataType(const FString& _FilePath) const;

public:
    // AutoRoad Plugin Function
    void GetObjects(const std::vector<hadmap::txLaneId>& _Envelope, const std::vector<hadmap::OBJECT_TYPE>& _ObjTypes,
        hadmap::txObjects& objects);
    int getJunctions(hadmap::txJunctions& junctions);
    int getLaneLink(const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr);
    int getRoad(const hadmap::roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr);

    hadmap::txPoint getTxObjectPos(const hadmap::txObjects::value_type);
    void getTxObjectUserData(const hadmap::txObjects::value_type object, std::map<std::string, std::string>& map);

    void getTxJunctionLanLink(const hadmap::txJunctions::value_type object, std::vector<hadmap::lanelinkpkid>& map);
};
}    // namespace hadmapue4

#define SHadmap hadmapue4::HadmapManager::Get()
