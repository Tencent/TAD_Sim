HadmapSvr核心Api
# 1. 接口清单
| 序号 | 接口名称 | 接口 |
| :-: | :- | :- |
| 1 | 地图加载 | hadmapConnect | 
| 2 | 地图卸载 | hadmapClose  |
| 3 | 获取道路中心线 | getLane 或 getLanes |
| 4 | 获取路口连接线 | getLaneLinks | 
| 5 | 获取道路数据 | getRoads | 
| 6 | 获取道路其他物体元素| getObjects | 

# 2. 接口说明
## 2.1 地图加载
| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 加载地图数据，将地图数据保存到缓存中 ||||
| API | hadmapConnect ||||
| 参数值 | 类型 | 必填项 | 说明 ||
| fileInfo |  const char* | Y | 地图路径 |
| type | const MAP_DATA_TYPE& | Y | 地图格式类型 |
| pHandle | txMapHandle** | Y | 数据访问指针 
| <b>示例</b> |||||
```c++
	const char * path = "D:XXX.xodr";
	txMapHandle* pHandle = NULL;
	if (TX_HADMAP_HANDLE_OK == hadmapConnect(path, OPENDRIVE, &pHandle)) {
		return true;
	}
```
| <b>地图数据类型</b> |<br>
当前项目中维护的为 OpenDrive数据，其他为历史遗留地图格式数据
```c++
    //地图数据类型
    enum MAP_DATA_TYPE
	{
		NONE		= 0,
		SQLITE		= 1,
		MYSQL		= 2,
		REMOTE		= 3, 
		MEMORY		= 4,
		OPENDRIVE	= 5, //OpenDrive数据类型
	};
```

## 2.2 地图卸载
| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 清理地图数据缓存 ||||
| API | hadmapClose  ||||
| 参数值 | 类型 | 必填项 | 说明 ||
| pHandle | txMapHandle** | Y | 数据访问指针 |
| <b>示例</b> |||||
```c++
	if (TX_HADMAP_HANDLE_OK == hadmapClose (&pHandle)) {
		return true;
	}
```

## 2.3 车道中心线查询
## 2.3.1 获取车道中心线
| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 指定经纬度位置，获取最近的车道中心线 ||||
| API | int getLane(txMapHandle* pHandle,const hadmap::txPoint& loc,hadmap::txLanePtr& lanePtr); ||||
| 参数值 | 类型 | 必填项 | 说明 ||
| pHandle | txMapHandle* | Y | 数据访问指针 |
| loc |  const hadmap::txPoint& | Y | 指定经纬度位置 |
| lanePtr |hadmap::txLanePtr& lanePtr | Y | 查询到的车道中心线指针 |
| <b>示例</b> |||||
```c++
//获取车道中心线
  hadmap::txPoint loc1(113.481715596,23.026869, -100000);
  hadmap::txLanePtr laneptr;
  hadmap::getLane(
  	pHandle, \
  	loc1, \
  	laneptr
  );
```
## 2.3.2 获取多组车道中心线
| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 获取以center为圆心（经纬度），radius为半径（km）范围内的车道中心线 ||||
| API |  int getLanes(txMapHandle* pHandle, const hadmap::txPoint& center,const double& radius,hadmap::txLanes& lanes); ||||
| 参数值 | 类型 | 必填项 | 说明 ||
| pHandle | txMapHandle* | Y | 数据访问指针 |
| center | const hadmap::txPoint& | Y | 指定经纬度位置 |
| radius | const double& | Y | 经纬度半径 |
| lanePtr |hadmap::txLanes& lanes | Y | 查询到的多组车道中心线指针数列 |
| <b>示例</b> |||||
```c++
  //获取多组车道中心线
  hadmap::txPoint center(113.481715596,23.026869, -100000);
  double radius = 5；
  hadmap::txLanes& lanes；
  hadmap::getLanes(
  	pHandle, \
  	center, \
  	radius，\
    lanes
  );
```

## 2.4 获取路口连接线

| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 获取以center为圆心（经纬度），radius为半径（km）范围内的车道中心线 ||||
| API | int getLaneLinks(txMapHandle* pHandle,const uint64_t& fromRoadId, const uint64_t& toRoadId, hadmap::txLaneLinks& lanelinks); ||||
| 参数值 | 类型 | 必填项 | 说明 ||
| pHandle | txMapHandle* | Y | 数据访问指针 |
| center | const hadmap::txPoint& | Y | 指定经纬度位置 |
| radius | const double& | Y | 经纬度半径 |
| lanePtr |hadmap::txLanes& lanes | Y | 查询到的多组车道中心线指针数列 |
| <b>示例</b> |||||

