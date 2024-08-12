import EditorGlobals from './EditorGlobals'

/**
 * simuScene全局错误提示方法
 */
export class MessageBox {
  static promptEditorUIMessage (type, message, posCb, negCb) {
    console.error(message)
    EditorGlobals.signals.promptMessage.dispatch({ type, msg: message, posCb, negCb })
  }
}
