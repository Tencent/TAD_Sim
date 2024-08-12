#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from pathlib import Path

# 创建一个包含要替换的子字符串及其替换字符串的字典
replacements = {
    "ENCAP2O19": "ENCAP2019",
    "CNCAPZ021": "CNCAP2021",
    "CBLAZ5": "CBLA25",
    "CVNAZ5": "CVNA25",
    "CPNAZ5": "CPNA25",
    "CPNCSO": "CPNC50",
    # ...在此添加更多替换规则
    "50TIF": "SOTIF",
    "__": "_",
    "Lnchange": "LaneChange",
    "Lanechange": "LaneChange",
    "OGO": "060",
    "StraightNone": "Straight_None",
    "PerceptionMod_": "PerceptionModule_",
    "PerceptionMc_": "PerceptionModule_",
    "_1J": "_1",
    "B.": "8.",
    "Uniformspd": "UniformSpd",
    "_J_": "_1_",
    "_SOTIF025": "_SOTIF_025",
    "SOTIF061": "SOTIF_061",
    "Egochangelane": "EgoChangeLane",
    "EgoChangelane": "EgoChangeLane",
    "EgoChanezleft": "EgoChange2Left",
    "EgoChanezRight": "EgoChange2Right",
    "Turnleft": "TurnLeft",
    "Changelane": "ChangeLane",
    "Egozleft": "Ego2Left",
    "52N": "S2N",
    "WZN": "W2N",
    "EZN": "E2N",
    "WZE": "W2E",
    "NZS": "N2S",
    "Withoutcar": "WithoutCar",
    "Withcar": "WithCar",
    "Driverlanechange": "DriverLaneChange",
    "EgolowSpeedCutln": "EgoLowSpeedCutIn",
    "EgoHighspeedCutln": "EgoHighSpeedCutIn",
    "Sixlane": "SixLane",
    "DriverlaneChange": "DriverLaneChange",
    "CNCAP2O2T": "CNCAP2021",
    "CNCAP202I": "CNCAP2021",
    "Unifomspd": "UnifomSpd",
    "Targetcar": "TargetCar",
    "Speedctrol": "SpeedCtrol",
    "EgoWithoffset": "EgoWithOffset",
    "ReponseotherVehicles": "ReponseOtherVehicles",
    "_1_1": "_1",
    "_2_1": "_2",
    "_3_1": "_3",
    "_4_1": "_4",
    "_5_1": "_5",
    "_6_1": "_6",
    "_7_1": "_7",
    "_8_1": "_8",
    "_9_1": "_9",
}

# 使用 pathlib.Path 类来表示包含 PNG 文件的目录
png_files_directory = Path(__file__).resolve().parent

print(png_files_directory)

# 使用 Path.glob 方法来获取目录中所有的 PNG 文件
png_files = png_files_directory.rglob("*.png")

# 遍历所有 PNG 文件
for png_file in png_files:
    # 获取文件名（不包括扩展名）
    file_name = png_file.stem

    # 检查文件名中是否包含字典中的任何子字符串
    for old, new in replacements.items():
        if old in file_name:
            # 使用 str.replace 方法将子字符串替换为相应的替换字符串
            new_file_name = file_name.replace(old, new)

            # 检查新的文件名是否已经存在
            new_png_file = png_file.with_stem(new_file_name)
            SUFFIX_COUNT = 1
            while new_png_file.exists():
                # 为新文件名添加一个数字后缀
                new_file_name_with_suffix = f"{new_file_name}_{SUFFIX_COUNT}"
                new_png_file = png_file.with_stem(new_file_name_with_suffix)
                SUFFIX_COUNT += 1

            # 使用 Path.rename 方法将文件重命名为新的文件名
            png_file.rename(new_png_file)

            # 文件已重命名, 跳出内部循环
            break
