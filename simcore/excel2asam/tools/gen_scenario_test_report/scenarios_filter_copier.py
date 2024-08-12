#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import shutil
from pathlib import Path


class ScenariosFilterCopier:
    def __init__(self, source_folder, dest_folder):
        self.source_folder = Path(source_folder)
        self.dest_folder = Path(dest_folder)

    def create_dest_folder(self):
        self.dest_folder.mkdir(parents=True, exist_ok=True)

    def copy_files(self):
        for xosc_file in self.source_folder.glob("**/*.xosc"):
            # 检查文件名是否以 _1.xosc 结尾
            if xosc_file.name.endswith("_1.xosc"):
                destination_file = self.dest_folder / xosc_file.name

                # 检查源文件和目标文件是否相同
                if xosc_file.resolve() == destination_file.resolve():
                    print(f"Skipping {xosc_file} because it is the same as the destination file")
                    continue

                shutil.copy(xosc_file, destination_file)
                print(f"Copied {xosc_file.name} to {destination_file.name}")

    def run(self):
        self.create_dest_folder()
        self.copy_files()


# 使用类
SOURCE_FOLDER = "C:/Users/xingboliu/Desktop/0920"
DEST_FOLDER = "filter"

copier = ScenariosFilterCopier(SOURCE_FOLDER, DEST_FOLDER)
copier.run()
