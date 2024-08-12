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

from sim_msg import vehState_pb2

from data_process import DataProcess


@dataclass(order=True)
class ChassisProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.vehstate_json = collections.OrderedDict()
        self.vehstate_json["topic"] = "VEHICLE_STATE"
        self.vehstate_json["sheet_name"] = "chassis"
        self.vehstate_json["column"] = collections.OrderedDict()
        self.vehstate_json["column"]["t"] = []
        self.vehstate_json["column"]["FrontWheelAngle"] = []
        self.vehstate_json["column"]["SteeringWheelAngle"] = []
        self.vehstate_json["column"]["BrakePedal"] = []
        self.vehstate_json["column"]["AccPedal"] = []
        self.vehstate_json["column"]["FLWheelSpd"] = []
        self.vehstate_json["column"]["FRWheelSpd"] = []
        self.vehstate_json["column"]["RLWheelSpd"] = []
        self.vehstate_json["column"]["RRWheelSpd"] = []
        self.vehstate_json["column"]["YawRate"] = []
        self.vehstate_json["column"]["EngineSpd"] = []
        self.vehstate_json["column"]["gearEngaged"] = []
        self.vehstate_json["column"]["EngineTrq"] = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of vehstate_json
            if event["channel"] == self.vehstate_json["topic"]:
                # Parsing protobuf messages in events
                msg = vehState_pb2.VehicleState()
                msg.ParseFromString(event["pb_msg_str"])

                self.vehstate_json["column"]["t"].append(event["timestamp"] / 1000000.0)

                self.vehstate_json["column"]["FrontWheelAngle"].append(msg.chassis_state.SteeringWheelAngle)
                self.vehstate_json["column"]["SteeringWheelAngle"].append(msg.chassis_state.SteeringWheelAngleSign)
                self.vehstate_json["column"]["BrakePedal"].append(msg.chassis_state.BrakePedalPos)
                self.vehstate_json["column"]["AccPedal"].append(msg.powertrain_state.accpedal_position)
                self.vehstate_json["column"]["FLWheelSpd"].append(msg.chassis_state.wheel_speed.frontLeft)
                self.vehstate_json["column"]["FRWheelSpd"].append(msg.chassis_state.wheel_speed.frontRight)
                self.vehstate_json["column"]["RLWheelSpd"].append(msg.chassis_state.wheel_speed.rearLeft)
                self.vehstate_json["column"]["RRWheelSpd"].append(msg.chassis_state.wheel_speed.rearRight)
                self.vehstate_json["column"]["YawRate"].append(msg.chassis_state.VehDynYawRate)
                self.vehstate_json["column"]["EngineSpd"].append(msg.powertrain_state.engine_speed)
                self.vehstate_json["column"]["gearEngaged"].append(msg.powertrain_state.gear_engaged)
                self.vehstate_json["column"]["EngineTrq"].append(msg.chassis_state.ESP_MasterCylindBrakePress)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | chassis data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.vehstate_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from vehstate_json
        xlsx_sheet = workbook.add_worksheet(self.vehstate_json["sheet_name"])
        # Get the number of data rows
        rows = len(self.vehstate_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.vehstate_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.vehstate_json["column"].keys():
                if r < len(self.vehstate_json["column"][key_sheet]):
                    value = self.vehstate_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
