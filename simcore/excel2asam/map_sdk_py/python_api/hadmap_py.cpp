// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the python binding for the hadmap_py.

#include "hadmap_py.h"
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <structs/hadmap_junction.h>
#include <iostream>
#include <string>
#include <vector>
#include "hadmap_map.h"

namespace py = pybind11;
using namespace hadmap;

/**
 * @brief
 *
 * @param m
 */
void bind_txLaneLink(py::module& m) {
  py::class_<txLaneLink, txLaneLinkPtr>(m, "txLaneLink")
      .def(py::init<>())
      .def(py::init<const txLaneLink&>())
      // 添加转换为字符串的方法，这将用于Python中的print和str函数
      .def("__repr__", [](const txLaneLink& j) { return "<txLaneLink with ID: " + std::to_string(j.getId()) + ">"; })
      // 添加 get_id 方法
      .def("get_id", &txLaneLink::getId)
      //
      .def("from_road_id", &txLaneLink::fromRoadId)
      //
      .def("set_junction_id", &txLaneLink::setJunctionId);
}

void bind_txRoad(py::module& m) {
  py::class_<txRoad, txRoadPtr>(m, "txRoad")
      // 构造函数
      .def(py::init<>())
      .def(py::init<roadpkid, const std::string&>())
      .def(py::init<const txRoad&>())
      // 添加转换为字符串的方法，这将用于Python中的print和str函数
      .def("__repr__", [](const txRoad& j) { return "<txRoad with ID: " + std::to_string(j.getId()) + ">"; })
      // 添加 get_id 方法
      .def("get_id", &txRoad::getId);
}

void bind_txJunction(py::module& m) {
  // 注册 txJunction 类及其相关类型
  py::class_<txJunction, txJunctionPtr>(m, "txJunction")
      // 构造函数
      .def(py::init<const junctionpkid&>())
      .def(py::init<const txJunction&>())
      // 添加转换为字符串的方法，这将用于Python中的print和str函数
      .def("__repr__", [](const txJunction& j) { return "<txJunction with ID: " + std::to_string(j.getId()) + ">"; })
      // 添加 get_id 方法
      .def("get_id", &txJunction::getId)
      // 添加 get_lanelink 方法
      .def("get_lanelink_ids",
           [](const txJunction& self) {
             std::vector<lanelinkpkid> linkIds;
             self.getLaneLink(linkIds);
             return linkIds;
           })
      // 添加 get_entrance_road_ids 方法
      .def("get_entrance_road_ids", [](const txJunction& self) {
        // 创建一个非 const 临时变量, 因为 getEntranceRoads 函数在 txJunction 类中没有声明为 const 方法
        txJunction non_const_self(self);
        std::vector<roadpkid> rIds;
        non_const_self.getEntranceRoads(rIds);
        return rIds;
      });
}

