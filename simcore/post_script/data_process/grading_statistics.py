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
class GradingStatistics(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        # Initialize an ordered dictionary for storing scoring statistics
        self.grading_json = collections.OrderedDict()
        # Setting up topics for scoring statistics
        self.grading_json["topic"] = "GRADING_STATISTICS"
        # Set the worksheet name for scoring statistics
        self.grading_json["sheet_name"] = "grading_statistics"
        # Initialize an ordered dictionary to store the rating statistics for each row
        self.grading_json["row"] = collections.OrderedDict()

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of grading_json
            if event["channel"] == self.grading_json["topic"]:
                # Parsing protobuf messages in events
                msg = grading_pb2.Grading.Statistics()
                msg.ParseFromString(event["pb_msg_str"])

                self.grading_json["row"]["scene_name"] = msg.scene_name
                self.grading_json["row"]["report_date"] = msg.report_date
                self.grading_json["row"]["is_pass"] = msg.is_pass

                # Determine if the message has passed
                if not msg.is_pass:
                    # If the message fails, log the reason in the row field of grading_json
                    self.grading_json["row"]["reason"] = msg.reason
                else:
                    # If the message passes, clear the reason in the row field of grading_json
                    self.grading_json["row"]["reason"] = ""

                self.grading_json["row"]["total_time"] = msg.total_time
                self.grading_json["row"]["total_milleage"] = msg.total_milleage
                self.grading_json["row"]["is_timeout"] = msg.is_timeout
                self.grading_json["row"]["grading_xml"] = msg.grading_xml

                self.grading_json["row"]["detail"] = collections.OrderedDict()

                self.grading_json["row"]["detail"]["碰撞"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.collision.eval_value,
                        "data_key": "collision",
                        "is_enabled": msg.detail.collision.is_enabled,
                        "is_pass": msg.detail.collision.is_pass,
                    }
                )

                self.grading_json["row"]["detail"]["超速次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_overSpeed.eval_value,
                        "data_key": "speed",
                        "is_enabled": msg.detail.count_overSpeed.is_enabled,
                        "is_pass": msg.detail.count_overSpeed.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["加速度过大次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_overAcceleration.eval_value,
                        "data_key": "acc",
                        "is_enabled": msg.detail.count_overAcceleration.is_enabled,
                        "is_pass": msg.detail.count_overAcceleration.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["减速度过大次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_overDecelleration.eval_value,
                        "data_key": "dec",
                        "is_enabled": msg.detail.count_overDecelleration.is_enabled,
                        "is_pass": msg.detail.count_overDecelleration.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["横向加速度过大次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_overTurningAcceleration.eval_value,
                        "data_key": "lateralAcc",
                        "is_enabled": msg.detail.count_overTurningAcceleration.is_enabled,
                        "is_pass": msg.detail.count_overTurningAcceleration.is_pass,
                    }
                )

                self.grading_json["row"]["detail"]["跟车时距过小次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_tooCloseTTC.eval_value,
                        "data_key": "ttc",
                        "is_enabled": msg.detail.count_tooCloseTTC.is_enabled,
                        "is_pass": msg.detail.count_tooCloseTTC.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["跟车距离过小次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_tooCloseDistance.eval_value,
                        "data_key": "dist2Fellow",
                        "is_enabled": msg.detail.count_tooCloseDistance.is_enabled,
                        "is_pass": msg.detail.count_tooCloseDistance.is_pass,
                    }
                )

                self.grading_json["row"]["detail"]["横向偏移距离过大次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_overMaxPosError_H.eval_value,
                        "data_key": "maxPosError_H",
                        "is_enabled": msg.detail.count_overMaxPosError_H.is_enabled,
                        "is_pass": msg.detail.count_overMaxPosError_H.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["平均横向偏移距离"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.average_PosError_H.eval_value,
                        "data_key": None,
                        "is_enabled": msg.detail.average_PosError_H.is_enabled,
                        "is_pass": msg.detail.average_PosError_H.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["左侧最大横向偏移距离"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.max_PosError_H_L.eval_value,
                        "data_key": None,
                        "is_enabled": msg.detail.max_PosError_H_L.is_enabled,
                        "is_pass": msg.detail.max_PosError_H_L.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["右侧最大横向偏移距离"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.max_PosError_H_R.eval_value,
                        "data_key": None,
                        "is_enabled": msg.detail.max_PosError_H_R.is_enabled,
                        "is_pass": msg.detail.max_PosError_H_R.is_pass,
                    }
                )

                self.grading_json["row"]["detail"]["压实线次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_CrossSolidLine.eval_value,
                        "data_key": "isCrossSolidLine",
                        "is_enabled": msg.detail.count_CrossSolidLine.is_enabled,
                        "is_pass": msg.detail.count_CrossSolidLine.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["闯红灯次数"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.count_RanRedLight.eval_value,
                        "data_key": "RunTrafficLight",
                        "is_enabled": msg.detail.count_RanRedLight.is_enabled,
                        "is_pass": msg.detail.count_RanRedLight.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["是否到达终点"] = collections.OrderedDict(
                    {
                        "eval_value": msg.detail.reachEndPoint.eval_value,
                        "data_key": "is_ReachEndPoint",
                        "is_enabled": msg.detail.reachEndPoint.is_enabled,
                        "is_pass": msg.detail.reachEndPoint.is_pass,
                    }
                )

                self.grading_json["row"]["detail"]["客户定制是否到达终点"] = collections.OrderedDict(
                    {
                        "eval_value": msg.custom_detail.custom_ReachEndPoint.eval_value,
                        "data_key": "distance_to_stop",
                        "is_enabled": msg.custom_detail.custom_ReachEndPoint.is_enabled,
                        "is_pass": msg.custom_detail.custom_ReachEndPoint.is_pass,
                    }
                )
                self.grading_json["row"]["detail"]["客户定制横向偏移过大"] = collections.OrderedDict(
                    {
                        "eval_value": msg.custom_detail.custom_maxPosError_Exceed.eval_value,
                        "data_key": "road_follow_error",
                        "is_enabled": msg.custom_detail.custom_maxPosError_Exceed.is_enabled,
                        "is_pass": msg.custom_detail.custom_maxPosError_Exceed.is_pass,
                    }
                )
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | grading statistics data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.grading_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from grading_json
        xlsx_sheet = workbook.add_worksheet(self.grading_json["sheet_name"])

        # Setting the format
        style1 = workbook.add_format({"border": 1, "align": "left", "valign": "vcenter"})

        # row data
        r = 0
        # Iterate over all keys of the "row" field in grading_json
        for key in self.grading_json["row"].keys():
            # Write the key to the current row, first column of the Excel file, using the style1 style
            xlsx_sheet.write(r, 0, key, style1)
            if key == "detail":
                # Line number plus one
                r = r + 1
                # Iterate over each key and mention the corresponding information and format.
                for each_detail_key in self.grading_json["row"][key].keys():
                    xlsx_sheet.write(r, 1, each_detail_key, style1)
                    xlsx_sheet.write(r, 2, "eval_value", style1)
                    xlsx_sheet.write(r, 3, self.grading_json["row"][key][each_detail_key]["eval_value"], style1)
                    xlsx_sheet.write(r, 4, "data_key", style1)
                    xlsx_sheet.write(r, 5, self.grading_json["row"][key][each_detail_key]["data_key"], style1)
                    xlsx_sheet.write(r, 6, "is_enabled", style1)
                    xlsx_sheet.write(r, 7, self.grading_json["row"][key][each_detail_key]["is_enabled"], style1)
                    xlsx_sheet.write(r, 8, "is_pass", style1)
                    xlsx_sheet.write(r, 9, self.grading_json["row"][key][each_detail_key]["is_pass"], style1)
                    r = r + 1
            else:
                # If the current key is not "detail", write the corresponding value to the current row,
                # second column of the Excel file, using the style
                xlsx_sheet.write(r, 1, self.grading_json["row"][key], style)
                r = r + 1
