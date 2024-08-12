#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import collections
import fnmatch
import gzip
import os
import sys
import traceback
from pathlib import Path
from dataclasses import dataclass

import glog
import xlsxwriter

import utils

CURRENT_PATH_PY = Path(__file__).resolve().parent
sys.path.append(str(CURRENT_PATH_PY) + "/sim_msg")
sys.path.append(str(CURRENT_PATH_PY))

from data_process.chassis_process import ChassisProcess
from data_process.control_process import ControlProcess
from data_process.gps_process import GpsProcess
from data_process.grading_process import GradingProcess
from data_process.grading_statistics import GradingStatistics
from data_process.imu_process import ImuProcess
from data_process.imu_rigid_process import ImuRigidProcess
from data_process.location_process import LocationProcess
from data_process.traffic_car_process import TrafficCarProcess
from data_process.traffic_dynamic_process import TrafficDynamicProcess


@dataclass(order=True)
class PostProcess:
    def __post_init__(self) -> None:
        self._args_parser = argparse.ArgumentParser()
        self._args = None
        self.set_arguments()

        self.pb_log_file = ""
        self.pb_log_file_path = ""
        self.pb_log_file_name = ""
        self.output_path = ""
        self.xlsx_name = ""
        self.json_file = ""

        self.pb_process = utils.PblogReader()
        self.utils = utils.Utils()
        self.json_process = utils.HandleJson()

        # tasks
        self.tasks = collections.OrderedDict()
        self.tasks["location"] = LocationProcess()

    def set_arguments(self) -> None:
        # -g to enable pnc
        self._args_parser.add_argument("--grading", "-g", action="store_true", help="enable grading", default=False)

        # -t to enable pnc
        self._args_parser.add_argument("--traffic", "-t", action="store_true", help="enable traffic", default=False)

        # -p to enable pnc
        self._args_parser.add_argument("--pnc", "-p", action="store_true", help="enable pnc", default=False)

        # -i to enable imu/gps
        self._args_parser.add_argument("--imu_gps", "-i", action="store_true", help="enable imu/gps", default=False)

        # -f to set pblog
        self._args_parser.add_argument(
            "--file", "-f", dest="pblog_file", help="pblog file", required=True, metavar="FILE"
        )

        # -f to set pblog
        self._args_parser.add_argument(
            "--max_pblog_number",
            "-n",
            dest="max_pblog_number",
            help="max pblog files to keep",
            required=False,
            type=int,
            default=10,
        )

        self._args = self._args_parser.parse_args()

    def parse_arguments(self) -> None:
        glog.info("pb | args %s.", str(self._args.pblog_file))
        glog.info("pb | args %s.", str(self._args))

        if os.path.exists(self._args.pblog_file):
            self.pb_log_file = self._args.pblog_file
            self.pb_log_file_path = os.path.dirname(os.path.abspath(self.pb_log_file)) + "/"
            self.pb_log_file_name = os.path.basename(self.pb_log_file)

            # new path for xlsx file
            self.output_path = os.path.join(self.pb_log_file_path, "../" + self.utils.get_date_string_ymd() + "/")

            try:
                if not os.path.exists(self.output_path):
                    os.mkdir(self.output_path)
            except Exception as e:  # pylint: disable=broad-except
                glog.error("error:{}".format(e))
                traceback.print_exc()

            self.xlsx_name = self.output_path + self.pb_log_file_name + ".xlsx"

            glog.info("pb | output data dir is " + self.output_path)
            glog.info("pb | output xlsx file is " + self.xlsx_name)

            if self._args.grading:
                self.tasks["grading_statistics"] = GradingStatistics()
                self.tasks["grading"] = GradingProcess()

            if self._args.traffic:
                self.tasks["traffic_car"] = TrafficCarProcess()
                self.tasks["traffic_dynamic"] = TrafficDynamicProcess()

            if self._args.pnc:
                self.tasks["control"] = ControlProcess()
                self.tasks["chassis"] = ChassisProcess()
                glog.info("pb | enable pnc related protobuf data parser.")

            if self._args.imu_gps:
                self.tasks["imu"] = ImuProcess()
                self.tasks["imu_rigid"] = ImuRigidProcess()
                self.tasks["gps"] = GpsProcess()
                glog.info("pb | enable imu/gps protobuf data parser.")

            return True
        else:
            glog.error("usage: python PostProcess.py -f <pblogfile>")
            return False

    def save_xlsx(self) -> None:
        """
        # save data into xlsx
        """
        workbook = xlsxwriter.Workbook(self.xlsx_name)
        glog.info("pb | saving xlsx " + self.xlsx_name)

        style = workbook.add_format({"border": 1, "align": "center", "valign": "vcenter"})

        for key in self.tasks.keys():
            self.tasks[key].write_data_to_xlsx(workbook, style)

        workbook.close()

    def collect_dict(self) -> None:
        for key in self.tasks.keys():
            self.tasks[key] = self.tasks[key].get_dict_data()

    def delete_pblog(self, file_regular_expression: str = "*.pblog", max_reserve_number: int = 8):
        glog.info("max_reserve_number is {}.".format(max_reserve_number))
        if max_reserve_number < 0:
            glog.info("less than 0, no deletion.".format())
            return

        # keep lastest max_reserve_number pblog file
        pblog_dir = os.path.dirname(self.pb_log_file)
        pblog_files = []
        for file in os.listdir(pblog_dir):
            if fnmatch.fnmatch(file, file_regular_expression):
                file_full_path = os.path.join(pblog_dir, file)
                pblog_files.append(file_full_path)
        pblog_number = len(pblog_files)
        if pblog_number <= max_reserve_number:
            glog.info(
                "{} file number in {} is <= {}, no deletion.".format(
                    file_regular_expression, pblog_dir, max_reserve_number
                )
            )
            return

        # sort by create time
        pblog_files = sorted(pblog_files, key=lambda t: os.path.getctime(t))

        # do deletion
        for index in range(pblog_number - max_reserve_number):
            glog.info("delete file:{}".format(pblog_files[index]))
            os.remove(pblog_files[index])

    def compress_pblog(self, file_regular_expression: str = "*.pblog"):
        pblog_dir = os.path.dirname(self.pb_log_file)
        pblog_files = []
        for file in os.listdir(pblog_dir):
            if fnmatch.fnmatch(file, file_regular_expression):
                file_full_path = os.path.join(pblog_dir, file)
                pblog_files.append(file_full_path)

        # sort by create time
        pblog_files = sorted(pblog_files, key=lambda t: os.path.getctime(t))

        for pblog_file in pblog_files:
            with open(pblog_file, "rb") as f_in:
                gzip_file = pblog_file + ".gz"
                if os.path.exists(gzip_file):
                    glog.info("{} exists, no compression.".format(gzip_file))
                    continue
                with gzip.open(gzip_file, "wb") as zipped_file:
                    zipped_file.writelines(f_in)
                glog.info("{} compressed.".format(gzip_file))
                f_in.close()

    def process_pblog(self) -> None:
        # full path of pblog file
        self.pb_log_file = os.path.abspath(self.pb_log_file)
        glog.info("full path of pblog file {}".format(self.pb_log_file))

        # check if pblog file exists
        if not os.path.exists(self.pb_log_file):
            glog.error("{} does not exists.".format(self.pb_log_file))

        # compress .pblog files
        try:
            self.compress_pblog("*.pblog")
        except Exception as e:  # pylint: disable=broad-except
            glog.error("error in compression {}".format(str(e)))

        # process
        if self.pb_process.open_pblog(self.pb_log_file):
            try:
                event = {"channel": "", "pb_msg_str": "", "timestamp": 0}

                while self.pb_process.read_next_event(event):
                    for key in self.tasks.keys():
                        self.tasks[key].process_data(event)

                self.save_xlsx()

            except Exception as e:  # pylint: disable=broad-except
                glog.error("error while post process {}".format(str(e)))
            finally:
                # close pblog file
                self.pb_process.close_pblog()

                # delete old .pblog and .pblog.gz files
                try:
                    # delete old .pblog files
                    self.delete_pblog("*.pblog", min(3, self._args.max_pblog_number))
                except Exception as e:  # pylint: disable=broad-except
                    glog.error("error in delete .pblog {}".format(str(e)))
                try:
                    # keep latest self._args.max_pblog_number .pblog.gz files
                    self.delete_pblog("*.pblog.gz", self._args.max_pblog_number)
                except Exception as e:  # pylint: disable=broad-except
                    glog.error("error in delete .pblog.gz {}".format(str(e)))


if __name__ == "__main__":
    llp = PostProcess()
    if llp.parse_arguments():
        llp.process_pblog()
