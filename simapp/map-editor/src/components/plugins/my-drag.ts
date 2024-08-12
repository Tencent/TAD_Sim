import { dropElements } from '@/utils/tools'
import {
  handleDrag,
  handleDragEnd,
  handleDragStart,
  resetDragElement,
} from '@/main3d/controls/editObjectHandler'

interface IDragElement extends HTMLElement {
  twin?: HTMLElement
  pointerdownHandle: (evt: PointerEvent) => void
  pointermoveHandle: (evt: PointerEvent) => void
  pointerupHandle: (evt: PointerEvent) => void
}

export const myDragConfig = {
  mounted (element: HTMLElement, params: any) {
    const el = element as IDragElement
    const imgElement = (el.children[0] as HTMLElement) || null
    if (!imgElement) return
    const twin = imgElement.cloneNode(true) as HTMLElement
    el.twin = twin

    Object.assign(twin.style, {
      position: 'fixed',
      zIndex: '1000',
      top: '0',
      left: '0',
      opacity: '0.9',
    })

    el.pointerdownHandle = (evt) => {
      const { pointerId, pageX, pageY } = evt
      if (!el.hasPointerCapture(pointerId)) {
        el.setPointerCapture(pointerId)
        const { width, height } = imgElement.getBoundingClientRect()
        Object.assign(twin.style, {
          width: `${width}px`,
          height: `${height}px`,
          top: `${pageY}px`,
          left: `${pageX}px`,
        })
        document.body.appendChild(twin)

        // 触发三维场景的交互
        handleDragStart({
          width,
          height,
          option: params.value,
        })
      }
    }
    el.pointermoveHandle = (evt) => {
      const { pointerId, pageX, pageY } = evt
      if (el.hasPointerCapture(pointerId)) {
        Object.assign(twin.style, {
          top: `${pageY}px`,
          left: `${pageX}px`,
        })
        const isValid = handleDrag(evt)
        // 如果在有效区域，则更新鼠标样式
        const cursorStyle = isValid ? 'copy' : 'default'
        el.style.cursor = cursorStyle
      }
    }
    el.pointerupHandle = (evt) => {
      const { pointerId, pageX, pageY } = evt
      if (el.hasPointerCapture(pointerId)) {
        el.releasePointerCapture(pointerId)
        document.body.removeChild(twin)
        // 还原指针样式
        el.style.cursor = 'default'
        const target = Array.from(dropElements).find((e) => {
          const { left, top, width, height } = e.getBoundingClientRect()
          return (
            pageX >= left &&
            pageX <= left + width &&
            pageY >= top &&
            pageY <= top + height
          )
        })
        if (target) {
          handleDragEnd(evt)
        } else {
          // 如果选中了物体列表中的元素，但是没放置到三维场景中，需要取消放置
          resetDragElement()
        }
      }
    }
    el.addEventListener('pointerdown', el.pointerdownHandle)
    el.addEventListener('pointermove', el.pointermoveHandle)
    el.addEventListener('pointerup', el.pointerupHandle)
  },
  unmounted (el: IDragElement) {
    el.removeEventListener('pointerdown', el.pointerdownHandle)
    el.removeEventListener('pointermove', el.pointermoveHandle)
    el.removeEventListener('pointerup', el.pointerupHandle)
    delete el.twin
  },
}
