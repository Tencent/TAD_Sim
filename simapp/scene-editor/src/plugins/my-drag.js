const dropElements = new Set()

/**
 * Vue 插件，提供自定义指令 my-drag 和 my-drop，实现将图片拖放到场景的功能
 * @param {object} app - Vue 应用实例
 * @param {object} opts - 可选配置对象
 */

export default {
  install (app, opts = {}) {
    /**
     * 自定义指令 my-drag，用于实现拖拽功能
     * @type {object}
     */
    app.directive('my-drag', {
      mounted (el, { value: id }) {
        const twin = el.twin = el.cloneNode(true)
        // 克隆一个节点，并设置样式，使其在拖拽时显示
        Object.assign(twin.style, {
          position: 'fixed',
          zIndex: '1000',
          top: '0',
          left: '0',
          opacity: '0.5',
        })
        el.pointerdownHandle = (evt) => {
          const { pointerId } = evt
          if (!el.hasPointerCapture(pointerId)) {
            el.setPointerCapture(pointerId)
            const {
              width,
              height,
              top,
              left,
            } = el.getBoundingClientRect()
            Object.assign(twin.style, {
              width: `${width}px`,
              height: `${height}px`,
              top: `${top}px`,
              left: `${left}px`,
            })
            document.body.appendChild(twin)
          }
        }
        el.pointermoveHandle = (evt) => {
          const { pointerId, pageX, pageY } = evt
          if (el.hasPointerCapture(pointerId)) {
            Object.assign(twin.style, {
              top: `${pageY}px`,
              left: `${pageX}px`,
            })
          }
        }
        el.pointerupHandle = (evt) => {
          const { pointerId, pageX, pageY } = evt
          if (el.hasPointerCapture(pointerId)) {
            el.releasePointerCapture(pointerId)
            document.body.removeChild(twin)
            const target = Array.from(dropElements).find((e) => {
              const { left, top, width, height } = e.getBoundingClientRect()
              return pageX >= left && pageX <= left + width && pageY >= top && pageY <= top + height
            })
            if (target) {
              target.onMyDrop({
                id,
                pageX,
                pageY,
              })
            }
          }
        }
        el.addEventListener('pointerdown', el.pointerdownHandle)
        el.addEventListener('pointermove', el.pointermoveHandle)
        el.addEventListener('pointerup', el.pointerupHandle)
      },
      beforeUnmount (el) {
        el.removeEventListener('pointerdown', el.pointerdownHandle)
        el.removeEventListener('pointermove', el.pointermoveHandle)
        el.removeEventListener('pointerup', el.pointerupHandle)
        delete el.twin
      },
    })
    app.directive('my-drop', {
      /**
       * 自定义指令 my-drop，用于实现拖拽功能
       * @type {object}
       */
      mounted (el, { value }) {
        dropElements.add(el)
        el.onMyDrop = value
      },
      beforeUnmount (el) {
        el.onMyDrop = null
        dropElements.delete(el)
      },
    })
  },
}