void bind_Waypoint(py::module& m) {
  /*! \~Chinese
   * @brief 添加 Orientation 类绑定
   *
   */
  py::class_<Orientation>(m, "Orientation")
      .def(py::init<>())
      .def(py::init<double, double, double>(), py::arg("h"), py::arg("p"), py::arg("r"))
      .def(py::init<const Orientation&>())
      .def_readwrite("h", &Orientation::h)
      .def_readwrite("p", &Orientation::p)
      .def_readwrite("r", &Orientation::r)
      .def("__copy__", [](const Orientation& o) { return Orientation(o); })
      .def("assign", &Orientation::operator=, "Assignment operator")
      .def("__eq__", &Orientation::operator==, "Equality check")
      .def("__ne__", [](const Orientation& a, const Orientation& b) { return !(a == b); })
      .def("is_zero", &Orientation::operator!, "Check if zero")
      .def("__add__", &Orientation::operator+, "Addition")
      .def("__sub__", &Orientation::operator-, "Subtraction")
      .def("__mul__", &Orientation::operator*, "Multiplication by scalar")
      .def("__truediv__", &Orientation::operator/, "Division by scalar")
      .def("__repr__", [](const Orientation& o) {
        return "Orientation(h=" + std::to_string(o.h) + ", p=" + std::to_string(o.p) + ", r=" + std::to_string(o.r) +
               ")";
      });

  /*! \~Chinese
   * @brief 添加 PositionWorld 类绑定
   *
   */
  py::class_<PositionWorld>(m, "PositionWorld")
      .def(py::init<>())
      .def(py::init<double, double, double>(), py::arg("x"), py::arg("y"), py::arg("z"))
      .def(py::init<double, double, double, double, double, double>(), py::arg("x"), py::arg("y"), py::arg("z"),
           py::arg("h"), py::arg("p"), py::arg("r"))
      .def(py::init<const PositionWorld&>())
      .def_readwrite("x", &PositionWorld::x)
      .def_readwrite("y", &PositionWorld::y)
      .def_readwrite("z", &PositionWorld::z)
      .def_readwrite("h", &PositionWorld::h)
      .def_readwrite("p", &PositionWorld::p)
      .def_readwrite("r", &PositionWorld::r)
      .def("__copy__", [](const PositionWorld& p) { return PositionWorld(p); })
      .def("assign", &PositionWorld::operator=, "Assignment operator")
      .def("__eq__", &PositionWorld::operator==, "Equality check")
      .def("__ne__", [](const PositionWorld& a, const PositionWorld& b) { return !(a == b); })
      .def("is_zero", &PositionWorld::operator!, "Check if zero")
      .def("__add__", &PositionWorld::operator+)
      .def("__sub__", &PositionWorld::operator-)
      .def("__mul__", &PositionWorld::operator*, py::is_operator())
      .def("__truediv__", &PositionWorld::operator/, py::is_operator())
      .def("__repr__", [](const PositionWorld& p) {
        return "PositionWorld(x=" + std::to_string(p.x) + ", y=" + std::to_string(p.y) + ", z=" + std::to_string(p.z) +
               ", h=" + std::to_string(p.h) + ", p=" + std::to_string(p.p) + ", r=" + std::to_string(p.r) + ")";
      });

  /*! \~Chinese
   * @brief 添加 PositionLane 类绑定
   *
   */
  py::class_<PositionLane>(m, "PositionLane")
      .def(py::init<>())
      .def(py::init<uint64_t, int64_t, double, double>(), py::arg("road_id"), py::arg("lane_id"), py::arg("offset"),
           py::arg("s"))
      .def(py::init<uint64_t, int64_t, double, double, const Orientation&>(), py::arg("road_id"), py::arg("lane_id"),
           py::arg("offset"), py::arg("s"), py::arg("orientation"))
      .def(py::init<const PositionLane&>())
      .def_readwrite("road_id", &PositionLane::road_id)
      .def_readwrite("lane_id", &PositionLane::lane_id)
      .def_readwrite("offset", &PositionLane::offset)
      .def_readwrite("s", &PositionLane::s)
      .def_readwrite("orientation", &PositionLane::orientation)
      .def("__copy__", [](const PositionLane& p) { return PositionLane(p); })
      .def("assign", &PositionLane::operator=, "Assignment operator")
      .def("__eq__", &PositionLane::operator==, "Equality check")
      .def("__ne__", [](const PositionLane& a, const PositionLane& b) { return !(a == b); })
      .def("is_zero", &PositionLane::operator!, "Check if zero")
      .def("__add__", &PositionLane::operator+)
      .def("__sub__", &PositionLane::operator-)
      .def("__mul__", &PositionLane::operator*, py::is_operator())
      .def("__truediv__", &PositionLane::operator/, py::is_operator())
      .def("__repr__", [](const PositionLane& p) {
        return "PositionLane(road_id=" + std::to_string(p.road_id) + ", lane_id=" + std::to_string(p.lane_id) +
               ", offset=" + std::to_string(p.offset) + ", s=" + std::to_string(p.s) +
               ", orientation=" + "Orientation(h=" + std::to_string(p.orientation.h) +
               ", p=" + std::to_string(p.orientation.p) + ", r=" + std::to_string(p.orientation.r) + "))";
      });

  /*! \~Chinese
   * @brief 添加 Position 类绑定
   *
   */
  py::class_<Position>(m, "Position")
      .def(py::init<>())
      .def(py::init<PositionWorld>(), py::arg("world"))
      .def(py::init<PositionLane>(), py::arg("lane"))
      .def(py::init<PositionWorld, PositionLane>(), py::arg("world"), py::arg("lane"))
      .def(py::init<const Position&>())
      .def_readwrite("world", &Position::world)
      .def_readwrite("lane", &Position::lane)
      .def("__copy__", [](const Position& p) { return Position(p); })
      .def("assign", &Position::operator=, "Assignment operator")
      .def("__eq__", &Position::operator==)
      .def("__ne__", [](const Position& a, const Position& b) { return !(a == b); })
      .def("is_zero", &Position::operator!, "Check if zero")
      .def("__add__", &Position::operator+)
      .def("__sub__", &Position::operator-)
      .def("__mul__", &Position::operator*, py::is_operator())
      .def("__truediv__", &Position::operator/, py::is_operator())
      .def("__repr__", [](const Position& p) {
        return "Position(world=PositionWorld(x=" + std::to_string(p.world.x) + ", y=" + std::to_string(p.world.y) +
               ", z=" + std::to_string(p.world.z) + ", h=" + std::to_string(p.world.h) +
               ", p=" + std::to_string(p.world.p) + ", r=" + std::to_string(p.world.r) +
               "), lane=PositionLane(road_id=" + std::to_string(p.lane.road_id) +
               ", lane_id=" + std::to_string(p.lane.lane_id) + ", offset=" + std::to_string(p.lane.offset) +
               ", s=" + std::to_string(p.lane.s) +
               ", orientation=" + "Orientation(h=" + std::to_string(p.lane.orientation.h) +
               ", p=" + std::to_string(p.lane.orientation.p) + ", r=" + std::to_string(p.lane.orientation.r) + ")))";
      });

  /*! \~Chinese
   * @brief 添加 Waypoint 类绑定
   *
   */
  py::class_<Waypoint, std::shared_ptr<Waypoint>>(m, "Waypoint")
      .def(py::init<>())
      .def(py::init<Position>(), py::arg("position"))
      .def(py::init<std::shared_ptr<Map>, Position>(), py::arg("map"), py::arg("position"))
      .def(py::init<std::shared_ptr<Map>, Position, bool>(), py::arg("map"), py::arg("position"),
           py::arg("is_junction"))
      .def(py::init<const Waypoint&>())
      .def_readwrite("map", &Waypoint::map)
      .def_readwrite("position", &Waypoint::position)
      .def_readwrite("is_junction", &Waypoint::is_junction)
      .def("__copy__", [](const Waypoint& w) { return Waypoint(w); })
      .def("assign", &Waypoint::operator=, "Assignment operator")
      .def("__eq__", &Waypoint::operator==, "Equality check")
      .def("__ne__", [](const Waypoint& a, const Waypoint& b) { return !(a == b); })
      .def("is_zero", &Waypoint::operator!, "Check if zero")
      .def("__add__", &Waypoint::operator+)
      .def("__sub__", &Waypoint::operator-)
      .def("__mul__", &Waypoint::operator*, py::is_operator())
      .def("__truediv__", &Waypoint::operator/, py::is_operator())
      .def("update",
           static_cast<Waypoint& (Waypoint::*)(uint64_t, int64_t, double, double, double, double, double, double,
                                               double, double)>(&Waypoint::Update),
           py::arg("road_id") = 0, py::arg("lane_id") = 0, py::arg("offset") = 0.0, py::arg("s") = 0.0,
           py::arg("h") = 0.0, py::arg("x") = 0.0, py::arg("y") = 0.0, py::arg("z") = 0.0, py::arg("p") = 0.0,
           py::arg("r") = 0.0)
      .def("next", &Waypoint::Next, py::arg("distance") = 0.0, py::arg("direction") = "T");
}

/*! \~Chinese
 * @brief 添加 Map 类绑定.
 *
 * @param m
 */
void bind_Map(py::module& m) {
  py::class_<Map, std::shared_ptr<Map>>(m, "Map")
      .def(py::init<>())
      .def("load", &Map::Load)
      .def("release", &Map::Release)
      .def("get_all_junctions", &Map::GetAllJunctions)
      .def("get_waypoint", &Map::GetWaypoint);
}

/*! \~Chinese
 * @brief 通过 PYBIND11 将 C++ 封装为 Python 接口.
 *
 * @param m
 */
PYBIND11_MODULE(hadmap_py, m) {
  bind_Waypoint(m);
  bind_txLaneLink(m);
  bind_txRoad(m);
  bind_txJunction(m);
  bind_Map(m);
}
