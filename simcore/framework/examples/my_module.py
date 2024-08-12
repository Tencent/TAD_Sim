#备注: python语言算法接入示例

import struct
import argparse
from txsim_module import *

# tcp topic例子
TEST_TCP_TOPIC = 'py_topic'
# 共享内存topic例子
TEST_SHMEM_TOPIC = 'py_topic_shmem'
# 备注:
# 参数:
# 返回值:


class MyModule(SimModule):
    def __init__(self):
        print('my Python module constructed.')
    # 备注: 该方法在算法第一次连接仿真系统时候被回调
    # 参数:
    #    helper: 参考算法接入文档
    # 返回值:
    def on_init(self, helper):
        print("========== my module on_init called. ==========")
        v = helper.get_parameter('max_step')
        self.max_step_time = 0
        if (len(v) > 0):
            self.max_step_time = int(v) * 1000
            print("max step time = {}s".format(v))
        self.pub = bool(helper.get_parameter('pub'))
        if self.pub:
            print("I'm a msg publisher.")
            helper.publish(TEST_TCP_TOPIC)
            helper.publish_shmem(TEST_SHMEM_TOPIC, 512)
        else:
            print("I'm a msg subscriber.")
            helper.subscribe(TEST_TCP_TOPIC)
            helper.subscribe_shmem(TEST_SHMEM_TOPIC)
    # 备注: 该方法在算法重置场景的时候被回调
    # 参数:
    #    helper: 参考算法接入文档
    # 返回值:
    def on_reset(self, helper):
        print("========== my module on_reset called. ==========")
        print("map file path: {}".format(helper.map_file_path()))
        lon, lat, alt = helper.map_local_origin()
        print("the map local origin is ({}, {}, {})".format(lon, lat, alt))
        lon, lat, alt = helper.ego_destination()
        print("the destination of the ego car is ({}, {}, {})".format(lon, lat, alt))
        print("speed limit: {}".format(helper.ego_speed_limit()))
        print("geo fence: {}".format(helper.geo_fence()))
        print("scenario file path: {}".format(helper.scenario_file_path()))
        print("ego path: {}".format(helper.ego_path()))
        vms = helper.vehicle_measurements()
        print("got {} vehicle measurements.".format(len(vms)))
        if len(vms) > 0:
            print("vehicle measurement: <id->{}, size->{}>".format(vms[0][0], len(vms[0][1])))
    # 备注: 该方法在仿真的时候被回调
    # 参数:
    #    helper: 参考算法接入文档
    # 返回值:
    def on_step(self, helper):
        print("========== my module on_step called. =========")
        t = helper.timestamp()
        print("current simulation timestamp: {}ms".format(t))
        if self.pub:
            msg = 'msg of py pub module on {} ms.'.format(t)
            helper.publish_message(TEST_TCP_TOPIC, msg.encode('utf-8'))
            buf = helper.get_published_shmem_buffer(TEST_SHMEM_TOPIC)
            if buf:
                print("got shared memory buffer with size of {} bytes.".format(len(buf)))
                struct.pack_into('I{}s'.format(len(msg)), buf, 0,
                                 len(msg), msg.encode('utf-8'))
        else:
            msg = helper.get_subscribed_message(TEST_TCP_TOPIC)
            print('received {} msg: {}'.format(
                TEST_TCP_TOPIC, msg.decode('utf-8')))
            buf = helper.get_subscribed_shmem_data(TEST_SHMEM_TOPIC)
            if buf:
                msg_size = struct.unpack_from('I', buf, 0)[0]
                print("found {} bytes data in shared memory.".format(msg_size))
                shmem_data = struct.unpack_from(
                    '{}s'.format(msg_size), buf, 4)[0]
                print("data in shared memory: {}".format(shmem_data))
        if self.max_step_time > 0 and t > self.max_step_time:
            helper.stop_scenario("max step time reached!")

    # 备注: 该方法在结束仿真系统时候被回调
    # 参数:
    #    helper: 参考算法接入文档
    # 返回值:
    def on_stop(self, helper):
        print("========== my module on_stop called. ==========")
        helper.set_feedback("my_key", "my_value")


# 备注: python入口函数
# 参数:
#    helper: 参考算法接入文档
# 返回值:
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--name', required=True)
    args = parser.parse_args()
    m = MyModule()
    s = SimModuleService()
    s.serve(args.name, m)
    print("python sim module service exit.")
