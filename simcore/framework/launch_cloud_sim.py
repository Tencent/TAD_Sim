"""
这个模块用于启动场景仿真调度服务。
"""
import argparse
import json
import os
import subprocess
import sys

# 仿真引擎配置文件
TXSIM_INPUT_CONFIG_FILE = "local_service.config.in"
# 仿真引擎配置文件
TXSIM_CONFIG_FILE = "local_service.config"
# 仿真引擎配置文件根目录文件夹名称
TXSIM_HOLDER_SIM_ROOT = "SimRoot"
# 仿真引擎算法配置文件夹名称
TXSIM_KEY_MODULE_LAUNCHER_PATH = "moduleLauncherPath"
# 仿真引擎日志文件夹名称
TXSIM_KEY_MODULE_LOG_DIR = "moduleLogDirectory"
# 消息队列字段
TXSIM_KEY_MSG_QUEUE = "mqInfo"
# 虚拟城市配置字段
TXSIM_KEY_CITY_CONFIG = "cityInfo"
# 模组名称字段
TXSIM_KEY_SCHEME = "scheme"
# 调度模式字段
TXSIM_KEY_CODR_MODE = "coordinationMode"
# 自动结束字段
TXSIM_KEY_AUTO_STOP = "autoStop"
# 评测指标目录
TXSIM_KEY_KPI_DIR = "gradingKpiDirectory"
TXSIM_DIR_BIN = "bin"
# 场景仿真服务
TXSIM_BIN_WORLD_SERVICE = "txsim-cloud-service"
# 虚拟城市服务
TXSIM_BIN_CITY_SERVICE = "txsim-cloud-city-service"
# 算法动态库启动模块
TXSIM_BIN_MODULE_LAUNCHER = "txsim-module-launcher"

"""
将输入的字符串转换成json格式。
如果转换失败，则打印错误信息并退出程序。
参数:
    json_str: 输入的字符串。
    ec: 错误码。
    err_str: 错误信息。
返回值:
    转换后的json格式。
"""


def try_read_json(json_str, ec, err_str):
    try:
        return json.loads(json_str)
    except ValueError:
        print("{}: {}".format(err_str, json_str))
        sys.exit(ec)


if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser()
    # 添加参数
    # --city: 是否启动城市仿真。
    arg_parser.add_argument("--city", action="store_true",
                            help="whether to launch the city sim instead of the default world sim.")
    # --app-dir: 指定根目录，在其中查找配置文件。
    arg_parser.add_argument("--app-dir", required=True,
                            help="specify the root sim directory, in which the config file located.")
    # --log-dir: 指定日志文件输出目录。
    arg_parser.add_argument("--log-dir", required=True,
                            help="specify where the log file are written to.")
    # --module-path: 算法启动路径。
    arg_parser.add_argument(
        "--module-paths", help="the uploaded module path info.")
    # --mq-config: 消息队列配置。
    arg_parser.add_argument("--mq-config", required=True,
                            help="specify the output message queue info.")
    # --city-config: 城市仿真配置。
    arg_parser.add_argument(
        "--city-config", help="specify the city sim relative configs. only valid when --city is enabled.")
    arg_parser.add_argument("--log-level", default=0,
                            help="the logging level.")
    args = arg_parser.parse_args()
    # temporary fix, need to be removed later.
    args.app_dir = os.path.join(args.app_dir, "sim")

    print("reading input config file ...")
    with open(os.path.join(args.app_dir, TXSIM_INPUT_CONFIG_FILE), 'r') as input_cfg:
        config = input_cfg.read()

    print("replacing input config file with module paths ...")
    if args.module_paths:
        module_paths = try_read_json(
            args.module_paths, 1, "invalid module-paths argument")
        for k, v in module_paths.items():
            config = config.replace("@{{{}}}".format(k), v)
    config = config.replace(
        "@{{{}}}".format(TXSIM_HOLDER_SIM_ROOT), args.app_dir)

    print("adding cloud info relative json items ...")
    config_json = try_read_json(
        config, 2, "invalid json configuration content after replaced")
    for mc in config_json["configs"]:
        mc["initArgs"]["_log_level"] = args.log_level

    mq_cfg_json = try_read_json(args.mq_config, 3, "invalid json mq-config")
    config_json[TXSIM_KEY_MSG_QUEUE] = mq_cfg_json

    if args.city:
        config_json[TXSIM_KEY_AUTO_STOP] = False
        config_json[TXSIM_KEY_KPI_DIR] = "./data/scene"
        if not args.city_config:
            print("--city-config has to be provided when --city is enabled.")
            sys.exit(3)
        city_cfg_json = try_read_json(
            args.city_config, 4, "invalid json city-config")
        config_json[TXSIM_KEY_CITY_CONFIG] = city_cfg_json

    config_json[TXSIM_KEY_MODULE_LAUNCHER_PATH] = os.path.join(
        args.app_dir, TXSIM_DIR_BIN, TXSIM_BIN_MODULE_LAUNCHER)
    config_json[TXSIM_KEY_MODULE_LOG_DIR] = os.path.join(args.log_dir, "user")

    if TXSIM_KEY_SCHEME not in config_json:
        active_scheme = { "id": 0, "name": "SimCloud", "modules": [] }
        for m in config_json["configs"]:
            active_scheme["modules"].append(m["name"])
        config_json[TXSIM_KEY_SCHEME] = { "active": 0, "schemes": [] }
        config_json[TXSIM_KEY_SCHEME]["schemes"].append(active_scheme)
        config_json[TXSIM_KEY_CODR_MODE] = 1

    print("writing output service config file ...")
    with open(os.path.join(args.app_dir, TXSIM_CONFIG_FILE), 'w') as output_cfg:
        output_cfg.write(json.dumps(config_json))

    if "LD_LIBRARY_PATH" in os.environ and not os.environ["LD_LIBRARY_PATH"]:
        os.environ["LD_LIBRARY_PATH"] += (";" +
                                          os.path.join(args.app_dir, "deps"))
    else:
        os.environ["LD_LIBRARY_PATH"] = os.path.join(args.app_dir, "deps")
    launching_path = os.path.join(args.app_dir, TXSIM_DIR_BIN, TXSIM_BIN_CITY_SERVICE) if args.city else os.path.join(
        args.app_dir, TXSIM_DIR_BIN, TXSIM_BIN_WORLD_SERVICE)
    print("launching {} ...".format(launching_path))
    ret = subprocess.Popen(args=[launching_path, "--root", args.app_dir,
                                 "--log-dir", os.path.join(args.log_dir, "sys"), "--log-level", args.log_level])
