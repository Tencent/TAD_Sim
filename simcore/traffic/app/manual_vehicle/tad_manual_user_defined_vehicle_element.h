// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_manual_handler.h"
#include "tad_user_defined_vehicle_element.h"
TX_NAMESPACE_OPEN(TrafficFlow)
#if Use_ManualVehicle
class TAD_Manual_UserDefined_VehicleElement : public TAD_UserDefined_VehicleElement, public IManualHandler {
  using ParentClass = TAD_UserDefined_VehicleElement;

 public:
  virtual txBool Pre_Update(const TimeParamManager& timeMgr,
                            std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;
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
};

using TAD_Manual_UserDefined_VehicleElementPtr = std::shared_ptr<TAD_Manual_UserDefined_VehicleElement>;
#endif /*Use_ManualVehicle*/

TX_NAMESPACE_CLOSE(TrafficFlow)
