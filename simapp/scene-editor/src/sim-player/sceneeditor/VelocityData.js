import { DistanceMode, TriggerCondition } from '../common/Constant'

/**
 * 速度数据
 */
class VelocityData {
  constructor () {
    this.id = ''
    this.profiles = ''
    this.nodes = []
  }

  /**
   * 复制数据
   * @param {*} other
   */
  copyNoModel (other) {
    this.id = other.id
    this.profiles = other.profiles

    const len = other.nodes.length
    for (let i = 0; i < len; ++i) {
      this.nodes[i] = {}
      this.nodes[i].condition = {}
      this.nodes[i].condition.type = other.nodes[i].condition.type
      this.nodes[i].condition.value = other.nodes[i].condition.value
      this.nodes[i].condition.mode = other.nodes[i].condition.mode
      this.nodes[i].velocity = other.nodes[i].velocity
    }
  }

  copyNoID (other) {
    const id = this.id
    this.copyNoModel(other)
    this.id = id
  }

  /**
   * 从后端数据中获取数据
   * @param {*} data
   */
  setFromBackEndData (data) {
    this.id = data.id
    const nodesLen = data.profile.length
    for (let j = 0; j < nodesLen; ++j) {
      const velocityNode = {}
      velocityNode.condition = {}
      const p = data.profile[j]
      if (p?.length) {
        if (p.length === 2) {
          velocityNode.condition.type = TriggerCondition.TimeAbsolute
          velocityNode.condition.value = p[0]
          velocityNode.condition.mode = DistanceMode.None
          velocityNode.velocity = p[1]
          velocityNode.count = -1
        } else if (p.length === 3) {
          velocityNode.condition.type = p[0]
          velocityNode.condition.value = p[1]
          velocityNode.condition.mode = DistanceMode.None
          velocityNode.velocity = p[2]
          velocityNode.count = 1
        } else if (p.length === 4) {
          velocityNode.condition.type = p[0]
          velocityNode.condition.value = p[1]
          velocityNode.condition.mode = p[2]
          velocityNode.velocity = p[3]
          velocityNode.count = 1
        } else if (p.length === 5) {
          velocityNode.condition.type = p[0]
          velocityNode.condition.value = p[1]
          velocityNode.condition.mode = p[2]
          velocityNode.velocity = p[3]
          velocityNode.count = p[4]
        }
      } else {
        velocityNode.condition.type = TriggerCondition.TimeAbsolute
        velocityNode.condition.value = 0
        velocityNode.condition.mode = DistanceMode.None
        velocityNode.velocity = 0
        velocityNode.count = -1
      }

      if (velocityNode.condition.type === TriggerCondition.TimeAbsolute) {
        velocityNode.count = -1
      }
      this.nodes[j] = velocityNode
    }
  }
}

export default VelocityData
