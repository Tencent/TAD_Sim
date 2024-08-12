import { cloneDeep } from 'lodash-es'
import store from '@/store'

class Obstacle {
  constructor () {
    this.id = ''
    this.routeId = ''
    this.laneId = ''
    this.type = ''
    this.startShift = 0.0
    this.startOffset = 0.0
    this.length = 4.5
    this.width = 2
    this.height = 1.8
    this.direction = 0
    this.start_angle = ''
    this.model = null
    this.boundingBox = null
  }

  /**
   * 复制障碍物对象，不包括模型
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.routeId = other.routeId
    this.laneId = other.laneId
    this.type = other.type
    this.startShift = other.startShift
    this.startOffset = other.startOffset
    this.length = other.length
    this.width = other.width
    this.height = other.height
    this.direction = other.direction
    this.startAlt = other.startAlt
    this.start_angle = other.start_angle
    this.boundingBox = other.boundingBox
  }

  setVisible (show) {
    if (this.model) {
      this.model.visible = !!show
    }
  }

  obstacleFormData () {
    const data = {}
    this.copyToObstacleFormData(data)
    return data
  }

  copyToObstacleFormData (obstacleData) {
    obstacleData.id = this.id
    obstacleData.routeId = this.routeId
    obstacleData.laneId = this.laneId
    obstacleData.obstacleType = this.type
    obstacleData.length = this.length
    obstacleData.width = this.width
    obstacleData.height = this.height
    obstacleData.transfer = this.startShift
    obstacleData.offset = this.startOffset
    obstacleData.direction = this.direction
    obstacleData.index = this.index
    obstacleData.startAlt = this.startAlt
    obstacleData.start_angle = this.start_angle
    obstacleData.boundingBox = this.boundingBox
  }

  /**
   * 从后端数据中获取障碍物信息
   * @param data
   */
  setFromBackEndData (data) {
    this.id = data.id
    this.routeId = data.routeID
    this.laneId = data.laneID
    this.type = data.type
    this.startShift = data.start_s
    this.startOffset = data.l_offset
    this.startAlt = data.start_alt || 0
    this.length = data.length
    this.width = data.width
    this.height = data.height
    this.direction = data.direction
    this.start_angle = data.start_angle
    this.boundingBox = data.boundingBox
  }

  /**
   * 创建一个默认的障碍物对象
   * @param type
   * @return {{
   * boundingBox: BoundingBox, offset: number, start_angle: string,
   * length: number, maxOffset: number, laneId: string, routeId: string,
   * transfer: number, width: number, obstacleType: string, maxShift: number,
   * height: number, direction: number,
   * }}
   */
  static createObstacleData (type) {
    const { obstacleList } = store.state.catalogs
    const obstacle = obstacleList.find(item => item.variable === type)
    return {
      routeId: '',
      laneId: '',
      obstacleType: 'Sedan',
      length: 2,
      width: 2,
      height: 2,
      transfer: 0,
      offset: 0,
      direction: 0,
      maxShift: 10,
      maxOffset: 2,
      start_angle: '',
      boundingBox: cloneDeep(obstacle.catalogParams.boundingBox),
    }
  }

  get trajectoryEnabled () {
    // 障碍物轨迹跟踪固定是true
    return true
  }
}

export default Obstacle
