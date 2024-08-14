/**
 * Copyright (c) 1998-2014 Tencent Inc. All Rights Reserved
 *
 * This software is L5 API for c++,
 * If you need php/python/java version, it can be found here:
 *
 * @file qos_client.h
 * @brief L5 API interface
 */

/**
 * If you have L5 api,L5Agent and L5 OSS problems, all docs and details are found here:
 * and you could contact with @h_l5_helper.
 */

/**
 * @history
 *  2017.01.27 flyyfyan version 4.1.1
 *     1.创建udp时绑定本地回环地址，防止系统默认绑定INADDR_ANY导致安全隐患
 *  2017.01.10 flyyfyan version 4.1.1
 *     1.添加对timeout的限制(当timeout大于1s时按1s算)，防止错误的使用timeout,导致程序长时间挂起
 *     2.修改同步方式ApiInitRoute的实现，解决Init时获取一次ip但无法上报,最终导致agent动态权重调整
 *  2015.11.27 flyyfyan version 4.0.7
 *     1.添加异步api的接口
 *     注:在使用异步接口时,不要在有调用过异步接口,且fork子进程后没有执行exec类函数的情况下在子进程中
 *       继续使用异步接口(fork调用只会复制调用的线程，导致异步工作线程未创建),如果需要fork,请在没有调用任何异步接口之前进行fork调用,
 *       现阶段已经没有更好的方法了(可以新增接口提供反初始化异步接口的功能,子进程中立即调用则可达到效果,
 *           或者使用pthread_atfork, 但两种方法终究也不能解决有可能死锁的问题)
 *     同步版本api由于进行了无锁化设计且没有工作线程，故没有这方面的担忧
 *     2.添加两个错误码
 *          QOS_RTN_LOCAL_ERROR:
 * api内部错误如分配空间,创建类,创建socket等等，用于与QOS_RTN_SYSERR区分，使得错误码意义更加明确
 *          QOS_RTN_ASYNC_INIT_ERROR:异步版本未初始化获取初始化错误
 *  2015.08.11 flyyfyan version 4.0.0
 *     1.经过代码review后删除一些基本不用的接口
 *     2.删除一些引用不到的头文件
 *  2015.08.09 flyyfyan version 4.0.0
 *     1.为了兼容之前api调用方式(不用调用ApiUninit函数进行资源释放),删除了ApiUninit接口,
 *       并添加了自动进行垃圾回收的功能
 *  2015.08.04 flyyfyan version 4.0.0
 *     1.添加命名空间cl5防止类冲突
 *  2015.07.16 flyyfyan version 4.0.0
 *     1.新增采用一致性hash进行负载均衡的路由获取接口
 *     2.采用线程变量的方式进行了去锁优化,提供了一个资源释放的接口(ApiUninit)
 *        调用者在线程结束时可以调用该函数进行资源释放
 *  2014.03.20 version 3.3.1
 *     1. 新增 ApiRouteResultUpdate, 由调用者指定延时单位，API规范以微秒上报到L5Agent,
 *        解决因SPP同步,SPP异步,L5API混合使用的延时单位不同，而导致分配不均衡不平滑.
 *     2. 解决 qos_client.h 在3.3.0与3.2.0接口声明不兼容
 *
 *  2013.11.25 version 3.3.0
 *     1. 在原先支持sid的基础上，新增支持名字服务 sname,  支持OMG zkname (sid与sname 一一对应)
 *     2. 去掉第一次获取路由失败时输出stderr
 *     3. 解决批量上报延时,因累计而整数溢出的缺陷
 *
 *  2013.09.10 version 3.2.0
 *     1. 可配置静态权重
 *     2. 解决在CGI模式下可能造成SID的第一台被调机过载
 *
 *  2013.06.17 version 3.1.0
 *     1. 增加l5api及l5agent版本号统计功能
 *     2. 修改l5api及l5agent的通讯地址为 127.0.0.1
 *
 *  2013.05.02 version 2.9.4
 *     1. 修复在同一应用中加载多次l5api静态库导致串包的BUG
 */
#pragma once
#include <stdint.h>
#include <sys/time.h>
#include <string>
#include <vector>

