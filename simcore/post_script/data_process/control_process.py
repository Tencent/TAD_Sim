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

from sim_msg import control_pb2

from data_process import DataProcess


@dataclass(order=True)
class ControlProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.control_json = collections.OrderedDict()
        self.control_json["topic"] = "CONTROL"
        self.control_json["sheet_name"] = "control"
        self.control_json["column"] = collections.OrderedDict()
        self.control_json["column"]["t"] = []
        self.control_json["column"]["targetAcc"] = []
        self.control_json["column"]["targetFrontWheelAngle"] = []
        self.control_json["column"]["controlMode"] = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of control_json
            if event["channel"] == self.control_json["topic"]:
                # Parsing protobuf messages in events
                msg = control_pb2.Control()
                msg.ParseFromString(event["pb_msg_str"])

                self.control_json["column"]["t"].append(event["timestamp"] / 1000000.0)

                self.control_json["column"]["targetAcc"].append(msg.acc_cmd.acc)
                self.control_json["column"]["targetFrontWheelAngle"].append(msg.acc_cmd.front_wheel_angle)
                self.control_json["column"]["controlMode"].append(msg.control_mode)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | control data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.control_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from control_json
        xlsx_sheet = workbook.add_worksheet(self.control_json["sheet_name"])
        # Get the number of data rows
        rows = len(self.control_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.control_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.control_json["column"].keys():
                if r < len(self.control_json["column"][key_sheet]):
                    value = self.control_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
