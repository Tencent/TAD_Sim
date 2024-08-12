# Display-Lidar

## 1. 背景

主要负责激光相关的传感器仿真：

* 模拟激光雷达的工作过程
* 不同型号的内参和UDP封装方法
* 三种探测方式：raycast、depth和raytracing
* 队列异步处理

## 2. 系统结构

### 2.1 整体结构

整体的传感器框架如下，详细定义可参考上一级的README，这里只展示 lidar 的结构。

1. capture method：它是一个基本类，定义了通用的属性和方法。Raycast类、Raytracing 类和Depth类分别继承了Method类，并实现了不同的渲染和数据获取功能。
2. Lidar：它是一个基类，定义了 lidar 工作方式和 udp 封装方法。Hesai 和 RS 不仅仅是一个类，它表示禾赛和速腾雷达厂商的不同型号的多个类，主要定义了这些雷达型号的内参和工作参数。

```plantuml
@startuml

rectangle InitSensor
rectangle Update
rectangle "获取方法" as Method 
rectangle 打包UDP

rectangle Lidar
rectangle 禾赛
rectangle 速腾聚创

InitSensor -down->> Lidar
Lidar -down->> 禾赛
Lidar -down->> 速腾聚创
InitSensor -down->> Method
InitSensor -> Update
Update <---> Update
Update <-down-> 打包UDP

rectangle Raycast
rectangle Depth
rectangle Raytracing

Method -down->> Raycast
Method -down->> Depth
Method -down->> Raytracing


@enduml
```

### 2.2 型号接口

大部分激光lidar都是旋转或扫描式，像二维图像一样，上下由N个射线获取，水平由机械旋转或微电子扫描获取，所以一帧点云数据有很明显的二维特性。相关名词解释：

* RaysNum线数：激光束的个数。从上到下的数据量。
* channel：顾名思义，激光的通道，指的是哪一条光线
* block：一次扫描，从上到下的RaysNum个channel组成一个block
* 水平分辨率：水平方向上，两次扫描的最小水平角度间隔

接口采用**抽象工厂模式**，接口有：

* RaysNum: 定义激光线数
* HorizontalScanMinUnit: 水平最小单元的扫描个数，这个值和udp包的格式有关，等于udp包包含的block个数。这个值用于保证每次扫描的数据可以完整的封装到upd包里面，而不会有多余的数据。
* RotationFrequency: 帧率，Hz
* Range: 探测距离
* ReturnMode：返回模式，目前只有单回波，按照最强回波处理
* YawPitchAngle：输入水平刻度和垂直channel好，返回该射线的pitch和yaw角度

```plantuml
@startuml

class Lidar {
+void {abstract} getType()
}

class TraditionalLidar {
GetID()
+{abstract} SetID(int id)
+{abstract} getRaysNum()=0
+{abstract} getHorizontalScanMinUnit()=0
+{abstract} getRotationFrequency()=0
+{abstract} getRange()=0
+{abstract} getReturnMode()=0
+{abstract} getYawPitchAngle(uint32_t pos, uint32_t r)=0
getHorizontalScanCount()
getYawPitchAngle()
getLaserRadius()
getLaserHeight()
}

Lidar <|- TraditionalLidar
TraditionalLidar <|-up- HSLidar
TraditionalLidar <|-down- RSLidar
TraditionalLidar <|- RSSLidar
HSLidar <|-up- HSLidar32
HSLidar <|-up- HSLidar128
HSLidar <|-up- HSLidar128AT
HSLidar <|-up- "other HS"

RSLidar <|-down- RSLidar16
RSLidar <|-down- RSLidar32
RSLidar <|-down- RSRuby
RSLidar <|-down- "other RS"
RSSLidar <|- RSM1
@enduml
```

RSM1的工作原理比较特殊，它的射线角度不在满足扫描式的特点，所以它的角度定义包含了单帧所有的角度，而其他型号的角度定义只有每个block的就可以了。

### 2.3 Method接口

Lidar模型需要环境数据的输入，才能输出当前的仿真结果。环境输入需要：

* 深度（距离）
* 表面法向
* 物体标签

目前支持3种环境数据获取方式：

