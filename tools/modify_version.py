#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations
import json
import datetime
from pathlib import Path


VERSION_MAJOR = "1"
VERSION_MINOR = "0"
VERSION_DATA = datetime.datetime.now().strftime("%Y%m%d%H%M")


def modify_version(pathfile_json: Path, new_version: str) -> None:
    """
    Modify the 'version' key in a JSON file with a new version string.

    Args:
        pathfile_json (Path): The path to the JSON file.
        new_version (str): The new version string to be set.

    Returns:
        None
    """

    try:
        data = json.loads(pathfile_json.read_text())
        data["version"] = new_version

        pathfile_json.write_text(json.dumps(data, indent=4))
        print(f"Version updated to {new_version} in {pathfile_json}")
    except FileNotFoundError:
        print(f"File not name: {pathfile_json}")
    except json.JSONDecodeError:
        print(f"Invalid JSON format in {pathfile_json}")
    except KeyError:
        print(f"Key 'version' not found in {pathfile_json}")


if __name__ == "__main__":
    # Prompt the user to enter the path of the JSON file
    current_path_py = Path(__file__).resolve().parent

    # Generate a new version string based on the current date and time
    pathfile_json = current_path_py.parent / "build/package.json"
    new_version = f'{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_DATA}'

    # Call the modify_version function with the provided path and new version
    modify_version(pathfile_json, new_version)
