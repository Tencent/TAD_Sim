import os
import sys
import argparse

# 获取当前文件的绝对路径，并获取其所在目录
gWorkDir = os.path.dirname(os.path.abspath(__file__))
# 定义消息的路径，该路径是相对于当前文件所在目录的上一级目录
gSimMsgPath = os.path.join(gWorkDir, "../sim_msg")
# 将路径添加到系统路径中，以便可以导入该路径下的模块
sys.path.append(gSimMsgPath)

import grading_kpi_pb2
from google.protobuf import json_format


class KPIUpgrade:
    # 定义一个名为__init__的构造函数，用于初始化对象
    def __init__(self, old_json_path, new_json_path):
        # 将传入的旧JSON文件路径赋值给实例变量__old_json_path
        self.__old_json_path: str = old_json_path
        # 将传入的新JSON文件路径赋值给实例变量__new_json_path
        self.__new_json_path: str = new_json_path
        # 创建一个GradingKpiGroup类型的对象_kpi_proto，用于处理KPI相关的数据
        self._kpi_proto = grading_kpi_pb2.GradingKpiGroup()

    # 定义升级方法
    def upgrade(self):
        old_json_content = ""
        # 打开旧JSON文件并读取内容
        with open(self.__old_json_path, "r") as old_handle:
            # load and parse old proto message
            old_json_content = old_handle.read()
            json_format.Parse(old_json_content, self._kpi_proto)
            # 调用升级函数
            self.upgrade_function()

        # 打开新JSON文件写入
        with open(self.__new_json_path, "w") as new_handle:
            # 将原型消息转换为JSON字符串，包括默认值字段，再写入新JSON文件
            json_string = json_format.MessageToJson(self._kpi_proto, including_default_value_fields=True)
            new_handle.write(json_string)

    def upgrade_function(self):
        # self.add_score_default_score()
        self.set_default_score_to_none()

    # 定义一个方法，用于将KPI的默认分数设置为None
    def set_default_score_to_none(self):
        # 遍历KPI原型中的所有KPI
        for kpi in self._kpi_proto.kpi:
            # 获取KPI的scoreMap1d属性，清除scoreMap1d中的"u"字段、"y"字段
            score_map1d = kpi.parameters.scoreMap1d
            score_map1d.ClearField("u")
            score_map1d.ClearField("y")
            # 遍历KPI的阈值参数，清空阈值参数中的scoreMap1d的"u"字段、"y"字段
            for thresh in kpi.parameters.thresholds:
                thresh.scoreMap1d.u.extend([])
                thresh.scoreMap1d.y.extend([])

    def add_score_default_score(self):
        # 遍历所有的KPI
        for kpi in self._kpi_proto.kpi:
            # 获取当前KPI的分数映射一维数组
            score_map1d = kpi.parameters.scoreMap1d
            # 如果分数映射一维数组的'u'值为空
            if len(score_map1d.u) == 0:
                # 如果KPI分类标签中包含"STATISTICS"
                if "STATISTICS" in kpi.category.labels:
                    # 则分数映射一维数组的'u'和'y'值都扩展为空列表
                    score_map1d.u.extend([])
                    score_map1d.y.extend([])
                else:
                    # 否则，分数映射一维数组的'u'值添加0和1，'y'值添加100和0
                    score_map1d.u.append(0)
                    score_map1d.u.append(1)
                    score_map1d.y.append(100)
                    score_map1d.y.append(0)
            # 遍历当前KPI的所有阈值
            for thresh in kpi.parameters.thresholds:
                # 对每个阈值的分数映射一维数组，'u'和'y'值都扩展为空列表
                thresh.scoreMap1d.u.extend([])
                thresh.scoreMap1d.y.extend([])


if __name__ == "__main__":
    # 创建一个ArgumentParser对象
    parser = argparse.ArgumentParser()
    # 添加命令行参数，--old和--new分别对应旧版和新版json文件的路径
    parser.add_argument("--old", type=str, help="old json file path")
    parser.add_argument("--new", type=str, help="old json file path")
    # 解析命令行参数，并将结果存储在args变量中
    args = parser.parse_args()
    # 创建KPIUpgrade类的实例，传入旧版和新版json文件的路径
    upgrade = KPIUpgrade(args.old, args.new)
    # 调用upgrade方法，执行升级操作
    upgrade.upgrade()