* Raycast：UE提供的API方法，效率低，在CPU端运行
* Depth：基于深度渲染的方法，用Camera的方式模拟激光扫描，通过编写PP材质，计算每个像素对应的环境数据
* Raytracing：光线追踪方式，定义每条光线，直接获取环境数据，和Raycast的方式差不多，但它在Shader中编写，运行在GPU中。目前只有Windows的DX12提供了光线追踪的API支持。

#### 2.3.1 类接口

Method接口采用**抽象工厂模式**，接口有：

* Init: 根据配置初始化，比如Depth需要将当前的fov拆成多个Camera
* GetTBuffer: 根据当前的时间信息，获取buffer数据，其中buffer是中间数据，也是抽象的，用于GetPoints。
* GetPoints: 将buffer仿真成点云

```plantuml
@startuml

class LidarBufferFun {
+{abstract} Init=0
+{abstract} GetTBuffer=0
+{abstract} GetPoints=0
}

LidarBufferFun <|-down- ALidarBufferDepth
LidarBufferFun <|-down- ALidarBufferRaycast
LidarBufferFun <|-down- ALidarBufferRayTracing

@enduml
```

#### 2.3.2 调用流程

```plantuml
@startuml

rectangle step
rectangle timespan
rectangle getTbuffer
rectangle "data queue" as queue

step -> timespan
timespan -> getTbuffer
getTbuffer -> queue

rectangle "send worker" as send
rectangle getPoints
queue -down-> getPoints
getPoints -right-> send
send -> UDP

@enduml
```

1. 过程定义
    * step：每一步长的开始
    * timespan：计算当前步长的扫码区间、
    * getTbuffer：获取渲染数据，这个数据是抽象的，不同渲染方法的类型是不一样的，比如 depth和 Raytracing 获取到的是深度编码数据，raycast方法获取的数据为 材质数据。
    * data queue：（简称queue）定义了数据处理队列，由另一个线程执行。
    * send worker（简称send）定义了数据打包队列，由另一个线程执行。
    * udp：封装成 udp 数据，并发送出去

2. 线程说明
   * queue：一般为多个线程，用于处理每个时间片段的数据，将渲染编码数据转成点云数据
   * send：一个线程，保证 udp 数据的先后顺序

#### 2.3.3 编码方法

Raycast方法的代码直接在cpp中实现，而后两者的代码运行在GPU中，所以需要将所需的环境数据编码在RGBA中，才可以进行下一步的建模仿真。

* 1 距离按照5mm为单位，转换成整数，占用16位整数，最大65535，表示327米
* 2 高8位存储在B通道
* 3 低8位存储在G通道
* 4 sin(θ)的值归一化到255，存储在R通道
* 5 标签值（场景tag）储存在A通道

### 2.4 UDP Package

UDP Package设计为生产消费者模式，池子最多容纳100个（step产生的）数据体，该线程负责两个事情：

* 管理其他异步的多线程进行udp封包处理，并维持先后顺序，最后，send（udp协议）数据
* 单帧数据搜集，用于publish message

## 3. 核心机制

### 3.1 衰减模型

激光的衰减模型：

$$
P_{\mathrm{r}}=\eta_{\mathrm{syst}} \cdot \frac{E_{\mathrm{p}}}{2 \tau} \cdot {\rho} \cdot \frac{\pi A_{\mathrm{r}}^{2}}{R^{2}} \cdot e^{-2 {\sigma}_{\mathrm{aer}} R}

$$

* Pr为激光雷达接收到的回波信号功率，
* Ep为激光雷达发射信号功率，
* R为目标与激光雷达之间的距离，
* ηsys为激光雷达系统参数（光学因子）， 为信号在大气传输过程中的影响因子，
* ρ是BRDF的反射率,
* Ar是孔径半径，
* τ=2

令

$$
A=\frac{(πA_r^2)} 4 η_{sys} η_{atm}
$$

化简后，

$$
P_r=E_p\times A\times\cos\theta\times \frac \rho {R^2}
$$

* Pr是接收功率（强度），
* Ep发射功率，
* R是距离，
* ρ是正面反射率,
* A是综合系数，
* θ是入射与物体法射夹角
  
论文结论：

