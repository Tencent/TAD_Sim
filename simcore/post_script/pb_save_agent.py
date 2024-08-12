#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import base64
import json
import os
import struct
import sys
from abc import abstractmethod
from typing import Dict

import glog

sys.path.append(os.path.dirname(sys.path[0]))

work_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(work_dir)
sys.path.append(os.path.join(work_dir, "sim_msg"))

from sim_msg import location_pb2, traffic_pb2


class PbEvent:
    def __init__(self, timestamp: float, topic: str, payload: bytes):
        self.timestamp: float = timestamp  # unit ms
        self.topic: str = topic  # topic of payload
        self.payload: bytes = payload  # serialized bytes of protobuf message


class PblogWriter:
    def __init__(self):
        self.pblog_file = "/path/do/not/exist"
        self.pblog_file_handle = None

    def __del__(self):
        self.close()

    def open_pblog(self, pblog_file) -> bool:
        self.pblog_file = pblog_file
        if os.path.exists(self.pblog_file):
            self.pblog_file_handle = open(self.pblog_file, "ab")
        else:
            self.pblog_file_handle = open(self.pblog_file, "wb")
        return True

    def write_event(self, topic: str, pb_bytes: bytes, t_microseconds: int):
        if self.pblog_file_handle:
            if len(topic) > 0:
                # glog.info("write topic : {}".format(topic))
                self.pblog_file_handle.write(struct.pack("I", len(topic)))
                self.pblog_file_handle.write(struct.pack("I", len(pb_bytes)))
                self.pblog_file_handle.write(struct.pack("l", t_microseconds))
                self.pblog_file_handle.write(bytes(topic, "utf8"))
                self.pblog_file_handle.write(pb_bytes)

    def close(self):
        if self.pblog_file_handle:
            self.pblog_file_handle.close()
            self.pblog_file_handle = None


class PblogReader:
    """
    .pblog file reader
    """

    def __init__(self):
        self.pb_log_file = None
        self.pb_log_file_handle = None

    def __del__(self):
        self.close()

    def open(self, pb_log_file):
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
            glog.info("pb | load protobuf log file from {}".format(self.pb_log_file))
        else:
            glog.info("pb | protobuf log file do not exist, {}".format(self.pb_log_file))
            return False

        return True

    def close(self):
        """
        close pblog file
        :return: None
        """
        if self.pb_log_file_handle:
            self.pb_log_file_handle.close()
            self.pb_log_file_handle = None
            glog.info("pb | close protobuf log file " + self.pb_log_file)

    def read_one(self, event_out: PbEvent) -> bool:
        """
        read one protobuf msg from .pblog file
        :param event_out: {"timestamp": 0.0, "topic": "", "payload": bytes()}
        :return: True/False
        """
        try:
            topic_size = struct.unpack("i", self.pb_log_file_handle.read(4))
            pb_size = struct.unpack("i", self.pb_log_file_handle.read(4))
            event_out.timestamp = struct.unpack("q", self.pb_log_file_handle.read(8))[0] / 1000.0
            event_out.topic = self.pb_log_file_handle.read(topic_size[0]).decode()
            event_out.payload = self.pb_log_file_handle.read(pb_size[0])
            # glog.info("{}".format(event_out))
        except Exception as e:  # pylint: disable=broad-except
            glog.info("pb | reach end of read file, {}".format(e))
            return False

        return True


class MsgParser(object):
    def __init__(self):
        pass

    @abstractmethod
    def parse(self, event: PbEvent) -> bool:
        raise NotImplementedError("parse in {} is a pure virtual function".format(MsgParser.__class__))

    @abstractmethod
    def show(self):
        raise NotImplementedError("show in {} is a pure virtual function".format(MsgParser.__class__))


