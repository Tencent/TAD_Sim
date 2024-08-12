# coding:utf-8
import json
import os
import sys
import traceback

old_new_dict = {
    "碰撞": "Collision",
    "超速次数": "MaxSpeed_V",
    "加速度过大次数": "MaxAcceleration_V",
    "减速度过大次数": "MaxDeceleration_V",
    "横向加速度过大次数": "MaxTurningAccelerate",
    "跟车时距过小次数": "MinAveragerTimeHeadway",
    "跟车距离过小次数": "MinDistanceFromCar",
    "横向偏移距离过大次数": "MaxPosError_H",
    "压实线次数": "CrossSolidLine",
    "闯红灯次数": "RunTrafficLight",
    "是否到达终点": "EndPoint"
}

new_old_dict = {
    "Collision": "collision",  # "碰撞"
    "MaxSpeed_V": "speed",  # "超速次数"
    "MaxAcceleration_V": "acc",  # "加速度过大次数"
    "MaxDeceleration_V": "dec",  # "减速度过大次数"
    "MaxTurningAccelerate": "lateralAcc",  # "横向加速度过大次数"
    "MinAveragerTimeHeadway": "ttc",  # "跟车时距过小次数"
    "MinDistanceFromCar": "dist2Fellow",  # "跟车距离过小次数"
    "MaxPosError_H": "maxPosError_H",  # "横向偏移距离过大次数"
    "CrossSolidLine": "isCrossSolidLine",  # "压实线次数"
    "RunTrafficLight": "RunTrafficLight",  # "闯红灯次数"
    "EndPoint": "is_ReachEndPoint",  # "是否到达终点",
    "TotalTime": "TotalTime"  # 超时检测
}

category_dict = {
    "MaxAcceleration_V": "General-COMFORTABILITY",
    "CrossSolidLine": "Planning-COMPLIANCE",
    "TotalTime": "General-EFFICIENCY",
    "MaxSpeed_V": "General-COMPLIANCE",
    "MaxDeceleration_V": "General-COMFORTABILITY",
    "MaxTurningAccelerate": "General-COMFORTABILITY",
    "MinAveragerTimeHeadway": "General-SECURITY",
    "MinDistanceFromCar": "General-SECURITY",
    "MaxPosError_H": "Control-SECURITY",
    "Collision": "Planning-SECURITY",
    "RunTrafficLight": "Planning-COMPLIANCE",
    "EndPoint": "Planning-EFFICIENCY",
    "MaxYawRate": "Planning-COMFORTABILITY",
    "LaneRefLineOffset": "Planning-STATISTICS",
    "EgoChangeLane_Indicator": "Planning-COMPLIANCE",
    "EgoChangeLane_Duration": "Planning-EFFICIENCY",
    "Custom_MaxPosError": "Custom-COMFORTABILITY",
    "Custom_ReachEndRadius": "Custom-EFFICIENCY"
}

