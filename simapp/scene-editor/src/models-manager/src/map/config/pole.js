// 竖杆
const verticalPoleConfig = {}

// 横杆
const horizontalPoleConfig = {}

export const poleConfig = [
  {
    mainType: 'pole',
    name: 'Vertical_Pole', // 原有的竖杆
    type: 'pole',
    subtype: '',
    // 竖杆三维模型的样式配置
    style: {
      vRadius: 0.1,
      height: 6,
      segment: 16,
    },
  },
  {
    mainType: 'pole',
    name: 'Cross_Pole', // 原有的横杆
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
  },
  {
    mainType: 'pole',
    name: 'Cantilever_Pole', // 悬臂式柱
    type: 'pole',
    subtype: '',
    style: {
      vRadius: 0.1,
      hRadius: 0.05,
      height: 6,
      length: 10, // 水平延伸部分长10m
      topOffset: 0,
      segment: 16,
    },
  },
  {
    mainType: 'pole',
    name: 'Pillar_Pole_3m', // 3米柱
    type: 'pole',
    subtype: '',
    style: {
      vRadius: 0.1,
      height: 3,
      segment: 16,
    },
  },
  {
    mainType: 'pole',
    name: 'Pillar_Pole_6m', // 6米柱
    type: 'pole',
    subtype: '',
    style: {
      vRadius: 0.1,
      height: 6,
      segment: 16,
    },
  },
]

// 通过 name 属性来查找杆的配置，目前只有竖杆和横杆两种类型
export function getPoleConfig (name) {
  for (const option of poleConfig) {
    if (option.name === name) {
      return option
    }
  }

  return null
}
