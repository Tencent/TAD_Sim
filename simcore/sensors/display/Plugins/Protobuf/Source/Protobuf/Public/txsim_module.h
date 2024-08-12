#pragma once

#include "txsim_messenger.h"
#include "visibility.h"


namespace tx_sim {

/*! \~Chinese
 * \brief 所有需要接入TADSim仿真系统的算法模块均需要继承SimModule类型。
 *
 * 用户的(C/C++)代码通过继承 tx_sim::SimModule 类型并实现其定义的4个回调(Callback)方法以接入TADSim仿真系统,四个回调
 * 方法分别对应从前端界面控制仿真系统(及各个模块)进行的初始化(#Init)、重置(#Reset)/运行(#Step)/结束(#Stop)场景的过程。
 *
 * 接入仿真系统的各个模块(SimModule)在运行某个仿真场景时其执行流程统一由系统(或前端界面操作)调度，因此用户的算法需
 * 要接受来自外部(仿真系统)的授时(虚拟的仿真时间)，而不应该依赖于特定平台的真实的物理时间。比如算法在计算每一帧的位
 * 移时：
 * \f[ ds = v * dt \f]
 * 其中dt应该是相邻两次 #Step 调用时获取的仿真时间的差值\f$ dt_{\text{simulation}} \f$，而不是两次调用的物理时间间隔
 * \f$ dt_{\text{physical}} \f$，仿真系统保证\f$ dt_{\text{simulation}} \f$永远等于相应模块在TADSim中配置的step time，
 * 以此来确保算法在同一个场景的可复现性 [1]，而依赖于物理时间的算法在仿真场景中的运行结果将不可复现(甚至是错误的)，因为
 * \f$ dt_{\text{simulation}} \f$往往不等于\f$ dt_{\text{physical}} \f$，而场景中其他物体的状态变化均由仿真时间计算得出。
 * 任何一个场景的起始仿真时间总是从0s时刻开始。
 *
 * - [1] 前提是算法中不包含任何随机因素。
 *
 * \note 继承SimModule的子类型必须提供一个无参数的默认构造器，所有需要通过构造器传入的参数以及相应的初始化过程可以在
 * Init方法中进行(Init方法有可能被多次调用，需要确保该方法的幂等性质，详见 #Init Note)。
 *
 * \note 在子类型覆盖的4个接口方法(Init/Reset/Step/Stop)中抛出的所有异常均应属于
 * [std::exception] (https://en.cppreference.com/w/cpp/error/exception)的子类型，TADSim会通过
 * [std::exception::what] (https://en.cppreference.com/w/cpp/error/exception/what)方法获取异常信息并显示在前端界面，
 * 其他类型的异常前端只会显示为"Unknown error type"。
 */
class SimModule {
 public:
  /*! \~Chinese
   * \brief 仿真模块在仿真系统初始化时被调用的方法。
   *
   * 仿真系统通过调用各个仿真模块实现的Init方法来对其进行定制化的初始化流程。用户可以通过由系统传入的InitHelper对象获取
   * 到该模块初始化所需要的任何参数(字符串类型的键值对，类似于main函数中argc/argv的作用) [1]，然后进行相应的初始化操作。
   * 另外在该方法中还需要指定该模块在仿真系统中需要订阅和发布的topic种类（如果有）。详见 tx_sim::InitHelper 。
   *
   * - [1] 需要用户在TADSim前端的模块配置界面中添加相应的初始化参数，除此之外系统默认的一些参数可直接通过API获取。
   *
   * @param[in,out] helper 由仿真系统负责在调用该方法前进行相应初始化并传入的助手类型，提供模块初始化所需要的一些API。
   *
   * \note 该方法保证在Reset、Step和Stop方法之前被调用并在整个程序运行周期中只被调用一次。
   */
  virtual void Init(InitHelper& helper) = 0;
  /*! \~Chinese
   * \brief 仿真模块在仿真系统重置场景时被调用的方法。
   *
   * 在该方法中可以通过参数helper对象获取到当前场景的一些基本信息，比如当前场景对应的地图文件路径、主车的起终点信息等等，
   * 详见 tx_sim::ResetHelper 。
   *
   * 该方法一般在Init或Stop方法之后被调用。
   *
   * @param[in] helper 由仿真系统负责在调用该方法前进行相应初始化并传入的助手类型，提供模块加载场景时所需要的一些API。
   */
  virtual void Reset(ResetHelper& helper) = 0;
  /*! \~Chinese
   * \brief 仿真模块在仿真系统运行场景时被调用的方法，仿真系统通过不断调用各个模块的Step方法来推进场景的演进。
   *
   * 仿真系统会根据对应模块配置的调用频率(1/step_time)每隔step_time(ms，仿真/逻辑时间)调用一次该模块的Step方法，调用时可
   * 根据参数helper对象获取到当前仿真系统的时间，算法中所有的计算均应基于此仿真时间而不是真实的物理时间，参考 #SimModule
   * 中关于外部授时的说明。另外还可以通过helper对象获取订阅的消息内容，发布消息，停止当前场景等，详见 tx_sim::StepHelper。
   * 该方法只有在所有模块的Reset方法成功返回后才会被调用。
   *
   * @param[in,out] helper 由仿真系统负责在调用该方法前进行相应初始化并传入的助手类型，提供模块运行场景时所需要的一些API。
   *
   * \note 所有模块在场景开始时均会被调用Step方法(第0帧)，此时通过helper获取到的当前仿真系统时间为0，如果当前模块订阅了
   * 定位消息(topic LOCATION)，那么在第0帧时获取到的定位消息和Reset方法中通过 tx_sim::ResetHelper::ego_start_location 获
   * 取到的定位消息的内容是相同的。
   */
  virtual void Step(StepHelper& helper) = 0;
  /*! \~Chinese
   * \brief 仿真模块在仿真系统结束场景时被调用的方法。
   *
   * 该方法目前主要用来收集各个模块对当前场景运行的一些自定义反馈信息以用来在前端界面展示，详见 tx_sim::StopHelper 。
   *
   * @param[out] helper 由仿真系统负责在调用该方法前进行相应初始化并传入的助手类型，提供模块在场景结束时所需要的一些API。
   */
  virtual void Stop(StopHelper& helper) = 0;
 /// @cond
 public:
  virtual ~SimModule() {};
 /// @endcond
};

} // namespace tx_sim