* 激光雷达回波强度数据与目标的漫反射（反射率）线性相关
* 激光雷达回波强度与出射角成余弦关系
* 实际上，近距离处的强度值和R2不成反比，而在距离较远时，与R^2成反比。不同lidar的临界R不同
* 强度噪声具有加性高斯白噪声的特征

### 3.2 反射率模型

$$
f=f_{min}+(f_{max}-f_{min})\times \lim_{0 \to 1} \mu
$$
其中

$$
\mu = \varphi + rand_{±σ}
$$

φ满足布朗运动

![Brownian motion][BrownianMotion]

## 4. 数据库存储设计

无

## 5. 核心接口

### 5.1 配置输入

|名称|Name|默认值|范围|含义|
|---|---|---|---|---|
型号|Type|RS32|RS32，RS16，RSBpearl ，RSHelios，RSRuby，RSM1，RSM1P，HS32，HS40，HS64，HS64QT，HS128，HS128AT，Velodyne32, User|型号
频率(Hz)|Frequency|10|5 | 10 | 20，Model = User 时此项可手动填 [1,128]|激光雷达旋转频率。具体特定型号时，只能选择5、10、20hz，User型号时频率开放
IP|IP|127.0.0.1（单机版）||接收者IP，空时不发送udp
端口|Port|2000,2001（单机版）|[1025, 60000]|端口，速腾需要2个，禾赛需要1个
衰减标定值|Attenuation|0|[0.0,1.0) 小数点后8位|不同型号的标定值不一样，填写后会覆盖默认值
角度标定数据|AngleDefinition|||角度定义文件，填写后会覆盖默认值
大气消光系数|ExtinctionCoe|0|[0.0,1.0]|Atmospheric Extinction Coefficient
线数|RayNum|16|[1,320]|
半径范围(m)|Radius|150|(0.0,1000.0]|
反射率类型|ReflectionType|0|反射率0，tag值1，衰减值2|
水平分辨率(°)|HorizontalRes|1|(0.0,360.0]|
上仰角(°)|FovUp|10|(0.0,90.0]|
下仰角(°)|FovDown|10|(0.0,90.0]|
起点角度(°)|FovStart|60|(0.0,360.0]|向左
结束角度(°)|FovEnd|60|(0.0,360.0]|向右

### 5.2 点云输出

#### 5.2.1 PB方式

点云会按照单帧的形式填充到PB数据结构中。

``` proto

// 当前图像位置和姿态（外参）
message SensorPose {
  double longitude = 1;
  double latitude = 2;
  double altitude = 3;
  float roll = 4;
  float pitch = 5;
  float yaw = 6;
}


// lidar数据
message LidarRaw {
  message Point{
    float x = 1;
    float y = 2;
    float z = 3;
    float i = 4; // intensity
    uint32 t = 5; // type
  }  
  enum CoordType{
    COORD_SENSOR =0;
    COORD_VEHICLE = 1;
    COORD_ENU = 2;
  }

  int32 id = 1;  
  double timestamp_begin = 2; // begin of this frame
  double timestamp_end = 3; // end of this frame
  repeated Point points = 4; // Low speed: serialization & deserialization of Point
  bytes point_lists = 5; //Fast speed: stored in binary array, the bytes size = count * sizeof(Point)
  uint32 count = 6; // point count  
  SensorPose pose_first = 7;
  SensorPose pose_last = 8;
  CoordType coord_type = 9; 

}

```

SensorPose 详细定义：

|字段|类型|单位|说明|
|----|----|----|----|
|longitude|double|角度|经度|
latitude|double|角度|纬度
altitude|double|米|高程
roll|float|弧度|滚角
pitch|float|弧度|俯仰角
yaw|float|弧度|航向角

LidarRaw 详细定义：

|字段|类型|单位|说明|
|----|----|----|----|
Point.x|float|米|单帧局部坐标x
Point.y|float|米|单帧局部坐标y
Point.z|float|米|单帧局部坐标z
Point.i|float||反射率
Point.t|uint||类型,点云标签值|
id|int32||传感器ID
timestamp_begin|double|毫秒|开始时间戳
timestamp_end|double|毫秒|结束时间戳
points|repeated Point||点云,(repeat序列化很慢，基本不用。)
point_lists|bytes||点云的二进制形式,(速度快，按照float数组存储，一个点按照Point里字段的顺序占用5个float，反射率也是float)
count|uint||点数
pose_first|SensorPose ||初始pose
pose_last|SensorPose ||最后pose
coord_type|CoordType枚举||COORD_SENSOR =0;COORD_VEHICLE = 1;COORD_ENU = 2;

