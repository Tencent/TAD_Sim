import signals from 'signals'

export default {

  signals: (function () {
    const Signal = signals.Signal
    const allSignals = {
      /// //// 场景编辑器
      modifyObject: new Signal(),
      // addObject: new Signal(),
      resetScene: new Signal(),
      addRoutePoint: new Signal(),
      closeEditeDialog: new Signal(),
      // routeDialogCheck: new Signal(),
      // 场景加载完成，指所有数据加载完成（包括地图），可用于获取场景中的对象
      sceneLoaded: new Signal(),
      // 播放页车辆实时数据
      trafficVPos: new Signal(),
      // 地图加载完成，仅用于单独加载地图的时候，与sceneLoaded互斥
      mapLoaded: new Signal(),
      playControllerStatus: new Signal(),
      submitScenario: new Signal(),
      // 聚焦选中物体
      // locateObject: new Signal(),
      // 场景中点选拾取物体
      pickedObject: new Signal(),
      // 拾取道路关键点
      pathNodeSelected: new Signal(),
      // 拖拽添加物体
      // dropAddObject: new Signal(),
      // 拖拽修改物体
      dropModifyObject: new Signal(),
      // 主车终点设置
      egoCarRouteEnd: new Signal(),
      // 取消选中物体==》通知界面
      cancelSelectObject2UI: new Signal(),
      // 鼠标点在场景中的位置信息==》通知界面
      mousePosition2UI: new Signal(),
      // 缩放消息
      zoomView: new Signal(),
      // 通知消息
      promptMessage: new Signal(),
      // 界面测量信息变更
      measurementChanged: new Signal(),

      // change camera mode
      operationMode: new Signal(),

      // 同步ui状态
      updateRenderState: new Signal(),

      // back to front
      updateUIState: new Signal(),

      cameraChanged: new Signal(),
    }

    return allSignals
  })(),
}
