declare namespace common {
  export interface pluginConfig {
    name: pluginName | ''
    description: string
    iconDesc: string
    iconComponentName: string
    componentPaths?: {
      leftArea?: Array<string>
      property?: Array<string>
    }
  }
  // 调整尺寸的模式: 三维等比例，二维等比例，长度单独调整（针对长条状的物体），长宽单独调整，长宽高单独调整
  // l:length, w:width, h:height
  export type adjustSizeMode = 'ratio3' | 'ratio2' | 'l' | 'lw' | 'lwh'
  export type colorType = 'White' | 'Yellow' | 'Red' | 'Green' | 'Blue'
  // 物体的交互状态：常规 | 鼠标经过 | 选中
  export type objectStatus = 'normal' | 'hovered' | 'selected'

  export type coordinate = 'x' | 'y' | 'z'

  export interface vec2 {
    x: number
    y: number
  }
  export interface vec3 {
    x: number
    y: number
    z: number
  }

  export type vec3Arr = [number, number, number]

  export interface vec3WithId extends vec3 {
    id: string
  }

  export type diffType = 'ADD' | 'DELETE' | 'UPDATE'
  export type elementType = 'road' | 'junction' | 'object'

  // TODO 形参的类型需要更精确
  export interface IDragDispatcher {
    dragStart: (options: any) => void
    dragEnd: (options: any) => void
    drag: (options: any) => void
    click: (options: any) => void
    hoverOn: (options: any) => void
    hoverOff: (options: any) => void
  }

  export interface IDragOptions {
    type: string
    object: any
  }
}
