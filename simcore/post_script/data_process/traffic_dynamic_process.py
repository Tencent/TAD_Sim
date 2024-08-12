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

from sim_msg import traffic_pb2

from data_process import DataProcess


@dataclass(order=True)
class TrafficDynamicProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.traffic_json = collections.OrderedDict()
        self.traffic_json["topic"] = "TRAFFIC"
        self.traffic_json["sheet_name"] = "traffic_dynamic"
        self.traffic_json["column"] = collections.OrderedDict()
        self.traffic_json["column"]["t"] = []
        self.traffic_json["column"]["id"] = []
        self.traffic_json["column"]["x"] = []
        self.traffic_json["column"]["y"] = []
        self.traffic_json["column"]["heading"] = []
        self.traffic_json["column"]["v"] = []
        self.traffic_json["column"]["vl"] = []
        self.traffic_json["column"]["acc"] = []
        self.traffic_json["column"]["length"] = []
        self.traffic_json["column"]["width"] = []

        self.traffic_json["column"]["show_abs_velocity"] = []
        self.traffic_json["column"]["show_abs_acc"] = []
        self.traffic_json["column"]["show_relative_velocity"] = []
        self.traffic_json["column"]["show_relative_acc"] = []
        self.traffic_json["column"]["show_relative_velocity_horizontal"] = []
        self.traffic_json["column"]["show_relative_acc_horizontal"] = []
        self.traffic_json["column"]["show_relative_dist_vertical"] = []
        self.traffic_json["column"]["show_relative_dist_horizontal"] = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of traffic_json
            if event["channel"] == self.traffic_json["topic"]:
                # Parsing protobuf messages in events
                msg = traffic_pb2.Traffic()
                msg.ParseFromString(event["pb_msg_str"])

                for fellow in msg.dynamicObstacles:
                    t = fellow.t * 0.001

                    self.traffic_json["column"]["t"].append(t)
                    self.traffic_json["column"]["id"].append(fellow.id)

                    self.traffic_json["column"]["x"].append(fellow.x)
                    self.traffic_json["column"]["y"].append(fellow.y)

                    self.traffic_json["column"]["heading"].append(fellow.heading)
                    self.traffic_json["column"]["v"].append(fellow.v)
                    self.traffic_json["column"]["vl"].append(fellow.vl)
                    self.traffic_json["column"]["acc"].append(fellow.acc)

                    self.traffic_json["column"]["length"].append(fellow.length)
                    self.traffic_json["column"]["width"].append(fellow.width)

                    self.traffic_json["column"]["show_abs_velocity"].append(fellow.show_abs_velocity)
                    self.traffic_json["column"]["show_abs_acc"].append(fellow.show_abs_acc)
                    self.traffic_json["column"]["show_relative_velocity"].append(fellow.show_relative_velocity)
                    self.traffic_json["column"]["show_relative_acc"].append(fellow.show_relative_acc)
                    self.traffic_json["column"]["show_relative_velocity_horizontal"].append(
                        fellow.show_relative_velocity_horizontal
                    )
                    self.traffic_json["column"]["show_relative_acc_horizontal"].append(
                        fellow.show_relative_acc_horizontal
                    )
                    self.traffic_json["column"]["show_relative_dist_vertical"].append(
                        fellow.show_relative_dist_vertical
                    )
                    self.traffic_json["column"]["show_relative_dist_horizontal"].append(
                        fellow.show_relative_dist_horizontal
                    )
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | traffic data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.traffic_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from traffic_json
        xlsx_sheet = workbook.add_worksheet(self.traffic_json["sheet_name"])
        # Get the number of data rows
        rows = len(self.traffic_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.traffic_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.traffic_json["column"].keys():
                if r < len(self.traffic_json["column"][key_sheet]):
                    value = self.traffic_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
