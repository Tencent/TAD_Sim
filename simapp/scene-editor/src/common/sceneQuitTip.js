import i18n from '@/locales'

let sceneDirty = false

// 定义一个场景退出提示类
class SceneQuitTip {
  /**
   * 静态方法，用于在退出应用时显示未保存更改的提示
   * @param {Event} event - 事件对象
   * @return {string} 返回提示信息
   */
  static tip (event) {
    const returnValue = i18n.t('tips.exitWithoutSave')
    event.returnValue = returnValue
    return returnValue
  }

  get dirty () {
    return sceneDirty
  }

  /**
   * 设置场景是否已更改的状态，并根据状态更新窗口标题和退出提示
   * @param {boolean} val - 场景是否已更改的状态
   * @return {boolean} 返回设置后的状态
   */
  set dirty (val) {
    val = !!val
    let { title } = document
    if (title.startsWith('*')) {
      title = title.substring(1)
    }
    if (window.desktop) {
      window.desktop.modifyEditor(val)
    } else {
      window.onbeforeunload = val ? SceneQuitTip.tip : null
      document.title = val ? `*${title}` : title
    }
    sceneDirty = val
    return val
  }
}

// 创建一个 SceneQuitTip 实例
const sceneQuitTip = new SceneQuitTip()

export default sceneQuitTip
