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

from sim_msg import grading_pb2

from data_process import DataProcess


@dataclass(order=True)
class GradingProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.max_fellow = 128
        self.grading_data = collections.OrderedDict()
        self.grading_data["topic"] = "GRADING"
        self.grading_data["sheet_name"] = "grading"
        self.grading_data["column"] = collections.OrderedDict()
        self.grading_data["column"]["t"] = []

        self.grading_data["column"]["speed_threshold"] = []
        self.grading_data["column"]["speed_value"] = []
        self.grading_data["column"]["is_overspeed"] = []

        self.grading_data["column"]["acc_threshold"] = []
        self.grading_data["column"]["acc_value"] = []
        self.grading_data["column"]["is_overAcc"] = []

        self.grading_data["column"]["dec_threshold"] = []
        self.grading_data["column"]["dec_value"] = []
        self.grading_data["column"]["is_overDec"] = []

        self.grading_data["column"]["lateralAcc_threshold"] = []
        self.grading_data["column"]["lateralAcc_value"] = []
        self.grading_data["column"]["is_overlateralAcc"] = []

        self.grading_data["column"]["collision"] = []
        self.grading_data["column"]["location"] = []

        self.grading_data["column"]["ttc_threshold"] = []
        self.grading_data["column"]["ttc_value"] = []
        self.grading_data["column"]["is_ttcOverLow"] = []

        self.grading_data["column"]["dist2Fellow_threshold"] = []
        self.grading_data["column"]["dist2Fellow_value"] = []
        self.grading_data["column"]["is_dist2FellowOverLow"] = []

        self.grading_data["column"]["isCrossSolidLine"] = []
        self.grading_data["column"]["isOnDottedLine"] = []
        self.grading_data["column"]["isOnStopLine"] = []

        self.grading_data["column"]["RunTrafficLight_thresh"] = []
        self.grading_data["column"]["RunTrafficLight_value"] = []
        self.grading_data["column"]["is_RunTrafficLight"] = []

        self.grading_data["column"]["maxPosError_H_threshold"] = []
        self.grading_data["column"]["maxPosError_H_value"] = []
        self.grading_data["column"]["is_overMaxPosError_H"] = []

        self.grading_data["column"]["is_ReachEndPoint"] = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of grading_data, and len > 0
            if event["channel"] == self.grading_data["topic"] and len(event["pb_msg_str"]) > 0:
                # Parsing protobuf messages in events
                msg = grading_pb2.Grading()
                msg.ParseFromString(event["pb_msg_str"])

                # add info to self.grading_data
                self.grading_data["column"]["t"].append(event["timestamp"] / 1000000.0)

                self.grading_data["column"]["speed_threshold"].append(msg.speed.threshold)
                self.grading_data["column"]["speed_value"].append(msg.speed.speed)
                self.grading_data["column"]["is_overspeed"].append(msg.speed.state)

                self.grading_data["column"]["acc_threshold"].append(msg.acceleration.threshold)
                self.grading_data["column"]["acc_value"].append(msg.acceleration.acceleration)
                self.grading_data["column"]["is_overAcc"].append(msg.acceleration.state)

                self.grading_data["column"]["dec_threshold"].append(msg.dec_acc.threshold)
                self.grading_data["column"]["dec_value"].append(msg.dec_acc.acceleration)
                self.grading_data["column"]["is_overDec"].append(msg.dec_acc.state)

                self.grading_data["column"]["lateralAcc_threshold"].append(msg.lateral_acc.threshold)
                self.grading_data["column"]["lateralAcc_value"].append(msg.lateral_acc.acceleration)
                self.grading_data["column"]["is_overlateralAcc"].append(msg.lateral_acc.state)

                # If there is collision information in the message
                if len(msg.collision) > 0:
                    # Add 1 to the value of the "collision" key under the "column" key in the grading_data dictionary.
                    self.grading_data["column"]["collision"].append(1)

                    collision_location_str = ""
                    times_collision = len(msg.collision)
                    counter_collision = 0
                    for each_collision in msg.collision:
                        counter_collision = counter_collision + 1
                        loc_str = (
                            str(each_collision.location.x)
                            + ", "
                            + str(each_collision.location.y)
                            + ", "
                            + str(each_collision.location.z)
                        )
                        collision_location_str = collision_location_str + loc_str
                        if counter_collision < times_collision:
                            collision_location_str = collision_location_str + "\n"
                    self.grading_data["column"]["location"].append(collision_location_str)
                else:
                    # If there is no collision information, add 0 to the value of the "collision" key
                    # under the "column" key in the grading_data dictionary.
                    self.grading_data["column"]["collision"].append(0)
                    self.grading_data["column"]["location"].append(None)

                self.grading_data["column"]["ttc_threshold"].append(msg.timeHeadway.threshold)
                # Use DataProcess.is_valid_value method to check if msg.timeHeadway.ATH is a valid value
                if DataProcess.is_valid_value(msg.timeHeadway.ATH):
                    # If valid, append the value of msg.timeHeadway.ATH to the "ttc_value" list
                    self.grading_data["column"]["ttc_value"].append(msg.timeHeadway.ATH)
                else:
                    # If invalid, append None to the "ttc_value" list.
                    self.grading_data["column"]["ttc_value"].append(None)
                self.grading_data["column"]["is_ttcOverLow"].append(msg.timeHeadway.state)

                self.grading_data["column"]["dist2Fellow_threshold"].append(msg.distHeadway.threshold)
                # Check if msg.distHeadway.dist_to_fellow is a valid value
                if DataProcess.is_valid_value(msg.distHeadway.dist_to_fellow):
                    # If it is a valid value, add it to the "dist2Fellow_value" list in the grading_data dictionary.
                    self.grading_data["column"]["dist2Fellow_value"].append(msg.distHeadway.dist_to_fellow)
                else:
                    # If not a valid value, add None to the list of "dist2Fellow_value".
                    self.grading_data["column"]["dist2Fellow_value"].append(None)
                self.grading_data["column"]["is_dist2FellowOverLow"].append(msg.distHeadway.dist_state)

                self.grading_data["column"]["isCrossSolidLine"].append(msg.lane.isOnSolidLine)
                self.grading_data["column"]["isOnDottedLine"].append(msg.lane.isOnDottedLine)
                self.grading_data["column"]["isOnStopLine"].append(msg.lane.isOnStopLine)

                self.grading_data["column"]["RunTrafficLight_thresh"].append(msg.trafficeLight.run_trafficlight_thresh)
                self.grading_data["column"]["RunTrafficLight_value"].append(msg.trafficeLight.run_trafficlight_count)
                self.grading_data["column"]["is_RunTrafficLight"].append(msg.trafficeLight.state)

                self.grading_data["column"]["maxPosError_H_threshold"].append(msg.control.threshold)
                self.grading_data["column"]["maxPosError_H_value"].append(msg.control.posError_H)
                self.grading_data["column"]["is_overMaxPosError_H"].append(msg.control.state)

                self.grading_data["column"]["is_ReachEndPoint"].append(msg.planning.is_reachEndpoint)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | grading data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.grading_data

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from grading_data
        xlsx_sheet = workbook.add_worksheet(self.grading_data["sheet_name"])
        # Get the number of data rows
        rows = len(self.grading_data["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.grading_data["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.grading_data["column"].keys():
                if r < len(self.grading_data["column"][key_sheet]):
                    value = self.grading_data["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