TEMPLATE_CONTENT = """
    '{"report":{"info":{"reportId":"550e8400-e29b-41d4-a716-224541958355","sceneName":
    "by_traffic_01.sim","sceneId":"-1","date":{"startSecond":1648714900,"endSecond":1648714997,
    "durationSecond":97},"mileage":0.5240796018416015,"result":{"state":"FAIL","reason":
    "above max speed;above max deceleration;ttc too low;not reach custom endpoint"},"additionals":
    [{"key":"version","value":"2.0"}]},"summary":{"caseSize":18,"casePassSize":14,"caseFailSize":4,
    "caseSummary":{"name":"指标列表","sheetData":[{"header":"指标名称","data":["MaxAcceleration_V",
    "CrossSolidLine","TotalTime","MaxSpeed_V","MaxDeceleration_V","MaxTurningAccelerate",
    "MinAveragerTimeHeadway","MinDistanceFromCar","MaxPosError_H","Collision","RunTrafficLight",
    "EndPoint","MaxYawRate","LaneRefLineOffset","EgoChangeLane_Indicator","EgoChangeLane_Duration",
    "Custom_MaxPosError","Custom_ReachEndRadius"]},{"header":"结果","data":[]},{"header":"分类","data":[]}]}},
    "cases":[{"info":{"name":"MaxAcceleration_V","result":{"state":"PASS","reason":"max acceleration"},
    "desc":"纵向加速度过大次数","category":{"level1":"General","level2":"COMFORTABILITY"},"thresh":3.6},
    "steps":[{"attach":[{"xyplot":[{"name":"acceleration","xAxis":{"name":"t","unit":"s","axisData":[]},
    "yAxis":[{"name":"acc","unit":"m/s2","axisData":[]}]}]}]}]},{"info":{"name":"CrossSolidLine",
    "result":{"state":"PASS","reason":"cross solid line"},"desc":"车轮接触道路实线的次数",
    "category":{"level1":"Planning","level2":"COMPLIANCE"}},"steps":[{"attach":[{"xyplot":[{"name":
    "cross solid line","xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"on solid line",
    "unit":"N/A","axisData":[]}]}]}]}]},{"info":{"name":"TotalTime","result":{"state":"PASS",
    "reason":"timout"},"desc":"是否超过场景最大运行时长","category":{"level1":"General","level2":"EFFICIENCY"},
    "thresh":300},"steps":[{"attach":[{"pairData":[{"key":"is timeout","value":"0"}]}]}]},{"info":
    {"name":"MaxSpeed_V","result":{"state":"FAIL","reason":"above max speed"},"desc":"超过道路限速次数",
    "category":{"level1":"General","level2":"COMPLIANCE"},"thresh":20},"steps":[{"attach":[{"xyplot":
    [{"name":"speed limit","xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"speed",
    "unit":"m/s","axisData":[]}]}]}]}]},{"info":{"name":"MaxDeceleration_V","result":{"state":"FAIL",
    "reason":"above max deceleration"},"desc":"纵向刹车过大次数","category":{"level1":"General",
    "level2":"COMFORTABILITY"},"thresh":-3.6},"steps":[{"attach":[{"xyplot":[{"name":"deceleration",
    "xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"acc","unit":"m/s2","axisData":[]}]}]}]}]},
    {"info":{"name":"MaxTurningAccelerate","result":{"state":"PASS","reason":"max lateral acceleration"},
    "desc":"横向转向过猛次数","category":{"level1":"General","level2":"COMFORTABILITY"},"thresh":2},
    "steps":[{"attach":[{"xyplot":[{"name":"lateral acceleration","xAxis":{"name":"t","unit":"s",
    "axisData":[]},"yAxis":[{"name":"lateral acc","unit":"m/s2","axisData":[]}]}]}]}]},{"info":{"name":
    "MinAveragerTimeHeadway","result":{"state":"FAIL","reason":"ttc too low"},
    "desc":"碰撞交通车所花费的时长小于时距阈值的次数","category":{"level1":"General","level2":"SECURITY"},
    "thresh":2.7},"steps":[{"attach":[{"xyplot":[{"name":"time to collision","xAxis":{"name":"t",
    "unit":"s","axisData":[]},"yAxis":[{"name":"ttc","unit":"s","axisData":[]}]}]}]}]},{"info":{"name":
    "MinDistanceFromCar","result":{"state":"PASS","reason":"thw"},"desc":"与交通车距离小于阈值的次数",
    "category":{"level1":"General","level2":"SECURITY"},"thresh":5},"steps":[{"attach":[{"xyplot":
    [{"name":"thw","xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"thw","unit":"m",
    "axisData":[]}]}]}]}]},{"info":{"name":"MaxPosError_H","result":{"state":"PASS","reason":
    "max lateral position error"},"desc":"与规划路径的横向偏移绝对值与阈值的对比","category":{"level1":
    "Control","level2":"SECURITY"},"thresh":1},"steps":[{"attach":[{"pairData":[{"key":"average lateral offset",
    "value":"0.024522"},{"key":"max left lateral offset","value":"0.169898"},{"key":"max right lateral offset",
    "value":"-0.120815"}],"xyplot":[{"name":"lateral offset","xAxis":{"name":"t","unit":"s","axisData":[]},
    "yAxis":[{"name":"lateral offset","unit":"m","axisData":[]}]}]}]}]},{"info":{"name":"Collision",
    "result":{"state":"PASS","reason":"collision"},"desc":"主车是否发生碰撞","category":{"level1":"Planning",
    "level2":"SECURITY"},"thresh":1},"steps":[{"attach":[{"xyplot":[{"name":"collision","xAxis":
    {"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"collision","unit":"N/A","axisData":[]}]}]}]}]},
    {"info":{"name":"RunTrafficLight","result":{"state":"PASS","reason":"run traffic light"},
    "desc":"主车通过路口时信号灯为红灯的次数","category":{"level1":"Planning","level2":"COMPLIANCE"}},
    "steps":[{"attach":[{"xyplot":[{"name":"run traffic light","xAxis":{"name":"t","unit":"s","axisData":[]},
    "yAxis":[{"name":"run traffic light","unit":"N/A","axisData":[]}]}]}]}]},{"info":{"name":"EndPoint",
    "result":{"state":"PASS","reason":"reach endpoint"},"desc":"主车是否到达终点","category":{"level1":"Planning",
    "level2":"EFFICIENCY"}},"steps":[{"attach":[{"xyplot":[{"name":"reach endpoint","xAxis":{"name":"t",
    "unit":"s","axisData":[]},"yAxis":[{"name":"reach endpoint","unit":"N/A","axisData":[]}]}]}]}]},
    {"info":{"name":"MaxYawRate","result":{"state":"PASS","reason":"max yaw rate"},"desc":
    "主车横摆角速度是否超过给定阈值","category":{"level1":"Planning","level2":"COMFORTABILITY"},"thresh":1},
    "steps":[{"attach":[{"xyplot":[{"name":"yaw rate","xAxis":{"name":"t","unit":"s","axisData":[]},
    "yAxis":[{"name":"yaw_rate","unit":"rad/s","axisData":[]}]}]}]}]},{"info":{"name":"LaneRefLineOffset",
    "result":{"state":"PASS","reason":"max lateral offset to reference line"},"desc":"主车与车道中心线横向偏距方差",
    "category":{"level1":"Planning","level2":"STATISTICS"},"thresh":1},"steps":[{"attach":[{"pairData":
    [{"key":"reference line lateral offset variance","value":"0.000000"}],"xyplot":[{"name":
    "reference line lateral offset","xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":
    "lateral offset","unit":"m","axisData":[]}]}]}]}]},{"info":{"name":"EgoChangeLane_Indicator",
    "result":{"state":"PASS","reason":"lane change indicator"},"desc":"主车换道时正确使用转向灯","category":
    {"level1":"Planning","level2":"COMPLIANCE"},"thresh":1},"steps":[{"attach":[{"xyplot":[{
    "name":"lane change check","xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"lane change",
    "unit":"N/A","axisData":[]}]},{"name":"lane change indicator check","xAxis":{"name":"t","unit":"s",
    "axisData":[]},"yAxis":[{"name":"lane change indicator","unit":"N/A","axisData":[]}]}]}]}]},{
    "info":{"name":"EgoChangeLane_Duration","result":{"state":"PASS","reason":"lane change duration"},
    "desc":"主车换道时耗时","category":{"level1":"Planning","level2":"EFFICIENCY"},"thresh":1},"steps":[{
    "attach":[{"sheetData":[{"name":"ego lane change duration","sheetData":[{"header":"start time [s]"},{
    "header":"lane change duration [s]"}]}]}]}]},{"info":{"name":"Custom_MaxPosError","result":{"state":"PASS",
    "reason":"custom road follow error"},"desc":"规划路径的横向偏移值，左右偏移的平均值","category":{
    "level1":"Custom","level2":"COMFORTABILITY"},"thresh":100},"steps":[{"attach":[{"xyplot":[{"name":
    "custom lateral offset","xAxis":{"name":"t","unit":"s"},"yAxis":[{"name":"custom lateral offset",
    "unit":"cm"}]}]}]}]},{"info":{"name":"Custom_ReachEndRadius","result":{"state":"FAIL","reason":
    "not reach custom endpoint"},"desc":"主车是否到达终点","category":{"level1":"Custom","level2":
    "EFFICIENCY"}},"steps":[{"attach":[{"xyplot":[{"name":"custom reach endpoint","xAxis":{"name":"t",
    "unit":"s"},"yAxis":[{"name":"custom reach endpoint","unit":"N/A"}]}]}]}]}]},"meta":[{"name":
    "acceleration","xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"acc","unit":
    "m/s2","axisData":[]}]},{"name":"lateral acceleration","xAxis":{"name":"t","unit":"s",
    "axisData":[]},"yAxis":[{"name":"lateral acc","unit":"m/s2","axisData":[]}]},{"name":"speed limit",
    "xAxis":{"name":"t","unit":"s","axisData":[]},"yAxis":[{"name":"speed","unit":"m/s",
    "axisData":[]}]},{"name":"time to collision","xAxis":{"name":"t","unit":"s","axisData":[]},
    "yAxis":[{"name":"ttc","unit":"s","axisData":[]}]},{"name":"thw","xAxis":{"name":"t","unit":"s",
    "axisData":[]},"yAxis":[{"name":"thw","unit":"m","axisData":[]}]}]}'
"""

