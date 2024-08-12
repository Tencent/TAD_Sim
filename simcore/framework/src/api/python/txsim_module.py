# Copyright 2024 Tencent Inc. All rights reserved.

from ctypes import *
import os
import platform


# 基类导出方法
class SimModule:
    def __init__(self):
        pass
    # 算法第一次连接的时候被回调
    def on_init(self, helper):
        raise NotImplementedError
    # 重置仿真的时候被回调
    def on_reset(self, helper):
        raise NotImplementedError
    # 仿真开始的时候被回调
    def on_step(self, helper):
        raise NotImplementedError
    # 仿真结束的时候被回调
    def on_stop(self, helper):
        raise NotImplementedError


class SimModuleService:
    # 构造函数中，传入一个SimModule对象，并初始化SimModuleService
    def __init__(self):
        try:
            self.lib = cdll.LoadLibrary(_TXSIM_MODULE_IMPL_LIB_NAME)
            self.func_err_msg = self.lib.txsim_impl_error_message
            self.func_err_msg.argtypes = [c_void_p]
            self.func_err_msg.restype = c_char_p
            self.func_err_del = self.lib.txsim_impl_error_destruct
            self.func_err_del.argtypes = [c_void_p]
            self.func_err_del.restype = None
            self.func_impl_new = self.lib.txsim_new_module_impl
            self.func_impl_new.argtypes = [
                c_void_p, c_void_p, c_void_p, c_void_p, c_int32, POINTER(c_void_p)]
            self.func_impl_new.restype = c_void_p
            self.func_impl_del = self.lib.txsim_delete_module_impl
            self.func_impl_del.argtypes = [c_void_p]
            self.func_impl_del.restypes = None
            self.func_serve = self.lib.txsim_serve
            self.func_serve.argtypes = [
                c_void_p, py_object, c_char_p, c_char_p, POINTER(c_void_p)]
            self.func_serve.restype = None
            self.func_shutdown = self.lib.txsim_shutdown
            self.func_shutdown.argtypes = [c_void_p]
            self.func_shutdown.restype = None
        except OSError as e:
            print('** loading {} failed. Did you install the TAD Sim correctly?\n** details: {}'.format(
                _TXSIM_MODULE_IMPL_LIB_NAME, e))
            os._exit(1)

    def serve(self, name, module, address=''):
        err_p = c_void_p(0)
        self.impl = self.func_impl_new(
            _sim_init, _sim_reset, _sim_step, _sim_stop, 0, pointer(err_p))
        if err_p:
            print(_cstr2pystr(self.func_err_msg(err_p)))
            self.func_err_del(err_p)
            os._exit(2)
        self.func_serve(self.impl, py_object(module), _pystr2cstr(
            name), _pystr2cstr(address), pointer(err_p))
        if err_p:
            print(_cstr2pystr(self.func_err_msg(err_p)))
            self.func_err_del(err_p)
            os._exit(3)
        self.func_impl_del(self.impl)

    def shutdown(self):
        self.func_shutdown(self.impl)


class _InitHelper:
    def __init__(self, data, cbs):
        self.data = data
        self.cbs = cbs
    # 获取前端配置输入参数
    def get_parameter(self, key):
        v = _InitHelper.FUNC_PARAMETER(self.cbs[1])(
            self.data, _pystr2cstr(key))
        return _cstr2pystr(v)
    # 订阅tcp通信方式的topic
    def subscribe(self, topic):
        _InitHelper.FUNC_SUBSCRIBE(self.cbs[2])(self.data, _pystr2cstr(topic))
    # 发布tcp通信方式的topic
    def publish(self, topic):
        _InitHelper.FUNC_PUBLISH(self.cbs[3])(self.data, _pystr2cstr(topic))
    # 订阅共享内存通信方式的topic
    def subscribe_shmem(self, topic):
        _InitHelper.FUNC_SUBSCRIBE_SHMEM(self.cbs[4])(
            self.data, _pystr2cstr(topic))
    # 发布共享内存通信方式的topic
    def publish_shmem(self, topic, mem_size):
        _InitHelper.FUNC_PUBLISH_SHMEM(self.cbs[5])(
            self.data, _pystr2cstr(topic), c_uint32(mem_size))

    FUNC_PARAMETER = CFUNCTYPE(c_char_p, c_void_p, c_char_p)
    FUNC_SUBSCRIBE = CFUNCTYPE(None, c_void_p, c_char_p)
    FUNC_PUBLISH = CFUNCTYPE(None, c_void_p, c_char_p)
    FUNC_SUBSCRIBE_SHMEM = CFUNCTYPE(None, c_void_p, c_char_p)
    FUNC_PUBLISH_SHMEM = CFUNCTYPE(None, c_void_p, c_char_p, c_uint32)


