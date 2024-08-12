import { AccelerationTerminationType, DistanceMode, TriggerCondition } from '../common/Constant'

class AccelerationData {
  constructor () {
    this.id = ''
    this.profiles = ''
    this.nodes = []
  }

  /**
   * 复制另一个AccelerationData对象的id和profiles属性，以及nodes属性中的每个节点对象的条件和加速度信息。
   * 这个方法不会复制节点对象的termination属性。
   *
   * @param {AccelerationData} other - 要复制的AccelerationData对象
   */
  copyNoModel (other) {
    this.id = other.id
    this.profiles = other.profiles

    const len = other.nodes.length
    for (let i = 0; i < len; ++i) {
      this.nodes[i] = {}
      this.nodes[i].condition = {}
      this.nodes[i].condition.type = other.nodes[i].condition.type
      this.nodes[i].condition.mode = other.nodes[i].condition.mode
      this.nodes[i].condition.value = other.nodes[i].condition.value
      // this.nodes[i].condition = other.nodes[i].condition
      this.nodes[i].acc = other.nodes[i].acc
      this.nodes[i].termination = {}
      this.nodes[i].termination.type = other.nodes[i].termination.type
      this.nodes[i].termination.value = other.nodes[i].termination.value
    }
  }

  /**
   * 复制另一个AccelerationData对象的所有属性，但不包括id属性。
   *
   * @param {AccelerationData} other - 要复制的AccelerationData对象
   */
  copyNoID (other) {
    const id = this.id
    this.copyNoModel(other)
    this.id = id
  }

  /**
   * 根据后端返回的数据设置AccelerationData对象的属性。
   *
   * @param {Object} data - 包含id和profile属性的后端数据
   */
  setFromBackEndData (data) {
    this.id = data.id
    data.profile.forEach((p, j) => {
      const accNode = {}
      accNode.condition = {}
      // 设置默认的终止条件为None
      accNode.termination = { type: AccelerationTerminationType.None, value: 0 }
      // 不同的长度有不同的condition
      if (p?.length) {
        if (p.length === 2) {
          accNode.condition.type = TriggerCondition.TimeAbsolute
          accNode.condition.value = p[0]
          accNode.condition.mode = DistanceMode.None
          accNode.acc = p[1]
          accNode.count = -1
        } else if (p.length === 3) {
          accNode.condition.type = p[0]
          accNode.condition.value = p[1]
          accNode.condition.mode = DistanceMode.None
          accNode.acc = p[2]
          accNode.count = 1
        } else if (p.length === 4) {
          accNode.condition.type = p[0]
          accNode.condition.value = p[1]
          accNode.condition.mode = p[2]
          accNode.acc = p[3]
          accNode.count = 1
        } else if (p.length === 5) {
          accNode.condition.type = p[0]
          accNode.condition.value = p[1]
          accNode.condition.mode = p[2]
          accNode.acc = p[3]
          accNode.count = p[4]
        } else if (p.length === 7) {
          accNode.condition.type = p[0]
          accNode.condition.value = p[1]
          accNode.condition.mode = p[2]
          accNode.acc = p[3]
          accNode.count = p[4]
          accNode.termination.type = p[5]
          accNode.termination.value = p[6]
        }
      } else {
        accNode.condition.type = TriggerCondition.TimeAbsolute
        accNode.condition.value = 0
        accNode.condition.mode = DistanceMode.None
        accNode.acc = 0
        accNode.count = -1
      }

      if (accNode.condition.type === TriggerCondition.TimeAbsolute) {
        accNode.count = -1
      }

      this.nodes[j] = accNode
    })
  }
}

export default AccelerationData