def tool_gen_uuid():
    import uuid
    return str(uuid.uuid4())


class Convertor:

    def __init__(self, old_file):
        self.old_file = old_file
        self.old_report = None
        self.new_report = None

    def prepare(self):
        with open(self.old_file) as f:
            self.old_report = json.load(f)

        self.new_report = json.loads(TEMPLATE_CONTENT)

    def convert(self):
        self.convert_report()
        self.convert_meta()

        # 旧的文件，添加文件名后缀为 .old
        # 新的文件，沿用旧的文件名

        os.rename(self.old_file, self.old_file + '.old')
        # os.rename(self.old_file, self.old_file.replace('.old',''))
        # F.rt_01.sim.2022_4_7_15_46_5.pblog.json convert to ==> F.rt_01.sim.2022_4_7_15_46_5.dict
        new_filename = self.old_file.replace('.pblog.json', '.dict')
        with open(new_filename, 'w') as f:
            json.dump(self.new_report, f)

    def pack_date(self):
        from datetime import datetime

        report_date = self.old_report['grading_statistics']['row']['report_date']
        total_time = self.old_report['grading_statistics']['row']['total_time']
        # 2022_3_31_8_36_40
        finish_time = datetime.strptime(report_date, '%Y_%m_%d_%H_%M_%S')

        end_second = int(finish_time.timestamp())
        start_second = end_second - int(total_time)
        duration_second = int(total_time)
        date = {
            "startSecond": start_second,
            "endSecond": end_second,
            "durationSecond": duration_second
        }
        return date

    def package_result(self):
        result = dict()
        result['state'] = 'PASS' if self.old_report['grading_statistics']['row']['is_pass'] else 'FAIL'
        result['reason'] = self.old_report['grading_statistics']['row']['reason']
        return result

    def convert_info(self):
        info = self.new_report['report']['info']
        info['reportId'] = tool_gen_uuid()
        info['sceneName'] = self.old_report['grading_statistics']['row']['scene_name']
        # info.sceneId = "-1"
        info['date'] = self.pack_date()
        info['mileage'] = self.old_report['grading_statistics']['row']['total_milleage']
        info['result'] = self.package_result()
        # info.additionals = [
        #     {
        #         "key": "version",
        #         "value": "2.0"
        #     }
        # ]

    def calc_case_size(self):
        # 用新的来统计
        sheet_data = self.new_report['report']['summary']['caseSummary']['sheetData']
        size = len(sheet_data[1]['data'])
        pass_size = len([item for item in sheet_data[1]['data'] if item == 'PASS'])
        fail_size = size - pass_size
        return [size, pass_size, fail_size]

    def pack_summary(self):
        sheet_data = self.new_report['report']['summary']['caseSummary']['sheetData']
        # 指标名称
        # 旧的名称映射到新的
        detail = self.old_report['grading_statistics']['row']['detail']
        new_code = []
        result = []
        category = []
        for key, value in detail.items():
            if value['data_key'] is not None and key not in ['平均横向偏移距离', '左侧最大横向偏移距离', '右侧最大横向偏移距离', '客户定制是否到达终点',
                                                             '客户定制横向偏移过大']:
                code = old_new_dict[key]
                new_code.append(code)
                if value['is_pass'] is True:
                    state = 'PASS'
                else:
                    state = 'FAIL'
                result.append(state)
                category.append(category_dict[code])

        # 处理 闯红灯次数， data_key 为null的情况，没数据

        # 处理 TotalTime
        new_code.append('TotalTime')
        state = 'PASS' if self.old_report['grading_statistics']['row']['is_timeout'] is False else 'FAIL'
        result.append(state)
        category.append(category_dict[code])

        sheet_data[0]['data'] = new_code
        # 指标结果
        sheet_data[1]['data'] = result
        # 指标分类
        sheet_data[2]['data'] = category

    def convert_summary(self):
        self.pack_summary()
        summary = self.new_report['report']['summary']
        summary['caseSize'], summary['casePassSize'], summary['caseFailSize'] = self.calc_case_size()

    def pack_case_info(self, item, code_list, result_list):
        # 第几个
        name = item['info']['name']
        index = code_list.index(name)
        state = result_list[index]
        # 设置
        item['info']['result']['state'] = state
        # reason的处理
        item['info']['result']['reason'] = item['info']['result']['reason'] + ' check ' + state.lower()
        # 处理info的thresh
        data_key = new_old_dict[name]
        if data_key not in ['collision', 'isCrossSolidLine',
                            'is_ReachEndPoint', 'distance_to_stop',
                            'road_follow_error', 'TotalTime']:
            if data_key == 'RunTrafficLight':
                thresh_key = 'RunTrafficLight_thresh'
            else:
                thresh_key = data_key + '_' + 'threshold'
            item['info']['thresh'] = self.old_report['grading']['column'][thresh_key][0]

        else:
            # CrossSolidLine 没有阈值
            # TotalTime 统一默认为300
            # Collision 默认为1
            # EndPoint 没有阈值
            # 所以都不用处理
            if name in ['CrossSolidLine', 'TotalTime', 'Collision', 'EndPoint']:
                # item['info']['thresh'] = -1
                pass

    def find_pair(pair_data, name):
        for item in pair_data:
            if name == pair_data['key']:
                return item
        return None

    def pack_case_step(self, item):
        # TODO 可能有多种情况
        name = item['info']['name']

        attach = item['steps'][0]['attach']
        if 'xyplot' in attach[0]:
            # 处理坐标值
            xyplot = attach[0]['xyplot']
            # TODO 可能有多个坐标值
            xy = xyplot[0]
            # 横坐标
            xy['xAxis']['axisData'] = self.old_report['grading']['column']['t']
            # 纵坐标

            data_key = new_old_dict[name]
            value_key = None
            if data_key not in ['collision', 'isCrossSolidLine', 'is_ReachEndPoint', 'distance_to_stop',
                                'road_follow_error']:
                if data_key == 'RunTrafficLight':
                    value_key = 'RunTrafficLight_thresh'
                else:
                    value_key = data_key + '_' + 'value'

            if value_key is not None:
                xy['yAxis'][0]['axisData'] = self.filter_data(self.old_report['grading']['column'][value_key])
            else:
                if data_key == 'isCrossSolidLine':
                    xy['yAxis'][0]['axisData'] = self.filter_data([int(n) for n in
                                                                   self.old_report['grading']['column'][
                                                                       'isCrossSolidLine']])
                elif data_key == 'collision':
                    xy['yAxis'][0]['axisData'] = self.filter_data(self.old_report['grading']['column']['collision'])
                elif data_key == 'is_ReachEndPoint':
                    xy['yAxis'][0]['axisData'] = self.filter_data(
                        [int(n) for n in self.old_report['grading']['column']['is_ReachEndPoint']])

        # 处理 TotalTime
        if name == 'TotalTime':
            value = 0 if self.old_report['grading_statistics']['row']['is_timeout'] is False else 1
            attach[0]['pairData'][0]['value'] = value

        # 处理 平均横向偏移距离、左侧最大横向偏移距离、右侧最大横向偏移距离
        if name == 'MaxPosError_H':
            pair_data = attach[0]['pairData']
            # 平均横向偏移距离
            pair_data[0]['value'] = round(
                self.old_report['grading_statistics']['row']['detail']['平均横向偏移距离']['eval_value'], 6)
            # 左侧最大横向偏移距离
            pair_data[1]['value'] = round(
                self.old_report['grading_statistics']['row']['detail']['左侧最大横向偏移距离']['eval_value'],
                6)
            # 右侧最大横向偏移距离
            pair_data[2]['value'] = round(
                self.old_report['grading_statistics']['row']['detail']['右侧最大横向偏移距离']['eval_value'],
                6)

    def convert_cases(self):
        cases = self.new_report['report']['cases']
        # 从 cases 这个list中，去掉模板里不需要的指标
        code_list = self.new_report['report']['summary']['caseSummary']['sheetData'][0]['data']
        filter_cases = [item for item in cases if item['info']['name'] in code_list]
        result_list = self.new_report['report']['summary']['caseSummary']['sheetData'][1]['data']
        for item in filter_cases:
            # 处理info的result
            self.pack_case_info(item, code_list, result_list)
            # 处理 steps
            self.pack_case_step(item)

        self.new_report['report']['cases'] = filter_cases

    def convert_report(self):
        self.convert_info()
        self.convert_summary()
        self.convert_cases()

    def find_meta(self, meta_list, name):
        for item in meta_list:
            if item['name'] == name:
                return item
        return None

    def filter_data(self, axis_data):
        res = map(lambda x: None if x == 'null' else x, axis_data)
        return list(res)

    def convert_meta(self):
        meta_list = self.new_report['meta']

        # 加速度
        item_acc = self.find_meta(meta_list, 'acceleration')
        if item_acc is not None:
            item_acc['xAxis']['axisData'] = self.old_report['location']['column']['t']
            item_acc['yAxis'][0]['axisData'] = self.filter_data(self.old_report['location']['column']['ax'])
            pass

        # 侧向加速度
        item_lateral_acc = self.find_meta(meta_list, 'lateral acceleration')
        if item_lateral_acc is not None:
            item_lateral_acc['xAxis']['axisData'] = self.old_report['location']['column']['t']
            item_lateral_acc['yAxis'][0]['axisData'] = self.filter_data(self.old_report['location']['column']['ay'])
            pass

        # 速度
        item_speed = self.find_meta(meta_list, 'speed limit')
        if item_speed is not None:
            item_speed['xAxis']['axisData'] = self.old_report['location']['column']['t']
            item_speed['yAxis'][0]['axisData'] = self.filter_data(self.old_report['location']['column']['v_sqrt'])
            pass

        # 跟车时距
        item_ttc = self.find_meta(meta_list, 'time to collision')
        if item_ttc is not None:
            item_ttc['xAxis']['axisData'] = self.old_report['grading']['column']['t']
            item_ttc['yAxis'][0]['axisData'] = self.filter_data(self.old_report['grading']['column']['ttc_value'])
            pass

        # 跟车距离
        item_thw = self.find_meta(meta_list, 'thw')
        if item_thw is not None:
            item_thw['xAxis']['axisData'] = self.old_report['grading']['column']['t']
            item_thw['yAxis'][0]['axisData'] = self.filter_data(
                self.old_report['grading']['column']['dist2Fellow_value'])


