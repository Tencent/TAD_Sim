import { Group } from 'three'

export interface IHelperParams {
  render: Function
}

class TemplateHelper {
  container: Group
  render: Function
  constructor (params: IHelperParams) {
    this.render = params.render
  }

  init () {}
  onActivate () {}
  onDeactivate () {}
  clear () {}
  dispose () {}
  initContainer (name: string) {
    const containerGroup = new Group()
    containerGroup.name = name
    this.container = containerGroup
  }

  async activate () {
    this.setContainerVisible(true)
    await this.onActivate()
    this.render()
  }

  async deactivate () {
    this.setContainerVisible(false)
    await this.onDeactivate()
    this.render()
  }

  setContainerVisible (visible: boolean) {
    if (this.container.visible === visible) return
    this.container.visible = visible
  }
}

export default TemplateHelper
