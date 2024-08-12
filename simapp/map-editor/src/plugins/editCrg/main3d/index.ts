import { Mesh } from 'three'
import { disposeRoadMask, renderRoadMask } from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeMaterial } from '@/utils/common3d'
import { useRoadStore } from '@/stores/road'
import { getLaneBoundaryOnBothSides } from '@/stores/road/lane'

class EditCrgHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editCrgContainer')
  }

  // 计算所有道路的交互蒙层元素
  async computeRoadMask () {
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

    // 一次性渲染所有的辅助蒙层
    await Promise.all(renderMaskPromise)
  }

  async onActivate () {
    await this.computeRoadMask()
  }

  onDeactivate () {
    // eslint-disable-next-line no-unused-expressions
    this.clear
  }

  clear () {
    // 销毁所有道路和路口的蒙层
    disposeRoadMask({ parent: this.container })
    this.container.clear()
  }

  dispose () {
    this.container.traverse((child) => {
      if (child instanceof Mesh) {
        child.geometry.dispose()
        disposeMaterial(child)
      }
    })
  }
}

export default EditCrgHelper
