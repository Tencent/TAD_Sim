import { isEmpty } from 'lodash-es'
import SelfLocationData from '../pushmsg/SelfLocation.js'
import TrafficLocation from '../pushmsg/TrafficLocation.js'
import SelfTrajectory from '../pushmsg/SelfTrajectory.js'
import SelfTrajectoryFollow from '../pushmsg/SelfTrajectoryFollow'
import playerGlobals from '../PlayerGlobals'
import { remappingReg } from '../../common/Constant'
import { topicMap } from '@/common/proto-define'

/**
 * @class LocalSceneApplier
 * @description 本地场景数据处理器
 * @export
 */
class LocalSceneApplier {
  constructor (playerScene) {
    this.playerScene = playerScene
    this.trajectoryFollowReceived = false
    this.log2world = {}
    playerGlobals.signals.log2worldChanged.removeAll()
    playerGlobals.signals.log2worldChanged.add((log2world) => {
      Object.assign(this.log2world, log2world)
    })
  }

  reset () {
    this.trajectoryFollowReceived = false
  }

  /**
   * 接收一帧数据
   * @param data
   */
  apply (data) {
    if (data) {
      const mappedTopicMap = new Map()
      data.messages.forEach((msg) => {
        const { topic, content } = msg
        const matchResult = remappingReg.exec(topic)
        if (matchResult?.length) {
          const id = Number.parseInt(matchResult[2], 10)
          const realTopic = matchResult[3]
          if (topicMap.has(realTopic)) {
            content.id = id
            if (mappedTopicMap.has(realTopic)) {
              mappedTopicMap.get(realTopic).push(content)
            } else {
              mappedTopicMap.set(realTopic, [content])
            }
          } else {
            console.warn(`Unregistered topic: ${realTopic}`)
          }
        } else {
          this.receivedMsg(topic, content)
        }
      })
      mappedTopicMap.forEach((value, key) => {
        this.receivedMsg(key, value)
      })
    }
  }

  /**
   * 处理接收到的消息
   * @param topic
   * @param content
   */
  receivedMsg (topic, content) {
    const isLogsim = this.playerScene.sceneParser.trafficType === 'simrec'
    if (!isEmpty(content)) {
      if (topic === 'LOCATION') {
        const isTransparent = isLogsim ? !this.log2world.egoLog2world : false
        const data = content.map((c) => {
          // 本车位置数据
          const selfLoc = new SelfLocationData()
          selfLoc.parse(c)
          selfLoc.isTransparent = isTransparent
          return selfLoc
        })
        this.playerScene.processSelfLoc(data)
      } else if (topic === 'LOCATION_REPLAY') {
        // 主车Logsim
        const isTransparent = this.log2world.egoLog2world
        const data = content.map((c) => {
          // 本车位置数据
          const selfLoc = new SelfLocationData()
          selfLoc.parse(c)
          selfLoc.isTransparent = isTransparent
          selfLoc.replay = true
          return selfLoc
        })

        this.playerScene.processSelfLoc(data)
      } else if (topic === 'LOCATION_TRAILER') {
        // todo: 多主车下的拖车位置
        const selfLoc = new SelfLocationData()
        selfLoc.parse(content)
        this.playerScene.processTrailer(selfLoc)
      } else if (topic === 'LOCATION_TRAILER_REPLAY') {
        const isTransparent = this.log2world.egoLog2world
        // todo: 多主车下的拖车位置
        const selfLoc = new SelfLocationData()
        selfLoc.parse(content)
        this.playerScene.processTrailer(selfLoc, isTransparent)
      } else if (topic === 'TRAFFIC') {
        // 交通流数据
        const trfcLoc = new TrafficLocation()
        if (content[0]) {
          trfcLoc.parse(content[0])
          this.playerScene.processTrafficLoc(trfcLoc)
        }
      } else if (topic === 'TRAFFIC_REPLAY') {
        // 交通流数据
        const trfcLoc = new TrafficLocation()
        trfcLoc.parse(content)
        this.playerScene.processTrafficLoc(trfcLoc, this.log2world.trafficLog2world, true)
      } else if (topic === 'TRAJECTORY') {
        const isTransparent = !this.log2world.egoLog2world
        const data = content.map((c) => {
          // 本车路径数据
          const selfTrj = new SelfTrajectory()
          selfTrj.parse(c)
          selfTrj.isTransparent = isTransparent
          return selfTrj
        })

        this.playerScene.processSelfTrj(data)
      } else if (topic === 'TRAJECTORY_REPLAY') {
        const isTransparent = this.log2world.egoLog2world
        // 本车路径数据
        const selfTrj = new SelfTrajectory()
        const data = content.map((c) => {
          // 本车路径数据
          const selfTrj = new SelfTrajectory()
          selfTrj.parse(c)
          selfTrj.isTransparent = isTransparent
          return selfTrj
        })

        this.playerScene.processSelfTrj(data)
      } else if (topic === 'TRAJECTORY_FOLLOW') {
        // 控制模块的轨迹数据
        if (!this.trajectoryFollowReceived) {
          const selfCtlTrj = new SelfTrajectoryFollow()
          selfCtlTrj.parse(content)
          this.playerScene.processSelfCtlTrj(selfCtlTrj)
          this.trajectoryFollowReceived = true
        }
      } else if (topic === 'GRADING') {
        this.playerScene.processGrading(content)
      } else {
        console.warn('unknonw msg type: ', topic)
      }
    }
  }
}

export default LocalSceneApplier
