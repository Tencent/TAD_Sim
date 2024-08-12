HadmapSvr核心Api
# 1. 接口清单
| 序号 | 接口名称 | URI | Method |
| :-: | :- | :- | :-: |
| 1 | 地图查询接口 | /hadmapdata | POST |
| 2 | 打开地图接口 | /api/hadmaps/open/v3/{name} | GET |
| 3 | 保存地图接口 | /api/hadmap/save/v3 | POST |
| 4 | 打开场景接口 | /scenario/open | POST |
| 5 | 保存场景接口 | /scenario/save | POST |

# 2. 接口说明
## 2.1. 地图查询接口
| 总体说明 |||||
| :- | :- | :-: | :- | :-: |
| 目标 | 查询地图中道路、车道、交叉口等详情数据 ||||
| URI | /hadmapdata ||||
| Method | POST ||||
| <b>请求Header</b> |||||
| Content-Type | application/json ||||
| <b>请求路径参数</b> |||||
| 名称 | 类型 | 必填项 | 说明 ||
| <b>请求查询参数</b> |||||
| 名称 | 类型 | 必填项 | 说明 ||
| <b>请求Body参数</b> |||||
| 名称 | 类型 | 必填项 | 说明 | 适用范围 |
| cmd | string | Y | loadhadmap <br> getalllanedata <br> getalllaneboundarydata <br> getalllanelinkdata <br> getmapobjectdata <br> getallroaddata <br> gettrafficlights <br> querylonlat <br> querylonlatbypoint <br> queryinfobypt | ALL |
| mapname | string | Y | 查询地图名称，例如：xxx.xodr或xxx.sqlite | ALL |
| version | string | N | 查询地图版本，例如：1632973527 | ALL |
| startLon | double | N | 起点经度 | querylonlat <br> querylonlatbypoint <br> queryinfobypt |
| startLat | double | N | 起点纬度 | querylonlat <br> querylonlatbypoint <br> queryinfobypt |
| id | uint64 | N | element id | querylonlatbypoint |
| type | uint64 | N | element type | querylonlatbypoint |
| roadid | uint64 | N | road id | querylonlat |
| secid | uint64 | N | section id | querylonlat |
| laneid | int64 | N | lane id | querylonlat |
| shift | double | N | shift | querylonlat <br> querylonlatbypoint |
| offset | double | N | offset | querylonlat <br> querylonlatbypoint |
| cosBucket | string | N |  COS桶 | ALL |
| cosKey | string | N | COS路径，获取地图文件名，优先级高于mapname字段 | ALL |
| fineRsp | bool | N | | loadhadmap |
| <b>应答Header</b> |||||
| Content-Type | application/json ||||
| <b>应答Body参数</b> |||||
| 名称 | 类型 | 必填项 | 说明 ||
| <b>示例</b> |||||
```json
{
    "cmd":"loadhadmap",
    "mapname":"x.xodr",
    "version":"1670310633",
    "fineRsp":true
}
```
```json
// rsp
{
    "code":0,
    "data":{
        "unrealLevelIndex":0,
        "min":"x,y,z",
        "max":"x,y,z",
        "center":"x,y,z"
    },
    "message":"ok"
}
```
```json
// others
//// road
{
  "array": [
    {
      "array": [
        {"x": 32.16960467,"y": 21.5564208,"z": -0.00011779000000000001},
        {"x": 31.49588062,"y": 101.71935437,"z": -0.00089434},
        ...
      ],
      "count": 2,
      "dir": 0,
      "len": 80.16576465364858,
      "mat": 0,
      "roadid": 30,
      "type": 0
    },
    ...
  ],
  "center": "118.509368896484,0.000277545885,0.000000000000",
  "count": 8,
  "max": "118.510719299316,0.001197464648,0.000000000931",
  "min": "118.508018493652,-0.000642372877,-0.000000000931"
}
//// lane
{
  "array": [
    {
      "arr": 0,
      "array": [
        {"x": 33.91482499,"y": 21.57208486,"z": -0.00012689},
        {"x": 33.24111767,"y": 101.73302562,"z": -0.00090342},
        ...
      ],
      "count": 2,
      "id": -1,
      "lbid": 13,
      "len": 80.16377177043474,
      "rbid": 14,
      "rid": 30,
      "sid": 0,
      "sl": 0,
      "type": 1,
      "wid": 3.5
    },
    ...
  ],
  "center": "118.509368896484,0.000277545885,0.000000000000",
  "count": 16,
  "max": "118.510719299316,0.001197464648,0.000000000931",
  "min": "118.508018493652,-0.000642372877,-0.000000000931"
}
//// laneboundary
{
  "array": [
    {
      "array": [
        {"x": 32.16959629,"y": 21.55741807,"z": -0.0001178},
        {"x": 31.495889000000002,"y": 101.7183571,"z": -0.00089433},
        ...
      ],
      "count": 2,
      "id": 13,
      "mark": 65553
    },
    ...
  ],
  "center": "118.509368896484,0.000277545885,0.000000000000",
  "count": 24,
  "max": "118.510719299316,0.001197464648,0.000000000931",
  "min": "118.508018493652,-0.000642372877,-0.000000000931"
}
//// lanelink
{
  "array": [
    {
      "array": [
        {"x": 1.69310982,"y": -10.258414160000001,"z": -0.00000853},
        {"x": 4.85289856,"y": -10.619907040000001,"z": -0.00001074},
        ...
      ],
      "count": 28,
      "fid": -2,
      "frid": 10,
      "fsid": 0,
      "id": 36,
      "junctionid": 2,
      "len": 34.257578481539305,
      "tid": -2,
      "trid": 40,
      "tsid": 0
    },
    ...
  ],
  "center": "118.509368896484,0.000277545885,0.000000000000",
  "count": 24,
  "max": "118.510719299316,0.001197464648,0.000000000931",
  "min": "118.508018493652,-0.000642372877,-0.000000000931"
}
//// mapobject
{
  "array": [
    {
      "count": 0,
      "groundHeight": 0,
      "height": 0.6,
      "id": 5,
      "length": 0,
      "name": "pole-1",
      "pitch": 0,
      "roll": 0,
      "strSubType": "l2",
      "strType": "trafficLight",
      "type": 16,
      "userData": [
        {
          "s": "0.05"
        },
        {
          "t": "-7.088798366670616"
        }
      ],
      "width": 0.2,
      "x": 64.75977017134414,
      "y": -4.1113178653783224,
      "yaw": 3.09616898344662,
      "z": 5.999669900724851
    }
  ],
  "center": "118.509368896484,0.000277545885,0.000000000000",
  "count": 8,
  "max": "118.510719299316,0.001197464648,0.000000000931",
  "min": "118.508018493652,-0.000642372877,-0.000000000931"
}
```

