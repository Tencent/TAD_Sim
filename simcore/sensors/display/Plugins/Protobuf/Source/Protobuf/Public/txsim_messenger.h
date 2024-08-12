#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>


/// @cond
#define TXSIM_CALLBACK_SET_ERROR(data, cbs, err)   ((void (*)(void*, const char*))cbs[0])(data, err)
#define TXSIM_CALLBACK_INIT_PARAMETER(key)         ((const char* (*)(void*, const char*))cbs_[1])(data_, key)
#define TXSIM_CALLBACK_INIT_SUBSCRIBE(topic)       ((void (*)(void*, const char*))cbs_[2])(data_, topic)
#define TXSIM_CALLBACK_INIT_PUBLISH(topic)         ((void (*)(void*, const char*))cbs_[3])(data_, topic)
#define TXSIM_CALLBACK_INIT_SUBSCRIBE_SHMEM(topic) ((void (*)(void*, const char*))cbs_[4])(data_, topic)
#define TXSIM_CALLBACK_INIT_PUBLISH_SHMEM(topic, max_size) \
  ((void (*)(void*, const char*, uint32_t))cbs_[5])(data_, topic, max_size)

#define TXSIM_CALLBACK_RESET_MAP_FILE_PATH() ((const char* (*)(void*))cbs_[1])(data_)
#define TXSIM_CALLBACK_RESET_MAP_LOCAL_ORIGIN(lon, lat, alt) \
  ((void (*)(void*, double*, double*, double*))cbs_[2])(data_, lon, lat, alt)
#define TXSIM_CALLBACK_RESET_EGO_DESTINATION(lon, lat, alt) \
  ((void (*)(void*, double*, double*, double*))cbs_[3])(data_, lon, lat, alt)
#define TXSIM_CALLBACK_RESET_EGO_SPEED_LIMIT()      ((double (*)(void*))cbs_[4])(data_)
#define TXSIM_CALLBACK_RESET_LOCATION_RAW_MSG(raw)  ((uint32_t(*)(void*, const char**))cbs_[5])(data_, raw)
#define TXSIM_CALLBACK_RESET_GEO_FENCE(point_array) ((uint32_t(*)(void*, double*))cbs_[6])(data_, point_array)
#define TXSIM_CALLBACK_RESET_SCENARIO_FILE_PATH()   ((const char* (*)(void*))cbs_[7])(data_)
#define TXSIM_CALLBACK_RESET_EGO_PATH(point_array)  ((uint32_t(*)(void*, double*))cbs_[8])(data_, point_array)
#define TXSIM_CALLBACK_RESET_VEHICLE_MEASUREMENT_RAW_MSG(idx, raw) \
  ((uint32_t(*)(void*, int64_t*, const char**))cbs_[9])(data_, idx, raw)
#define TXSIM_CALLBACK_RESET_SCENCE_PB(raw)  ((uint32_t(*)(void*, const char**))cbs_[10])(data_, raw)
#define TXSIM_CALLBACK_RESET_SETTING_PB(raw)  ((uint32_t(*)(void*, const char**))cbs_[11])(data_, raw)
#define TXSIM_CALLBACK_RESET_GROUP_NAME()    ((const char* (*)(void*))cbs_[12])(data_)


#define TXSIM_CALLBACK_STEP_SUBSCRIBED_RAW_MSG(topic, msg) \
  ((uint32_t(*)(void*, const char*, const char**))cbs_[1])(data_, topic, msg)
#define TXSIM_CALLBACK_STEP_PUBLISH_RAW_MSG(topic, payload, len) \
  ((void (*)(void*, const char*, const char*, uint32_t))cbs_[2])(data_, topic, payload, len)
#define TXSIM_CALLBACK_STEP_STOP_SCENARIO(reason) ((void (*)(void*, const char*))cbs_[3])(data_, reason)
#define TXSIM_CALLBACK_STEP_TIMESTAMP()           ((double (*)(void*))cbs_[4])(data_)
#define TXSIM_CALLBACK_STEP_SUBSCRIBED_SHMEM_DATA(topic, mem_addr) \
  ((uint32_t(*)(void*, const char*, const char**))cbs_[5])(data_, topic, mem_addr)
