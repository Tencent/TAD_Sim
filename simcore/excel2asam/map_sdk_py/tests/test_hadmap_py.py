#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys
from pathlib import Path

import pytest

current_path_py = Path(__file__).resolve().parent
sys.path.append(f"{current_path_py.parent}/build/bin/Release")
import hadmap_py


PATHDIR_MAPS = current_path_py / "maps"

MAP_DATAS = [
    {
        "filename_map": "crossroad_signals.xodr",
        "routes": [
            # 道路上的坐标
            {
                "road_id": 1,
                "lane_id": -2,
                "offset": 0.0,
                "s": 1500,
                "x": 0.0134747,
                "y": -0.0000472,
                "z": 0.0000006,
            },
            {
                "road_id": 1,
                "lane_id": -2,
                "offset": 0.0,
                "s": 1800,
                "x": 0.01616968,
                "y": -0.00004716,
                "z": 0.0,
            },
        ],
    },
    {
        "filename_map": "crossroad_signals.xodr",
        "routes": [
            # junction 上的坐标
            # {
            #     "road_id": 1,
            #     "lane_id": 11,
            #     "offset": 0.151,
            #     "s": 11.548,
            #     "x": 0.01807005,
            #     "y": -0.00001436,
            #     "z": 0.0,
            # },
            # lane 上的坐标
            {
                "road_id": 4,
                "lane_id": -1,
                "offset": -0.282,
                "s": 65.531,
                "x": 0.01891432,
                "y": -0.00001827,
                "z": 0.0,
            },
        ],
    },
]


class TestOrientation:
    def test_init(self):
        orientation = hadmap_py.Orientation(1.0, 2.0, 3.0)
        assert orientation.h == 1.0
        assert orientation.p == 2.0
        assert orientation.r == 3.0


class TestPositionWorld:
    def test_init(self):
        pos_world = hadmap_py.PositionWorld(1.0, 2.0, 3.0)
        assert pos_world.x == 1.0
        assert pos_world.y == 2.0
        assert pos_world.z == 3.0


class TestPositionLane:
    def test_init(self):
        pos_lane = hadmap_py.PositionLane(1, 2, 3.0, 4.0)
        assert pos_lane.road_id == 1
        assert pos_lane.lane_id == 2
        assert pos_lane.offset == 3.0
        assert pos_lane.s == 4.0


class TestPosition:
    def test_init(self):
        pos_world = hadmap_py.PositionWorld(1.0, 2.0, 3.0)
        pos_lane = hadmap_py.PositionLane(1, 2, 3.0, 4.0)
        position = hadmap_py.Position(pos_world, pos_lane)
        assert position.world == pos_world
        assert position.lane == pos_lane


class TestWaypoint:
    def test_init(self):
        pos_world = hadmap_py.PositionWorld(1.0, 2.0, 3.0)
        pos_lane = hadmap_py.PositionLane(1, 2, 3.0, 4.0)
        position = hadmap_py.Position(pos_world, pos_lane)
        mapper = hadmap_py.Map()
        wpt = hadmap_py.Waypoint(mapper, position)

        assert wpt.map == mapper
        assert wpt.position.world == pos_world
        assert wpt.position.lane == pos_lane
        assert wpt.position == position
        assert wpt.is_junction is False

    def test_init_with_junction(self):
        pos_world = hadmap_py.PositionWorld(1.0, 2.0, 3.0)
        pos_lane = hadmap_py.PositionLane(1, 2, 3.0, 4.0)
        position = hadmap_py.Position(pos_world, pos_lane)
        mapper = hadmap_py.Map()
        wpt = hadmap_py.Waypoint(mapper, position, True)

        assert wpt.map == mapper
        assert wpt.position.world == pos_world
        assert wpt.position.lane == pos_lane
        assert wpt.position == position
        assert wpt.is_junction is True

    @pytest.mark.parametrize("mapdata", MAP_DATAS)
    def test_convert_position(self, mapdata):
        r0 = mapdata["routes"][0]
        #
        pos_lane = hadmap_py.PositionLane(r0["road_id"], r0["lane_id"], r0["offset"], r0["s"])
        pos_world = hadmap_py.PositionWorld(r0["x"], r0["y"], r0["z"])

        #
        mapper = hadmap_py.Map()
        mapper.load(str(PATHDIR_MAPS / mapdata["filename_map"]))

        wptlane = hadmap_py.Waypoint(mapper, hadmap_py.Position(pos_lane), True)
        assert wptlane.map == mapper
        assert wptlane.position.lane == pos_lane
        assert wptlane.position.world == pos_world
        assert wptlane.position == hadmap_py.Position(pos_world, pos_lane)
        assert wptlane.is_junction is True

        wptworld = hadmap_py.Waypoint(mapper, hadmap_py.Position(pos_world), False)
        assert wptworld.map == mapper
        assert wptworld.position.world == pos_world
        assert wptworld.position.lane == pos_lane
        assert wptworld.position == hadmap_py.Position(pos_world, pos_lane)
        assert wptworld.is_junction is False

    @pytest.mark.parametrize("mapdata", MAP_DATAS)
    def test_next(self, mapdata):
        routes = mapdata["routes"]

        if len(routes) <= 1:
            return

        mapper = hadmap_py.Map()
        mapper.load(str(PATHDIR_MAPS / mapdata["filename_map"]))

        r0 = routes[0]
        wpt0 = mapper.get_waypoint(r0["road_id"], r0["lane_id"], r0["offset"], r0["s"])

        r1 = routes[1]
        wpt1 = mapper.get_waypoint(r1["road_id"], r1["lane_id"], r1["offset"], r1["s"])

        distance = r1["s"] - r0["s"]
        assert wpt0.next(distance, "T") == wpt1


class TestTxRoad:
    def test_init(self):
        road = hadmap_py.txRoad(1, "road_name")
        assert road.get_id() == 1


# class TestTxLaneLink:
#     def test_init(self):
#         lane_link = hadmap_py.txLaneLink()
#         assert lane_link.get_id() == 0


class TestTxJunction:
    def test_init(self):
        junction = hadmap_py.txJunction(1)
        assert junction.get_id() == 1


class TestMap:
    @pytest.mark.parametrize(
        "filename_map",
        ["crossroad_signals.xodr"],
    )
    def test_load_and_release(self, filename_map):
        mapper = hadmap_py.Map()
        mapper.load(str(PATHDIR_MAPS / filename_map))
        mapper.release()


# 更多测试用例...

if __name__ == "__main__":
    pytest.main()
    # TestOrientation().test_init()
