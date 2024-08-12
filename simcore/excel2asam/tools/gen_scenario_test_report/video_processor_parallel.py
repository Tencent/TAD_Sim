#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from __future__ import annotations

import argparse
import re
import shutil
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from pathlib import Path
from typing import List

import cv2
import easyocr
import numpy as np
from loguru import logger


@dataclass(order=True)
class VideoProcessor:
    # 你的视频文件路径
    pathfile_video: Path
    # 场景结束文字模板图片路径
    pathdir_template: Path = field(default=Path("event_template"))

    def __post_init__(self) -> None:
        self.pathdir_output = self._create_folder_structure(self.pathfile_video)
        self._set_log_file(pathdir=self.pathdir_output)

        self.threshold = 0.7

        # 提高处理速度
        self.skip_sec = 3  # 每隔多少秒处理一次

        # 初始化EasyOCR
        self.reader = easyocr.Reader(["en", "ch_sim"])

        # 场景名识别指定区域
        # ocr 不指定区域识别效果不好, 指定区域识别效果更好,
        # 但是对于录屏有要求,  27寸显示器, 分辨率 3840 * 2160
        # tadsim 全屏录制
        self.filename_roi = (353, 153, 2647, 77)  # (x, y, width, height)

        # 场景名识别关键字
        start_char = r"场景名: "
        end_char = r"xosc|sim|\."
        self.pat_find = re.compile(f"{start_char}(.*?)(?:{end_char})", re.IGNORECASE)

        #
        self.replacement_dict = {
            " ": "_",
            ".": "",
            "__": "_",
            "0g": "09",
            r"LASO|LA5O": "LA50",
            "ZO": "20",
            "Z1": "21",
            r"_G$": "_9",
            r"_Z$": "_2",
        }
        self.pat_replace = re.compile("|".join(re.escape(key) for key in self.replacement_dict))

    def _set_log_file(self, pathdir: Path, filename_log: str = "file") -> None:
        """
        > Create log files

        Args:
          pathdir (Path): The path of the output directory
          filename_log (str): The name of the log file

        Returns:
          None
        """
        # 创建 log 文件
        logger.opt(lazy=True).add(pathdir / f"{filename_log}.log", rotation="100 MB", encoding="utf-8")

    def _create_folder_structure(self, pathfile_video: Path) -> Path:
        """Create output folders"""
        # 创建 xosc xodr 子文件夹
        try:
            pathdir_output = Path(__file__).resolve().parent / f"{pathfile_video.stem}"

            # 创建输出文件夹(检查文件夹是否存在,存在则删除重新创建)
            if pathdir_output.is_dir():
                shutil.rmtree(pathdir_output)
            pathdir_output.mkdir(parents=True, exist_ok=True, mode=0o755)

            return pathdir_output

        except OSError as e:
            logger.opt(lazy=True).error(f"Error creating output directories: {e}")
            raise

    def _get_templates(self, pathdir_template: Path) -> List[np.ndarray]:
        """
        > Get templates
        """
        return [cv2.imread(str(file), 0) for file in pathdir_template.glob("*") if file.is_file()]

    def _event_occurred(self, gray_frame: np.ndarray, template: np.ndarray, i: int) -> bool:
        # 在视频帧中查找模板图片
        res = cv2.matchTemplate(gray_frame, template, cv2.TM_CCOEFF_NORMED)
        _, max_val, _, _ = cv2.minMaxLoc(res)

        # 比较当前帧和特定事件图片, 返回是否出现特定事件
        logger.opt(lazy=True).debug(f"在视频帧中查找模板图片{i}, max_val = {round(max_val, 3)}")

        return max_val >= self.threshold

    def _event_occurred_any(self, gray_frame: np.ndarray, templates: List[np.ndarray]) -> bool:
        return any(self._event_occurred(gray_frame, template, i) for i, template in enumerate(templates, start=1))

    def _replace_with_dict(self, text: str) -> str:
        def replacement_func(match):
            return self.replacement_dict[match.group(0)]

        return self.pat_replace.sub(replacement_func, text)

    def _extract_between_chars(self, text: str) -> str:
        outs = self.pat_find.findall(text)
        print(f"{outs = }")

        return self._replace_with_dict(outs[0]) if outs else ""

    def _find_related_filename(self, frame: np.ndarray):
        logger.opt(lazy=True).debug("使用 OCR 读取整个帧中的文字, 并找到需要的内容")

        # 从指定区域的文字中识别并提取特定字段
        x, y, w, h = self.filename_roi
        text_list = self.reader.readtext(frame[y : y + h, x : x + w], detail=0)

        logger.opt(lazy=True).info(f"text_list: {text_list}")

        text_str = " ".join(text_list).strip()

        return self._extract_between_chars(text_str)

    def _save_capture_frame(self, frame: np.ndarray, pathfile: Path) -> None:
        logger.opt(lazy=True).info(f"截取连续事件的第一帧并保存: {pathfile.name}")
        cv2.imwrite(str(pathfile), frame)

    def process_video_part(self, start_frame, end_frame) -> None:
        # 初始化参数
        flag_event = False
        frame_count = start_frame

        # 全部模板图片转换为 List[np.ndarray]
        templates = self._get_templates(self.pathdir_template)
        # print(f"templates: {templates}")

        # 加载视频
        cap = cv2.VideoCapture(str(self.pathfile_video))

        # 获取视频的帧率
        fps = cap.get(cv2.CAP_PROP_FPS)
        frame_count_skip = self.skip_sec * fps

        # while cap.isOpened():
        while frame_count <= end_frame:
            # 读取视频帧
            ret, frame = cap.read()

            # 视频帧读取失败, 退出循环
            if not ret:
                logger.opt(lazy=True).debug("视频帧读取失败, 退出循环")
                break

            # 跳帧处理
            frame_count += 1
            if frame_count % frame_count_skip != 0:
                continue

            # 将视频帧转换为灰度图像
            gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

            # 当不是指定事件时, 重置 flag_event 标志, 并跳过当前帧
            if not self._event_occurred_any(gray_frame, templates):
                flag_event = False
                continue

            # 检测到是指定事件, 且 flag_event 为 False时
            # 从识别的文字中提取特定字段, 则将其用作截图文件名
            # 存储图片
            # 重置 flag_event 标志, 以便仅保存第一帧
            if not flag_event:
                filename = self._find_related_filename(frame)
                self._save_capture_frame(frame=frame, pathfile=self.pathdir_output / f"{filename}.png")
                flag_event = True

        # 释放视频资源
        cap.release()

    def process_video_parallel(self, num_threads=4):
        cap = cv2.VideoCapture(str(self.pathfile_video))
        total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        cap.release()

        frames_per_thread = total_frames // num_threads

        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            futures = []

            for i in range(num_threads):
                start_frame = i * frames_per_thread
                end_frame = (i + 1) * frames_per_thread - 1 if i < num_threads - 1 else total_frames - 1

                future = executor.submit(self.process_video_part, start_frame, end_frame)
                futures.append(future)

            for future in futures:
                future.result()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--file", "-f", dest="pathfile_video", help="pathfile_video", required=True, metavar="FILE")
    args = parser.parse_args()
    args.pathfile_video = Path(args.pathfile_video)

    import time

    t0 = time.time()
    vp = VideoProcessor(args.pathfile_video)
    vp.process_video_parallel()
    print(f"time: {time.time() - t0}")


if __name__ == "__main__":
    main()
