import { defineStore } from 'pinia'
import { Box3, Mesh, Vector3 } from 'three'
import { useConfig3dStore } from './config3d'
import { getRoad } from '@/utils/mapCache'
import root3d from '@/main3d/index'
import ee, { CustomEventType } from '@/utils/event'
import {
  getGravityCenter,
  getVerticalVector,
  oCameraOffset,
  pCameraOffset,
} from '@/utils/common3d'

type dialogType =
  | 'mapList'
  | 'saveMap'
  | 'saveAsMap'
  | 'mapNotSave'
  | 'crg'
  | 'uploadModel'
type dialogVariableName =
  | 'mapFileDialogShow'
  | 'saveMapDialogShow'
  | 'saveAsMapDialogShow'
  | 'mapNotSaveDialogShow'
  | 'selectCrgDialogShow'
  | 'uploadModelDialogShow'

export const useHelperStore = defineStore('helper', {
  state: () => ({
    shortcutKeysEnabled: true,
    mapFileDialogShow: false,
    saveMapDialogShow: false,
    saveAsMapDialogShow: false,
    mapNotSaveDialogShow: false,
    selectCrgDialogShow: false,
    uploadModelDialogShow: false,
    // TODO 后续再优化成 promise 的形式
    mapNotSavaConfirmCallback: null as Function | null,
    selectedElement: '', // 当前选中的元素，形如 'type-id'
    // contextmenu 相关属性
    contextMenuShow: false,
    contextMenuLeft: 0,
    contextMenuTop: 0,
    removeElementId: '', // 要删除的元素
  }),
  getters: {},
  actions: {
    hideContextMenu () {
      this.removeElementId = ''
      this.contextMenuShow = false
      this.contextMenuLeft = 0
      this.contextMenuTop = 0
    },
    showContextMenu (params: { element: string, left: number, top: number }) {
      this.contextMenuShow = true
      this.removeElementId = params.element
      this.contextMenuLeft = params.left
      this.contextMenuTop = params.top
    },
    // 从结构树中删除地图元素
    removeElement (element: string) {
      ee.emit(CustomEventType.element.remove, element)
    },
    clearSelected () {
      this.selectedElement = ''
    },
    setSelectedElement (params: {
      element: string
      focus?: boolean // 是否要将相机看向元素
      emitEvent?: boolean // 是否要触发事件，让编辑模式更新状态
    }) {
      const { element, focus = false, emitEvent = false } = params
      this.selectedElement = element
      const [name, id] = element.split('-')

      if (emitEvent) {
        ee.emit(CustomEventType.element.select, element)
      }

      // 是否将相机聚焦到元素上
      if (focus) {
        let lookAtPoint
        const mapContainer = root3d.mapElementsContainer
        if (name === 'road') {
          const road = getRoad(id)
          // 获取道路参考线的中点
          if (road) {
            // 道路参考线中点
            const halfPoint = road.keyPath.getPointAt(0.5)
            const tangent = road.keyPath.getTangentAt(0.5)
            // 获取道路的宽度
            let roadWidth = 0
            road.sections[0].lanes.forEach((lane) => {
              roadWidth += lane.normalWidth
            })
            const vertical = getVerticalVector(tangent)
            const offsetPoint = halfPoint.clone()
            offsetPoint.addScaledVector(vertical, roadWidth / 2)
            lookAtPoint = offsetPoint.clone()
          }
        } else if (name === 'junction') {
          // 获取路口网格的中心
          let junctionMesh: Mesh
          mapContainer.traverse((child) => {
            if (
              child instanceof Mesh &&
              child.name === 'junction' &&
              child.junctionId === id
            ) {
              junctionMesh = child
            }
          })
          // @ts-expect-error
          if (junctionMesh) {
            // 计算路口几何体的包围盒
            junctionMesh.geometry.computeBoundingBox()
            if (!junctionMesh.geometry.boundingBox) return

            const box = new Box3()
            box
              .copy(junctionMesh.geometry.boundingBox)
              .applyMatrix4(junctionMesh.matrixWorld)
            // 获取包围盒的中心点
            const junctionCenter = new Vector3()
            box.getCenter(junctionCenter)

            lookAtPoint = junctionCenter.clone()
          }
        } else if (
          name === 'pole' ||
          name === 'roadSign' ||
          name === 'signalBoard' ||
          name === 'trafficLight' ||
          name === 'sensor' ||
          name === 'other' ||
          name === 'customModel' // 新增自定义模型
        ) {
          const gravityCenter = getGravityCenter({
            parent: mapContainer,
            objectId: id,
          })
          if (gravityCenter) {
            lookAtPoint = gravityCenter.clone()
          }
        }
        // TODO 切换相机视角看向元素（区分正交相机和透视相机）
        if (lookAtPoint) {
          const config3dStore = useConfig3dStore()
          if (config3dStore.cameraType === 'O') {
            // 如果是正交相机视角
            // 计算默认正交相机对应的位置偏移量
            const cameraPos = {
              x: lookAtPoint.x + oCameraOffset.x,
              y: lookAtPoint.y + oCameraOffset.y,
              z: lookAtPoint.z + oCameraOffset.z,
            }
            // 切换当前相机的视角看相指定的地图中心点
            root3d.updateCamera({
              type: 'O',
              position: cameraPos,
              target: lookAtPoint,
            })
          } else {
            // 透视视角
            const cameraPos = {
              x: lookAtPoint.x + pCameraOffset.x,
              y: lookAtPoint.y + pCameraOffset.y,
              z: lookAtPoint.z + pCameraOffset.z,
            }
            // 切换当前相机的视角看相指定的地图中心点
            root3d.updateCamera({
              type: 'P',
              position: cameraPos,
              target: lookAtPoint,
            })
          }

          root3d.core.render()
        }
      }
    },
    setMapNotSaveConfirmCallback (fn: Function) {
      if (!fn) return
      this.mapNotSavaConfirmCallback = fn
    },
    // 切换对话框的显示和隐藏状态
    toggleDialogStatus (stateName: dialogType, visible?: boolean) {
      let variableName: dialogVariableName | '' = ''
      switch (stateName) {
        case 'mapList': {
          variableName = 'mapFileDialogShow'
          break
        }
        case 'saveMap': {
          variableName = 'saveMapDialogShow'
          break
        }
        case 'saveAsMap': {
          variableName = 'saveAsMapDialogShow'
          break
        }
        case 'mapNotSave': {
          variableName = 'mapNotSaveDialogShow'
          break
        }
        case 'crg': {
          variableName = 'selectCrgDialogShow'
          break
        }
        case 'uploadModel': {
          variableName = 'uploadModelDialogShow'
          break
        }
      }
      if (!variableName) return false

      if (visible === undefined) {
        // 如果不存在指定状态，则取反切换
        this[variableName] = !this[variableName]

        return true
      }
      // 如果有指定状态，但跟上一个状态一致，则无须切换
      if (this[variableName] === visible) return false

      this[variableName] = visible
    },
    // 响应键盘热键相关
    openShortcutKeys () {
      this.shortcutKeysEnabled = true
    },
    closeShortcutKeys () {
      this.shortcutKeysEnabled = false
    },
  },
})
