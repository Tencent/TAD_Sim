/* eslint-disable */
import signals from 'signals'

let Signal = signals.Signal

export default {
  signals: (function () {
    let allSignals = {
      // 场景加载完成，指所有数据加载完成（包括地图），可用于获取场景中的对象
      sceneLoaded: new Signal(),
      // 播放页车辆实时数据
      trafficVPos: new Signal(),
      // 地图加载完成，仅用于单独加载地图的时候，与sceneLoaded互斥
      mapLoaded: new Signal(),
      playControllerStatus: new Signal(),
      // 缩放消息
      zoomView: new Signal(),
      // 通知消息
      promptMessage: new Signal(),
      // 界面通知消息
      monitorUIInfo: new Signal(),

      cameraChanged: new Signal(),
      // log2world变更通知
      log2worldChanged: new Signal(),
      // 聚焦选中物体
      locateObject: new Signal(),
    }

    return allSignals
  })(),
}