class TrafficParser(MsgParser):
    """
    traffic.proto parser
    """

    def __init__(self):
        super(TrafficParser, self).__init__()
        self.msg = traffic_pb2.Traffic()

    def parse(self, event: PbEvent) -> bool:
        if event and event.topic == "TRAFFIC":
            self.msg.ParseFromString(event.payload)
            return True
        return False

    def show(self):
        # cars in traffic
        for fellow in self.msg.cars:
            glog.info(
                "traffic cars: id:{}, t:{}, x:{}, y:{}, heading:{}, v:{}, length:{}, width:{}, height:{}".format(
                    fellow.id,
                    fellow.t,
                    fellow.x,
                    fellow.y,
                    fellow.heading,
                    fellow.v,
                    fellow.length,
                    fellow.width,
                    fellow.height,
                )
            )

        # dynamic obstacles in traffic
        for fellow in self.msg.dynamicObstacles:
            glog.info(
                "traffic dynamic obstacles: id:{}, t:{}, x:{}, y:{}, heading:{}, v:{}, length:{}, width:{}, "
                "height:{}".format(
                    fellow.id,
                    fellow.t,
                    fellow.x,
                    fellow.y,
                    fellow.heading,
                    fellow.v,
                    fellow.length,
                    fellow.width,
                    fellow.height,
                )
            )

        # traffic lights in traffic
        for light in self.msg.trafficLights:
            glog.info(
                "traffic lights: id:{}, x:{}, y:{}, heading:{}, color:{}, age:{}".format(
                    light.id, light.x, light.y, light.heading, light.color, light.age
                )
            )
            for lane in light.control_lanes:
                glog.info(
                    "traffic lights: tx_road_id:{}, tx_section_id:{}, tx_lane_id:{}".format(
                        lane.tx_road_id, lane.tx_section_id, lane.tx_lane_id
                    )
                )
            for phase in light.control_phases:
                glog.info(" traffic lights: phase:{}".format(phase))

    def get_base64(self):
        # 将 proto 消息序列化为字节
        serialized_message = self.msg.SerializeToString()
        # 将字节转换为 Base64 格式
        base64_str = base64.b64encode(serialized_message).decode("utf-8")
        return base64_str


class LocationParser(MsgParser):
    """
    location.proto parser
    """

    def __init__(self):
        super(LocationParser, self).__init__()
        self.msg = location_pb2.Location()

    def parse(self, event: PbEvent):
        if event and event.topic == "LOCATION":
            self.msg.ParseFromString(event.payload)
            return True
        return False

    def show(self):
        glog.info(
            "location: t:{}, x:{}, y:{}, z:{}, vx:{}, vy:{}, heading:{}, angular_vz:{}".format(
                self.msg.t,
                self.msg.position.x,
                self.msg.position.y,
                self.msg.position.z,
                self.msg.velocity.x,
                self.msg.velocity.y,
                self.msg.rpy.z,
                self.msg.angular.z,
            )
        )

    def get_base64(self):
        # 将 proto 消息序列化为字节
        serialized_message = self.msg.SerializeToString()
        # 将字节转换为 Base64 格式
        base64_str = base64.b64encode(serialized_message).decode("utf-8")
        return base64_str


class TrafficRecords4LogsimParser(MsgParser):
    """
    traffic.TrafficRecords4Logsim.proto parser
    """

    def __init__(self):
        super(TrafficRecords4LogsimParser, self).__init__()
        self.msg = traffic_pb2.TrafficRecords4Logsim()

    def serialize(self, topic: str, parser_dict: Dict[str, MsgParser]):
        if topic == TOPIC.LOCATION:
            self.msg.ego_location.append(parser_dict[topic].msg)
        elif topic == TOPIC.TRAFFIC:
            self.msg.traffic_record.append(parser_dict[topic].msg)

    def get_bytes(self) -> bytes:
        return self.msg.SerializeToString()


class TOPIC:
    LOCATION = "LOCATION"
    TRAFFIC = "TRAFFIC"
    TrafficRecords4LogSimParser = "TrafficRecords4LogSimParser"


