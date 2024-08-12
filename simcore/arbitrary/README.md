# arbitrary 仲裁模块

## 1. 背景

arbitrary 模块主要完成横纵向控制及控制模式的选择, 需配合驾驶员模型以及动力学一起使用.

## 2. 系统结构

驾驶员模型和 ADAS 算法都按照 control_V2.proto 进行消息发送, 仲裁模块通过对应信号进行选择, 其中 ADAS 算法的优先级会高于驾驶员模型.

<div align="center"><img src="./images/arbitrary_arch.png" alt="" width=300px"></div>

