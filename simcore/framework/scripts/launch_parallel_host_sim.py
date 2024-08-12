"""
并行启动仿真引擎.
该脚本可以并行启动多个仿真引擎，以提高仿真效率。
多个仿真引擎的端口号会依次递增，从30000开始。
"""


import argparse
import json
import http.client
import os
import shutil
import subprocess
import sys
import time


# simcore安装目录
TXSIM_APP_INSTALL_DIR = "/opt/tadsim/resources/app/service"
# simcore安装目录下的依赖目录
TXSIM_DEP_DIR = "simdeps"
# 单机版服务名
TXSIM_BIN_LOCAL_SERVICE = "txsim-local-service"
# 用户配置目录
TXSIM_USER_DATA_DIR = ".config/tadsim"
# 用户配置文件名
TXSIM_LOCAL_SERVICE_CONFIG = "local_service.config"
# 数据库名
TXSIM_CONFIG_DB = "service.sqlite"
# 场景目录
TXSIM_SCENARIO_DIR = "scenario"
# 系统日志目录
TXSIM_SYS_LOG_DIR = "sys_log"
# 用户日志目录
TXSIM_USR_LOG_DIR = "usr_log"
# 依赖库路径环境变量
TXSIM_ENVVAR_LDLIBPATH = "LD_LIBRARY_PATH"
# 模块日志目录
TXSIM_CONFIG_KEY_MODULE_LOGDIR = "moduleLogDirectory"
# 服务端口起始值
TXSIM_DEFAULT_SERVICE_PORT_START = 30000

"""
获取用户日志目录和配置目录
参数:
    runtime_dir: 运行目录。
    parallel_count: 并行数量。
"""


def prepare_run_env(runtime_dir, parallel_count):
    user_dir_abs = os.path.join(os.path.expanduser("~"), TXSIM_USER_DATA_DIR)
    local_service_config_path = os.path.join(user_dir_abs, TXSIM_LOCAL_SERVICE_CONFIG)
    scenario_dir = os.path.join(user_dir_abs, TXSIM_SCENARIO_DIR)
    if not os.path.exists(local_service_config_path):
        sys.exit("the local config file not exist: " + local_service_config_path, file=sys.stderr)
    if not os.path.exists(scenario_dir):
        sys.exit("the scenario directory not exist: " + local_service_config_path, file=sys.stderr)

    with open(local_service_config_path, 'r') as input_cfg:
        config = input_cfg.read()
    config_json = json.loads(config)

    if not os.path.exists(runtime_dir):
        os.makedirs(runtime_dir)
    instance_dirs = []
    for i in range(parallel_count):
        instance_dir = os.path.join(runtime_dir, "sim" + str(i + 1))
        if not os.path.exists(instance_dir):
            os.mkdir(instance_dir)
        config_json[TXSIM_CONFIG_KEY_MODULE_LOGDIR] = os.path.join(instance_dir, TXSIM_USR_LOG_DIR)
        with open(os.path.join(instance_dir, TXSIM_LOCAL_SERVICE_CONFIG), 'w') as output_cfg:
            output_cfg.write(json.dumps(config_json))
        sys_log_dir = os.path.join(instance_dir, TXSIM_SYS_LOG_DIR)
        if not os.path.exists(sys_log_dir):
            os.mkdir(sys_log_dir)
        shutil.copy(os.path.join(user_dir_abs, TXSIM_CONFIG_DB), instance_dir)
        instance_dirs.append(instance_dir)
    return instance_dirs