#define TXSIM_CALLBACK_STEP_GET_PUBLISHED_SHMEM_BUFFER(topic, mem_addr) \
  ((uint32_t(*)(void*, const char*, char**))cbs_[6])(data_, topic, mem_addr)
#define TXSIM_CALLBACK_STEP_CURRENTTIMESTAMP() ((double (*)(void*))cbs_[7])(data_)

#define TXSIM_CALLBACK_STOP_SET_FEEDBACK(key, value) \
  ((void (*)(void*, const char*, const char*))cbs_[1])(data_, key, value)
#define TXSIM_CALLBACK_STOP_MODULE_STATUS() ((const char* (*)(void*))cbs_[2])(data_)

#define TXSIM_MESSENGER_API_VERSION 6130803

#define TXSIM_MSGER_CLS_DEF(cls) \
 private:                        \
  void* data_;                   \
  void** cbs_;                   \
                                 \
 public:                         \
  cls(void* data, void** cbs) : data_(data), cbs_(cbs) {}
/// @endcond


namespace tx_sim {

namespace topic {
/// 对应 traffic.proto ，定义场景中所有障碍物(车、行人、静态物体等)以及信号灯。
const std::string kTraffic = "TRAFFIC";
/// 对应 egoMapData.proto 或者 laneMarks.proto ，定义地图元素(车道线、车道中心线等)信息。
const std::string kLaneMark = "LANE_MARKS";
/// 对应 grading.proto ，定义场景评测指标。
const std::string kGrading = "GRADING";
/// 对应 location.proto ，定义主车定位信息。
const std::string kLocation = "LOCATION";
/// 对应 vehState.proto ，定义主车底盘相关状态信息。
const std::string kVehicleState = "VEHICLE_STATE";
/// 对应 trajectory.proto ，定义主车决策规划算法输出的轨迹信息。
const std::string kTrajectory = "TRAJECTORY";
/// 对应 control.proto ，定义主车控制相关信息。（即将废弃，请使用control_v2.proto）
const std::string kControl = "CONTROL";
/// 对应 control_v2.proto ，定义主车控制相关信息。
const std::string kControl_V2 = "CONTROL_V2";
/// 对应 planStatus.proto ，定义决策输出的相关状态信息。
const std::string kPlanStatus = "PLAN_STATUS";
/// 对应 trajectory.proto 中的 Predictions message，定义当前帧所有障碍物的预测轨迹。
const std::string kTraffic_billborad = "TRAFFIC_BILLBOARD";
/// 对应 traffic_billborad.proto 中的 Billboard message，场景中统计、导航信息。
const std::string kEnviroment = "ENVIRONMENTAL";
/// 对应 enviroment.proto 中的 EnvironmentalConditions message，环境数据结构定义。
const std::string kPredictions = "PREDICTION";
/// 对应 debugmessage.proto,用于用户输出自定义调试信息并在前端显示
const std::string kDebugMessage = "DEBUG_MESSAGE";
/// 对于多主车topic标志,比如要订阅所有主车位置数据,topic为EgoUnion/LOCATION
const std::string kUnionFlag = "EgoUnion/";
/// 仿真系统动态调整模块输出日志的等级，整数的字符串表示形式，模块输出的日志等级应小于等于该值。
const std::string kLogLevel = "_LOG_LEVEL";
}  // namespace topic

namespace constant {
/// 仿真系统在Init回调中默认传入的参数，当用户的算法以动态库形式接入时，表示系统加载的该动态库所在目录的绝对路径。
const std::string kInitKeyModuleSharedLibDirectory = "_cwd";
/// 仿真系统在Init回调中默认传入的参数，表示该模块被调用的周期，单位：毫秒(ms)。
const std::string kInitKeyModuleExecutePeriod = "_exec_period";
/// 仿真系统初始化模块输出日志的等级，整数的字符串表示形式，模块输出的日志等级应小于等于该值。
const std::string kInitKeyModuleLogLevel = "_log_level";
}  // namespace constant


/// \~Chinese 3元数组，一般用于API返回位置信息的类型。
typedef struct {
  double x, y, z;
} Vector3d;
class InitHelper {
  /// @cond
  TXSIM_MSGER_CLS_DEF(InitHelper)
  /// @endcond

