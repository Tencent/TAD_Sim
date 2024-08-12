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

from sim_msg import osi_gps_pb2

from data_process import DataProcess


@dataclass(order=True)
class GpsProcess(DataProcess):

    def __post_init__(self) -> None:
        super().__post_init__()

        self.gps_json = collections.OrderedDict()
        self.gps_json["topic"] = "GPS_SIM"
        self.gps_json["sheet_name"] = "gps"
        self.gps_json["column"] = collections.OrderedDict()
        self.gps_json["column"]["t"] = []
        self.gps_json["column"]["longitude"] = []
        self.gps_json["column"]["latitude"] = []
        self.gps_json["column"]["height"] = []
        self.gps_json["column"]["vel_hrz"] = []
        self.gps_json["column"]["track"] = []
        self.gps_json["column"]["vel_vrt"] = []
        self.gps_json["column"]["latSdtDev"] = []
        self.gps_json["column"]["lonSdtDev"] = []
        self.gps_json["column"]["hgtSdtDev"] = []
        self.gps_json["column"]["SolSt"] = []
        self.gps_json["column"]["PosType"] = []
        self.gps_json["column"]["Undulation"] = []

    def process_data(self, event: Dict) -> None:
        """
        Define a method that handles the data, receiving an event as a parameter.

        Args:
            event (dict): event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

        Returns:
            None
        """

        try:
            # If the event channel is the same as the topic of gps_json
            if event["channel"] == self.gps_json["topic"]:
                # Parsing protobuf messages in events
                msg = osi_gps_pb2.GPS()
                msg.ParseFromString(event["pb_msg_str"])

                self.gps_json["column"]["t"].append(event["timestamp"] / 1000000.0)

                self.gps_json["column"]["longitude"].append(msg.longitude)
                self.gps_json["column"]["latitude"].append(msg.latitude)
                self.gps_json["column"]["height"].append(msg.height)
                self.gps_json["column"]["vel_hrz"].append(msg.vel_hrz)
                self.gps_json["column"]["track"].append(msg.track)
                self.gps_json["column"]["vel_vrt"].append(msg.vel_vrt)
                self.gps_json["column"]["latSdtDev"].append(msg.latSdtDev)
                self.gps_json["column"]["lonSdtDev"].append(msg.lonSdtDev)
                self.gps_json["column"]["hgtSdtDev"].append(msg.hgtSdtDev)
                self.gps_json["column"]["SolSt"].append(msg.SolSt)
                self.gps_json["column"]["PosType"].append(msg.PosType)
                self.gps_json["column"]["Undulation"].append(msg.Undulation)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | chassis data error, " + str(e))

    def get_dict_data(self) -> Dict:
        """
        Get and return dictionary data

        Returns:
            Dict
        """

        return self.gps_json

    def write_data_to_xlsx(self, workbook: Workbook, style: Format) -> None:
        """
        Write data to an Excel file

        Args:
            workbook (Workbook): Representing the Excel
            style (Format): Representing the format to be applied to the written data.

        Returns:
            None
        """

        # Get the sheet object in the workbook, get the sheet name from gps_json
        xlsx_sheet = workbook.add_worksheet(self.gps_json["sheet_name"])
        # Get the number of data rows
        rows = len(self.gps_json["column"]["t"])

        # write column header
        c = 0
        for key_sheet in self.gps_json["column"].keys():
            xlsx_sheet.write(0, c, key_sheet, style)
            c = c + 1

        # write column data
        for r in range(rows):
            c = 0
            for key_sheet in self.gps_json["column"].keys():
                if r < len(self.gps_json["column"][key_sheet]):
                    value = self.gps_json["column"][key_sheet][r]
                    xlsx_sheet.write(r + 1, c, value, style)
                c = c + 1
