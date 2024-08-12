import type { Object3D } from 'three'
import type TemplateHelper from './helper'
import root3d from '@/main3d'

export interface IInitPluginParams {
  scene: Object3D
  render: Function
}

class TemplatePlugin {
  enabled: boolean // 插件是否可用
  config: common.pluginConfig
  scene: Object3D
  render: Function
  editHelper: TemplateHelper
  dragDispatchers: common.IDragDispatcher
  constructor () {
    this.enabled = false
    this.config = {
      name: '',
      description: '',
      iconDesc: '',
      iconComponentName: '',
    }
    this.dragDispatchers = {
      dragStart: () => {},
      dragEnd: () => {},
      drag: () => {},
      click: () => {},
      hoverOn: () => {},
      hoverOff: () => {},
    }
  }

  addDragListener () {
    const dragControls = root3d.dragControls
    dragControls.addEventListener(
      'dragstart',
      this.dragDispatchers.dragStart,
    )
    dragControls.addEventListener('dragend', this.dragDispatchers.dragEnd)
    dragControls.addEventListener('drag', this.dragDispatchers.drag)
    dragControls.addEventListener('click', this.dragDispatchers.click)
    dragControls.addEventListener('hoveron', this.dragDispatchers.hoverOn)
    dragControls.addEventListener('hoveroff', this.dragDispatchers.hoverOff)
  }

  removeDragListener () {
    const dragControls = root3d.dragControls
    dragControls.removeEventListener(
      'dragstart',
      this.dragDispatchers.dragStart,
    )
    dragControls.removeEventListener('dragend', this.dragDispatchers.dragEnd)
    dragControls.removeEventListener('drag', this.dragDispatchers.drag)
    dragControls.removeEventListener('click', this.dragDispatchers.click)
    dragControls.removeEventListener('hoveron', this.dragDispatchers.hoverOn)
    dragControls.removeEventListener(
      'hoveroff',
      this.dragDispatchers.hoverOff,
    )
  }

  activate () {
    this.enabled = true
    this.editHelper && this.editHelper.activate()
    this.onActivate()
  }

  deactivate () {
    this.enabled = false
    this.editHelper && this.editHelper.deactivate()
    this.onDeactivate()
  }

  onActivate () {}
  onDeactivate () {}
  onClick () {}
  onDragStart () {}
  onDragEnd () {}
  onDrag () {}
  onHoverOn () {}
  onHoverOff () {}
  // init() {}
  registry () {}
  unRegistry () {}
  dispose () {}
}

export default TemplatePlugin
