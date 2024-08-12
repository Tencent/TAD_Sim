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
class ImuProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.imu_json = collections.OrderedDict()
        self.imu_json["topic"] = "IMU_SIM"
        self.imu_json["sheet_name"] = "imu"
        self.imu_json["column"] = collections.OrderedDict()
        self.imu_json["column"]["t"] = []
        self.imu_json["column"]["ax"] = []
        self.imu_json["column"]["ay"] = []
        self.imu_json["column"]["az"] = []
        self.imu_json["column"]["av_x"] = []
        self.imu_json["column"]["av_y"] = []
        self.imu_json["column"]["av_z"] = []
        self.imu_json["column"]["wheel_spd_l"] = []
        self.imu_json["column"]["wheel_spd_r"] = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of imu_json
            if event["channel"] == self.imu_json["topic"]:
                # Parsing protobuf messages in events
                msg = osi_imu_pb2.imu()
                msg.ParseFromString(event["pb_msg_str"])
                msg = msg.imu_data

                self.imu_json["column"]["t"].append(event["timestamp"] / 1000000.0)

                self.imu_json["column"]["ax"].append(msg.accel.x)
                self.imu_json["column"]["ay"].append(msg.accel.y)
                self.imu_json["column"]["az"].append(msg.accel.z)

                self.imu_json["column"]["av_x"].append(msg.angular_v.x)
                self.imu_json["column"]["av_y"].append(msg.angular_v.y)
                self.imu_json["column"]["av_z"].append(msg.angular_v.z)

                self.imu_json["column"]["wheel_spd_l"].append(msg.wheelspd.wheelSpdRL)
                self.imu_json["column"]["wheel_spd_r"].append(msg.wheelspd.wheelSpdRR)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | chassis data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.imu_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from imu_json
        xlsx_sheet = workbook.add_worksheet(self.imu_json["sheet_name"])
        # Get the number of data rows
        rows = len(self.imu_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.imu_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.imu_json["column"].keys():
                if r < len(self.imu_json["column"][key_sheet]):
                    value = self.imu_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
