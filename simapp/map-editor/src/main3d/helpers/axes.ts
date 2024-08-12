import { AxesHelper } from 'three'

interface IAxesParams {
  size?: number
}

class Axes {
  object: AxesHelper
  constructor (params: IAxesParams = {}) {
    const { size = 50 } = params
    this.object = new AxesHelper(size)
  }

  setVisible (isVisible: boolean) {
    if (this.object.visible === isVisible) return
    this.object.visible = isVisible
  }

  dispose () {
    this.object.dispose()
  }
}

export default Axes
