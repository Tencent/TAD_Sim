import type { Intersection } from 'three'
import { createDefaultObjectData } from '../common'
import type { IRoadSignConfig } from '@/config/roadSign'
import { genObjectId } from '@/utils/guid'
import { setObject } from '@/utils/mapCache'
import { Constant } from '@/utils/business'

// 创建非机动车禁驶区标线
export function createNonMotorVehicle (params: {
  option: IRoadSignConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const { name, showName, type, subtype } = option
  const roadSignData: biz.IRoadSign = {
    ...createDefaultObjectData(),
    size: [0, 0],
    basicWidth: 0,
    basicLength: 0,
    basicHeight: 0,
  }
  roadSignData.id = genObjectId()
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype

  // 基于鼠标交互结果，更新物体的【空间属性】
  const { object, point: intersectionPoint } = intersection
  // @ts-expect-error
  const { junctionId } = object
  if (!junctionId) return null

  // 非机动车禁驶区提供默认的尺寸
  roadSignData.length = Constant.nonMotorVehicleSize.default
  roadSignData.width = Constant.nonMotorVehicleSize.default

  // 放置在路口中
  // 更新路面标识源数据的属性
  roadSignData.position = {
    x: intersectionPoint.x,
    y: intersectionPoint.y,
    z: intersectionPoint.z,
  }
  roadSignData.junctionId = junctionId

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}