 public:
  /*! \~Chinese
   * 获取用户在前端对应模块配置中添加的初始化参数键值对(key-value pair)。目前系统默认
   * 发送(无需在前端配置)的初始化参数包括：
   * \li key:\c stepTime value: 当前模块配置的step time值(单位：毫秒)，可通过std::stod转化
   * 为double类型。
   *
   * @param[in] key 初始化参数的键。
   * @return 对应该键的字符串值，如果前端没有添加相应的初始化参数键，则返回空字符串。
   */
  std::string GetParameter(const std::string& key) { return TXSIM_CALLBACK_INIT_PARAMETER(key.c_str()); }

  /*! \~Chinese
   * 向系统注册当前模块需要订阅的topic。只有通过该方法注册的订阅才会在稍后通过
   * tx_sim::StepHelper::GetSubscribedMessage 方法获取到对应topic的消息内容，否则返回的消息内容为
   * 空字符串。如果当前系统中没有模块发布此topic，系统将会报错提示。
   *
   * @param[in] topic 当前模块需要订阅的topic。
   *
   * \note TADSim仿真系统有一些内置的topic(及对应的消息格式)供用户订阅或发布，可以方便的使用
   * tx_sim::topic 中定义的常量作为参数传入，如果用户接入多个模块并且需要定义新的topic以及消息
   * 格式在用户自己的模块之间流转，则直接使用自定义的字符串作为参数传入即可，消息格式由用户自己
   * 统一，可以为字符串或者序列化后的字节数组，详见 tx_sim::StepHelper::GetSubscribedMessage 和
   * tx_sim::StepHelper::PublishMessage ，仿真系统不做解析。为了避免与系统内置的topic冲突，建议
   * 自定义的topic字符串加上定制的前缀，比如"XXX_LOCAL_LOCATION"。 #Publish 同理。
   */
  void Subscribe(const std::string& topic) { TXSIM_CALLBACK_INIT_SUBSCRIBE(topic.c_str()); }

  /*! \~Chinese
   * 向系统注册当前模块需要发布的topic。只有通过该方法注册的topic才会在稍后通过
   * tx_sim::StepHelper::PublishMessage 方法将对应topic的消息内容发布给其他模块，否则其他模块将
   * 不会收到相应消息。多个模块不能向同一个topic发布消息，否则系统将报错提示。
   *
   * @param[in] topic 当前模块需要发布的topic。
   */
  void Publish(const std::string& topic) { TXSIM_CALLBACK_INIT_PUBLISH(topic.c_str()); }

  /*! \~Chinese
   * 同 #Subscribe ，但是指定该topic对应的数据是从共享内存中得到，而不是通过默认的tcp通信传输。对指定共享
   * 内存的映射操作会延迟到 tx_sim::SimModule::Reset 方法调用之前，如果映射失败则系统会报错提示。
   *
   * @param[in] topic 当前模块需要订阅的topic。
   */
  void SubscribeShmem(const std::string& topic) { TXSIM_CALLBACK_INIT_SUBSCRIBE_SHMEM(topic.c_str()); }

  /*! \~Chinese
   * 同 #Publish ，但是指定该topic对应的数据是写入共享内存中，而不是通过默认的tcp通信传输。调用该方法后仿真
   * 系统会在稍后尝试向操作系统申请对应大小空间的共享内存（如果系统中没有对应共享内存资源）并进行初始化（所有字
   * 节清零），如有任何失败则系统会报错提示。该共享内存会在所有对其有引用的模块正常退出后销毁。
   *
   * @param[in] topic 当前模块需要发布的topic。
   * @param[in] max_memory_size 指定需要申请的共享内存的大小，单位：字节。
   */
  void PublishShmem(const std::string& topic, size_t max_memory_size) {
    TXSIM_CALLBACK_INIT_PUBLISH_SHMEM(topic.c_str(), max_memory_size);
  }
};


class ResetHelper {
  /// @cond
  TXSIM_MSGER_CLS_DEF(ResetHelper)
  /// @endcond

