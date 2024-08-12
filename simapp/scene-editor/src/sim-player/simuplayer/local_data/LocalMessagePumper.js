import CachedFrameManager from './CachedFrameManager'
import LocalSceneApplier from './LocalSceneApplier'
import LocalDataParser from './LocalDataParser'

/**
 * @class LocalMessagePumper
 * @constructor
 * @desc 帧数据推送器
 * @param {PlayerScene} playerScene
 */
class LocalMessagePumper {
  constructor (playerScene) {
    this.playerScene = playerScene
    this.cachedFrameManager = new CachedFrameManager()
    this.applier = new LocalSceneApplier(playerScene)
    this.parser = new LocalDataParser()
  }

  reset () {
    this.applier.reset()
  }

  clear () {
    this.cachedFrameManager.clear()
  }

  bind () {
    window.addEventListener('message', this.processMsg)
  }

  unbind () {
    window.removeEventListener('message', this.processMsg)
  }

  /**
   * @desc 处理消息
   * @param data
   */
  processMsg (data) {
    const pd = this.parser.parse(data)
    this.cachedFrameManager.produce(pd)
  }

  /**
   * 处理最后一帧数据
   * @return {*}
   */
  applyLatestData () {
    const lastFrame = this.cachedFrameManager.consume()
    this.applier.apply(lastFrame)
    return lastFrame
  }
}

export default LocalMessagePumper
