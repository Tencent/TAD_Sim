import { DistanceMode, TriggerCondition } from '../common/Constant'

/**
 * merge数据
 * @constructor
 */
class MergeData {
  constructor () {
    this.id = ''
    this.profile = ''
    this.merges = []
  }

  /**
   * 从另一个MergeData对象复制
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.profile = other.profile

    const len = other.merges.length
    for (let i = 0; i < len; ++i) {
      this.merges[i] = {}
      this.merges[i].condition = {}
      this.merges[i].condition.type = other.merges[i].condition.type
      this.merges[i].condition.value = other.merges[i].condition.value
      this.merges[i].condition.mode = other.merges[i].condition.mode
      this.merges[i].merge = other.merges[i].merge
      this.merges[i].direction = other.merges[i].direction
    }
  }

  copyNoID (other) {
    const id = this.id
    this.copyNoModel(other)
    this.id = id
  }

  /**
   * 解析后端数据，生成MergeData对象
   * @param data
   */
  setFromBackEndData (data) {
    this.id = data.id
    const nodesLen = data.profile.length
    for (let j = 0; j < nodesLen; ++j) {
      const mergeNode = {}
      mergeNode.condition = {}
      const p = data.profile[j]
      if (p?.length) {
        // 不同的长度代表不同的merge类型
        if (p.length === 2) {
          mergeNode.condition.type = TriggerCondition.TimeAbsolute
          mergeNode.condition.value = p[0]
          mergeNode.condition.mode = DistanceMode.None
          mergeNode.merge = p[1]
          mergeNode.duration = 4.5
          mergeNode.offset = 0
          mergeNode.count = -1
        } else if (p.length === 3) {
          mergeNode.condition.type = p[0]
          mergeNode.condition.value = p[1]
          mergeNode.condition.mode = DistanceMode.None
          mergeNode.merge = p[2]
          mergeNode.duration = 4.5
          mergeNode.offset = 0
          mergeNode.count = 1
        } else if (p.length === 4) {
          mergeNode.condition.type = p[0]
          mergeNode.condition.value = p[1]
          mergeNode.condition.mode = p[2]
          mergeNode.merge = p[3]
          mergeNode.duration = 4.5
          mergeNode.offset = 0
          mergeNode.count = 1
        } else if (p.length === 7) {
          mergeNode.condition.type = p[0]
          mergeNode.condition.value = p[1]
          mergeNode.condition.mode = p[2]
          mergeNode.merge = p[3]
          mergeNode.duration = p[4]
          mergeNode.offset = p[5]
          mergeNode.count = p[6]
        } else {
          console.error('merges fromat error!')
        }

        // 解析merge值为字符串
        if (mergeNode.merge === 1) {
          mergeNode.direction = 'left'
        } else if (mergeNode.merge === -1) {
          mergeNode.direction = 'right'
        } else if (mergeNode.merge === 2) {
          mergeNode.direction = 'laneleft'
        } else if (mergeNode.merge === -2) {
          mergeNode.direction = 'laneright'
        } else {
          mergeNode.merge = 0
          mergeNode.direction = 'static'
        }
      } else {
        // 没有profile则创建一份默认的
        mergeNode.condition.type = TriggerCondition.TimeAbsolute
        mergeNode.condition.mode = DistanceMode.None
        mergeNode.condition.value = 0
        mergeNode.merge = 0
        mergeNode.direction = 'static'
        mergeNode.duration = 4.5
        mergeNode.offset = 0
        mergeNode.count = -1
      }

      if (mergeNode.condition.type === TriggerCondition.TimeAbsolute) {
        mergeNode.count = -1
      }
      this.merges[j] = mergeNode
    }
  }
}

export default MergeData
