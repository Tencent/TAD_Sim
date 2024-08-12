// 杆相关的配置

import i18n from '@/locales'
import { getPermission } from '@/utils/permission'
import { ImgUrls } from '@/utils/urls'

export interface IPoleConfig {
  mainType: 'pole'
  name: string
  showName: string
  iconUrl: string
  type: string
  subtype: string
  // 可放置区域的名称
  placementAreaName: Array<string>
  // 三维模型的样式配置
  style: {
    // 垂直部分半径
    vRadius: number
    height: number
    // 三维模型网格密度划分
    segment: number
    // 水平部分半径
    hRadius?: number
    // 水平部分长度
    length?: number
    // 水平部分跟垂直部分连接处，在竖杆从上往下的距离
    topOffset?: number
  }
}

export const _poleConfig: Array<IPoleConfig> = [
  {
    mainType: 'pole',
    name: 'Vertical_Pole', // 原有竖杆
    showName: i18n.global.t('desc.editObject.verticalPole'),
    iconUrl: ImgUrls.pole.verticalPole,
    type: 'pole',
    subtype: '',
    // 竖杆三维模型的样式配置
    style: {
      vRadius: 0.1,
      height: 6,
      segment: 16,
    },
    // 可放置区域的名称
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'pole',
    name: 'Cross_Pole', // 原有横杆
    showName: i18n.global.t('desc.editObject.horizontalPole'),
    iconUrl: ImgUrls.pole.horizontalPole,
    type: 'pole',
    subtype: '',
    // 横杆三维模型的样式配置
    style: {
      vRadius: 0.1,
      hRadius: 0.05,
      height: 6,
      length: 8,
      topOffset: 0,
      segment: 16,
    },
    // 可放置区域的名称
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'pole',
    name: 'Cantilever_Pole', // 悬臂式柱
    showName: i18n.global.t('desc.editObject.cantileverPole'),
    iconUrl: ImgUrls.pole.horizontalPole,
    type: 'pole',
    subtype: '',
    // 横杆三维模型的样式配置
    style: {
      vRadius: 0.1,
      hRadius: 0.05,
      height: 6,
      length: 10, // 横杆长度 10m
      topOffset: 0,
      segment: 16,
    },
    // 可放置区域的名称
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'pole',
    name: 'Pillar_Pole_6m', // 6m柱
    showName: i18n.global.t('desc.editObject.pillarPole6m'),
    iconUrl: ImgUrls.pole.verticalPole,
    type: 'pole',
    subtype: '',
    // 竖杆三维模型的样式配置
    style: {
      vRadius: 0.1,
      height: 6,
      segment: 16,
    },
    // 可放置区域的名称
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'pole',
    name: 'Pillar_Pole_3m', // 3米柱
    showName: i18n.global.t('desc.editObject.pillarPole3m'),
    iconUrl: ImgUrls.pole.verticalPole,
    type: 'pole',
    subtype: '',
    // 竖杆三维模型的样式配置
    style: {
      vRadius: 0.1,
      height: 3,
      segment: 16,
    },
    // 可放置区域的名称
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
]
export const poleConfig: Array<IPoleConfig> = _poleConfig.filter((item) => {
  // 横杠
  const isHorizontal = ['Cross_Pole', 'Cantilever_Pole']
  // 竖行
  const isVertical = ['Vertical_Pole', 'Pillar_Pole_6m', 'Pillar_Pole_3m']

  if (isHorizontal.includes(item.name)) {
    return getPermission('action.mapEditor.models.pole.horizontal.view.enable')
  }
  if (isVertical.includes(item.name)) {
    return getPermission('action.mapEditor.models.pole.vertical.view.enable')
  }
  return true
})

// 通过 name 属性来查找杆的配置，目前只有竖杆和横杆两种类型
export function getPoleConfig (name: string) {
  for (const option of poleConfig) {
    if (option.name === name) {
      return option
    }
  }

  return null
}
