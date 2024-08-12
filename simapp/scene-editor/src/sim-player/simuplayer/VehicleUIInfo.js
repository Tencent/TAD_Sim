import { Vector3 } from 'three'

/**
 * 交通流UI类，展示状态信息
 */
class VehicleUIInfo {
  constructor (tid, type) {
    this.id = tid
    this.v = 0
    this.d = 0
    this.pos = new Vector3()
    this.visible = true
    // self, self_logsim, trailer, trailer_logsim, car, car_logsim, staticObj, dynamicObj, signlight
    this.type = type
  }
}

export default VehicleUIInfo
