#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from pathlib import Path

from excel2asam.openx.wrapper_xodr import GenXodr

scene = {
    "MapFile": "xodr_split",
    #
    "Road.Type": "split",
    "Road.Curve": 0,
    "Road.Direction": "same",
    "Road.Speedlimit": 0,
    #
    "Junction.Type": "none",
    "Junction.Num": 0,
    #
    "Lane.Type": "shoulder",
    "Lane.Num": 3,
    "Lane.Width": 3.5,
    "Lane.Edge": "none",
    #
    "Marking.Type": "solidwhite",
    "Marking.Status": "normal",
}

GenXodr(gen_folder=Path("output"), odrmajor=1, odrminor=5).generate(scene)


# print(settings.as_dict())