#### 5.2.2 UDP方式

点云会按照相应的型号定义，以UDP网包的形式发送出来，具体格式需要参考个厂商的规格定义。

## 6. 主要业务时序

### 6.1 初始化

初始化流程：首先，SimSDK要求LidarSensor组件进行初始化；然后，LidarSensor组件分别要求TraditionalLidar和LidarBufferFun组件进行类型和方法的初始化；接着，TraditionalLidar要求RS或Hesai组件进行打包初始化；最后，LidarBufferFun要求Depth或Raytracing组件进行数据获取的初始化。

```plantuml
@startuml
SimSDK -> LidarSensor: init
LidarSensor -> TraditionalLidar: 初始化型号
TraditionalLidar -> "RS or Hesai": 初始化打包
LidarSensor -> LidarBufferFun: 初始化方法
LidarBufferFun -> "Depth or Raytracing": 初始化探测方法
@enduml
```

该过程描述了系统中不同组件之间的初始化关系。

1. 有五个组件：SimSDK、LidarSensor、TraditionalLidar、LidarBufferFun和Depth或Raytracing。
2. SimSDK组件向LidarSensor组件发送init消息：SimSDK要求LidarSensor进行初始化。
3. LidarSensor组件收到init消息后，向TraditionalLidar组件发送init type消息：LidarSensor要求TraditionalLidar组件进行类型初始化。
4. TraditionalLidar组件收到init type消息后，向RS或Hesai组件发送init package消息：TraditionalLidar要求RS或Hesai组件进行相应的包初始化。
5. LidarSensor组件还向LidarBufferFun组件发送init method消息：LidarSensor要求LidarBufferFun组件进行方法初始化。
6. LidarBufferFun组件收到init method消息后，向Depth或Raytracing组件发送init capture消息：LidarBufferFun要求Depth或Raytracing组件进行数据获取初始化。

### 6.2 Step更新

该过程描述了系统中不同组件之间的更新、计算、缓冲和发布关系，以及SimSDK的逐步执行过程。首先，SimSDK要求Update组件进行更新操作；然后，Update组件要求Scan Range进行计算，并获取LidarBufferFun的缓冲数据；接着，LidarBufferFun将缓冲数据推送到Buffer queue中，Buffer queue要求Points进行数据模拟；最后，Points将模拟结果通过网络发布，并通知Update完成一帧数据的处理，Update将处理结果发布给SimSDK，SimSDK按照逐步执行的方式处理更新和发布的结果。

```plantuml
@startuml
SimSDK -> Update: step
Update -> "Scan Range": 计算探测范围
"Scan Range" -> LidarBufferFun: 获取buffer
LidarBufferFun -> "Buffer queue": buffer放入队列

"Buffer queue" -> Points: 模型仿真
Points -> UDP: UDP广播
Points -> Update: 单帧融合
Update -> SimSDK: 广播

SimSDK -> SimSDK: step by step

@enduml
```

1. 有七个组件：SimSDK、Update、Scan Range、LidarBufferFun、Buffer queue、Points和UDP。
2. SimSDK组件向Update组件发送step消息：SimSDK要求Update进行更新操作。
3. Update组件收到step消息后，向Scan Range组件发送calculate消息：Update要求Scan Range计算当前步长的扫描范围。
4. Scan Range组件收到计算消息后，向LidarBufferFun组件发送get buffer消息：Scan Range要求LidarBufferFun获取缓冲数据。
5. LidarBufferFun组件收到get buffer消息后，向Buffer queue组件发送push buffer消息：LidarBufferFun将缓冲数据推送到Buffer queue中。
6. Buffer queue组件向Points组件发送simulation消息：Buffer queue要求Points进行点云数据模拟。
7. Points组件向UDP组件发送net publish消息：Points将模拟结果通过udp 广播发布。
8. Points组件向Update组件发送one frame消息：Points完成一帧数据的处理。
9. Update组件向SimSDK组件发送publish消息：Update将处理结果发布给SimSDK。
10. SimSDK重复以上过程，连续更新和发布的结果。

