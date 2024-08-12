#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json
import os
import shutil
import struct
import time

import glog
import xlrd


class HandleJson:
    def __init__(self):
        pass

    def dict2json(self, dict_in):
        """
        convert dict to json
        :param dict_in:
        :return: json
        """
        json_out = {}
        try:
            json_out = json.dumps(dict_in)
        except Exception as e:  # pylint: disable=broad-except
            json_out = {}
            glog.error("pb | fail to convert dict to json, " + str(e))

        return json_out

    def dump_json2file(self, json_in, json_file):
        """
        save json into file
        :param json_in:
        :param json_file:
        :return: True/False
        """
        try:
            json_file_tmp = json_file + ".tmp"
            with open(json_file_tmp, "w") as json_handle:
                json_handle.write(json_in)
                glog.info("pb | json write into file " + json_file_tmp)

            shutil.move(json_file_tmp, json_file)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("pb | fail to save json, " + str(e))
            return False

        return True


class Utils:
    def __init__(self):
        pass

    def get_date_string_ymd(self):
        """
        get date string of year-month-day
        :return: string of year-month-day
        """
        loc_time = time.localtime(time.time())
        loc_time_str = str(loc_time.tm_year) + "_" + str(loc_time.tm_mon) + "_" + str(loc_time.tm_mday)
        return loc_time_str

    def get_date_string(self):
        """
        get date string of year-month-day-hour-minute-second
        :return: string of year-month-day-hour-minute-second
        """
        loc_time = time.localtime(time.time())
        loc_time_str = self.get_date_string()
        loc_time_str += "_" + str(loc_time.tm_hour) + "_" + str(loc_time.tm_min) + "_" + str(loc_time.tm_sec)
        return loc_time_str


class XlsReader:
    def __init__(self, xlsx_file):
        self._xlsx_file = xlsx_file
        self._xlsx_handle = xlrd.open_workbook(self._xlsx_file)

    def load_column_data(self, sheet_name_, column_index) -> list:
        column_data = []

        try:
            xlsx_sheet = self._xlsx_handle.sheet_by_name(sheet_name_)
            column_data = xlsx_sheet.col_values(colx=column_index, start_rowx=0)
        except Exception as e:  # pylint: disable=broad-except
            glog.error("error, xlsx file:{}, reason:{}".format(self._xlsx_file, e))

        return column_data


class PblogReader:
    def __init__(self):
        self.pb_log_file = None
        self.pb_log_file_handle = None

    def open_pblog(self, pb_log_file):
        """
        open protobuf log file
        :param pb_log_file: protobuf log file
        :return: True/False
        """
        self.pb_log_file = pb_log_file

        # check if file exist
        if os.path.exists(self.pb_log_file):
            self.pb_log_file_handle = open(self.pb_log_file, "rb")
            self.pb_log_file_handle.seek(0)
            glog.info("pb | load protobuf log file from " + self.pb_log_file)
        else:
            glog.info("pb | protobuf log file do not exist. " + self.pb_log_file)
            return False

        return True

    def close_pblog(self):
        """
        close pblog file
        :return: None
        """
        if self.pb_log_file_handle:
            self.pb_log_file_handle.close()
            self.pb_log_file_handle = None
            glog.info("pb | close protobuf log file " + self.pb_log_file)

    def read_next_event(self, event_out):
        """
        read one protobuf data
        :param event_out: one protobuf log data
        :return: True/False
        """
        try:
            topic_size = struct.unpack("i", self.pb_log_file_handle.read(4))
            pb_size = struct.unpack("i", self.pb_log_file_handle.read(4))
            event_out["timestamp"] = struct.unpack("q", self.pb_log_file_handle.read(8))[0]
            event_out["channel"] = self.pb_log_file_handle.read(topic_size[0]).decode()
            event_out["pb_msg_str"] = self.pb_log_file_handle.read(pb_size[0])
            # glog.info("pb | topic:" + event_out["channel"] + ", message size:" + str(len(event_out["pb_msg_str"])))
        except Exception as e:  # pylint: disable=broad-except
            glog.info("pb | reach end of read file, " + str(e))
            return False

        return True


class PblogWriter:
    def __init__(self):
        self.pb_log_file = None
        self.pb_log_file_handle = None

    def create_pblog(self, pb_log_file):
        self.pb_log_file = pb_log_file

        self.pb_log_file_handle = open(self.pb_log_file, "wb")

        return self.pb_log_file_handle is None

    def write_next_event(self, topic, pb_bytes, t_microseconds):
        if self.pb_log_file_handle:
            if len(topic) > 0 and len(pb_bytes) > 0:
                self.pb_log_file_handle.write(struct.pack("I", len(topic)))
                self.pb_log_file_handle.write(struct.pack("I", len(pb_bytes)))
                self.pb_log_file_handle.write(struct.pack("l", t_microseconds))
                self.pb_log_file_handle.write(bytes(topic, "utf8"))
                self.pb_log_file_handle.write(pb_bytes)

    def close_pblog(self):
        self.pb_log_file_handle.close()
