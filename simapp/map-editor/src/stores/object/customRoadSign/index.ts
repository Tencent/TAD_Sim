import { getVerticalVector } from '@/utils/common3d'

/**
 * 获取一条曲线上某个超过范围对应百分比的偏移点
 * @param percent
 */
export function getPointOutOfScope (params: {
  curve: biz.ICurve3
  percent: number
  offset: number
}) {
  const { curve, percent, offset: tOffset } = params
  const roadLength = curve.getLength()
  // 取首尾顶点的切线方向
  let _percent = 0
  // 计算s方向的偏移量
  let sOffset = 0
  if (percent < 0) {
    sOffset = roadLength * (percent - 0)
  } else {
    _percent = 1
    sOffset = roadLength * (percent - 1)
  }

  const tangent = curve.getTangentAt(_percent)
  const endPoint = curve.getPointAt(_percent)
  // 切线延长线方向上的延伸点
  const refLinePoint = endPoint.clone().addScaledVector(tangent, sOffset)
  const vertical = getVerticalVector(tangent).normalize()
  const targetPoint = refLinePoint.clone().addScaledVector(vertical, tOffset)

  return targetPoint
}
