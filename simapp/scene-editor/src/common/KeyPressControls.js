/**
 * 键盘事件监听
 *
 * Example:
 * const control = new KeyPressControls({ onKeyPress })
 * function onKeyPress(codes: string) {
 *   console.log(codes) // ['Ctrl', 'c']
 * }
 */
class KeyPressControls {
  constructor (props) {
    const { onKeyPress } = props || {}
    let keydownList = []

    // 绑定键盘事件
    window.addEventListener('keydown', onKeyDown, false)
    window.addEventListener('keyup', onKeyUp, false)
    // 绑定视窗脱焦事件
    window.addEventListener('blur', onCancel, false)

    // 销毁本实例
    this.dispose = function () {
      window.removeEventListener('keydown', onKeyDown, false)
      window.removeEventListener('keyup', onKeyUp, false)
      window.removeEventListener('blur', onCancel, false)
      keydownList = []
    }

    // 键盘按下时的回调
    function onKeyDown (event) {
      const keyCode = getKeyCode(event)

      // 忽略输入框等元素的按键事件
      const ignore = ignoreEvent()
      if (ignore) return

      // 忽略重复按键
      const last = keydownList[keydownList.length - 1]
      if (last === keyCode) return

      // 记录按键码
      keydownList.push(keyCode)
    }

    // 键盘抬手时的回调
    function onKeyUp () {
      // 触发按键回调
      if (keydownList.length && onKeyPress) {
        onKeyPress(keydownList)
      }

      // 重置操作
      onCancel()
    }

    // 重置操作
    function onCancel () {
      keydownList = []
    }

    // 忽略输入框等元素的按键事件
    function ignoreEvent () {
      const activeElementTag = document.activeElement?.tagName
      if (activeElementTag) {
        const tag = activeElementTag.toLocaleLowerCase()
        if (['input', 'textarea'].includes(tag)) return true
      }
      return false
    }
  }

  dispose () {}
}
export default KeyPressControls

// 获取按键字符串
function getKeyCode (event) {
  const keyCode = event.key
  return keyCode
}