class _ResetHelper:
    def __init__(self, data, cbs):
        self.data = data
        self.cbs = cbs
    # 获取地图文件路径
    def map_file_path(self):
        return _cstr2pystr(_ResetHelper.FUNC_MAP_PATH(self.cbs[1])(self.data))
    # 获取地图原点
    def map_local_origin(self):
        px = c_double(0)
        py = c_double(0)
        pz = c_double(0)
        _ResetHelper.FUNC_MAP_LOCAL_ORIGIN(self.cbs[2])(
            self.data, pointer(px), pointer(py), pointer(pz))
        return px.value, py.value, pz.value
    # 获取主车起点和终点
    def ego_destination(self):
        px = c_double(0)
        py = c_double(0)
        pz = c_double(0)
        _ResetHelper.FUNC_EGO_DESTINATION(self.cbs[3])(
            self.data, pointer(px), pointer(py), pointer(pz))
        return px.value, py.value, pz.value
    # 获取车辆最大速度
    def ego_speed_limit(self):
        return _ResetHelper.FUNC_EGO_SPEED_LIMIT(self.cbs[4])(self.data)
    # 获取车辆起点坐标
    def ego_start_location(self):
        msg = c_char_p()
        size = _ResetHelper.FUNC_LOCATION_RAW(
            self.cbs[5])(self.data, pointer(msg))
        return string_at(msg, size)
    # 获取车辆终点坐标
    def geo_fence(self):
        ret = []
        size = _ResetHelper.FUNC_EGO_FENCE(self.cbs[6])(
            self.data, POINTER(c_double)())
        if size > 0:
            p = (c_double * (size * 2))()
            _ResetHelper.FUNC_EGO_FENCE(self.cbs[6])(self.data, p)
            for i in range(size):
                ret.append((p[i * 2], p[i * 2 + 1]))
        return ret
    # 获取场景文件路径
    def scenario_file_path(self):
        return _cstr2pystr(_ResetHelper.FUNC_SCENARIO_PATH(self.cbs[7])(self.data))
    # 获取轨迹点
    def ego_path(self):
        ret = []
        size = _ResetHelper.FUNC_EGO_PATH(self.cbs[8])(
            self.data, POINTER(c_double)())
        if size > 0:
            p = (c_double * (size * 3))()
            _ResetHelper.FUNC_EGO_PATH(self.cbs[8])(self.data, p)
            for i in range(size):
                ret.append((p[i * 3], p[i * 3 + 1], p[i * 3 + 2]))
        return ret
    # 获取主车尺寸
    def vehicle_measurements(self):
        ret = []
        count = _ResetHelper.FUNC_VEHICLE_MEASUREMENT(self.cbs[9])(self.data, POINTER(c_int64)(), POINTER(c_char_p)())
        if count > 0:
            for i in range(count):
                msg = c_char_p()
                id = c_int64(i)
                size = _ResetHelper.FUNC_VEHICLE_MEASUREMENT(self.cbs[9])(self.data, pointer(id), pointer(msg))
                ret.append((id.value, string_at(msg, size)))
        return ret

    FUNC_MAP_PATH = CFUNCTYPE(c_char_p, c_void_p)
    FUNC_MAP_LOCAL_ORIGIN = CFUNCTYPE(None, c_void_p, POINTER(
        c_double), POINTER(c_double), POINTER(c_double))
    FUNC_EGO_DESTINATION = CFUNCTYPE(None, c_void_p, POINTER(
        c_double), POINTER(c_double), POINTER(c_double))
    FUNC_EGO_SPEED_LIMIT = CFUNCTYPE(c_double, c_void_p)
    FUNC_LOCATION_RAW = CFUNCTYPE(c_uint32, c_void_p, POINTER(c_char_p))
    FUNC_EGO_FENCE = CFUNCTYPE(c_uint32, c_void_p, POINTER(c_double))
    FUNC_SCENARIO_PATH = CFUNCTYPE(c_char_p, c_void_p)
    FUNC_EGO_PATH = CFUNCTYPE(c_uint32, c_void_p, POINTER(c_double))
    FUNC_VEHICLE_MEASUREMENT = CFUNCTYPE(c_uint32, c_void_p, POINTER(c_int64), POINTER(c_char_p))