```c++
  //获取多组车道中心线
  hadmap::txPoint center(113.481715596,23.026869, -100000);
  double radius = 5；
  hadmap::txLanes& lanes；
  if (TX_HADMAP_HANDLE_OK == hadmap::getLanes(pHandle, center, radius，lanes)）
  {
    return true；
  }
```

## 2.5 获取指定范围内道路数据
| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 获取指定范围的道路数据 ||||
| API | int getRoads(txMapHandle* pHandle,const hadmap::PointVec& envelope,const bool& wholeData,hadmap::txRoads& roads); ||||
| 参数值 | 类型 | 必填项 | 说明 ||
| pHandle | txMapHandle* | Y | 数据访问指针 |
| envelope | const hadmap::PointVec&  | Y | 指定范围，envelope[0]： leftbottom, envelope[1] righttop |
| wholeDat | const bool&  | Y | 是否只获取道路数据 |
| roads |hadmap::txRoads& | Y | 查询到的道路数据数列 |
| <b>示例</b> |||||
```c++
  //获取指定范围内道路数据
	hadmap::txRoads roads;
	if (TX_HADMAP_HANDLE_OK == hadmap::getRoads(pHandle, true, roads))
	{
		return true;
	}
```

## 2.6 获取指定范围内物体元素
| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 获取指定范围的道路数据 ||||
| API | 	int getObjects(txMapHandle* pHandle,const hadmap::PointVec& envelope, const std::vector< hadmap::OBJECT_TYPE >& types, hadmap::txObjects& objects);||||
| 参数值 | 类型 | 必填项 | 说明 ||
| pHandle | txMapHandle* | Y | 数据访问指针 |
| envelope | const hadmap::PointVec&  | Y | 指定范围，envelope[0]： leftbottom, envelope[1] righttop |
| types | std::vector< hadmap::OBJECT_TYPE >&  | Y | 查询物体类型，为空时查询所有物体类型|
| objects | hadmap::txObjects& | Y | 查询到的物体数列 |
| <b>示例</b> |||||
```c++
  //获取指定范围内道路数据
	hadmap::txObjects objects;
	if (TX_HADMAP_HANDLE_OK == hadmap::getObjects(pHandle, true, objects))
	{
		return true;
	}
```

# 3. 数据返回
MapSdk数据返回统一为以下数据：
```c++
#define TX_HADMAP_OK                0 // 数据操作OK
#define TX_HADMAP_HANDLE_ERROR      1 // 数据访问指针错误
#define TX_HADMAP_HANDLE_OK         2 // 数据访问指针OK
#define TX_HADMAP_DATA_ERROR        3 // 数据错误
#define TX_HADMAP_DATA_OK           4 // 数据正确加载返回信息
#define TX_HADMAP_DATA_EMPTY        5 // 数据为空
#define TX_HADMAP_PARAM_ERROR       6 // 参数错误
#define TX_HADMAP_DATA_NOT_SUPPORT  7 // 数据不支持
```