# 检查是否需要进行转换
def need_convert(filename):
    # 是否是目录
    if os.path.isdir(filename):
        return False
    # 是否json结尾
    if not filename.endswith('.json'):
        return False
    # 是否已经完成转换，存在.json.old文件
    if os.path.exists(filename + '.old'):
        return False
    # 是否已经转化完，是 version 2.0

    return True


def main():
    # 遍历文件夹的所有json报告，转换生成新的报告
    print(sys.argv)
    if len(sys.argv) > 1:
        report_dir = sys.argv[1]
    else:
        report_dir = 'pblog'
        # report_dir = 'pblog.json'
        # report_dir = 'Grading_pblog.json'
    print('pblog dir is : {}\n'.format(report_dir))
    if not os.path.exists(report_dir):
        print('pblog dir is not exists, {}'.format(report_dir))
        print('please set the pblog dir path')
        return

    for file in os.listdir(report_dir):
        # 每个文件，创建一个类，进行转换
        old_file = os.path.join(report_dir, file)
        print('==>visit file: {}'.format(old_file))
        try:
            if need_convert(old_file):
                convertor = Convertor(old_file=old_file)
                convertor.prepare()
                convertor.convert()
                print('convert success: {}'.format(old_file))
            else:
                print('already convert: {}'.format(old_file))
        except Exception as e:  # 捕获具体的异常类型
            print("Unexpected error:", e)
            traceback_details = traceback.format_exc()
            print(traceback_details)
            print('convert fail: {}'.format(old_file))

# Usage: python old2new.py pblog_dir_path
if __name__ == '__main__':
    main()
