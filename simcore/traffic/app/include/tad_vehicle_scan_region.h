// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#if USE_SUDOKU_GRID
#  include <array>
#  include "tad_vehicle_behavior_configure_utils.h"
#  include "tx_serialization.h"
#  include "tx_sim_point.h"
#  include "tx_traffic_element_base.h"
TX_NAMESPACE_OPEN(Scene)

class VehicleScanRegion2D {
 public:
  enum class SideGapType : Base::txInt { LeftSide, RightSide };
  VehicleScanRegion2D() TX_DEFAULT;
  ~VehicleScanRegion2D() TX_DEFAULT;

  Base::txBool Initialize(const Base::txFloat _len, const Base::txFloat _width) TX_NOEXCEPT {
    const Base::txFloat _half_len = 0.5 * _len;
    const Base::txFloat _half_width = 0.5 * _width;

    Base::txVec2& vehicle_left_top = vision_grid_4x4[1][1];
    Base::txVec2& vehicle_left_bottom = vision_grid_4x4[2][1];
    Base::txVec2& vehicle_right_bottom = vision_grid_4x4[2][2];
    Base::txVec2& vehicle_right_top = vision_grid_4x4[1][2];

    vehicle_left_top = Base::txVec2(_half_len, _half_width);
    vehicle_left_bottom = Base::txVec2(-1.0 * _half_len, _half_width);
    vehicle_right_bottom = Base::txVec2(-1.0 * _half_len, -1.0 * _half_width);
    vehicle_right_top = Base::txVec2(_half_len, -1.0 * _half_width);

#  define cFactor (0.3)
    vision_grid_4x4[0][0] = vehicle_left_top + Base::txVec2(VisionDistance_X, VisionDistance_Y);
    vision_grid_4x4[0][1] = vehicle_left_top + Base::txVec2(VisionDistance_X, cFactor * VisionDistance_Y);
    vision_grid_4x4[0][2] = vehicle_right_top + Base::txVec2(VisionDistance_X, -cFactor * VisionDistance_Y);
    vision_grid_4x4[0][3] = vehicle_right_top + Base::txVec2(VisionDistance_X, -1.0 * VisionDistance_Y);

    vision_grid_4x4[1][0] = vehicle_left_top + Base::txVec2(0.0 * cFactor * VisionDistance_X, VisionDistance_Y);
    vision_grid_4x4[1][3] = vehicle_right_top + Base::txVec2(0.0 * cFactor * VisionDistance_X, -1.0 * VisionDistance_Y);

    vision_grid_4x4[2][0] = vehicle_left_bottom + Base::txVec2(0.0 * -cFactor * VisionDistance_X, VisionDistance_Y);
    vision_grid_4x4[2][3] =
        vehicle_right_bottom + Base::txVec2(0.0 * -cFactor * VisionDistance_X, -1.0 * VisionDistance_Y);

    vision_grid_4x4[3][0] = vehicle_left_bottom + Base::txVec2(-1.0 * VisionDistance_X, VisionDistance_Y);
    vision_grid_4x4[3][1] = vehicle_left_bottom + Base::txVec2(-1.0 * VisionDistance_X, cFactor * VisionDistance_Y);
    vision_grid_4x4[3][2] = vehicle_right_bottom + Base::txVec2(-1.0 * VisionDistance_X, -cFactor * VisionDistance_Y);
    vision_grid_4x4[3][3] = vehicle_right_bottom + Base::txVec2(-1.0 * VisionDistance_X, -1.0 * VisionDistance_Y);

#  define cFrontFactor (0.0)
    const Base::txFloat cIDM_Vision_Width = Utils::VehicleBehaviorCfg::IDM_Vision_Width();
    const Base::txFloat cIDM_Vision_Length = Utils::VehicleBehaviorCfg::IDM_Vision_Length();
    front_vision_grid_2x2[1][0] = Base::txVec2(_half_len, 0.5 * cIDM_Vision_Width);
    front_vision_grid_2x2[1][1] = Base::txVec2(_half_len, -0.5 * cIDM_Vision_Width);
    front_vision_grid_2x2[0][0] = front_vision_grid_2x2[1][0] + Base::txVec2(cIDM_Vision_Length, 0.0);
    front_vision_grid_2x2[0][1] = front_vision_grid_2x2[1][1] + Base::txVec2(cIDM_Vision_Length, 0.0);
#  undef cFrontFactor
#  undef cFactor
    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eFront))] = {
        Base::txVec2I(0, 1), Base::txVec2I(0, 2), Base::txVec2I(1, 2), Base::txVec2I(1, 1)};
    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eBack))] = {
        Base::txVec2I(2, 1), Base::txVec2I(2, 2), Base::txVec2I(3, 2), Base::txVec2I(3, 1)};