namespace cl5 {
/**
 * L5API return code
 * @brief return code of l5api function
 * @enum _QOS_RTN  >=0 for success, <0 for errors
 */
enum _QOS_RTN {
  QOS_RTN_OK = 0,            // success
  QOS_RTN_ACCEPT = 1,        // success (forward compatiblility)
  QOS_RTN_LROUTE = 2,        // success (forward compatiblility)
  QOS_RTN_TROUTE = 3,        // success (forward compatiblility)
  QOS_RTN_STATIC_ROUTE = 4,  // success (forward compatiblility)
  QOS_RTN_INITED = 5,        // success (forward compatiblility)

  QOS_RTN_OVERLOAD = -10000,        // sid overload, all ip:port of sid(modid,cmdid) is not available
  QOS_RTN_TIMEOUT = -9999,          // timeout
  QOS_RTN_SYSERR = -9998,           // error
  QOS_RTN_SENDERR = -9997,          // send error
  QOS_RTN_RECVERR = -9996,          // recv error
  QOS_MSG_INCOMPLETE = -9995,       // msg bad format (forward compatiblility)
  QOS_CMD_ERROR = -9994,            // cmd invalid (forward compatiblility)
  QOS_MSG_CMD_ERROR = -9993,        // msg cmd invalid (forward compatiblility)
  QOS_INIT_CALLERID_ERROR = -9992,  // init callerid error
  QOS_RTN_PARAM_ERROR = -9991,      // parameter error
  QOS_RTN_LOCAL_ERROR = -9990,      // api local internal error
  QOS_RTN_ASYNC_INIT_ERROR = -9989  // async api not init or init error
};

// 单次访问的基本信息
typedef struct QOSREQUESTtag {
  int _flow;             // flow id
  int _modid;            // 被调模块编码
  int _cmd;              // 被调接口编码
  std::string _host_ip;  // 被调主机IP
  uint16_t _host_port;   // 被调主机PORT
  int _pre;              // pre value

  QOSREQUESTtag() : _flow(0), _modid(0), _cmd(0), _host_port(0), _pre(0) {}
} QOSREQUEST;

typedef struct QOSREQUESTMTTCEXTtag {
  int32_t _modid;        // 被调模块编码
  int32_t _cmdid;        // 被调接口编码
  int64_t _key;          // 被调接口编码
  int32_t _funid;        // 老版本遗留参数，缺省置0
  std::string _host_ip;  // 被调主机IP
  uint16_t _host_port;   // 被调主机PORT
} QOSREQUEST_MTTCEXT;

typedef struct ROUTEtag {
  std::string _host_ip;
  uint16_t _host_port;
  unsigned int _weight;
} QOSROUTE;

typedef struct QOSREQUESTROUTETABLEtag {
  int _modid;  // 被调模块编码
  int _cmdid;  // 被调接口编码
  std::vector<QOSROUTE> _route_tb;
} QOSREQUEST_RTB;

/*add by flyyfyan 2015-07-16 提供一致性hash负载均衡的请求结构*/
typedef struct QOSREQUESTCSTHASHtag {
  int32_t _modid;        // 被调模块编码
  int32_t _cmdid;        // 被调接口编码
  uint64_t _key;         // 关键字
  std::string _host_ip;  // 被调主机IP
  uint16_t _host_port;   // 被调主机PORT
} QOSREQUEST_CSTHASH;

/**
 * @brief initialize route table for sid(modid,cmdid) to prepare faster cache
 * @note Release Version: 1.7.0 (forward compatiblility)
 * Args:
 *   @param modid:      modid needed to be inited
 *   @param cmdid:      cmdid needed to be inited
 *   @param time_out:   time limit to fetch route, [0.2, 1.0] seconds is recommended
 *   @param err_msg:    error messange when return value<0
 *
 * @return report success or fail
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled
 */
/*注:此函数用于预拉取路由，功能和ApiGetRoute一致，业务可以不用调用*/
int ApiInitRoute(int modid, int cmdid, float time_out, std::string &err_msg);

/**
 * @brief get route which return IP:PORT in qos_req._host_ip and qos_req._host_port,
 *        it is recommended that U should use the first function @ApiGetRoute
 * @note Release Version: 1.7.0 (forward compatiblility)
 * Args:
 *   @param qos_req:   request route of sid(qos_req._modid,qos_req._cmd),
 *                     return IP:PORT in qos_req._host_ip and qos_req._host_port
 *   @param time_out:  time limit to fetch route,the actrual time limit is 1s* time_out
 *   @param err_msg:   error messange when return value<0
 *   @param tm_val:     time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 *
 * @return report success or fail,
 *         return IP:PORT in qos_req._host_ip and qos_req._host_port when success
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled
 */
int ApiGetRoute(QOSREQUEST &qos_req, float time_out, std::string &err_msg, struct timeval *tm_val = NULL);

/**
 * 获取有状态到sid路由
 */
int ApiGetRoute(QOSREQUEST_MTTCEXT &qos_req, float time_out, std::string &err_msg, struct timeval *tm_val = NULL);

/**
 * @brief get route of sname which is in space of zkname,
 *        return IP:PORT in qos_req._host_ip and qos_req._host_port
 * @note Release Version: 3.3.1 (forward compatiblility)
 * @warning the function is recommended for zkname
 * Args:
 *   @param qos_req:   request route of sid(qos_req._modid,qos_req._cmd),
 *                     return IP:PORT in qos_req._host_ip and qos_req._host_port
 *   @param sname:     unique name of zkname space
 *   @param time_out:  time limit to fetch route,the actrual time limit is 1s* time_out
 *   @param err_msg:   error messange when return value<0
 *   @param tm_val:     time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 *
 * @return success or fail,
 *         return IP:PORT in qos_req._host_ip and qos_req._host_port when success
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled
 */
int ApiGetRoute(QOSREQUEST &qos_req, const std::string &sname, float time_out, std::string &err_msg,
                struct timeval *tm_val = NULL);

/**
 * @brief report result and delay of qos_req._host_ip which return by ApiGetRoute
 * @note Release Version: 3.3.1 (forward compatiblility)
 * Args:
 *   @param qos_req:    qos_req(include last _modid,_cmd,_host_ip,_host_port) which return by ApiGetRoute
 *   @param ret:      report status to L5_agent,0 for normal,<0 for abnormal
 *   @param usetime_type: time unit(enum QOS_TIME_TYPE), microseconds or milliseconds or seconds
 *   @param usetime:      used time
 *   @param err_msg:    error messange when return value<0
 *   @param tm_val:        time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 * @return report success or fail
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled
 */
enum QOS_TIME_TYPE { TIME_MICROSECOND = 0, TIME_MILLISECOND = 1, TIME_SECOND = 2 };

int ApiRouteResultUpdate(QOSREQUEST &qos_req, int ret, int usetime_type, int usetime, std::string &err_msg,
                         struct timeval *tm_val);

/**
 * @brief report result and delay of qos_req._host_ip which return by ApiGetRoute,
 *        it is recommended that U should use the first function @ApiRouteResultUpdate
 * @note Release Version: 1.7.0 (forward compatiblility)
 * @warning Please be careful.
 *          U must use the same time unit as other client services (eg. spp, l5api) in one sid(modid, cmdid).
 * Args:
 *   @param qos_req:    qos_req(include last _modid,_cmd,_host_ip,_host_port) which return by ApiGetRoute
 *   @param ret:      report status to L5_agent,0 for normal,<0 for abnormal
 *   @param usetime:      used time, microseconds or milliseconds or seconds
 *                        Please be careful.
 *                        U must use the same time unit as other client services (eg. spp, l5api) in one sid(modid,
 * cmdid).
 *   @param err_msg:    error messange when return value<0
 *   @param tm_val:        time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 * @return report success or fail
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled
 */
int ApiRouteResultUpdate(QOSREQUEST &qos_req, int ret, int usetime_usec, std::string &err_msg,
                         struct timeval *tm_val = NULL);
int ApiRouteResultUpdate(QOSREQUEST_MTTCEXT &qos_req, int ret, int usetime_usec, std::string &err_msg,
                         struct timeval *tm_val = NULL);

#ifdef TEST_API
/*查询名字对应的sid*/
int ApiQuerySidOfSname(const std::string &name, int &modid, int &cmdid, float timeout, std::string &err,
                       struct timeval *tm = NULL);

/*获取所有被调接口,已经废弃的接口非常不建议使用!!!!*/
/*注意:!!!!!!!!
 * 1.返回的列表无负载均衡和容错策略:该方法本质是读取cl5 agent写入的一个文本文件,查找对应的CL5 SID,不会与CL5
 * Agent通信,所以没有容错和负载均衡 2.路由丢失问题:如果业务使用该方法并自己做负载均衡和容错,请注意:因为CL5
 * Agent目前有自动清除冗余路由(在业务机器上,某SID 一定时间(默认配置30天)内未访问,则认为路由冗余
 *      ,判断标准是:业务是否有在一定时间(默认配置30天)内通过ApiGetRoute等接口获取过路由。因为此方法没有与CL5
 * Agent通信,不会主动拉取,所以业务机器上该SID的访问关系丢失 针对路由丢失问题,目前得到了部分解决:
 *      a.该函数实现:加载的文件中找不到对应的CL5 SID,则调用ApiGetRoute接口与CL5 Agent通信
 *      b.CL5 Agent支持自动拉取功能,当路由缺失时,CL5 Agent会跟l5_config, l5 server通信索要对应的SID
 *     但是::
 *       目前没有解决跨set的自动拉取功能,所以如果业务机器要访问的SID不属于同一个set的情况,请不要使用本函数
 *      */
int ApiGetRouteTable(QOSREQUEST_RTB &req, std::string &err);
int ApiGetRouteTable(QOSREQUEST_RTB &req, const std::string &name, std::string &err);
#endif

/*add by flyyfyan 2015-07-16 提供通过一致性hash获取路由的接口*/
int ApiGetRoute(QOSREQUEST_CSTHASH &req, float time_out, std::string &err, struct timeval *tm = NULL);
int ApiGetRoute(QOSREQUEST_CSTHASH &req, const std::string &sname, float time_out, std::string &err,
                struct timeval *tm = NULL);
int ApiRouteResultUpdate(QOSREQUEST_CSTHASH &req, int ret, int usetime, std::string &err, struct timeval *tm = NULL);

/*add by flyyfyan 2015-11-27 提供异步版本api,使用异步版本时注意fork调用，具体请参考说明*/
/*注:使用异步版本api时，最好每个线程针对每种接口的sid调用一次AsyncInitRoute,
 * 这次调用将是同步阻塞调用，缺少此次调用肯能会导致异步版本前几次调用
 * 无法获取到ip,或从静态文件中拉取路由, time_out建议配置0.5(也即500ms),过短可能导致无法获取到ip列表*/
int AsyncInitRoute(QOSREQUEST &req, float time_out, std::string &err_msg);
int AsyncInitRoute(QOSREQUEST_MTTCEXT &req, float time_out, std::string &err);
int AsyncInitRoute(QOSREQUEST_CSTHASH &req, float time_out, std::string &err_msg);
int AsyncInitSname(const std::string &name, float time_out, std::string &err_msg);

/**
 * @brief get route which return IP:PORT in qos_req._host_ip and qos_req._host_port,
 *        it is recommended that U should use the first function @ApiGetRoute
 * @note Release Version: 5.0.0
 * Args:
 *   @param qos_req:   request route of sid(qos_req._modid,qos_req._cmd),
 *                     return IP:PORT in qos_req._host_ip and qos_req._host_port
 *   @param err_msg:   error messange when return value<0
 *
 * @return report success or fail,
 *         return IP:PORT in qos_req._host_ip and qos_req._host_port when success
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled
 */
int AsyncApiGetRoute(QOSREQUEST &qos_req, std::string &err_msg);

/*和AsyncApiGetRoute一致，不过是通过name去获取的,类似于dns*/
int AsyncApiGetRoute(QOSREQUEST &req, const std::string &sname, std::string &err);
int AsyncApiRouteResultUpdate(QOSREQUEST &req, int ret, int usetime, std::string &err);

/*获取有状态到sid路由*/
int AsyncApiGetRoute(QOSREQUEST_MTTCEXT &req, std::string &err);
int AsyncApiRouteResultUpdate(QOSREQUEST_MTTCEXT &req, int ret, int usetime, std::string &err);

/*一致性hash路由获取及结果上报异步接口*/
int AsyncApiGetRoute(QOSREQUEST_CSTHASH &req, std::string &err);
int AsyncApiGetRoute(QOSREQUEST_CSTHASH &req, const std::string &sname, std::string &err);
int AsyncApiRouteResultUpdate(QOSREQUEST_CSTHASH &req, int ret, int usetime, std::string &err);

#ifdef TEST_API
/*异步版本获取名字服务对应的sid*/
int AsyncApiQuerySidOfSname(const std::string &name, int &modid, int &cmdid, std::string &err);
#endif

/*end of namespace cl5*/
}  // namespace cl5
