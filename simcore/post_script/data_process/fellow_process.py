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
class FellowProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.max_fellow = 128
        self.grading_json = collections.OrderedDict()
        self.grading_json["topic"] = "GRADING"
        self.grading_json["sheet_name"] = "dist_2_fellows"
        self.grading_json["column"] = collections.OrderedDict()
        self.grading_json["column"]["t"] = []
        for i in range(self.max_fellow):
            self.grading_json["column"]["dist_2_fellow_" + str(i)] = []
            self.grading_json["column"]["dist_2_fellow_h_" + str(i)] = []
            self.grading_json["column"]["dist_2_fellow_v_" + str(i)] = []
            self.grading_json["column"]["dist_2_fellow_ttc_" + str(i)] = []

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
                msg = grading_pb2.Grading()
                msg.ParseFromString(event["pb_msg_str"])

                self.grading_json["column"]["t"].append(event["timestamp"] / 1000000.0)
                fellow_number = len(msg.expData.distFromCar)

                for i in range(fellow_number):
                    self.grading_json["column"]["dist_2_fellow_" + str(i)].append(msg.expData.distFromCar[i].value)
                    self.grading_json["column"]["dist_2_fellow_h_" + str(i)].append(msg.expData.distFromCar_H[i].value)
                    self.grading_json["column"]["dist_2_fellow_v_" + str(i)].append(msg.expData.distFromCar_V[i].value)
                    self.grading_json["column"]["dist_2_fellow_ttc_" + str(i)].append(msg.expData.thFromCar[i].value)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | chassis data error, " + str(e))

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
        # Get the number of data rows
        rows = len(self.grading_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.grading_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.grading_json["column"].keys():
                if r < len(self.grading_json["column"][key_sheet]):
                    value = self.grading_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