#  if __TX_Mark__("ID90259561")
    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeft))] = {
        Base::txVec2I(1, 0), Base::txVec2I(1, 1), Base::txVec2I(2, 1), Base::txVec2I(2, 0)};
    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftFront))] = {
        Base::txVec2I(0, 0), Base::txVec2I(0, 1), Base::txVec2I(1, 1), Base::txVec2I(1, 0)};
    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftBack))] = {
        Base::txVec2I(2, 0), Base::txVec2I(2, 1), Base::txVec2I(3, 1), Base::txVec2I(3, 0)};

    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRight))] = {
        Base::txVec2I(1, 2), Base::txVec2I(1, 3), Base::txVec2I(2, 3), Base::txVec2I(2, 2)};
    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightFront))] = {
        Base::txVec2I(0, 2), Base::txVec2I(0, 3), Base::txVec2I(1, 3), Base::txVec2I(1, 2)};
    vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightBack))] = {
        Base::txVec2I(2, 2), Base::txVec2I(2, 3), Base::txVec2I(3, 3), Base::txVec2I(3, 2)};
#  else  /*__TX_Mark__("ID90259561")*/
    vision_region_clockwise_close_index[Base::OBJECT_TARGET_RIGHT_DIRECTION] = {
        Base::txVec2I(1, 0), Base::txVec2I(1, 1), Base::txVec2I(2, 1), Base::txVec2I(2, 0)};
    vision_region_clockwise_close_index[Base::OBJECT_TARGET_RIGHT_FRONT_DIRECTION] = {
        Base::txVec2I(0, 0), Base::txVec2I(0, 1), Base::txVec2I(1, 1), Base::txVec2I(1, 0)};
    vision_region_clockwise_close_index[Base::OBJECT_TARGET_RIGHT_BACK_DIRECTION] = {
        Base::txVec2I(2, 0), Base::txVec2I(2, 1), Base::txVec2I(3, 1), Base::txVec2I(3, 0)};

    vision_region_clockwise_close_index[Base::OBJECT_TARGET_LEFT_DIRECTION] = {
        Base::txVec2I(1, 2), Base::txVec2I(1, 3), Base::txVec2I(2, 3), Base::txVec2I(2, 2)};
    vision_region_clockwise_close_index[Base::OBJECT_TARGET_LEFT_FRONT_DIRECTION] = {
        Base::txVec2I(0, 2), Base::txVec2I(0, 3), Base::txVec2I(1, 3), Base::txVec2I(1, 2)};
    vision_region_clockwise_close_index[Base::OBJECT_TARGET_LEFT_BACK_DIRECTION] = {
        Base::txVec2I(2, 2), Base::txVec2I(2, 3), Base::txVec2I(3, 3), Base::txVec2I(3, 2)};