class _StepHelper:
    def __init__(self, data, cbs):
        self.data = data
        self.cbs = cbs
    # 根据topic获取订阅的消息
    def get_subscribed_message(self, topic):
        msg = c_char_p()
        size = _StepHelper.FUNC_SUBSCRIBED_MSG(self.cbs[1])(
            self.data, _pystr2cstr(topic), pointer(msg))
        return string_at(msg, size)
    # 发布消息
    def publish_message(self, topic, payload):
        if type(payload) != bytes:
            raise TypeError(
                "the argument type of payload to helper.publish_message must be str or bytes.")
        _StepHelper.FUNC_PUBLISH_MSG(self.cbs[2])(
            self.data, _pystr2cstr(topic), payload, len(payload))
    # 停止仿真
    def stop_scenario(self, reason):
        _StepHelper.FUNC_STOP_SCENARIO(self.cbs[3])(
            self.data, _pystr2cstr(reason))
    # 获取仿真时间
    def timestamp(self):
        return _StepHelper.FUNC_TIMESTAMP(self.cbs[4])(self.data)
    # 根据topic获取订阅的共享内存数据
    def get_subscribed_shmem_data(self, topic):
        buf_addr = c_char_p()
        size = _StepHelper.FUNC_SUBSCRIBED_SHMEM_BUF(self.cbs[5])(
            self.data, _pystr2cstr(topic), pointer(buf_addr))
        return string_at(buf_addr, size) if buf_addr else None
    # 发布共享内存数据
    def get_published_shmem_buffer(self, topic):
        buf_addr = POINTER(c_char)()
        size = _StepHelper.FUNC_PUBLISHED_SHMEM_BUF(self.cbs[6])(
            self.data, _pystr2cstr(topic), pointer(buf_addr))
        return (c_char * size).from_address(addressof(buf_addr.contents)) if buf_addr else None

    FUNC_SUBSCRIBED_MSG = CFUNCTYPE(
        c_int32, c_void_p, c_char_p, POINTER(c_char_p))
    FUNC_PUBLISH_MSG = CFUNCTYPE(None, c_void_p, c_char_p, c_char_p, c_int32)
    FUNC_STOP_SCENARIO = CFUNCTYPE(None, c_void_p, c_char_p)
    FUNC_TIMESTAMP = CFUNCTYPE(c_double, c_void_p)
    FUNC_SUBSCRIBED_SHMEM_BUF = CFUNCTYPE(
        c_int32, c_void_p, c_char_p, POINTER(c_char_p))
    FUNC_PUBLISHED_SHMEM_BUF = CFUNCTYPE(
        c_int32, c_void_p, c_char_p, POINTER(POINTER(c_char)))


class _StopHelper:
    def __init__(self, data, cbs):
        self.data = data
        self.cbs = cbs
    # 设置反馈信息
    def set_feedback(self, key, value):
        _StopHelper.FUNC_SET_FEEDBACK(self.cbs[1])(
            self.data, _pystr2cstr(key), _pystr2cstr(value))

    FUNC_SET_FEEDBACK = CFUNCTYPE(None, c_void_p, c_char_p, c_char_p)


FUNC_TXSIM_MODULE_ERROR = CFUNCTYPE(None, c_void_p, c_char_p)


@CFUNCTYPE(None, py_object, c_void_p, POINTER(c_void_p))
def _sim_init(module, data, cbs):
    h = _InitHelper(data, cbs)
    try:
        module.on_init(h)
    except BaseException as e:
        print("sim init error.")
        FUNC_TXSIM_MODULE_ERROR(cbs[0])(data, _pystr2cstr(str(e)))


@CFUNCTYPE(None, py_object, c_void_p, POINTER(c_void_p))
def _sim_reset(module, data, cbs):
    h = _ResetHelper(data, cbs)
    try:
        module.on_reset(h)
    except BaseException as e:
        print("sim reset error.")
        FUNC_TXSIM_MODULE_ERROR(cbs[0])(data, _pystr2cstr(str(e)))


@CFUNCTYPE(None, py_object, c_void_p, POINTER(c_void_p))
def _sim_step(module, data, cbs):
    h = _StepHelper(data, cbs)
    try:
        module.on_step(h)
    except BaseException as e:
        print("sim step error.")
        FUNC_TXSIM_MODULE_ERROR(cbs[0])(data, _pystr2cstr(str(e)))


@CFUNCTYPE(None, py_object, c_void_p, POINTER(c_void_p))
def _sim_stop(module, data, cbs):
    h = _StopHelper(data, cbs)
    try:
        module.on_stop(h)
    except BaseException as e:
        print("sim stop error.")
        FUNC_TXSIM_MODULE_ERROR(cbs[0])(data, _pystr2cstr(str(e)))


def _cstr2pystr(s):
    return s.decode('utf-8')


def _pystr2cstr(s):
    return c_char_p(s.encode('utf-8'))


_tadsim_install_dir = os.getenv("TADSIM")
if platform.system() == 'Linux':
    _TXSIM_MODULE_IMPL_LIB_NAME = ('libtxsim-module-impl.so' if _tadsim_install_dir is None
                                    else os.path.join(_tadsim_install_dir, "service/simdeps/libtxsim-module-impl.so")
                                  )
elif platform.system() == 'Windows':
    _TXSIM_MODULE_IMPL_LIB_NAME = ('txsim-module-impl.dll' if _tadsim_install_dir is None
                                    else os.path.join(_tadsim_install_dir, "service/txsim-module-impl.dll")
                                    )
else:
    raise SystemExit("txsim-module-impl: Unsupported platform.")
