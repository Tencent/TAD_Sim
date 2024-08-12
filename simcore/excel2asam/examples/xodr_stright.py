#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from pathlib import Path

from excel2asam.openx.wrapper_xodr import GenXodr

scene = {
    "MapFile": "xodr_Straight",
    #
    "Road.Type": "straight",
    "Road.Curve": 0,
    "Road.Direction": "opposite",
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
    "Marking.Type": "solid_solidyellow",
    "Marking.Status": "normal",
}

GenXodr(gen_folder=Path("output"), odrmajor=1, odrminor=5).generate(scene)


# print(settings.as_dict())