class PblogReaderProcess:
    def __init__(self):
        # .pblog file path
        self.pb_log_file = "/path/donot/exist"

        # pblog reader writer, used to read and write pblog
        self.pblog_reader = PblogReader()
        self.parser_dict = {
            TOPIC.TRAFFIC: TrafficParser(),
            TOPIC.LOCATION: LocationParser(),
            TOPIC.TrafficRecords4LogSimParser: TrafficRecords4LogsimParser(),
        }

    def save2file(self, pathout: str, datas: list, mode: str = "w"):
        with open(pathout, mode) as f:
            for data in datas:
                f.write(data + "\n")

    def set_data_simrec(self):
        return self.parser_dict[TOPIC.TrafficRecords4LogSimParser].get_bytes()

    def set_data_frames(self, timestamp: float, ext_info: str):
        # 构造字典
        data = {
            "frameId": int(timestamp / 20 + 1),
            "extInfo": ext_info,
            "tfc": self.parser_dict[TOPIC.TRAFFIC].get_base64(),
            "loc": self.parser_dict[TOPIC.LOCATION].get_base64(),
        }

        # 将字典转换为字符串
        return json.dumps(data)

    def parse(self, pblog_path: str, start_ms: float, end_ms: float, simrec_path: str, frames_path: str, ext_info: str):
        """
        parse .pblog file
        :param pblog_file: .pblog file
        :return: None
        """

        # 异常处理 - pblog 文件不存在
        if not os.path.exists(pblog_path):
            glog.error("usage: python PostProcess.py -f <pblog file>")
            return False

        #
        self.pb_log_file = pblog_path
        # if simrec_file exists. remove
        if simrec_path and os.path.exists(simrec_path):
            os.remove(simrec_path)

        if frames_path and os.path.exists(frames_path):
            os.remove(frames_path)

        # 读取 pblog 文件, 并处理异常 - 读取失败
        if not self.pblog_reader.open(self.pb_log_file):
            glog.error("fail to open pblog file {}".format(self.pb_log_file))
            return False

        # 正常业务处理流程
        # protobuf msg read from .pblog file
        event = PbEvent(0.0, "", bytes())

        frames_dict = {"frameId": -1, "extInfo": "", "tfc": "", "loc": ""}

        # parse protobuf msg
        datas_frames = []
        while self.pblog_reader.read_one(event):
            if event.topic in self.parser_dict.keys() and start_ms <= event.timestamp <= end_ms:
                try:
                    if self.parser_dict[event.topic].parse(event):
                        self.parser_dict[TOPIC.TrafficRecords4LogSimParser].serialize(event.topic, self.parser_dict)

                    frame_id = int(event.timestamp / 20 + 1)

                    if frames_dict["frameId"] != -1 and frame_id != frames_dict["frameId"]:
                        # print("=================================")
                        datas_frames.append(json.dumps(frames_dict))

                    frames_dict["frameId"] = frame_id
                    frames_dict["extInfo"] = ext_info
                    # print(event.topic, frame_id)

                    if self.parser_dict[TOPIC.LOCATION].parse(event):
                        frames_dict["loc"] = self.parser_dict[TOPIC.LOCATION].get_base64()
                        # print(f"{frames_dict['loc'] = }")

                    if self.parser_dict[TOPIC.TRAFFIC].parse(event):
                        frames_dict["tfc"] = self.parser_dict[TOPIC.TRAFFIC].get_base64()
                        # print(f"{frames_dict['tfc'] = }")

                except Exception as ex:  # pylint: disable=broad-except
                    glog.error(f"error while process {event.topic}, error --> {str(ex)}")

        if simrec_path:
            # datas_simrec = [self.set_data_simrec()]
            # self.save2file(pathout=simrec_path, datas=datas_simrec, mode="wb")
            with open(simrec_path, "wb") as simrec_handle:
                simrec_handle.write(self.set_data_simrec())

        if frames_path:
            # print(f"{datas_frames = }")
            self.save2file(pathout=frames_path, datas=datas_frames, mode="w")

        # close file
        self.pblog_reader.close()

        return True


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--pblog_path", type=str, help="input filepath", required=True)
    parser.add_argument("--start_ms", type=float, help="data start time", required=True)
    parser.add_argument("--end_ms", type=float, help="data end time", required=True)
    parser.add_argument("--simrec_path", type=str, help="output simrec filepath", required=False)
    parser.add_argument("--frames_path", type=str, help="output frames filepath", required=False)
    parser.add_argument("--ext_info", type=str, help="ext infomation", required=False)
    # glog.setLevel(glog.debug)
    args = parser.parse_args()

    pb_process = PblogReaderProcess()

    if not args.simrec_path and not args.frames_path:
        sys.exit("Exiting the program due to simrec_path and frames_path require at least one input.")

    pb_process.parse(args.pblog_path, args.start_ms, args.end_ms, args.simrec_path, args.frames_path, args.ext_info)
