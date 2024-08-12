const eventRegistryMap = new WeakMap()

/**
 * 获取组件实例的事件注册信息
 * @param {object} instance - 组件实例
 * @returns {object} - 事件注册信息对象
 */
function getRegistry (instance) {
  let events = eventRegistryMap.get(instance)
  if (!events) {
    eventRegistryMap.set(instance, (events = Object.create(null)))
  }
  return events
}

/**
 * 为组件实例添加事件监听器
 * @param {object} instance - 组件实例
 * @param {string|Array<string>} event - 事件名称，可以是单个事件名或事件名数组
 * @param {Function} fn - 事件处理函数
 * @returns {object} - 返回组件实例
 */
export function $on (instance, event, fn) {
  if (Array.isArray(event)) {
    event.forEach(e => $on(instance, e, fn))
  } else {
    const events = getRegistry(instance)
    ;(events[event] || (events[event] = [])).push(fn)
  }
  return instance
}
export function $once (instance, event, fn) {
  const wrapped = (...args) => {
    $off(instance, event, wrapped)
    fn.call(instance, ...args)
  }
  wrapped.fn = fn
  $on(instance, event, wrapped)
  return instance
}
/**
 * 移除组件实例的事件监听器
 * @param {object} instance - 组件实例
 * @param {string|Array<string>} event - 事件名称，可以是单个事件名或事件名数组
 * @param {Function} [fn] - 要移除的事件处理函数，如果未指定，则移除所有事件监听器
 * @returns {object} - 返回组件实例
 */
export function $off (instance, event, fn) {
  const vm = instance
  // all
  if (!event) {
    eventRegistryMap.set(instance, Object.create(null))
    return vm
  }
  // array of events
  if (Array.isArray(event)) {
    event.forEach(e => $off(instance, e, fn))
    return vm
  }
  // specific event
  const events = getRegistry(instance)
  const cbs = events[event]
  if (!cbs) {
    return vm
  }
  if (!fn) {
    events[event] = undefined
    return vm
  }
  events[event] = cbs.filter(cb => !(cb === fn || cb.fn === fn))
  return vm
}
/**
 * 触发组件实例的事件
 * @param {object} instance - 组件实例
 * @param {string} event - 事件名称
 * @param {...any} args - 传递给事件处理函数的参数
 * @returns {object} - 返回组件实例
 */
export function $emit (instance, event, ...args) {
  instance && instance.$emit && instance.$emit(event, ...args)
  const cbs = getRegistry(instance)[event]
  if (cbs) {
    cbs.map(cb => cb.apply(instance, args))
  }
  return instance
}
