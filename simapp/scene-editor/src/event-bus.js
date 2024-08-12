const eventBus = new EventTarget()

// 对老版（2.0）的事件方法兼容
eventBus.$on = eventBus.addEventListener
eventBus.$off = eventBus.removeEventListener
eventBus.$emit = function (event, data) {
  eventBus.dispatchEvent(new CustomEvent(event, { detail: data }))
}

export default eventBus
