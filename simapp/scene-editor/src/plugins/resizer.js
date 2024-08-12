const positions = [
  'relative',
  'absolute',
  'fixed',
]

/**
 * 根据给定的方向，返回对应的尺寸属性（宽度或高度）
 * @param {string} direction - 方向，可以是 'left', 'right', 'top', 'bottom'
 * @returns {string} dimension - 返回尺寸属性，可以是 'width' 或 'height'
 */
function directionReduction (direction) {
  let dimension
  switch (direction) {
    case 'left':
    case 'right':
      dimension = 'width'
      break
    case 'top':
    case 'bottom':
      dimension = 'height'
      break
    default:
      dimension = 'width'
  }
  return dimension
}

let index = 0

export default {
  install (app, opts = {}) {
    /**
     * 自定义指令 sim-resize，用于实现元素的自适应缩放功能
     * @type {object}
     */
    app.directive('sim-resize', {
      beforeMount (el, { value: { width, height, min, max, direction } }) {
        const { position } = getComputedStyle(el)
        if (!positions.includes(position)) {
          el.style.position = 'relative'
        }

        // 创建一个层，用于显示拖拽的指示器
        const layer = document.createElement('div')
        const layerStyleArr = [
          'position: absolute',
          ' z-index: 1000',
        ]
        layer.className = `sim-resize-layer-${index}`
        el.appendChild(layer)
        el.dataset.layerIndex = `${index}`
        switch (direction) {
          case 'left':
            el.style.width = `${width}px`
            layerStyleArr.push('left: 0', 'top: 0', 'height: 100%', 'width: 3px', 'cursor: col-resize')
            break
          case 'right':
            el.style.width = `${width}px`
            layerStyleArr.push('right: 0', 'top: 0', 'height: 100%', 'width: 3px', 'cursor: col-resize')
            break
          case 'top':
            el.style.height = `${height}px`
            layerStyleArr.push('left: 0', 'top: 0', 'width: 100%', 'height: 3px', 'cursor: row-resize')
            break
          case 'bottom':
            el.style.height = `${height}px`
            layerStyleArr.push('left: 0', 'bottom: 0', 'width: 100%', 'height: 3px', 'cursor: row-resize')
            break
          default:
        }

        layer.style.cssText = layerStyleArr.join('; ')
        index++
      },
      mounted (el, { value: { width, height, min, max, direction } }) {
        // 获取元素的尺寸属性
        const dimension = directionReduction(direction)
        let startPos = 0
        let startPageX = 0
        let startPageY = 0
        let isMoving = false
        let pointerId = -1
        let distance = 0

        const layer = el.querySelector(`.sim-resize-layer-${el.dataset.layerIndex}`)

        layer.addEventListener('pointerdown', (evt) => {
          ({ pageX: startPageX, pageY: startPageY } = evt)
          isMoving = true;
          ({ pointerId } = evt)
          layer.setPointerCapture(evt.pointerId)

          switch (direction) {
            case 'left':
            case 'right':
              startPos = el.clientWidth
              break
            case 'top':
            case 'bottom':
              startPos = el.clientHeight
              break
            default:
          }
        })

        layer.addEventListener('pointerup', () => {
          layer.releasePointerCapture(pointerId)
          isMoving = false
        })

        layer.addEventListener('pointermove', (evt) => {
          if (isMoving) {
            switch (direction) {
              case 'left':
                distance = startPageX - evt.pageX
                break
              case 'right':
                distance = evt.pageX - startPageX
                break
              case 'top':
                distance = startPageY - evt.pageY
                break
              case 'bottom':
                distance = evt.pageY - startPageY
                break
              default:
            }

            // 计算目标位置
            const targetPos = Math.max(Math.min(startPos + distance, max), min)
            el.style[dimension] = `${targetPos}px`
          }
        })
      },
      beforeUnmount (el) {
        // 删除层
        const layer = el.querySelector(`.sim-resize-layer-${el.dataset.layerIndex}`)
        el.removeChild(layer)
      },
    })
  },
}