def launch_sim_services(instance_dirs, ports):
    sim_process = []
    for i in range(len(instance_dirs)):
        if TXSIM_ENVVAR_LDLIBPATH in os.environ and len(os.environ[TXSIM_ENVVAR_LDLIBPATH]) > 0:
            os.environ[TXSIM_ENVVAR_LDLIBPATH] += (";" + os.path.join(TXSIM_APP_INSTALL_DIR, TXSIM_DEP_DIR))
        else:
            os.environ[TXSIM_ENVVAR_LDLIBPATH] = os.path.join(TXSIM_APP_INSTALL_DIR, TXSIM_DEP_DIR)
        launching_path = os.path.join(TXSIM_APP_INSTALL_DIR, TXSIM_BIN_LOCAL_SERVICE)
        sim_dir = instance_dirs[i]
        print("launching sim-{} on direcotory {} ...".format(i + 1, sim_dir))
        p = subprocess.Popen(
            args=[
                launching_path,
                "--root", sim_dir,
                "--logdir", os.path.join(sim_dir, TXSIM_SYS_LOG_DIR),
                "--endpoint", "0.0.0.0:" + str(ports[i]), "--id", str(i + 1)
                ])
        sim_process.append(p)
    return sim_process


def run_scenarios(serv_ports, scenarios):
    for sc in scenarios:
        print("running scenario {} ...".format(sc))
        for sp in serv_ports:
            for i in range(3):
                print("launch time " + i)
                conn = http.client.HTTPConnection("127.0.0.1", port=sp)
                payload = json.dumps({
                  "scenarioPath": sc,
                  "pauseOnStart": False
                })
                headers = {
                  'Content-Type': 'application/json'
                }
                try:
                    conn.request("POST", "/api/play/run", payload, headers)
                except ConnectionRefusedError:
                    conn.close()
                    time.sleep(3)
                    continue
                res = conn.getresponse()
                data = json.loads(res.read().decode("utf-8"))
                if data["code"] == 0:
                    print("launch scenario @ port " + str(sp))
                else:
                    print("scenario launch failed: [{}] {}".format(data["code"], data["msg"]))
                conn.close()
                break


def poll_scenario_run_status(serv_ports):
    finished_port = set()
    while len(finished_port) != len(serv_ports):
        time.sleep(3)
        for sp in serv_ports:
            if sp in finished_port:
                continue
            conn = http.client.HTTPConnection("127.0.0.1", port=sp)
            conn.request("GET", "/api/play/poll", "", {})
            res = conn.getresponse()
            data = json.loads(res.read().decode("utf-8"))
            conn.close()
            if data["code"] == 100:
                if not data["scenario_running"]:
                    finished_port.add(sp)
                    print("scenario run finished @ port " + str(sp))
            else:
                print("poll scenario status failed: [{}] {}".format(data["code"], data["msg"]))
    print("all sim instance finished running scenario.")


if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser()
    # 指定临时目录
    arg_parser.add_argument("--rtdir", required=True,
                            help="specify which direcotry the temporary runtime data written into.")
    # 指定场景目录
    arg_parser.add_argument("--scenario", required=True,
                            help="specify which scenario(absolute file path) to run.")
    # 指定并行数量
    arg_parser.add_argument("--parallel", default=2, type=int,
                            help="specify how many simulation instance run on parallel.")
    # 指定起始端口号
    arg_parser.add_argument("--ports", default="",
                            help="specify which port the simulation services use. comma separated. "
                            "must be the same number as the --parallel specified.")
    args = arg_parser.parse_args()

    service_ports = []
    if len(args.ports) == 0:
        for i in range(args.parallel):
            service_ports.append(TXSIM_DEFAULT_SERVICE_PORT_START + i + 1)
    else:
        service_ports = list(map(int, args.ports.split(",")))
        for port in service_ports:
            if port <= 0 or port > 65535:
                sys.exit("the specified port is invalid: " + str(port), file=sys.stderr)
        if len(service_ports) != args.parallel:
            sys.exit("the number of port specified is not the same as the --parallel.", file=sys.stderr)

    sim_dirs = prepare_run_env(args.rtdir, args.parallel)
    sim_procs = launch_sim_services(sim_dirs, service_ports)
    run_scenarios(service_ports, [args.scenario])
    poll_scenario_run_status(service_ports)
    for p in sim_procs:
        p.terminate()
