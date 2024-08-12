import EventEmitter from 'eventemitter3'

const ee = new EventEmitter()

// 鼠标移动时缓存的 event 对象，主要用于视角 zoom 缩放时没有透传对应事件对象
let pointerMoveEvent: PointerEvent | null = null
export function updatePointerMoveEvent (event: PointerEvent) {
  pointerMoveEvent = event
}
export function getPointerMoveEvent () {
  return pointerMoveEvent
}

// 自定义的事件
export const CustomEventType = {
  element: {
    remove: 'element-remove',
    select: 'element-select',
  },
  controlPoint: {
    hover: 'controlpoint-hover',
    select: 'controlpoint-select',
  },
  window: {
    pointerdown: 'window-pointerdown',
    contextmenu: 'window-contextmenu',
  },
  webglArea: {
    pointermove: 'webglarea-pointermove',
  },
  other: {
    updateCustomModelConfig: 'update-custom-model-config',
  },
}

export default ee
