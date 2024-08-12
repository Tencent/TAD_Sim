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

from sim_msg import osi_imu_pb2

from data_process import DataProcess


@dataclass(order=True)
class ImuRigidProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.imu_rigid_json = collections.OrderedDict()
        self.imu_rigid_json["topic"] = "IMU_SIM"
        self.imu_rigid_json["sheet_name"] = "imu_rigid"
        self.imu_rigid_json["column"] = collections.OrderedDict()
        self.imu_rigid_json["column"]["t"] = []
        self.imu_rigid_json["column"]["x"] = []
        self.imu_rigid_json["column"]["y"] = []
        self.imu_rigid_json["column"]["z"] = []

        self.imu_rigid_json["column"]["vx"] = []
        self.imu_rigid_json["column"]["vy"] = []
        self.imu_rigid_json["column"]["vz"] = []

        self.imu_rigid_json["column"]["qw"] = []
        self.imu_rigid_json["column"]["qx"] = []
        self.imu_rigid_json["column"]["qy"] = []
        self.imu_rigid_json["column"]["qz"] = []

        self.imu_rigid_json["column"]["longti"] = []
        self.imu_rigid_json["column"]["lat"] = []
        self.imu_rigid_json["column"]["alt"] = []

        self.imu_rigid_json["column"]["roll"] = []
        self.imu_rigid_json["column"]["pitch"] = []
        self.imu_rigid_json["column"]["yaw"] = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of imu_rigid_json
            if event["channel"] == self.imu_rigid_json["topic"]:
                # Parsing protobuf messages in events
                msg = osi_imu_pb2.imu()
                msg.ParseFromString(event["pb_msg_str"])
                msg = msg.imu_rigid

                self.imu_rigid_json["column"]["t"].append(event["timestamp"] / 1000000.0)

                self.imu_rigid_json["column"]["x"].append(msg.posENU.x)
                self.imu_rigid_json["column"]["y"].append(msg.posENU.y)
                self.imu_rigid_json["column"]["z"].append(msg.posENU.z)

                self.imu_rigid_json["column"]["vx"].append(msg.velocity.x)
                self.imu_rigid_json["column"]["vy"].append(msg.velocity.y)
                self.imu_rigid_json["column"]["vz"].append(msg.velocity.z)

                self.imu_rigid_json["column"]["qw"].append(msg.quatRot.qw)
                self.imu_rigid_json["column"]["qx"].append(msg.quatRot.qx)
                self.imu_rigid_json["column"]["qy"].append(msg.quatRot.qy)
                self.imu_rigid_json["column"]["qz"].append(msg.quatRot.qz)

                self.imu_rigid_json["column"]["longti"].append(msg.gnss.longti)
                self.imu_rigid_json["column"]["lat"].append(msg.gnss.lat)
                self.imu_rigid_json["column"]["alt"].append(msg.gnss.alt)

                self.imu_rigid_json["column"]["roll"].append(msg.eulerAngle.roll)
                self.imu_rigid_json["column"]["pitch"].append(msg.eulerAngle.pitch)
                self.imu_rigid_json["column"]["yaw"].append(msg.eulerAngle.yaw)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | chassis data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.imu_rigid_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from imu_rigid_json
        xlsx_sheet = workbook.add_worksheet(self.imu_rigid_json["sheet_name"])
        # Get the number of data rows
        rows = len(self.imu_rigid_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.imu_rigid_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.imu_rigid_json["column"].keys():
                if r < len(self.imu_rigid_json["column"][key_sheet]):
                    value = self.imu_rigid_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