# 4. 物体类型表
## 4.1 signal标志
| 类型   | 子类型                                    | type（国标）          | subType | 腾讯          | 项目组评审 |
|------|----------------------------------------|-------------------|---------|---------------|-------|
| 指示标志 | 直行                                     | 1010300100002410  | -1      |  |       |
|      | 最低限速60                                 | 1010301500002410  | 60      | Y             |       |
|      | 最低限速40                                 | 1010301500002410  | 40      | Y             | 需支持   |
|      | 靠右侧道路行驶                                | 1010300700002410  | -1      | Y             |       |
|      | 人行横道                                   | 1010301800002610  | -1      | Y             |       |
|      | 向右转弯                                   | 1010300300002410  | -1      | Y             |       |
|      | 环岛行驶                                   | 1010301100002410  | -1      | Y             |       |
|      | 向左转弯                                   | 1010300200002410  | -1      | Y             |       |
|      | 直行和向右转弯                                | 1010300500002410  | -1      | Y             |       |
|      | 直行和向左转弯                                | 1010300400002410  | -1      | Y             |       |
|      | 最低限速50                                 | 1010301500002410  | 50      | Y             | 需支持   |
|      | 步行                                     | 1010301300002410  | -1      | Y             |       |
|      | 非机动车行驶                                 | 1010302014002410  | -1      | Y             |       |
|      | 机动车行驶                                  | 1010302012002410  | -1      | Y             |       |
|      | 靠左侧道路行驶                                | 1010300800002410  | -1      | Y             |       |
|      | 立交直行和右转弯行驶                             | 1010301000002410  | -1      | Y             |       |
|      | 立交直行和左转弯行驶                             | 1010300900002410  | -1      | Y             |       |
|      | 鸣喇叭                                    | 1010301400002410  | -1      | Y             | 非强制性  |
|      | 向左和向右转弯                                | 1010300600002410  | -1      | Y             |       |
|      | 停车位                                    | 1010302111002410  | -1      |               | 需支持   |
| 警告标志 | 慢行                                     | 1010103200001110  | -1      | Y             |       |
|      | 向左急转弯                                  | 1010100211001110  | -1      | Y             |       |
|      | 向右急转弯                                  | 1010100212001110  | -1      | Y             |       |
|      | T形交叉                                   | 1010100121001110  | -1      | Y             |       |
|      | T形交叉                                   | 1010100123001110  | -1      | Y             |       |
|      | 十字交叉                                   | 1010100111001110  | -1      | Y             |       |
|      | 注意儿童标志                                 | 1010101100001110  | -1      | Y             | 需支持   |
|      | 上坡路                                    | 1010100511001110  | -1      | Y             |       |
|      | 施工                                     | 1010103500001110  | -1      | Y             | 需支持   |
|      | 左侧变窄                                   | 1010100711001110  | -1      | Y             |       |
|      | 两侧变窄                                   | 1010100713001110  | -1      | Y             |       |
|      | 无人看守铁路道口                               | 1010102812001110  | -1      | Y             | 需支持   |
|      | T形交叉                                   | 1010100122001110  | -1      | Y             |       |
|      | 傍山险路-左                                 | 1010101811001110  | -1      | Y             |       |
|      | 傍山险路-右                                 | 1010101812001110  | -1      | Y             |       |
|      | 村庄标志                                   | 1010102000001110  | -1      | Y             |       |
|      | 堤坝路-左                                  | 1010101911001110  | -1      | Y             |       |
|      | 堤坝路-右                                  | 1010101912001110  | -1      | Y             |       |
|      | 丁字平面交叉                                 | 1010104012001110  | -1      | Y             |       |
|      | 渡口标志                                   | 1010102200001110  | -1      | Y             |       |
|      | 注意落石标志                                 | 1010101512001110  | -1      | Y             |       |
|      | 反向弯路-左                                 | 1010100311001110  | -1      | Y             |       |
|      | 反向弯路-右                                 | 1010100312001110  | -1      | Y             |       |
|      | 过水路面                                   | 1010102700001110  | -1      | Y             |       |
|      | 交叉路口                                   | 1010100112001110  | -1      | Y             |       |
|      | 车辆汇入-左                                 | 1010100131001110  | -1      | Y             |       |
|      | 车辆汇入-右                                 | 1010100132001110  | -1      | Y             |       |
|      | 环形交叉                                   | 1010100141001110  | -1      | Y             |       |
|      | 交叉路口                                   | 1010100134001110  | -1      | Y             |       |
|      | 连续弯路                                   | 1010100400001110  | -1      | Y             |       |
|      | 连续下坡                                   | 1010100711001110  | -1      | Y             |       |
|      | 路面不平                                   | 1010102400001110  | -1      | Y             |       |
|      | 注意雨（雪）天                                | 1010104312001110  | -1      | Y             |       |
|      | 注意残疾人                                  | 1010103000001110  | -1      | Y             | 需支持   |
|      | 注意野生动物                                 | 1010101300001110  | -1      | Y             |       |
|      | 事故易发路段                                 | 1010103100001110  | -1      | Y             |       |
|      | 注意潮汐车道                                 | 1010103800001110  | -1      | Y             |       |
|      | 注意不利气象条件                               | 1010104314001110  | -1      | Y             |       |
|      | 路面低洼                                   | 1010102600001110  | -1      | Y             |       |
|      | 路面高突                                   | 1010102500001110  | -1      | Y             |       |
|      | 下坡路                                    | 1010100600001110  | -1      | Y             |       |
|      | 注意前方车辆排队                               | 1010104400001110  | -1      | Y             |       |
|      | 十字平面交叉                                 | 1010104011001110  | -1      | Y             |       |
|      | 隧道标志                                   | 1010102100001110  | -1      | Y             |       |
|      | 隧道开车灯                                  | 1010103700001110  | -1      | Y             |       |
|      | 驼峰桥                                    | 1010102300001110  | -1      | Y             |       |
|      | 右侧变窄                                   | 1010100712001110  | -1      | Y             |       |
|      | 注意非机动车                                 | 1010102900001110  | -1      | Y             |       |
|      | 易滑标志                                   | 1010101700001110  | -1      | Y             |       |
|      | 注意信号灯                                  | 1010101400001110  | -1      | Y             |       |
|      | 右侧绕行                                   | 1010103313001110  | -1      | Y             |       |
|      | 窄桥标志                                   | 1010100800001110  | -1      | Y             |       |
|      | 注意保持车距                                 | 1010103900001110  | -1      | Y             |       |
|      | 注意合流                                   | 1010104111001110  | -1      | Y             |       |
|      | 注意横风标志                                 | 1010101600001110  | -1      | Y             |       |
|      | 注意路面结冰                                 | 1010104313001110  | -1      | Y             |       |
|      | 注意落石标志                                 | 1010101511001110  | -1      | Y             |       |
|      | 注意危险                                   | 1010103400001110  | -1      | Y             |       |
|      | 注意雾天                                   | 1010104313001110  | -1      | Y             |       |
|      | 注意牲畜标志                                 | 1010101200001110  | -1      | Y             |       |
|      | 左侧绕行                                   | 1010103313001110  | -1      | Y             |       |
|      | 左右绕行                                   | 1010103311001110  | -1      | Y             |       |
|      | 双向交通标志                                 | 1010100900001110  | -1      | Y             |       |
|      | 注意行人                                   | 1010101000001110  | -1      |               | 需支持   |
| 禁令标志 | 禁止直行                                   | 1010202400001410  | -1      | Y             |       |
|      | 禁止机动车驶入                                | 1010200600001410  | -1      | Y             |       |
|      | 限速120                                  | 1010203800001410  | 120     | Y             |       |
|      | 限速100                                  | 1010203800001410  | 100     | Y             |       |
|      | 限速80                                   | 1010203800001410  | 80      | Y             |       |
|      | 限速70                                   | 1010203800001410  | 70      | Y             | 需支持   |
|      | 限速60                                   | 1010203800001410  | 60      | Y             |       |
|      | 限速50                                   | 1010203800001410  | 50      | Y             | 需支持   |
|      | 限速40                                   | 1010203800001410  | 40      | Y             |       |
|      | 限速30                                   | 1010203800001410  | 30      | Y             |       |
|      | 停车让行                                   | 1010200100001910  | -1      | Y             |       |
|      | 限高5米                                   | 1010203500001410  | 5       | Y             |       |
|      | 限速20                                   | 1010203800001410  | 20      | Y             |       |
|      | 限速05                                   | 1010203800001410  | 5       | Y             |       |
|      | 禁止驶入                                   | 1010200500001510  | -1      | Y             |       |
|      | 禁止二轮摩托车驶入                              | 1010201400001410  | -1      | Y             |       |
|      | 限重55吨                                  | 1010203600001410  | 55      | Y             |       |
|      | 限重20吨                                  | 1010203700001410  | 20      | Y             |       |
|      | 禁止鸣喇叭                                  | 1010203300001410  | -1      | Y             |       |
|      | 禁止载货汽车驶入                               | 1010200700001410  | -1      | Y             |       |
|      | 限重30吨                                  | 1010203700001410  | 30      | Y             |       |
|      | 限重10吨                                  | 1010203700001410  | 10      | Y             |       |
|      | 禁止车辆临时或长时停放                            | 1010203111001710  | -1      | Y             | 需支持   |
|      | 限制轴重14吨                                | 1010203700001410  | 14      | Y             |       |
|      | 限制轴重13吨                                | 1010203700001410  | 13      | Y             |       |
|      | 限重40吨                                  | 1010203700001410  | 40      | Y             |       |
|      | 减速让行                                   | 1010200200002010  | -1      | Y             |       |
|      | 禁止向左转弯                                 | 1010202211001410  | -1      | Y             |       |
|      | 禁止运输危险物品车辆驶入                           | 1010204100001410  | -1      | Y             |       |
|      | 禁止拖拉机驶入                                | 1010201200001410  | -1      | Y             |       |
|      | 禁止三轮车机动车通行                             | 1010201300001410  | -1      | Y             |       |
|      | 禁止小型客车驶入                               | 1010201000001410  | -1      | Y             |       |
|      | 禁止直行和向左转弯                              | 1010202600001410  | -1      | Y             |       |
|      | 禁止人力车进入                                | 1010202000001410  | -1      | Y             |       |
|      | 禁止人力货运三轮车进入                            | 1010201800001410  | -1      | Y             |       |
|      | 禁止人力客运三轮车进入                            | 1010201900001410  | -1      | Y             |       |
|      | 禁止向右转弯                                 | 1010202311001410  | -1      | Y             |       |
|      | 禁止向左向右转弯                               | 1010202500001410  | -1      | Y             |       |
|      | 禁止直行和向右转弯                              | 1010202700001410  | -1      | Y             |       |
|      | 禁止通行                                   | 1010200400001210  | -1      | Y             |       |
|      | 会车让行                                   | 1010200300002110  | -1      | Y             |       |
|      | 禁止小客车向右转弯                              | 1010202312001410  | -1      | Y             |       |
|      | 禁止载货汽车右转                               | 1010202315001410  | -1      | Y             |       |
|      | 禁止骑自行车下坡                               | 国标中无，可以屏蔽         |         | Y             | 非强制要求 |
|      | 禁止骑自行车上坡                               | 国标中无，可以屏蔽         |         | Y             | 非强制要求 |
|      | 解除禁止超车                                 | 1010203000001610  | -1      | Y             | 需支持   |
|      | 禁止小客车向左转弯                              | 1010202212001410  | -1      | Y             |       |
|      | 禁止超车                                   | 1010202900001410  | -1      | Y             |       |
|      | 禁止畜力车进入                                | 1010201700001410  | -1      | Y             |       |
|      | 禁止大型客车驶入                               | 1010200900001410  | -1      | Y             |       |
|      | 禁止电动三轮车驶入                              | 1010200800001410  | -1      | Y             |       |
|      | 禁止非机动车进入                               | 1010201600001410  | -1      | Y             |       |
|      | 禁止载货汽车左转                               | 1010202215001410  | -1      | Y             |       |
|      | 禁止汽车拖、挂车驶入                             | 1010201100001410  | -1      | Y             |       |
|      | 禁止行人进入                                 | 1010202100001410  | -1      | Y             |       |
|      | 禁止某两种车驶入                               | 1010201500001410  | -1      | Y             |       |
|      | 限制高度3.5米                               | 1010203800001410  | 3.5     | Y             |       |
|      | 限制宽度3米                                 | 1010203400001410  | 3       | Y             |       |
|      | 限制轴重10吨                                | 1010203700001410  | 10      | Y             |       |
|      | 海关标志                                   | 1010204200001410  | -1      | Y             |       |
|      | 停车检查                                   | 1010204000001410  | -1      | Y             |       |
|      | 禁止车辆长时停放                               | 1010203200001710  | -1      | Y             |       |
|      | 解除限制速度40                               | 1010203900001610  | -1      | Y             | 需支持   |
|      | 禁止掉头                                   | 1010202800001410  | -1      |               | 需支持   |
| 信号灯  | 竖排全方位灯(verticalOmnidirectionalLight)   | 1000001           | -1      | Y             |       |
|      | 竖排直行圆灯(verticalStraightRoundLight)     | 1000011           | 60      | Y             | 需支持   |
|      | 竖排左转灯(verticalLeftTurnLight)           | 1000011           | 10      | Y             |       |
|      | 竖排直行灯(verticalStraightLight)           | 1000011           | 30      | Y             |       |
|      | 竖排右转灯(verticalRightTurnLight)          | 1000011           | 20      | Y             |       |
|      | 竖排掉头灯(verticalUTurnLight)              | 1000011           | 70      | Y             |       |
|      | 竖排人行灯(verticalPedestrianLight)         | 1000002           | -1      | Y             |       |
|      | 横排全方位灯(horizontalOmnidirectionalLight) | 1000003           | -1      | Y             |       |
|      | 横排直行圆灯(horizontalStraightRoundLight)   | 1000021           | 60      | Y             |       |
|      | 横排左转灯(horizontalLeftTurnLight)         | 1000021           | 10      | Y             |       |
|      | 横排直行灯(horizontalStraightLight)         | 1000021           | 30      | Y             |       |
|      | 横排右转灯(horizontalRightTurnLight)        | 1000021           | 20      | Y             |       |
|      | 横排掉头灯(horizontalUTurnLight)            | 1000021           | 70      | Y             |       |
|      | 单车指示灯（bicycleLight）                    | 1000013           | -1      |               |       |
|      | 双色指示灯（twoColorIndicatorLight）          | 1000009           | -1      |               |       |