## 7. 数据埋点

无

## 8. 日志格式

无，只有异常情况下才会输出警告或错误信息。

## 9. 业务状态

无

## 10. 代码结构

```
LidarSensors                                                  
│  DepthCamera.cpp                              depth方法采用的环境相机，由多个camera拼成360°
│  DepthCamera.h                                                                    
│  LidarSensorDef.h                             基础接口定义
│  TLidarBufferDepth.cpp                        depth方法
│  TLidarBufferDepth.h                          
│  TLidarBufferRaycast.cpp                      Raycast（射线）方法
│  TLidarBufferRaycast.h                        
│  TLidarBufferRayTracing.cpp                   RayTracing（光线追踪）方法
│  TLidarBufferRayTracing.h                     
│  TLidarSensor.cpp                             Lidar传感器核心业务
│  TLidarSensor.h                               
│
└─lidar                                         具体的型号目录
    │  HSLidar.cpp                              禾赛
    │  HSLidar.h                                
    │  Lidar.cpp                                型号接口
    │  Lidar.h                                  
    │  LidarModel.cpp                           三大模型：衰减、天气、反射率
    │  LidarModel.h                             
    │  RSLidar.cpp                              速腾聚创
    │  RSLidar.h                                
    │  USLidar.cpp                              自定义固态
    │  USLidar.h                                
    │  UTLidar.cpp                              自定义机械旋转
    │  UTLidar.h                                
    │  VeloLidar.cpp                            Velodyne
    │  VeloLidar.h                              
    │
    └─fbm                                       布朗运动算法，来自于https://github.com/ADDMD/Terrain-Generation
        │  fbm.hpp                              
        │  generate_normal_random.hpp           
        │
        └─impl                                  
                data_1d.hpp                     
                data_1d_out.hpp                 
                fbm_inl.hpp                     
                generate_normal_random_inl.hpp                                               
                is_power_of_two.hpp                                               
                variance_fbm_1d.hpp    
```

## 11. 开发调试与发布

### 11.1 安装UE

参考官网[安装虚幻引擎](https://docs.unrealengine.com/4.27/zh-CN/Basics/InstallingUnrealEngine/)

### 11.2 安装Visual Studio（推荐2019）

参考官网[安装 Visual Studio](https://learn.microsoft.com/zh-cn/visualstudio/install/install-visual-studio?view=vs-2019)，选择c++桌面开发

可选设置：[设置虚幻引擎的Visual Studio](https://docs.unrealengine.com/4.27/zh-CN/ProductionPipelines/DevelopmentSetup/VisualStudioSetup/)

### 11.3 构建Display工程

* 方法1：右键Display.uproject，选择Generate Visual Studio Project Files
* 方法2：C:\Program Files\Epic Games\UE_4.27\Engine\Binaries\DotNET\UnrealBuildTool.exe Display.uproject
* 方法3：执行GenerateProjectFiles.bat
  
### 11.4 编译

有4种type可选：

* Debug Game：调试运行，用于调试c++代码
* Debug Editor：调试运行Editor，用Editor打开工程，用于调试Editor工具
* Development Game：发布运行，无调试信息
* Development Editor：运行Editor，用Editor打开工程。等同于直接打开Display.uproject后提示的编译过程。
  
### 11.5 调试

1. 执行PakMapsConfig.exe 三维地图id，如PakMapsConfig.exe 44+1001，设置当前场景
2. 编译Editor
3. 用Editor打开工程：直接打开Display.uproject
4. 选择菜单-文件-cook for windows，等待结束
5. tadsim克隆一个Display模块，取消勾选自动启动
6. 切换到Debug Game，右键Display工程属性，设置调试参数-name=Display_clone
7. tadsim运行场景
8. 调试运行Display工程，等待连接成功

### 11.6 发布

执行Build.bat，参数为地图编号，例如Build.bat 44+1001，表示构建44号和1001号的美术场景。

[BrownianMotion]:data:image/png;base64,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
