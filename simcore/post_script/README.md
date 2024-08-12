# 1. 目标

TAD Sim 仿真工具在运行结束后, 会在 tadsim\service_data\sim_data\pblog 文件夹下生成与场景名相对应的 .pblog 文件.
这些文件中存储了仿真运行过程中各个模块之间的交互数据.
本文档将指导您如何使用 Python 脚本读取 .pblog 文件并解析仿真过程中的交互数据.
仿真数据的录制可参考TADSim使用说明中的protobuf_logger模块.

# 2. 环境

确保您的系统中已安装了 Python3, 同时安装 glog, grpcio-tools, pyinstaller 以及新增程序需要的第三方库.

```bash
python3 -m pip install glog grpcio-tools pyinstaller
```