#  endif /*__TX_Mark__("ID90259561")*/
    front_vision_region_clockwise_close_index[0] = {Base::txVec2I(0, 0), Base::txVec2I(0, 1), Base::txVec2I(1, 1),
                                                    Base::txVec2I(1, 0)};

    return CreateSideGapGrid(_len, _width);
  }

  Base::txBool CreateSideGapGrid(const Base::txFloat _len, const Base::txFloat _width) TX_NOEXCEPT {
    const Base::txFloat _half_len = 0.5 * _len;
    const Base::txFloat _half_width = 0.5 * _width;

    Base::txVec2& vehicle_left_top = side_gap_grid_4x4[1][1];
    Base::txVec2& vehicle_left_bottom = side_gap_grid_4x4[2][1];
    Base::txVec2& vehicle_right_bottom = side_gap_grid_4x4[2][2];
    Base::txVec2& vehicle_right_top = side_gap_grid_4x4[1][2];

    vehicle_left_top = Base::txVec2(_half_len, _half_width);
    vehicle_left_bottom = Base::txVec2(-1.0 * _half_len, _half_width);
    vehicle_right_bottom = Base::txVec2(-1.0 * _half_len, -1.0 * _half_width);
    vehicle_right_top = Base::txVec2(_half_len, -1.0 * _half_width);

    side_gap_grid_4x4[0][0] = vehicle_left_top + Base::txVec2(VisionDistance_X, VisionDistance_Y);
    side_gap_grid_4x4[0][1] = vehicle_left_top + Base::txVec2(VisionDistance_X, 0.0 * VisionDistance_Y);
    side_gap_grid_4x4[0][2] = vehicle_right_top + Base::txVec2(VisionDistance_X, 0.0 * VisionDistance_Y);
    side_gap_grid_4x4[0][3] = vehicle_right_top + Base::txVec2(VisionDistance_X, -1.0 * VisionDistance_Y);

    side_gap_grid_4x4[1][0] =
        vehicle_left_top + Base::txVec2(Utils::VehicleBehaviorCfg::SideFrontVisionDistance(), VisionDistance_Y);
    side_gap_grid_4x4[1][3] =
        vehicle_right_top + Base::txVec2(Utils::VehicleBehaviorCfg::SideFrontVisionDistance(), -1.0 * VisionDistance_Y);

    side_gap_grid_4x4[2][0] =
        vehicle_left_bottom +
        Base::txVec2(-1.0 * Utils::VehicleBehaviorCfg::SideRearVisionDistance(), VisionDistance_Y);
    side_gap_grid_4x4[2][3] =
        vehicle_right_bottom +
        Base::txVec2(-1.0 * Utils::VehicleBehaviorCfg::SideRearVisionDistance(), -1.0 * VisionDistance_Y);

    side_gap_grid_4x4[3][0] = vehicle_left_bottom + Base::txVec2(-1.0 * VisionDistance_X, VisionDistance_Y);
    side_gap_grid_4x4[3][1] = vehicle_left_bottom + Base::txVec2(-1.0 * VisionDistance_X, 0.0 * VisionDistance_Y);
    side_gap_grid_4x4[3][2] = vehicle_right_bottom + Base::txVec2(-1.0 * VisionDistance_X, 0.0 * VisionDistance_Y);
    side_gap_grid_4x4[3][3] = vehicle_right_bottom + Base::txVec2(-1.0 * VisionDistance_X, -1.0 * VisionDistance_Y);

    side_gap_grid_4x4[1][1] =
        vehicle_left_top + Base::txVec2(Utils::VehicleBehaviorCfg::SideFrontVisionDistance(), 0.0);
    side_gap_grid_4x4[1][2] =
        vehicle_right_top + Base::txVec2(Utils::VehicleBehaviorCfg::SideFrontVisionDistance(), 0.0);
    return true;
  }

  Base::txBool Rigid_Translate_Rotation(const Coord::txENU& vehicleCenter, const Base::txFloat _cos,
                                        const Base::txFloat _sin, const Base::txFloat _heading_cos,
                                        const Base::txFloat _heading_sin) TX_NOEXCEPT {
    vehicleCenter_current_frame = vehicleCenter;
    {
      const Base::txVec2 dst_pt = vehicleCenter.ENU2D();
      const auto& src_vision_grid_4x4 = vision_grid_4x4;
      for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
          auto& ref_dst_x = vision_grid_4x4_current_frame[r][c][0];
          auto& ref_dst_y = vision_grid_4x4_current_frame[r][c][1];
          const auto& src_x = src_vision_grid_4x4[r][c].x();
          const auto& src_y = src_vision_grid_4x4[r][c].y();
          ref_dst_x = (_cos * src_x - _sin * src_y) + dst_pt.x();
          TX_MARK("origin is (0.0), dst_pt is translation vector.");
          ref_dst_y = (_sin * src_x + _cos * src_y) + dst_pt.y();
        }
      }
    }

    // LOG(INFO) << "Rigid_Translate_Rotation : " << Str();
    return Rigid_Translate_Rotation_SideGapGrid(vehicleCenter, _cos, _sin);
  }

  Base::txBool Rigid_Translate_Rotation_IDM(const Coord::txENU& PosOnCenterLine, const Base::txFloat _lane_cos,
                                            const Base::txFloat _lane_sin) TX_NOEXCEPT {
    const Base::txVec2 dst_pt = PosOnCenterLine.ENU2D();
    for (int r = 0; r < 2; ++r) {
      for (int c = 0; c < 2; ++c) {
        auto& ref_dst_x = front_vision_grid_2x2_current_frame[r][c].x();
        auto& ref_dst_y = front_vision_grid_2x2_current_frame[r][c].y();
        const auto& src_x = front_vision_grid_2x2[r][c].x();
        const auto& src_y = front_vision_grid_2x2[r][c].y();
        ref_dst_x = (_lane_cos * src_x - _lane_sin * src_y) + dst_pt.x();
        TX_MARK("origin is (0.0), dst_pt is translation vector.");
        ref_dst_y = (_lane_sin * src_x + _lane_cos * src_y) + dst_pt.y();
      }
    }
    return true;
  }

  Base::txBool Rigid_Translate_Rotation_SideGapGrid(const Coord::txENU& vehicleCenter, const Base::txFloat _cos,
                                                    const Base::txFloat _sin) TX_NOEXCEPT {
    const Base::txVec2 dst_pt = vehicleCenter.ENU2D();
    const auto& src_side_gap_grid_4x4 = side_gap_grid_4x4;
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        auto& ref_dst_x = side_gap_grid_4x4_current_frame[r][c][0];
        auto& ref_dst_y = side_gap_grid_4x4_current_frame[r][c][1];
        const auto& src_x = src_side_gap_grid_4x4[r][c].x();
        const auto& src_y = src_side_gap_grid_4x4[r][c].y();
        ref_dst_x = (_cos * src_x - _sin * src_y) + dst_pt.x();
        TX_MARK("origin is (0.0), dst_pt is translation vector.");
        ref_dst_y = (_sin * src_x + _cos * src_y) + dst_pt.y();
      }
    }
    return true;
  }

  void Get_Region_Clockwise_Close_By_Enum(const Base::Enums::NearestElementDirection eValue,
                                          std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT {
    {
      const auto idx = __enum2idx__(eValue); /* eValue._to_index();*/
      retRegion.resize(5);
      const auto& clock_wise_close_index_array = vision_region_clockwise_close_index[idx];
      txAssert(4 == clock_wise_close_index_array.size());

      for (int i = 0; i < 4; ++i) {
        retRegion[i] =
            vision_grid_4x4_current_frame[clock_wise_close_index_array[i][0]][clock_wise_close_index_array[i][1]];
      }
      retRegion[4] = retRegion[0];
    }
  }

  void Get_IDM_Front_Region_Clockwise_Close(std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT {
    retRegion.resize(5);
    const auto& clock_wise_close_index_array = front_vision_region_clockwise_close_index[0];
    txAssert(4 == clock_wise_close_index_array.size());

    for (int i = 0; i < 4; ++i) {
      retRegion[i] =
          front_vision_grid_2x2_current_frame[clock_wise_close_index_array[i][0]][clock_wise_close_index_array[i][1]];
    }
    retRegion[4] = retRegion[0];
  }

  void Get_SideGap_Region_Clockwise_Close_By_Enum(const SideGapType side_dir,
                                                  std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT {
    if (SideGapType::LeftSide == side_dir) {
      retRegion.resize(5);
      const auto& clock_wise_close_index_array =
          vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeft))];
      txAssert(4 == clock_wise_close_index_array.size());

      for (int i = 0; i < 4; ++i) {
        retRegion[i] =
            side_gap_grid_4x4_current_frame[clock_wise_close_index_array[i][0]][clock_wise_close_index_array[i][1]];
      }
      retRegion[4] = retRegion[0];
    } else {
      retRegion.resize(5);
      const auto& clock_wise_close_index_array =
          vision_region_clockwise_close_index[__enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRight))];
      txAssert(4 == clock_wise_close_index_array.size());

      for (int i = 0; i < 4; ++i) {
        retRegion[i] =
            side_gap_grid_4x4_current_frame[clock_wise_close_index_array[i][0]][clock_wise_close_index_array[i][1]];
      }
      retRegion[4] = retRegion[0];
    }
  }

  Base::txString Str() TX_NOEXCEPT {
    std::ostringstream oss;

    oss << _StreamPrecision_ << vehicleCenter_current_frame.ToWGS84() << ". ";
    std::vector<Base::txVec2> curVisionRegion;
    for (int i = 0; i < Base::Enums::szNearestElementDirection; ++i) {
      std::ostringstream oss_sub;
      oss_sub << "{ " << __idx2enum__(Base::Enums::NearestElementDirection, i) << std::endl;
      Get_Region_Clockwise_Close_By_Enum(__idx2enum__(Base::Enums::NearestElementDirection, i), curVisionRegion);
      Coord::txWGS84 tmpWGS84;
      for (const auto cur2dPt : curVisionRegion) {
        tmpWGS84.FromENU(__East__(cur2dPt), __North__(cur2dPt));
        oss_sub << tmpWGS84 << ";" << std::endl;
      }
      oss_sub << "}," << std::endl;
      oss << oss_sub.str();
    }
    oss << "}." << std::endl;

    {
      std::ostringstream oss_sub;
      oss_sub << "{ LeftSide" << std::endl;
      Get_SideGap_Region_Clockwise_Close_By_Enum(SideGapType::LeftSide, curVisionRegion);
      Coord::txWGS84 tmpWGS84;
      for (const auto cur2dPt : curVisionRegion) {
        tmpWGS84.FromENU(__East__(cur2dPt), __North__(cur2dPt));
        oss_sub << tmpWGS84 << ";" << std::endl;
      }
      oss_sub << "}," << std::endl;
      oss << oss_sub.str();
    }
    {
      std::ostringstream oss_sub;
      oss_sub << "{ RightSide" << std::endl;
      Get_SideGap_Region_Clockwise_Close_By_Enum(SideGapType::RightSide, curVisionRegion);
      Coord::txWGS84 tmpWGS84;
      for (const auto cur2dPt : curVisionRegion) {
        tmpWGS84.FromENU(__East__(cur2dPt), __North__(cur2dPt));
        oss_sub << tmpWGS84 << ";" << std::endl;
      }
      oss_sub << "}," << std::endl;
      oss << oss_sub.str();
    }
    {
      std::ostringstream oss_sub;
      oss_sub << "{ IDM_Front_Side" << std::endl;
      Get_IDM_Front_Region_Clockwise_Close(curVisionRegion);
      Coord::txWGS84 tmpWGS84;
      for (const auto cur2dPt : curVisionRegion) {
        tmpWGS84.FromENU(__East__(cur2dPt), __North__(cur2dPt));
        oss_sub << tmpWGS84 << ";" << std::endl;
      }
      oss_sub << "}," << std::endl;
      oss << oss_sub.str();
    }
    return oss.str();
  }

 protected:
  const Base::txFloat VisionDistance_X = FLAGS_DETECT_OBJECT_DISTANCE;
  const Base::txFloat VisionDistance_Y = FLAGS_default_lane_width;
  enum { nRegionVertexSize = 16 /* 3 x 3 */ };
  std::array<std::array<Base::txVec2, 4>, 4> vision_grid_4x4, vision_grid_4x4_current_frame;
  std::array<std::array<Base::txVec2, 2>, 2> front_vision_grid_2x2, front_vision_grid_2x2_current_frame;
  Coord::txENU vehicleCenter_current_frame;
  std::array<std::array<Base::txVec2I, 4>, Base::Enums::szNearestElementDirection> vision_region_clockwise_close_index;
  std::array<std::array<Base::txVec2I, 4>, 1> front_vision_region_clockwise_close_index;

  std::array<std::array<Base::txVec2, 4>, 4> side_gap_grid_4x4, side_gap_grid_4x4_current_frame;

  Base::txFloat vehicle_length;
  Base::txFloat vehicle_width;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    archive(vision_grid_4x4);
    archive(vision_grid_4x4_current_frame);
    archive(front_vision_grid_2x2);
    archive(front_vision_grid_2x2_current_frame);
    archive(vehicleCenter_current_frame);
    archive(vision_region_clockwise_close_index);
    archive(front_vision_region_clockwise_close_index);
    archive(side_gap_grid_4x4);
    archive(side_gap_grid_4x4_current_frame);
    archive(vehicle_length);
    archive(vehicle_width);
  }
};

TX_NAMESPACE_CLOSE(Scene)
#endif /*USE_SUDOKU_GRID*/