/// @cond
extern "C" {
  TXSIM_API void* txsim_new_module();
  TXSIM_API void txsim_delete_module(void* p);
  TXSIM_API void txsim_init(void* module, void* data, void** cbs);
  TXSIM_API void txsim_reset(void* module, void* data, void** cbs);
  TXSIM_API void txsim_step(void* module, void* data, void** cbs);
  TXSIM_API void txsim_stop(void* module, void* data, void** cbs);
  TXSIM_API uint32_t txsim_messenger_api_version();
}
/// @endcond


/*! \~Chinese
 * \brief 用来接入TADSim的宏定义。
 *
 * 所有需要接入仿真系统的算法源文件中均需要加入此宏定义，否则无法被仿真系统正常
 * 加载。参数cls是需要接入仿真系统的继承 tx_sim::SimModule 的子类类名，该子类需要
 * 提供一个默认的无参数构造器(根据c++的规则，如果定义了其他带有参数的构造器，则
 * 需要显示申明并定义一个无参数构造器)，需要传入的参数以及对应的初始化过程可以放在
 * tx_sim::SimModule::Init 方法中进行。
 */
#define TXSIM_MODULE(cls)                                                      \
  extern "C" {                                                                 \
    void* txsim_new_module() {                                                 \
      return new cls();                                                        \
    }                                                                          \
    void txsim_delete_module(void* p) {                                        \
      delete reinterpret_cast<tx_sim::SimModule*>(p);                          \
    }                                                                          \
    void txsim_init(void* module, void* data, void** cbs) {                    \
      tx_sim::InitHelper helper(data, cbs);                                    \
      tx_sim::impl::TranslateExceptions(data, cbs, [&] {                       \
        reinterpret_cast<tx_sim::SimModule*>(module)->Init(helper);            \
      });                                                                      \
    }                                                                          \
    void txsim_reset(void* module, void* data, void** cbs) {                   \
      tx_sim::ResetHelper helper(data, cbs);                                   \
      tx_sim::impl::TranslateExceptions(data, cbs, [&] {                       \
        reinterpret_cast<tx_sim::SimModule*>(module)->Reset(helper);           \
      });                                                                      \
    }                                                                          \
    void txsim_step(void* module, void* data, void** cbs) {                    \
      tx_sim::StepHelper helper(data, cbs);                                    \
      tx_sim::impl::TranslateExceptions(data, cbs, [&] {                       \
        reinterpret_cast<tx_sim::SimModule*>(module)->Step(helper);            \
      });                                                                      \
    }                                                                          \
    void txsim_stop(void* module, void* data, void** cbs) {                    \
      tx_sim::StopHelper helper(data, cbs);                                    \
      tx_sim::impl::TranslateExceptions(data, cbs, [&] {                       \
        reinterpret_cast<tx_sim::SimModule*>(module)->Stop(helper);            \
      });                                                                      \
    }                                                                          \
    uint32_t txsim_messenger_api_version() {                                   \
      return TXSIM_MESSENGER_API_VERSION;                                      \
    }                                                                          \
  }