## 4.2 object标志
| 类型   | 子类型             | name（英文描述）                                          | type（ASAM规范优先） | 腾讯  | 项目组评审 |
|------|-----------------|-----------------------------------------------------|----------------|------|-------|
| 杆    | 竖杆              | Vertical_Pole                                       | pole           | Y    | 需支持   |
|      | 横杆              | Cross_Pole                                          | pole           | Y   | 需支持   |
|      | 悬臂式柱            | Cantilever_Pole                                     | pole           |     | 需支持   |
|      | 6米柱             | Pillar_Pole_6m                                      | pole           |     | 需支持   |
|      | 3米柱             | Pillar_Pole_3m                                      | pole           |     | 需支持   |
| 指示标线 | 左弯待转区线          | Turn_Left_Waiting                                   | none           | Y    | 需支持   |
|      | 人行横道线           | Crosswalk_Line                                      | none           | Y    | Y    |
|      | 人行横道预告标识线       | Crosswalk_Warning_Line                              | none           | Y    |      |
|      | 白色折线车距确认线       | White_Broken_Line_Vehicle_Distance_Confirmation     | none           | Y    |     |
|      | 白色半圆状车距确认线      | White_Semicircle_Line_Vehicle_Distance_Confirmation | none           | Y    |      |
|      | 指示直行            | Arrow_Forward                                       | none           | Y    | Y    |
|      | 指示前方可直行或左转      | Arrow_Left_And_Forward                              | none           | Y    | Y    |
|      | 指示前方左转          | Arrow_Left                                          | none           | Y    | Y      |
|      | 指示前方右转          | Arrow_Right                                         | none           | Y    | Y      |
|      | 指示前方可直行或右转      | Arrow_Right_And_Forward                             | none           | Y    | Y       |
|      | 指示前方掉头          | U_Turns                                             | none           | Y    | Y      |
|      | 指示前方可直行或掉头      | Arrow_Forward_And_U_Turns                           | none           | Y    | Y   |
|      | 指示前方可左转或掉头      | Arrow_Left_And_U_Turns                              | none           | Y    | Y   |
|      | 指示前方道路仅可左右转弯    | Arrow_Left_And_Right                                | none           | Y    | Y   |
|      | 转弯直行            | Arrow_Turn_And_Straight                             | none           |   | 需支持   |
|      | 提示前方道路有左弯或需向左合流 | Turn_And_Merge_Left                                 | none           | Y   | 需支持   |
|      | 提示前方道路有右弯或需向右合流 | Turn_And_Merge_Right                                | none           | Y   | 需支持   |
|      | 路面文字标记(100-120) | Word_Mark_100_120                                   | none           | Y    |    |
|      | 路面文字标记(80-100)  | Work_Mark_80_100                                    | none           | Y    |     |
|      | 非机动车道路面标记       | Non_Motor_Vehicle                                   | none           | Y    | Y     |
|      | 残疾人专用停车位路面标记    | Disabled_Parking_Space_Road_Mark                    | none           | Y    |        |
| 警告标线 | 车行道横向减速标线       | Lateral_Deceleration_Marking                        | none           | Y    |       |
|      | 车行道纵向减速标线       | Longitudinal_Deceleration_ Marking                  | none           | Y    |       |
| 禁止标线 | 停止线             | Stop_Line                                           | none           | Y    |    |
|      | 停车让行线           | Stop_To_Give_Way                                    | none           | Y    |      |
|      | 减速让行线           | Slow_Down_To_Give_Way                               | none           | Y    |    |
|      | 圆形中心圈           | Circular_Center_Circle                              | none           | Y    |      |
|      | 菱形中心圈           | Rhombus_Center_Circle                               | none           | Y    |      |
|      | 网状线             | Mesh_Line                                           | none           | Y    |     |
|      | 公交专用车道线         | Bus_Only_Lane_Line                                  | none           | Y    | 需支持   |
|      | 小型车专用车道线        | Small_Cars_Lane_Line                                | none           | Y     | 需支持   |
|      | 大型车专用车道线        | Big_Cars_Lane_Line                                  | none           | Y         | 需支持   |
|      | 非机动车道线          | Non_Motor_Vehicle                                   | none           | Y    |    |
|      | 禁止转弯标记          | Turning_Forbidden                                   | none           | Y    |       |
| 其他标线 | 停车位标线           | Parking_Space_Mark                                  | none           | Y       | 需支持   |
|      | 限时停车线           | Time_Limit_Parking_Space_Mark                       | none           | Y    |      |
| 停车位  | 6米停车位           | Parking_6m                                          | parkingSpace   |         | 需支持   |
|      | 5米停车位           | Parking_5m                                          | parkingSpace   |          | 需支持   |
|      | 45°停车位          | Parking_45deg                                       | parkingSpace   |         | 需支持   |
|      | 60°停车位          | Parking_60deg                                       | parkingSpace   |           | 需支持   |
| 路测设备 | 摄像头             | Camera                                              | none           | Y        | 需支持   |
|      | 毫米波雷达           | Millimeter_Wave_Radar                               | none           | Y          | 需支持   |
|      | RSU             | RSU                                                 | none           | Y         | 需支持   |
| 地面污损 | 坑洼              | Pothole                                             | none           | Y        | 需支持   |
|      | 补丁              | Patch                                               | patch          | Y          | 需支持   |
|      | 裂缝              | Crack                                               | barrier        | Y          | 需支持   |
|      | 沥青线             | Asphalt_Line                                        | none           | Y          | 需支持   |
|      | 车轮痕迹            | Rut_Track                                           | none           | Y          | 需支持   |
|      | 积水              | Stagnant_Water                                      | none           | Y          | 需支持   |
|      | 凸起              | Protrusion                                          | obstacle       | Y          | 需支持   |
| 交通管理 | 井盖              | Well_Cover                                          | none           | Y      | 需支持   |
|      | 交通锥             | Traffic_Cone                                        | obstacle       |        | 需支持   |
|      | 交通护栏            | Traffic_Barrier                                     | obstacle       |         | 需支持   |
|      | 水马              | Traffic_Horse                                       | obstacle       |      |            |
|      | 路灯              | Lamp                                                | none           |         | 需支持   |
|      | 垃圾桶             | GarbageCan                                          | none           |         | 需支持   |
|      | 障碍物             | Obstacle                                            | obstacle       |         | 需支持   |
|      | 路沿石             | Road_Curb                                           | barrier        |         | 需支持   |
|      | 减速带             | Deceleration_Zone                                   | barrier        | Y       | 需支持   |
|      | 反光路标            | Reflective_Road_Sign                                | none           | Y       | 需支持   |
|      | 停车杆             | Parking_Hole                                        | barrier        | Y       | 需支持   |
|      | 停车桩             | Parking_Lot                                         | barrier        | Y       | 需支持   |
|      | 地锁              | Ground_Lock                                         | barrier        | Y       | 需支持   |
|      | 塑胶挡车器           | Plastic_Vehicle_Stopper                             | barrier        | Y    | 需支持   |
|      | U型挡车器           | Parking_Limit_Position_Pole_2m                      | barrier        | Y   |       |
|      | 支撑型挡车器          | Support_Vehicle_Stopper                             | barrier        | Y    | 需支持   |
|      | 充电桩             | Charging_Station                                    | barrier        | Y    | 需支持   |
| 植被   | 树木              | Tree                                                | tree           | Y    |       |
|      | 灌木              | Shrub                                               | vegetation     |      | 需支持   |
|      | 草坪              | Grass                                               | vegetation     |      | 需支持   |
| 建筑   | 居民楼             | Building                                            | building       | Y    |       |
