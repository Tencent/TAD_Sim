import { Vector3 } from 'three'
import {
  createTextSpriteInJunction,
  disposeAllTextSprite,
  disposeJunctionHelperArea,
  disposeJunctionMask,
  disposeRoadHelperArea,
  disposeVirtualObject,
  renderJunctionHelperArea,
  renderJunctionMask,
  renderRoadHelperArea,
} from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { useRoadStore } from '@/stores/road'
import { useJunctionStore } from '@/stores/junction'
import { getLaneBoundaryOnBothSides } from '@/stores/road/lane'
import { Constant } from '@/utils/business'
import { getJunction, getRoad } from '@/utils/mapCache'
import { getEndPointAlongVec3 } from '@/utils/common3d'
import pool from '@/services/worker'
import i18n from '@/locales'
import root3d from '@/main3d/index'
import { resetAllRoadSignColor } from '@/main3d/render/roadSign'
import { disposeBox3Helper, resetAllObjectOnPole } from '@/main3d/render/object'
import { resetAllParkingSpaceColor } from '@/main3d/render/parkingSpace'
import { resetAllPoleColor } from '@/main3d/render/pole'
import { resetAllOtherObject } from '@/main3d/render/other'
import { resetAllCustomModel } from '@/main3d/render/customModel'

class EditObjectHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editObjectContainer')
  }

  // 获取两个点连接方向上，左右两侧偏移一定距离的坐标点
  getSidePoints (params: {
    leftPoint: common.vec3
    rightPoint: common.vec3
    rightOffset: number
    leftOffset: number
  }) {
    const { leftPoint, rightPoint, rightOffset, leftOffset } = params

    const lpVec3 = new Vector3(leftPoint.x, leftPoint.y, leftPoint.z)
    const rpVec3 = new Vector3(rightPoint.x, rightPoint.y, rightPoint.z)
    const directionVec3 = rpVec3.clone().sub(lpVec3).normalize()
    const rightOffsetPoint = rpVec3
      .clone()
      .addScaledVector(directionVec3, rightOffset)
    const leftOffsetPoint = lpVec3
      .clone()
      .addScaledVector(directionVec3, -leftOffset)
    return {
      left: leftOffsetPoint,
      right: rightOffsetPoint,
    }
  }

  // 计算所有【道路】和【路口】的路边辅助放置区域的采样点范围
  async computeRoadAndJunctionHelperArea () {
    const computeRoadPromise = []
    const roadStore = useRoadStore()

    const roadMap = new Map()

    for (const roadId of roadStore.ids) {
      const road = roadStore.getRoadById(roadId)
      if (!road) continue

      // 将当前道路的边缘区域拓宽，形成可以放置元素的辅助区域
      computeRoadPromise.push(
        (() => {
          return new Promise((resolve) => {
            const leftPoints = [] // 最左侧
            const rightPoints = [] // 最右侧
            // 先判断当前道路是否存在正反向的车道
            let hasForwardLane = false
            let hasReverseLane = false
            for (const lane of road.sections[0].lanes) {
              if (Number(lane.id) < 0) {
                hasForwardLane = true
              } else {
                hasReverseLane = true
              }
            }
            // 根据实际的车道反向，来判定具体的偏移量
            const rightOffset = hasForwardLane ?
              Constant.expandMaxOffset :
              Constant.expandMinOffset
            const leftOffset = hasReverseLane ?
              Constant.expandMaxOffset :
              Constant.expandMinOffset

            for (const section of road.sections) {
              const isHeadSection = section.id === '0'
              const isTailSection =
                Number(section.id) === road.sections.length - 1

              const { left: leftBoundary, right: rightBoundary } =
                getLaneBoundaryOnBothSides(section)
              if (!leftBoundary || !rightBoundary) continue

              // 手动添加首个section的第一个顶点
              if (isHeadSection) {
                const { left: lp, right: rp } = this.getSidePoints({
                  leftPoint: leftBoundary.samplePoints[0],
                  rightPoint: rightBoundary.samplePoints[0],
                  rightOffset,
                  leftOffset,
                })
                leftPoints.push(lp)
                rightPoints.push(rp)
              }

              // 中间的点通过遍历获取
              for (
                let i = 2;
                i < leftBoundary.samplePoints.length - 2;
                i += 2
              ) {
                // 每3个点取1个
                const { left: lp, right: rp } = this.getSidePoints({
                  leftPoint: leftBoundary.samplePoints[i],
                  rightPoint: rightBoundary.samplePoints[i],
                  rightOffset,
                  leftOffset,
                })
                leftPoints.push(lp)
                rightPoints.push(rp)
              }

              // 手动添加最后一个section的最后一组顶点
              if (isTailSection) {
                const { left: lp, right: rp } = this.getSidePoints({
                  leftPoint:
                    leftBoundary.samplePoints[
                      leftBoundary.samplePoints.length - 1
                    ],
                  rightPoint:
                    rightBoundary.samplePoints[
                      rightBoundary.samplePoints.length - 1
                    ],
                  rightOffset,
                  leftOffset,
                })
                leftPoints.push(lp)
                rightPoints.push(rp)
              }
            }

            // 缓存计算过的 road 相关数据，方便计算 junction 时查找
            roadMap.set(roadId, {
              roadId,
              leftPoints,
              rightPoints,
            })

            // 绘制每一条道路的辅助放置区域
            renderRoadHelperArea({
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
    // 先计算所有的道路的辅助放置区域
    await Promise.all(computeRoadPromise)

    // 在计算完成道路的辅助放置区域采样点后，再计算路口对应的区域
    const junctionStore = useJunctionStore()
    const computeJunctionPromise = []
    for (const junctionId of junctionStore.ids) {
      const junction = getJunction(junctionId)
      if (!junction) continue
      const { linkRoads } = junction
      const refBoundary: Array<biz.IRefRoad> = []

      // 如果交叉路口连接的道路数量小于 2，则无法形成一个路口的闭合区域
      if (linkRoads.length < 2) continue

      // 如果形成路口的连接道路为 2 条，则允许相近的端点作为路口区域边界关键点
      const enableClosePoint = linkRoads.length === 2

      for (const linkRoad of linkRoads) {
        const [roadId, percent, direction] = linkRoad.split('_')
        const roadOption = roadMap.get(roadId)
        const road = getRoad(roadId)
        if (!roadOption || !road) continue

        const { leftPoints, rightPoints } = roadOption
        const isTail = percent === '1'
        // 根据路口中，连接的道路首尾关系，获取对应的左右最外侧采样点
        const leftPoint = isTail ?
          leftPoints[leftPoints.length - 1] :
          leftPoints[0]
        const rightPoint = isTail ?
          rightPoints[rightPoints.length - 1] :
          rightPoints[0]
        refBoundary.push({
          roadId,
          isTail,
          direction,
          alongVec: getEndPointAlongVec3(road.keyPath, isTail),
          leftPoint,
          rightPoint,
        })
      }

      const payload = {
        refBoundary,
        enableClosePoint,
        isMultipleRoad: linkRoads.length > 2,
        junctionId,
      }

      // 通过 workpool 来辅助计算
      computeJunctionPromise.push(pool.exec('getJunctionGeoAttr', [payload]))
    }

    const computedJunctionRes = await Promise.all(computeJunctionPromise)

    // 根据 junction 结算结构，渲染 junction 的辅助放置区域的网格
    for (const res of computedJunctionRes) {
      const { junctionId, vertices, indices } = res
      if (!junctionId) continue

      renderJunctionHelperArea({
        junctionId,
        vertices,
        indices,
        parent: this.container,
      })
    }

    // 手动清除缓存
    roadMap.clear()
  }

  // 计算所有路口的辅助交互蒙层元素
  async computeJunctionMaskHelper () {
    const junctionStore = useJunctionStore()
    const renderMaskPromise = []
    for (const junctionId of junctionStore.ids) {
      const junction = getJunction(junctionId)
      if (!junction) continue

      // 创建路口的文字描述
      createTextSpriteInJunction({
        junction,
        content: `${i18n.global.t('desc.junction')}-${junctionId}`,
        parent: this.container,
      })

      const { geoAttr } = junction
      if (!geoAttr) continue
      const { vertices, indices } = geoAttr
      renderMaskPromise.push(
        (() => {
          return new Promise((resolve) => {
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

    // 一次性渲染所有辅助蒙层
    await Promise.all(renderMaskPromise)
  }

  async onActivate () {
    await this.computeRoadAndJunctionHelperArea()
    await this.computeJunctionMaskHelper()
  }

  async onDeactivate () {
    this.clear()
  }

  // 销毁拖拽交互过程中的辅助放置元素
  clearVirtualElements () {
    // 销毁不同类型物体的预览放置元素
    disposeVirtualObject({
      parent: this.container,
      isAll: true,
    })
  }

  // 清空当前编辑模式的容器中所有内容
  clear () {
    // 销毁道路和路口的辅助放置区域网格
    disposeRoadHelperArea({
      parent: this.container,
    })
    disposeJunctionHelperArea({
      parent: this.container,
    })
    disposeJunctionMask({
      parent: this.container,
    })
    disposeAllTextSprite(this.container)

    // 重置物体编辑模式所有物体的高亮效果
    resetAllRoadSignColor(root3d.mapElementsContainer)
    resetAllObjectOnPole(root3d.mapElementsContainer)
    resetAllParkingSpaceColor(root3d.mapElementsContainer)
    resetAllPoleColor(root3d.mapElementsContainer)
    resetAllOtherObject(root3d.mapElementsContainer)
    resetAllCustomModel(root3d.mapElementsContainer)

    // 销毁物体的包围盒外框
    disposeBox3Helper()

    this.clearVirtualElements()

    this.container.clear()
  }

  dispose (): void {}
}

export default EditObjectHelper