 public:
  /*! \~Chinese
   * 获取当前场景对应的地图文件路径。
   *
   * @return 当前场景对应的地图文件的绝对路径。
   */
  std::string map_file_path() { return TXSIM_CALLBACK_RESET_MAP_FILE_PATH(); }

  /*! \~Chinese
   * 获取当前场景对应的地图的建议原点。仿真系统中发出的障碍物及主车信息中的位置
   * 一般采用经纬度形式，通过该原点可以方便的将这些经纬度坐标转换为局部坐标系以便于
   * 计算。
   *
   * @return 当前场景对应地图的原点坐标(经度-纬度-海拔)。
   */
  Vector3d map_local_origin() {
    Vector3d rst;
    TXSIM_CALLBACK_RESET_MAP_LOCAL_ORIGIN(&rst.x, &rst.y, &rst.z);
    return rst;
  }

  /*! \~Chinese
   * 获取当前场景主车的终点位置(经纬度)。
   *
   * @return 当前场景主车的终点坐标(经度-纬度-海拔)。
   */
  Vector3d ego_destination() {
    Vector3d rst;
    TXSIM_CALLBACK_RESET_EGO_DESTINATION(&rst.x, &rst.y, &rst.z);
    return rst;
  }

  /*! \~Chinese
   * 获取当前场景限速信息。该限速区别于地图数据中不同路段的限速信息，通常是用户在
   * 场景编辑器中针对主车在当前整个场景中设置的一个最大速度，用户算法需要同时考虑
   * 地图元素和当前场景中的限速信息。
   *
   * @return 当前场景中针对主车的限速值(最大速度)，单位为米/秒(m/s)。
   */
  double ego_speed_limit() { return TXSIM_CALLBACK_RESET_EGO_SPEED_LIMIT(); }

  /*! \~Chinese
   * 获取当前场景中主车的起始定位信息。如果当前模块订阅了系统内置的定位消息
   * (topic LOCATION)，则和在第0帧的Step方法中获取到的定位消息相同。返回的结果是Google的
   * [protocol buffer] (https://developers.google.com/protocol-buffers) 序列化后的字节数组，
   * 详见 tx_sim::StepHelper::GetSubscribedMessage 。
   *
   * @return 经protocol buffer序列化后的主车初始定位消息。
   */
  const std::string ego_start_location() {
    const char* msg = nullptr;
    uint32_t len = TXSIM_CALLBACK_RESET_LOCATION_RAW_MSG(&msg);
    return std::string(msg, len);
  }

  /*! \~Chinese
   * 获取本车的地理围栏信息，由返回的一系列经纬度坐标点构成的首尾相接的闭合多边形所表示。
   *
   * @return 构成主车地理围栏边界的经纬度坐标点的有序列表，长度至少为3，首尾相接。
   */
  std::vector<Vector3d> geo_fence() {
    std::vector<Vector3d> ret;
    uint32_t len = TXSIM_CALLBACK_RESET_GEO_FENCE(nullptr);
    if (len > 0) {
      double* p = new double[len * 2];
      TXSIM_CALLBACK_RESET_GEO_FENCE(p);
      ret.reserve(len);
      for (size_t i = 0; i < len; i++) {
        ret.emplace_back();
        Vector3d& g = ret.back();
        g.x = p[i << 1];
        g.y = p[(i << 1) + 1];
        g.z = 0;
      }
      delete[] p;
    }
    return ret;
  }

  /*! \~Chinese
   * 获取当前场景对应的场景文件路径。
   *
   * @return 当前场景对应的场景文件的绝对路径。
   */
  std::string scenario_file_path() { return TXSIM_CALLBACK_RESET_SCENARIO_FILE_PATH(); }

