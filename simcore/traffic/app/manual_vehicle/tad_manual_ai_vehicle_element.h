// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_ai_vehicle_element.h"
#include "tad_manual_handler.h"
TX_NAMESPACE_OPEN(TrafficFlow)
#if Use_ManualVehicle
class TAD_Manual_AI_VehicleElement : public TAD_AI_VehicleElement, public IManualHandler {
  using ParentClass = TAD_AI_VehicleElement;

 public:
  virtual Base::txBool Update(TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

 public:
  using ManualHandlerType = IManualHandler::ManualHandlerType;
  virtual ManualHandlerType ManualElementType() const TX_NOEXCEPT TX_OVERRIDE { return ManualHandlerType::mVehicle; }
  virtual void UpdateManualLocation(Base::TimeParamManager const& timeMgr,
                                    const sim_msg::Location& _location) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txString ManualDesc() const TX_NOEXCEPT TX_OVERRIDE;

  virtual Base::txBool UpdateManualVehicle(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
  virtual void ComputeScalarVelocity(Base::TimeParamManager const& timeMgr,
                                     const Coord::txENU& egoMassCenter) TX_NOEXCEPT;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<TAD_AI_VehicleElement>(this));
    archive(cereal::base_class<IManualHandler>(this));
  }
};

using TAD_Manual_AI_VehicleElementPtr = std::shared_ptr<TAD_Manual_AI_VehicleElement>;
#endif /*Use_ManualVehicle*/

TX_NAMESPACE_CLOSE(TrafficFlow)

CEREAL_REGISTER_TYPE(TrafficFlow::TAD_Manual_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::TAD_AI_VehicleElement, TrafficFlow::TAD_Manual_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::IManualHandler, TrafficFlow::TAD_Manual_AI_VehicleElement);
