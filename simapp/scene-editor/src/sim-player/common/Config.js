import { getConfig } from '../api/scenario'
import EditorGlobals from '../sceneeditor/EditorGlobals.js'
import { RunMode } from './Constant'

class Config {
  constructor () {
    // 编辑器配置信息
    this.editor = {}
    this.editor.template = {}
    this.editor.template.current = undefined
    this.editor.template.itemMap = new Map()
    this.editor.useTopCamera = true

    // all configure
    this.runMode = RunMode.Local
    this.ignoreHeight = 1
    this.mergeGeom = true
    // 因为要区分车道类型，所以不在merge车道
    // this.mergeLaneGeom = false

    this.isPlaying = false // 区分是在播放还是在编辑中
    // 是否吸附到车道中心线
    this.snap = false

    EditorGlobals.signals.updateRenderState.add(this.updateState, this)

    this.isCloud = import.meta.env.VUE_APP_RUNTIME_MODE === 'cloud'
  }

  /**
   * 是否合并Geometry
   * @return {boolean}
   * @constructor
   */
  MergeGeom () {
    return this.mergeGeom
  }

  /**
   * 是否吸附到车道中心线
   * @return {boolean}
   */
  getSnap () {
    return this.snap
  }

  setSnap (value) {
    this.snap = value
  }

  /**
   * 加载配置
   * @return {Promise<boolean>}
   */
  async loadConfig () {
    if (this.isCloud) { // todo
      return true
    }
    const data = await getConfig()

    // editor config
    const editorConfigData = data.data.editor
    if (editorConfigData !== undefined) {
      const template = editorConfigData.template
      if (template) {
        if (template.current) {
          this.editor.template.current = template.current
        }
        this.editor.template.itemMap.clear()
        if (template.items) {
          template.items.forEach((value) => {
            this.editor.template.itemMap.set(value.map, value.tpl)
          })
        }
      }
    }
  }

  setUseTopCameraEditor (value) {
    this.editor.useTopCamera = value
  }

  setIgnoreHeight (value) {
    this.ignoreHeight = value
  }

  setIsPlaying (value) {
    this.isPlaying = value
  }

  updateState ({ type, key, value }) {
    this.editor[key] = value
  }
}

const GlobalConfig = new Config()

export default GlobalConfig
