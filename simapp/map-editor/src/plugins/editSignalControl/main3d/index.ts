import { Line, Mesh } from 'three'
import {
  disposeAllTextSprite,
  disposeJunctionMask,
  disposeRoadMask,
  renderJunctionMask,
  renderRoadMask,
} from './draw'
import TemplateHelper from '@/plugins/template/helper'
import { useJunctionStore } from '@/stores/junction'
import { useRoadStore } from '@/stores/road'
import { getLaneBoundaryOnBothSides } from '@/stores/road/lane'
import { disposeMaterial } from '@/utils/common3d'
import { getJunction } from '@/utils/mapCache'
import { disposeBox3Helper, resetAllObjectOnPole } from '@/main3d/render/object'
import root3d from '@/main3d/index'

interface IParams {
  render: Function
}

class EditSignalControlHelper extends TemplateHelper {
  constructor (params: IParams) {
    super(params)
  }

  init () {
    this.initContainer('editSignalControlContainer')
  }

  // 计算所有道路和路口辅助交互蒙层元素
  async computeRoadAndJunctionHelper () {
    const renderMaskPromise = []
    const roadStore = useRoadStore()
    for (const roadId of roadStore.ids) {
      const road = roadStore.getRoadById(roadId)
      if (!road) continue

      renderMaskPromise.push(
        (() => {
          return new Promise((resolve) => {
            const leftPoints = []
            const rightPoints = []
            for (const section of road.sections) {
              const isHeadSection = section.id === '0'
              const isTailSection =
                Number(section.id) === road.sections.length - 1

              const { left: leftBoundary, right: rightBoundary } =
                getLaneBoundaryOnBothSides(section)
              if (!leftBoundary || !rightBoundary) continue

              // 手动添加首个section的第一个顶点
              if (isHeadSection) {
                leftPoints.push(leftBoundary.samplePoints[0])
                rightPoints.push(rightBoundary.samplePoints[0])
              }

              // 中间的点通过遍历获取
              for (
                let i = 2;
                i < leftBoundary.samplePoints.length - 2;
                i += 2
              ) {
                // 每3个点取1个
                leftPoints.push(leftBoundary.samplePoints[i])
                rightPoints.push(rightBoundary.samplePoints[i])
              }

              // 手动添加最后一个section的最后一组顶点
              if (isTailSection) {
                leftPoints.push(
                  leftBoundary.samplePoints[
                    leftBoundary.samplePoints.length - 1
                  ],
                )
                rightPoints.push(
                  rightBoundary.samplePoints[
                    rightBoundary.samplePoints.length - 1
                  ],
                )
              }
            }

            // 绘制每一条道路的辅助放置区域
            renderRoadMask({
              roadId,
              leftPoints,
              rightPoints,
              parent: this.container,
            })

            resolve(roadId)
          })
        })(),
      )
    }

    const junctionStore = useJunctionStore()
    for (const junctionId of junctionStore.ids) {
      const junction = getJunction(junctionId)
      if (!junction) continue

      const { geoAttr } = junction
      if (!geoAttr) continue
      const { vertices, indices } = geoAttr

      renderMaskPromise.push(
        (() => {
          return new Promise((resolve) => {
            // 直接用路口元素的 geoAttr 来绘制辅助元素的结构
            renderJunctionMask({
              junctionId,
              vertices,
              indices,
              parent: this.container,
            })
            resolve(junctionId)
          })
        })(),
      )
    }

    // 一次性渲染所有的辅助蒙层
    await Promise.all(renderMaskPromise)
  }

  async onActivate () {
    await this.computeRoadAndJunctionHelper()
  }

  onDeactivate () {
    this.clear()
  }

  clear () {
    // 销毁所有道路和路口的蒙层
    disposeRoadMask(this.container)
    disposeJunctionMask(this.container)
    // 销毁所有的文字精灵
    disposeAllTextSprite(this.container)
    // 重置杆上所有信号灯的效果
    resetAllObjectOnPole(root3d.mapElementsContainer)
    // 销毁包围盒外框
    disposeBox3Helper()

    this.container.clear()
  }

  dispose () {
    this.container.traverse((child) => {
      if (child instanceof Mesh || child instanceof Line) {
        child.geometry.dispose()
        disposeMaterial(child)
      }
    })
  }
}

export default EditSignalControlHelper
