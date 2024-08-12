"""
测试脚本
"""

import re
import os
import errno
import subprocess

# 配置文件后缀名
TXSIM_PERF_CONFIG_SUFFIX = ".cfg.in"
# 配置文件名称
TXSIM_LOCAL_SERVICE_CONFIG_NAME = "local_service.config"
# 数据库文件名称
TXSIM_CONFIG_DB_NAME = "service.sqlite"
# 测试脚本文件名称
TXSIM_BUILD_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), "../build"))
# 测试脚本文件名称
TXSIM_PERF_RUN_DIR_BASE = os.path.join(TXSIM_BUILD_DIR, "perf_runs")
# 调度服务二进制文件路径
TXSIM_LOCAL_SERVICE_BIN_PATH = os.path.join(
    TXSIM_BUILD_DIR, "bin", "txsim-local-service")
# 调度服务二进制文件路径
TXSIM_PERF_TEST_BIN_PATH = os.path.join(TXSIM_BUILD_DIR, "bin", "perf-test")

"""
备注:

参数:

返回值:

"""


def remove_if_exist(file_path):
    try:
        os.remove(file_path)
    except OSError as e:
        if e.errno != errno.ENOENT:
            raise
"""
备注:

参数:

返回值:

"""


def mkdir_if_not_exist(dir_path):
    try:
        # 创建目录
        os.mkdir(dir_path)
    except FileExistsError:
        pass
"""
备注:

参数:

返回值:

"""


def launch_test_load(config_name, case_count, template_values, key_printer):
    # 获取运行目录
    run_dir = os.path.join(TXSIM_PERF_RUN_DIR_BASE, config_name)
    # 创建文件夹
    mkdir_if_not_exist(run_dir)
    run_log_dir = os.path.join(run_dir, "logs")
    # 创建目录文件夹
    mkdir_if_not_exist(run_log_dir)
    # 打开文件 with语句结束后释放上下文管理器
    with open(os.path.join(os.path.dirname(__file__), (config_name + TXSIM_PERF_CONFIG_SUFFIX)), 'r') as input_cfg, \
            open(os.path.join(run_dir, "result"), 'w') as rst_file:
        config = input_cfg.read().replace("@{buildDir}", TXSIM_BUILD_DIR)
        config = config.replace("@{test_name}", config_name)
        rst_file.write("varible: mean_step_period mean_step_exec_time min_step_period max_step_period "
            "min_step_exec_time max_step_exec_time\n")
        ret_code = 0
        for i in range(0, case_count, 1):
            cur_config = config
            v = ""
            for k, v_list in template_values.items():
                cur_config = cur_config.replace(k, v_list[i])
                if len(v) == 0:
                    v = v_list[i]
            with open(os.path.join(run_dir, TXSIM_LOCAL_SERVICE_CONFIG_NAME), 'w') as output_cfg:
                output_cfg.write(cur_config)
            print("running %s with setup<%s> ..." % (config_name, v))
            try:
                service_proc = subprocess.Popen(
                    [TXSIM_LOCAL_SERVICE_BIN_PATH, "--root", run_dir, "--logdir", run_log_dir])
                ret = subprocess.run(
                    TXSIM_PERF_TEST_BIN_PATH, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                ret_code = ret.returncode
                if ret_code == 0:
                    rst_file.write("{:<14}".format("" + key_printer(v) + ":") +
                                   ret.stdout.decode('utf-8') + "\n")
                else:
                    print(ret.stderr)
                service_proc.terminate()
            finally:
                try:
                    service_proc.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    service_proc.kill()
            if ret_code != 0:
                break
            remove_if_exist(os.path.join(run_dir, TXSIM_CONFIG_DB_NAME))
"""
备注: main函数,脚本文件入口函数

参数:

返回值:

"""


if __name__ == "__main__":
    mkdir_if_not_exist(TXSIM_PERF_RUN_DIR_BASE)
    # 测试用例
    launch_test_load("test_load_message_size", 12,
                     {"@{msg_size}": ["64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768",
                                      "65536", "131072"]},
                     lambda k: k + " bytes")
    # 测试用例
    launch_test_load("test_load_topic_size", 8,
                     {"@{sub_topics}": ["t1", "t1,t2", "t1,t2,t3", "t1,t2,t3,t4", "t1,t2,t3,t4,t5", "t1,t2,t3,t4,t5,t6",
                                        "t1,t2,t3,t4,t5,t6,t7", "t1,t2,t3,t4,t5,t6,t7,t8"],
                      "@{pub_topics}": ["t2", "t3,t4", "t4,t5,t6", "t5,t6,t7,t8", "t6,t7,t8,t9,t10",
                                        "t7,t8,t9,t10,t11,t12", "t8,t9,t10,t11,t12,t13,t14",
                                        "t9,t10,t11,t12,t13,t14,t15,t16"]},
                     lambda k: re.match('.*t([0-9]+)', k).group(1) + " topics")
    # 测试用例
    launch_test_load("test_load_module_size", 8,
                     {"@{moduleNames}": ["\"m1\"", "\"m1\",\"m2\"", "\"m1\",\"m2\",\"m3\"",
                                         "\"m1\",\"m2\",\"m3\",\"m4\"", "\"m1\",\"m2\",\"m3\",\"m4\",\"m5\"",
                                         "\"m1\",\"m2\",\"m3\",\"m4\",\"m5\",\"m6\"",
                                         "\"m1\",\"m2\",\"m3\",\"m4\",\"m5\",\"m6\",\"m7\"",
                                         "\"m1\",\"m2\",\"m3\",\"m4\",\"m5\",\"m6\",\"m7\",\"m8\""]},
                     lambda k: re.match('.*m([0-9]+)', k).group(1) + " modules")
