import { initRoadDispatcher } from './road'
import { initJunctionDispatcher } from './junction'
import { initObjectDispatcher } from './object'
import { initModelCacheDispatcher } from './common'

class RenderDispatcher {
  unsubscribeRoadStore: Function
  unsubscribeJunctionStore: Function
  unsubscribeObjectStore: Function
  unsubscribeModelCacheStore: Function
  constructor () {}
  init () {
    this.unsubscribeRoadStore = initRoadDispatcher()
    this.unsubscribeJunctionStore = initJunctionDispatcher()
    this.unsubscribeObjectStore = initObjectDispatcher()
    this.unsubscribeModelCacheStore = initModelCacheDispatcher()
  }

  clear () {}
  dispose () {
    this.unsubscribeRoadStore && this.unsubscribeRoadStore()
    this.unsubscribeJunctionStore && this.unsubscribeJunctionStore()
    this.unsubscribeObjectStore && this.unsubscribeObjectStore()
    this.unsubscribeModelCacheStore && this.unsubscribeModelCacheStore()
  }
}

export default new RenderDispatcher()
