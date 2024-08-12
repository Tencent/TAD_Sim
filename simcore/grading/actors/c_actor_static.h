// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "actors/c_actor_base.h"
#include "manager/map_manager.h"

namespace eval {
/**
 * @brief "静态障碍物", static actor type, which is from base actor.
 */
class CStaticActor : public CActorBase {
 public:
  const std::string _name = "CStaticActor";
  CStaticActor() { _type = Actor_Static; }
  void CopyFrom(const CStaticActor &actor_in);
  virtual ~CStaticActor() {}

 public:
  virtual inline const CRect &GetShape() const { return _shape; }
  virtual double GetBaseLength() const { return GetShape().GetLength() + GetShape().GetWidth(); }
  inline int64_t GetID() const { return _id; }
  inline LaneID GetLaneID() const { return _lane_id; }
  inline int64_t GetTypeID() const { return _type_id; }

 public:
  virtual inline CRect *MutableShape() { return &_shape; }
  virtual inline void SetID(int64_t id) { _id = id; }

  // update lane id of actor
  void SetLaneID(uint64_t road_id = INVALID_MAP_ID, uint64_t section_id = INVALID_MAP_ID,
                 int64_t lane_id = LANE_INVALID_ID, uint64_t lanelink_id = INVALID_MAP_ID);
  void SetLaneID(const LaneID &laneID);
  void SetTypeID(int64_t typeID);

  // translate point in CStaticActor coordinate to its base coordinate
  EVector3d TransPoint2BaseCoord(const Eigen::Vector3d &B_pos);

  // translate corner points in CStaticActor coordinate to its base coordinate
  RectCorners TransCorners2BaseCoord();

  // translate middle-front point in CStaticActor coordinate to its base coordinate
  EVector3d TransMiddleFront2BaseCoord();

  // translate middle-rear point in CStaticActor coordinate to its base coordinate
  EVector3d TransMiddleRear2BaseCoord();

 private:
  int64_t _id;
  CRect _shape;
  LaneID _lane_id;
  int64_t _type_id;
};

/**
 * @brief "静态障碍物建造者", static actor builder, which build the actors from the map and the msg.
 */
class CStaticActorBuilder : public CActorBuilder {
 public:
  virtual void Build(const EvalMsg &msg, ActorReposity &actor_repo) override;

 protected:
  static int BuildFrom(const EvalMsg &msg, ActorReposity &actor_repo);
};
}  // namespace eval