  /*! \~Chinese
   * 获取主车路径的控制点信息，每个控制点由经纬度以及高程组成。
   * 第一个控制点为主车起点，同 #ego_start_location 中的经纬度信息，最后一个控制点为主车的终点，同
   * #ego_destination ，其余为主车路径中的途径点。
   *
   * @return 主车路径的控制点列表，列表中的元素为浮点类型的三元组，分别代表经度、纬度、高程。
   */
  std::vector<Vector3d> ego_path() {
    std::vector<Vector3d> ret;
    uint32_t len = TXSIM_CALLBACK_RESET_EGO_PATH(nullptr);
    if (len > 0) {
      double* p = new double[len * 3];
      TXSIM_CALLBACK_RESET_EGO_PATH(p);
      ret.reserve(len);
      for (size_t i = 0; i < len; i++) {
        ret.emplace_back();
        Vector3d& pp = ret.back();
        size_t si = i * 3;
        pp.x = p[si];
        pp.y = p[si + 1];
        pp.z = p[si + 2];
      }
      delete[] p;
    }
    return ret;
  }

  /*! \~Chinese
   * 获取当前场景中（多个）主车的ID和对应车辆尺寸信息，具体参考vehicle_geometry.proto中的定义。
   * 返回时measurements数组中每个二元组第二项std::string是Google的
   * [protocol buffer] (https://developers.google.com/protocol-buffers) 序列化后的字节数组，
   * 需要反序列化为vehicle_geometry.proto中定义的VehicleGeometoryList对象然后再获取各个尺寸信息，
   * 详见 tx_sim::StepHelper::GetSubscribedMessage 。
   */
  void vehicle_measurements(std::vector<std::pair<int64_t, std::string>>& measurements) {
    measurements.clear();
    size_t count = TXSIM_CALLBACK_RESET_VEHICLE_MEASUREMENT_RAW_MSG(0, nullptr);
    measurements.resize(count);
    for (int64_t i = 0; i < (int64_t)count; ++i) {
      const char* msg = nullptr;
      int64_t id = i;
      size_t len = TXSIM_CALLBACK_RESET_VEHICLE_MEASUREMENT_RAW_MSG(&id, &msg);
      std::get<0>(measurements[i]) = id;
      std::get<1>(measurements[i]).assign(msg, len);
    }
  }
  /*! \~Chinese
   * 获取scene proto
   * 返回的结果是Google的
   * [protocol buffer] (https://developers.google.com/protocol-buffers) 序列化后的字节数组
   */
   const std::string scene_pb() { 
    const char* msg = nullptr;
    uint32_t len = TXSIM_CALLBACK_RESET_SCENCE_PB(&msg);
    return std::string(msg, len); 
  }
  /*! \~Chinese
   * setting proto
   * 返回的结果是Google的
   * [protocol buffer] (https://developers.google.com/protocol-buffers) 序列化后的字节数组，反序列后返回的是相关配置，具体内容如下
   * versioin:版本号
   * scenario_path:场景文件路径
   * hadmap_path：地图路径
   * vehicle_dynamic_cfg_path：动力学参数配置路径
   * sensor_cfg_path：传感器配置路径
   * environment_cfg_path：环境参数配置路径
   * default_xosc_path：主车配置路径
   * scenario_labels：场景标签
   */
   const std::string setting_pb() { 
    const char* msg = nullptr;
    uint32_t len = TXSIM_CALLBACK_RESET_SETTING_PB(&msg);
    return std::string(msg, len); 
  }
  /*! \~Chinese
   * 获取当前主车所在的组名
   *
   *  @return 主车的组名
   *
   * \note 对于多主车仿真每个主车算法会有一个对应的组,该组名会在创建场景的时候写到场景文件里面,
   * 对于旧的场景文件没有该字段的默认返回Ego001。
   */
  const std::string group_name() { return TXSIM_CALLBACK_RESET_GROUP_NAME(); }
};


class StepHelper {
  /// @cond
  TXSIM_MSGER_CLS_DEF(StepHelper)
  /// @endcond

