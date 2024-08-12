#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import collections
import sys
from pathlib import Path
from dataclasses import dataclass
from typing import Dict

import glog
from xlsxwriter import Workbook, Format

CURRENT_PATH_PY = Path(__file__).resolve().parent
sys.path.append(str(CURRENT_PATH_PY) + "../sim_msg")
sys.path.append(str(CURRENT_PATH_PY))

from sim_msg import location_pb2

from data_process import DataProcess


@dataclass(order=True)
class LocationProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.location_json = collections.OrderedDict()
        self.location_json["topic"] = "LOCATION"
        self.location_json["sheet_name"] = "location"
        self.location_json["column"] = collections.OrderedDict()
        self.location_json["column"]["t"] = []
        self.location_json["column"]["longitude"] = []
        self.location_json["column"]["latitude"] = []
        self.location_json["column"]["altitude"] = []
        self.location_json["column"]["vx"] = []
        self.location_json["column"]["vy"] = []
        self.location_json["column"]["vz"] = []
        self.location_json["column"]["v_sqrt"] = []
        self.location_json["column"]["ax"] = []
        self.location_json["column"]["ay"] = []
        self.location_json["column"]["az"] = []
        self.location_json["column"]["a_sqrt"] = []
        self.location_json["column"]["roll"] = []
        self.location_json["column"]["pitch"] = []
        self.location_json["column"]["yaw"] = []
        self.location_json["column"]["angular_vx"] = []
        self.location_json["column"]["angular_vy"] = []
        self.location_json["column"]["angular_vz"] = []
        self.location_json["column"]["road_id"] = []
        self.location_json["column"]["section_id"] = []
        self.location_json["column"]["lane_id"] = []
        self.location_json["column"]["dist_2_ref_line"] = []
        self.location_json["column"]["vx_body"]: list = []
        self.location_json["column"]["vy_body"]: list = []
        self.location_json["column"]["ax_body"]: list = []
        self.location_json["column"]["ay_body"]: list = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of location_json
            if event["channel"] == self.location_json["topic"]:
                # Parsing protobuf messages in events
                msg = location_pb2.Location()
                msg.ParseFromString(event["pb_msg_str"])

                self.location_json["column"]["t"].append(event["timestamp"] / 1000000.0)

                self.location_json["column"]["longitude"].append(msg.position.x)
                self.location_json["column"]["latitude"].append(msg.position.y)
                self.location_json["column"]["altitude"].append(msg.position.z)

                self.location_json["column"]["vx"].append(msg.velocity.x)
                self.location_json["column"]["vy"].append(msg.velocity.y)
                self.location_json["column"]["vz"].append(msg.velocity.z)

                v = math.sqrt(msg.velocity.x * msg.velocity.x + msg.velocity.y * msg.velocity.y)
                self.location_json["column"]["vx_body"].append(v)
                self.location_json["column"]["vy_body"].append(0.0)

                self.location_json["column"]["v_sqrt"].append(
                    math.sqrt(msg.velocity.x * msg.velocity.x + msg.velocity.y * msg.velocity.y)
                )

                self.location_json["column"]["ax"].append(msg.acceleration.x)
                self.location_json["column"]["ay"].append(msg.acceleration.y)
                self.location_json["column"]["az"].append(msg.acceleration.z)

                a = math.sqrt(msg.acceleration.x * msg.acceleration.x + msg.acceleration.y * msg.acceleration.y)
                self.location_json["column"]["a_sqrt"].append(a)
                self.location_json["column"]["ax_body"].append(a)
                self.location_json["column"]["ay_body"].append(0.0)

                self.location_json["column"]["roll"].append(msg.rpy.x)
                self.location_json["column"]["pitch"].append(msg.rpy.y)
                self.location_json["column"]["yaw"].append(msg.rpy.z)

                self.location_json["column"]["angular_vx"].append(msg.angular.x)
                self.location_json["column"]["angular_vy"].append(msg.angular.y)
                self.location_json["column"]["angular_vz"].append(msg.angular.z)

                self.location_json["column"]["road_id"].append(msg.ego_lane.roadpkid)
                self.location_json["column"]["section_id"].append(msg.ego_lane.sectionpkid)
                self.location_json["column"]["lane_id"].append(msg.ego_lane.lanepkid)
                self.location_json["column"]["dist_2_ref_line"].append(msg.ego_lane.dist_2_ref_line)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | control data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.location_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from location_json
        xlsx_sheet = workbook.add_worksheet(self.location_json["sheet_name"])
        # Get the number of data rows
        rows = len(self.location_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.location_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.location_json["column"].keys():
                if r < len(self.location_json["column"][key_sheet]):
                    value = self.location_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
