// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "location.pb.h"
#include "scene.pb.h"
#include "tad_sim.h"
#include "tx_header.h"
#include "tx_planning_flags.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(SceneLoader)

class Planning_SceneLoader : public Base::ISceneLoader {
 public:
  using txULong = Base::txULong;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txString = Base::txString;
  using txLpsz = Base::txLpsz;
  using ParentClass = Base::ISceneLoader;

 public:
  class EgoRouteViewer : public IRouteViewer {
   public:
    EgoRouteViewer() TX_DEFAULT;
    virtual ~EgoRouteViewer() TX_DEFAULT;
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return 0; }
    virtual ROUTE_TYPE type() const TX_NOEXCEPT TX_OVERRIDE { return +ROUTE_TYPE::ePos; }
    virtual txFloat startLon() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txFloat startLat() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txFloat endLon() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txFloat endLat() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::pair<txFloat, txFloat> > midPoints() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txRoadID roidId() const TX_NOEXCEPT TX_OVERRIDE { return 0; }
    virtual Base::txSectionID sectionId() const TX_NOEXCEPT TX_OVERRIDE { return 0; }
    virtual hadmap::txPoint startGPS() const TX_NOEXCEPT;
    virtual hadmap::txPoint endGPS() const TX_NOEXCEPT;
    virtual std::vector<hadmap::txPoint> midGPS() const TX_NOEXCEPT;
    virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE {
      std::ostringstream oss_midpoints;
      const auto& refMidPoints = midPoints();

      for (const auto& refpair : refMidPoints) {
        oss_midpoints << _StreamPrecision_ << "(" << refpair.first << ", " << refpair.second << ")";
      }
      std::ostringstream oss;
      oss << _StreamPrecision_ << TX_VARS(id()) << TX_VARS(startLon()) << TX_VARS(startLat()) << TX_VARS(endLon())
          << TX_VARS(endLat()) << TX_VARS(oss_midpoints.str()) << TX_VARS(roidId()) << TX_VARS(sectionId());
      return oss.str();
    }
    void Init(const std::vector<hadmap::txPoint>& vec_ego_path) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    std::vector<hadmap::txPoint> m_vec_ego_path;
  };
  using EgoRouteViewerPtr = std::shared_ptr<EgoRouteViewer>;

  class EgoStartViewer : public IViewer {
   public:
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return FLAGS_Default_EgoVehicle_Id; }
    virtual sim_msg::Location start_location() const { return m_ego_start_location; }
    void Init(const sim_msg::Location _ego_start_location) TX_NOEXCEPT {
      m_inited = true;
      m_ego_start_location.CopyFrom(_ego_start_location);
    }

   protected:
    Base::txBool m_inited = false;
    sim_msg::Location m_ego_start_location;
  };
  using EgoStartViewerPtr = std::shared_ptr<EgoStartViewer>;

 public:
  Planning_SceneLoader();
  virtual ~Planning_SceneLoader() TX_DEFAULT;
  virtual txBool IsValid() TX_NOEXCEPT TX_OVERRIDE { return true; }
  virtual txBool Load(txString const&) TX_NOEXCEPT TX_OVERRIDE { return false; }
  virtual txBool ParseSceneEvent() TX_NOEXCEPT { return false; }
  virtual txBool LoadObjects() TX_NOEXCEPT TX_OVERRIDE { return true; }
  virtual EStatus Status() const TX_NOEXCEPT TX_OVERRIDE { return EStatus::eReady; }
  virtual txLpsz ClassName() const TX_NOEXCEPT TX_OVERRIDE { return _class_name; }
  virtual txString Source() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

  virtual ESceneType GetSceneType() const TX_NOEXCEPT TX_OVERRIDE { return ESceneType::eTAD_Ego; }
  virtual EgoType GetEgoType() const TX_NOEXCEPT { return EgoType::eVehicle; }
  virtual Base::txBool GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT TX_OVERRIDE {
    refEgoData.CopyFrom(m_ego_start_location);
    return true;
  }
  virtual Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
      TX_NOEXCEPT TX_OVERRIDE {
    TX_MARK("SimEgoTemplate::GetMapManagerInitParams");
    return false;
  }
  virtual IRouteViewerPtr GetRouteData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual IRouteViewerPtr GetEgoRouteData() TX_NOEXCEPT TX_OVERRIDE;
  virtual IAccelerationViewerPtr GetAccsData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual IAccelerationViewerPtr GetAccsEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual IMergesViewerPtr GetMergesData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual IMergesViewerPtr GetMergesEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual IVelocityViewerPtr GetVelocityData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual IVelocityViewerPtr GetVelocityEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  virtual IPedestriansEventViewerPtr GetPedestriansEventData_TimeEvent(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return nullptr;
  }
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_VelocityEvent(Base::txSysId const id)
      TX_NOEXCEPT TX_OVERRIDE {
    return nullptr;
  }

  virtual txBool HasSceneEvent() const TX_NOEXCEPT { return _NonEmpty_(_seceneEventVec); }
  virtual std::vector<ISceneEventViewerPtr> GetSceneEventVec() const TX_NOEXCEPT { return _seceneEventVec; }

  virtual IVehiclesViewerPtr GetVehicleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual std::unordered_map<Base::txSysId, IVehiclesViewerPtr> GetAllVehicleData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, IVehiclesViewerPtr>();
  }
  virtual IPedestriansViewerPtr GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual std::unordered_map<Base::txSysId, IPedestriansViewerPtr> GetAllPedestrianData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, IPedestriansViewerPtr>();
  }
  virtual ISignlightsViewerPtr GetSignlightData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual std::unordered_map<Base::txSysId, ISignlightsViewerPtr> GetAllSignlightData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, ISignlightsViewerPtr>();
  }
  virtual IObstacleViewerPtr GetObstacleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual std::unordered_map<Base::txSysId, IObstacleViewerPtr> GetAllObstacleData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, IObstacleViewerPtr>();
  }
  virtual Base::txInt GetRandomSeed() const TX_NOEXCEPT TX_OVERRIDE { return FLAGS_ego_rnd_seed; }
  virtual IVehiclesViewerPtr GetEgoData() TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual IVehiclesViewerPtr GetEgoTrailerData() TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual Base::ISceneLoader::IViewerPtr GetEgoStartData() TX_NOEXCEPT;

  virtual txBool Init(const std::vector<hadmap::txPoint>& vec_ego_path, const sim_msg::Location ego_start_loction,
                      const sim_msg::Scene& sim_scene_pb) TX_NOEXCEPT;
  virtual std::vector<sim_msg::Waypoint> ControlPathPoints() const TX_NOEXCEPT;
  virtual Base::txFloat PlannerStartV() const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txFloat PlannerTheta() const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txFloat PlannerVelocityMax() const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txFloat PlannerAccMax() const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txFloat PlannerDeceMax() const TX_NOEXCEPT TX_OVERRIDE;
  virtual const sim_msg::Ego GetEgoFromGroupName() const TX_NOEXCEPT;

 protected:
  Base::txLpsz _class_name;
  sim_msg::Location m_ego_start_location;
  std::vector<hadmap::txPoint> m_vec_ego_path;
  sim_msg::Scene m_scene_pb;
  Base::txString m_groupname = "";
};

using Planning_SceneLoaderPtr = std::shared_ptr<Planning_SceneLoader>;
TX_NAMESPACE_CLOSE(SceneLoader)