 public:
  /*! \~Chinese
   * 获取当前模块订阅的消息内容。如何解析消息内容取决于发布该消息的模块如何编码该
   * 消息，仿真系统内置的topic对应的消息格式均由Google的
   * [Protocol Buffer] (https://developers.google.com/protocol-buffers)定义，因此获取到的仿真系统内置的
   * 消息是经过protobuf序列化后的字节数组。\n
   * 对于仿真系统内置的消息，可以通过protobuf库进行解析：
   * \code
   * std::string payload;
   * GetSubscribedMessage(tx_sim::topic::kTraffic, payload);
   * sim_msg::Traffic traffic_msg;
   * traffic_msg.ParseFromString(payload);
   * // using traffic_msg through generated protobuf getter methods...
   * \endcode
   * 对于用户自定义的消息格式，如果是简单的字符串(以'\0'结尾)，则可以通过该方法获取
   * 到字符串payload后直接使用，如果是序列化后的字节数组(按照 #PublishMessage 中的方法填入)，
   * 则可以如下解析：
   * \code
   * std::string payload;
   * GetSubscribedMessage("MY_CUSTOM_TOPIC", payload);
   * const char* buffer = payload.data();
   * size_t len = payload.size();
   * // using serialized byte array via buffer and len variables...
   * \endcode
   *
   * @param[in] topic 要获取的订阅消息对应的topic。
   * @param[out] payload 获取的订阅消息的内容(字符串或序列化后的字节数组)。
   */
  void GetSubscribedMessage(const std::string& topic, std::string& payload) {
    const char* msg = nullptr;
    uint32_t len = TXSIM_CALLBACK_STEP_SUBSCRIBED_RAW_MSG(topic.c_str(), &msg);
    payload.assign(msg, len);
  }

  /*! \~Chinese
   * 向系统发布消息，以使其他模块可以获取此消息。只有通过 tx_sim::InitHelper::Publish 注册
   * 的topic对应的消息才可以被发布到仿真系统中。发布的消息内容可以是字符串或者序列化后的字节
   * 数组，如何解析需要提前和接收此消息的模块协商好，对于需要和仿真系统交流的消息，需要采用
   * TADSim提供的内置消息类型进行序列化后发布。
   * 对于仿真系统内置的消息，可以通过protobuf库进行序列化：
   * \code
   * std::string payload;
   * traffic_msg.SerializeToString(&payload); // traffic_msg is sim_msg::Traffic type which provided by TADSim.
   * PublishMessage(tx_sim::topic::kTraffic, payload);
   * \endcode
   * 对于用户自定义的消息格式，如果是简单的字符串(以'\0'结尾)，则可以直接将字符串传入：
   * \code
   * PublishMessage("MY_CUSTOM_TOPIC", "Hello, my friend!");
   * \endcode
   * 如果是序列化后的字节流，则可以按如下方式发布：
   * \code
   * uint8_t buffer[512];
   * // size_t len = ... (the actual length of the serialized data)
   * // say we copy len data into this buffer here. be sure the len <= 512 that we allocated earlier.
   * std::string payload;
   * payload.assign(buffer, len);
   * PublishMessage("MY_CUSTOM_TOPIC", payload);
   * \endcode
   *
   * @param[in] topic 要发布的消息对应的topic。
   * @param[in] payload 发布的消息内容(字符串或序列化后的字节数组)。
   */
  void PublishMessage(const std::string& topic, const std::string& payload) {
    TXSIM_CALLBACK_STEP_PUBLISH_RAW_MSG(topic.c_str(), payload.c_str(), payload.size());
  }

  /*! \~Chinese
   * 通知仿真系统停止当前场景。仿真系统一般不会主动结束某一个场景，通常由用户算法来决定
   * 何时停止当前场景，比如主车到达当前场景设置的终点，或者其他原因导致算法认为无法再继续
   * 运行当前场景(发生事故、没有可选路径等)。传入的字符串参数将会显示在前端界面。
   *
   * \note 当用户的算法抛出异常且没有被自己的代码捕捉到时则会被仿真系统捕捉，此时系统也会
   * 停止当前场景，并且将出错信息和对应的模块名在前端界面显示。参考 tx_sim::SimModule Note。
   *
   * @param[in] reason 终止当前场景的原因，此信息将会在前端界面显示。
   */
  void StopScenario(const std::string& reason) { TXSIM_CALLBACK_STEP_STOP_SCENARIO(reason.c_str()); }