## 2.2. 打开地图接口
| 总体说明 ||||
| :- | :- | :-: | :- |
| 目标 | 地图编辑器后台部分，解析地图文件，返回结构化数据 |||
| URI | /api/hadmaps/open/v3/{name} |||
| Method | GET |||
| <b>请求Header</b> ||||
|||||
| <b>请求路径参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| name | string | Y | 地图文件名 |
| <b>请求查询参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| version | string | Y | 地图版本 |
| bucket | string | Y | COS桶 |
| mapFileKey | string | Y | COS路径，地图文件路径 |
| mapJsonKey | string | N | COS路径，地图JSON路径 |
| <b>请求Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>应答Header</b> ||||
|||||
| <b>应答Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>示例</b> ||||
```shell
/api/hadmaps/open/v3/test-Li.xodr?version=1666686371&bucket=simulation-dev-1251316161&mapFileKey=maps/editor/dcacac2bb6ac428db53ebb42f988ca31/cefb16838c9e41b1bb4ed875152c9449/test-Li.xodr&mapJsonKey=/maps/editor/dcacac2bb6ac428db53ebb42f988ca31/test-Li.xodr.json
```
```json
// rsp
{
    "data":{
        ...
    },
    "geometry":{
        "header":{
            "date":"Tue Oct 25 08:26:11 2022",
            "east":0,
            "geoReference":"+proj=utm +zone=51 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
            "map_file_path":"/tmp/48f1de818b824b57951a31410fb2e461_1666686371#test-Li.xodr",
            "name":"",
            "north":0,
            "revMajor":1,
            "revMinor":4,
            "south":0,
            "userData":[...],
            "vendor":"tadsim",
            "version":1,
            "west":0
        },
        "road":[...]
    },
    "name":"1666686371#test-Li.xodr"
}
```
## 2.3. 保存地图接口
| 总体说明 ||||
| :- | :- | :-: | :- |
| 目标 | 地图编辑器后台部分，解析结构化数据，生成地图文件 |||
| URI | /api/hadmap/save/v3 |||
| Method | POST |||
| <b>请求Header</b> ||||
| Content-Type | application/json |||
| <b>请求路径参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>请求查询参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| id | string | N | |
| desc | string | N | 描述 |
| name | string | Y | 地图文件名 |
| version | string | Y | 地图版本 |
| bucket | string | Y | COS桶 |
| jsonKey | string | Y | COS路径，地图JSON路径 |
| mapData | JSON | N | 地图数据 |
| <b>请求Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>应答Header</b> ||||
| Content-Type | application/json |||
| <b>应答Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>示例</b> ||||
```json
{
    "id":null,
    "name":"test-Li.xodr",
    "desc":null,
    "mapData":{...},
    "bucket":"simulation-dev-1251316161",
    "jsonKey":"/maps/editor/dcacac2bb6ac428db53ebb42f988ca31/test-Li.xodr.json"
}
```
```json
// rsp
{
    "code":0,
    "data":{
        "alt":0,
        "bl":{
            "alt":0,
            "lat":-0.00436587305739522,
            "lon":118.51116180419922
        },
        "id":"",
        "lat":-0.0036334197502583265,
        "lon":118.51117706298828,
        "mapFileKey":"/maps/editor/dcacac2bb6ac428db53ebb42f988ca31/cefb16838c9e41b1bb4ed875152c9449/test-Li.xodr",
        "mapJsonKey":"/maps/editor/dcacac2bb6ac428db53ebb42f988ca31/test-Li.xodr.json",
        "name":"test-Li.xodr",
        "size":5722,
        "tr":{
            "alt":0,
            "lat":-0.0029009664431214333,
            "lon":118.51119232177734
        },
        "baseVersion":""
    },
    "message":"ok"
}
```
## 2.4. 打开场景接口
| 总体说明 ||||
| :- | :- | :-: | :- |
| 目标 | 场景编辑器后台部分，解析场景文件，返回结构化数据 |||
| URI | /sceneario/open |||
| Method | POST |||
| <b>请求Header</b> ||||
| Content-Type | application/json |||
| <b>请求路径参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>请求查询参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| id | string | N | |
| name | string | Y | 场景名，xxx.xosc |
| version | string | N | 场景版本 |
| bucket | string | Y | COS桶 |
| key | string | Y | COS路径，场景文件所在目录 |
| mapName | JSON | Y | 地图名 |
| mapVersion | string | Y | 地图版本 |
| adsUrl | string | N | COS路径，场景文件路径 |
| mapUrl | string | N | COS路径，地图文件路径 |
| <b>请求Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>应答Header</b> ||||
| Content-Type | application/json |||
| <b>应答Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>示例</b> ||||
```json
{
    "id": 123,
    "name": "highway.sim",
    "bucket": "xxx",
    "key": "scenes/1/dd5a7f87-7220-419f-a914-0f60f99e375a/",
    "mapName": "xxx.sqlite",
    "mapVersion": "12345676543"
}
```
```json
// rsp
{
    "grading":"",
    "infos":{
        "id":1,
        "info":"",
        "label":"",
        "map":"NASTway.xodr",
        "name":"Accident_01",
        "traffictype":""
    },
    "sim":{...},
    "traffic":{...}
}
```
## 2.5. 保存场景接口
| 总体说明 ||||
| :- | :- | :-: | :- |
| 目标 | 场景编辑器后台部分，解析结构化数据，生成场景文件 |||
| URI | /sceneario/save |||
| Method | POST |||
| <b>请求Header</b> ||||
| Content-Type | application/json |||
| <b>请求路径参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>请求查询参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| id | string | N | 场景ID，如果新增场景则不填 |
| name | string | Y | 场景名，xxx.xosc |
| version | string | N | 场景版本 |
| bucket | string | Y | COS桶 |
| key | string | Y | COS路径，场景文件所在目录 |
| mapName | JSON | Y | 地图名 |
| mapVersion | string | Y | 地图版本 |
| adsUrl | string | N | COS路径，场景文件路径 |
| mapUrl | string | N | COS路径，地图文件路径 |
| content | string | Y | 场景数据，JSON |
| <b>请求Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>应答Header</b> ||||
| Content-Type | application/json |||
| <b>应答Body参数</b> ||||
| 名称 | 类型 | 必填项 | 说明 |
| <b>示例</b> ||||
```json
{
    "id": 123,
    "name": "NASTway.xosc",
    "mapName": "xxx.sqlite",
    "mapVersion": "12345676543",
    "format": "SIM",
    "bucket": "xxx",
    "content": "{...}"
}
```
```json
// rsp
{
    "code": 0,
    "message": "ok",
    "data": null
}
```
