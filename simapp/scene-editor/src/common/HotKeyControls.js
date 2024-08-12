import KeyPressControls from './KeyPressControls'

/**
 * 快捷键注册器
 *
 * Example:
 * const control = new HotKeyControls()
 * control.addShortcut('Ctrl + C', () => console.log('copy'))
 */
class HotKeyControls {
  constructor () {
    const shortcuts = new Map()

    const keypressControls = new KeyPressControls({ onKeyPress })

    let enabled = true

    // 销毁本实例
    this.dispose = function () {
      shortcuts.clear()
      keypressControls.dispose()
    }

    // 绑定快捷键，例如 addShortcut('Ctrl + c', () => console.log('copy'))
    this.addShortcut = function (userShortcut, callback) {
      const shortcut = convertShortcut(userShortcut)
      shortcuts.set(shortcut, callback)
    }

    // 解绑快捷键
    this.removeShortcut = function (userShortcut) {
      const shortcut = convertShortcut(userShortcut)
      shortcuts.delete(shortcut)
    }

    // 禁用快捷键
    this.setEnabled = function (value) {
      enabled = value
    }

    // 获取禁用状态
    this.getEnabled = function () {
      return enabled
    }

    // 响应按键，有快捷键匹配则运行其回调
    function onKeyPress (codes) {
      if (!enabled) return
      const shortcut = toShortcut(codes)
      const match = shortcuts.get(shortcut)
      if (!match) return
      const callback = match
      callback()
    }
  }

  dispose () {}
  addShortcut (_shortcut, _callback) {}
  removeShortcut (_shortcut) {}
  setEnabled (_value) {}
  getEnabled () {}
}
export default HotKeyControls

// 用户输入的快捷键转换成代码能理解的快捷键
// 暂不搞太复杂的边界判断，使用比较简单的 ' + ' 三个字符来区分
function convertShortcut (userShortcut) {
  return userShortcut
}

// 将按键码转换成快捷键
function toShortcut (codes) {
  return codes.join(' + ')
}