  /*! \~Chinese
   * 获取当前的仿真系统时间。该时间代表当前场景已经运行的时间，属于逻辑(虚拟)时间，
   * 和真实的物理时间不同，详见 tx_sim::SimModule 。
   *
   * @return 当前仿真系统的时间戳，单位：毫秒(ms)。
   */
  double timestamp() { return TXSIM_CALLBACK_STEP_TIMESTAMP(); }

   /*! \~Chinese
   * 获取当前的系统时间。该时间代表服务器当前获取到的时间戳，
   * 是真实的物理时间，详见 tx_sim::SimModule 。
   *
   * @return 当前仿真系统的时间戳，单位：秒(ms)，小数点后精确到微秒。
   */
  double current_timestamp() { return TXSIM_CALLBACK_STEP_CURRENTTIMESTAMP(); }
  /*! \~Chinese
   * 同 #GetSubscribedMessage ，但是从对应的共享内存中获取数据。
   *
   * @param[in] topic 要获取的订阅消息对应的topic。
   * @param[in] mem_data 指向共享内存中数据的起始地址，值为nullptr时表示该topic目前还没有模块写入数据。
   * @return 共享内存大小（字节数）。
   */
  uint32_t GetSubscribedShmemData(const std::string& topic, const char** mem_data) {
    return TXSIM_CALLBACK_STEP_SUBSCRIBED_SHMEM_DATA(topic.c_str(), mem_data);
  }

  /*! \~Chinese
   * 获取要发布topic对应的共享内存的起始地址，利用返回的内存地址指针进行要发布的数据的写入。
   *
   * \note 该方法返回的指向共享内存中的指针只能在本次调用的Step函数中使用，在函数外使用或者后续的Step调用中使用
   * 极有可能引起程序奔溃（每次被调用的Step方法中调用该方法返回的地址有可能不同）。
   *
   * @param[in] topic 要发布的消息对应的topic。
   * @param[in] mem_buf 发布的topic对应的共享内存的起始地址，如果为nullptr通常是因为在Init时没有通过
   *         tx_sim::InitHelper::PublishShmem 向仿真系统注册此topic，也有可能是因为操作系统无法创建此共享内存。
   * @return 共享内存大小（字节数）。
   */
  uint32_t GetPublishedShmemBuffer(const std::string& topic, char** mem_buf) {
    return TXSIM_CALLBACK_STEP_GET_PUBLISHED_SHMEM_BUFFER(topic.c_str(), mem_buf);
  }
};


class StopHelper {
  /// @cond
  TXSIM_MSGER_CLS_DEF(StopHelper)
  /// @endcond

 public:
  /*! \~Chinese
   * 将当前场景运行的一些统计信息以键值对的形式反馈给仿真系统以供在前端界面显示。
   * 此方法为可选，根据需要调用即可。
   *
   * @param[in] key 需要反馈的信息的键。
   * @param[in] value 需要反馈的信息的值(内容)。
   */
  void set_feedback(const std::string& key, const std::string& value) {
    TXSIM_CALLBACK_STOP_SET_FEEDBACK(key.c_str(), value.c_str());
  }


  /*! \~Chinese
   * 返回所有模块仿真过程状态信息，返回json字符串，key为模块名称
   * value为以下信息
   * stepCount：调度次数，算法模块被调用的次数
   * executePeriod：调度周期，该调度周期为仿真时间
   * timeOut：算法模块超时次数
   * totalStepRealTime：整个仿真过程模块实际运行的时间
   * perStepTime：模块平均调度时间，该时间为真实时间
   * 
   * 返回所有模块仿真过程状态信息

   * 此方法为可选，根据需要调用即可。
   */
  std::string get_module_status()
  {
    return TXSIM_CALLBACK_STOP_MODULE_STATUS();
  }
  
};


/// @cond
namespace impl {

template <typename Fn>
void TranslateExceptions(void* data, void** cbs, Fn&& fn) {
  // clang-format off
  try {
    fn();
  } catch (const std::exception& e) {
    TXSIM_CALLBACK_SET_ERROR(data, cbs, e.what());
  } catch (...) {
    TXSIM_CALLBACK_SET_ERROR(data, cbs, "Unknown error type");
  }
  // clang-format on
}

}  // namespace impl
/// @endcond

}  // namespace tx_sim
