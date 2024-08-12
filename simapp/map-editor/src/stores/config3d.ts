import { defineStore } from 'pinia'
import { OperationModel, RenderModel } from '@/utils/business'
import { fixedPrecision } from '@/utils/common3d'

export const useConfig3dStore = defineStore('config3d', {
  state: () => ({
    zoom: 1,
    cameraType: 'O', // P：透视相机，O：正交相机
    cameraAngle: 0, // 透视视角下，相机位置于目标点形成的角度（朝北时为 0）
    renderModel: RenderModel.simple, // 默认渲染模式
    operationModel: OperationModel.select,
    showLaneDirection: false, // 是否展示车道的行驶方向
    roadSampleInterval: 20, // 道路曲线的采样间隔
    viewportSampleInterval: 8, // 根据视图窗口内容调整的采样间隔
    enableAutoCreateJunctionByCoverRoad: false, // 是否支持通过互相覆盖的道路自动生成交叉路口
    defaultLaneNumber: 2, // 默认的道路车道数
    enableUTurnLane: true, // 默认内侧车道为可掉头车道，并填充对应的 link 线
    defaultLaneWidth: 3.5, // 默认的车道宽度（主要是机动车道）
    roadType: 'default', // 默认创建道路的类型（默认为双向双车道）
    // 仅适用于机动车道
    defaultUTurnLane: 1, // 默认可掉头的车道数
    // 适用于机动车道和非机动车道
    defaultLeftTurnLane: 1, // 默认可左转的车道数
    defaultRightTurnLane: 1, // 默认可右转的车道数
    // 拖拽物体列表栏图片到可放置区域，三维场景区域dom展示的鼠标样式
    dragCursor: 'none' as 'none' | 'copy',
    // 环形道路模式下，默认预留的路口数
    reserveJunction: 1,
    // 鼠标射线指向的坐标系中的 xy 坐标
    rayCoordinateX: '',
    rayCoordinateY: '',
  }),
  actions: {
    resetCamera () {
      this.zoom = 1
      this.cameraType = 'O'
      this.cameraAngle = 0
    },
    // 在透视视角下，更新相机的朝向角度
    updateCameraAngle (angle: number) {
      this.cameraAngle = angle
    },
    toggleCameraTypeOnly (type: biz.ICameraType) {
      // 如果类型没有改变，则直接返回
      if (this.cameraType === type) return false
      this.cameraType = type
      return true
    },
    updateCameraZoom (value: number) {
      if (this.zoom === value) return false
      this.zoom = value
      return true
    },
    updateDragCursor (value: 'none' | 'copy' = 'none') {
      this.dragCursor = value
    },
    updateReserveJunction (value: number) {
      this.reserveJunction = value
    },
    // 更新绘制的默认道路类型
    updateRoadType (value: string) {
      this.roadType = value
    },
    // 更新鼠标射线投射到坐标系中的 xy 坐标
    updateRayCoordinate (x: number, y: number) {
      // 保留2位小数
      this.rayCoordinateX = String(fixedPrecision(x, 2))
      this.rayCoordinateY = String(fixedPrecision(y, 2))
    },
  },
})
