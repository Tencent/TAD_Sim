<script setup lang="ts">
import { onMounted, ref } from 'vue'

const props = defineProps({
  direction: {
    type: String,
    required: true,
  },
  max: {
    type: Number,
    required: true,
  },
  min: {
    type: Number,
    required: true,
  },
  value: {
    type: Number,
    required: true,
  },
})

// 自定义触发的事件
const emit = defineEmits(['move'])

// 可拖拽区域的自定义 style
const style = ref({
  cursor: 'default',
  top: 'auto',
  bottom: 'auto',
  left: 'auto',
  right: 'auto',
  width: '',
  height: '',
})
// 是否是垂直维度的拖拽
const isVertical = ref(false)
// 当前是否处于拖拽的状态
const isMoving = ref(false)
// 指针事件元素的 id
const pointerId = ref(-1)
// 拖拽的初始位置
const startPos = ref(0)
// 拖拽组件起始的尺寸
const size = ref(0)

onMounted(() => {
  // 在 mounted 周期更新一次组件内部数据
  size.value = props.value
  switch (props.direction) {
    case 'top':
      style.value.cursor = 'row-resize'
      style.value.width = '100%'
      style.value.height = '3px'
      style.value.top = '0'
      style.value.left = '0'

      isVertical.value = true
      break
    case 'bottom':
      style.value.cursor = 'row-resize'
      style.value.width = '100%'
      style.value.height = '3px'
      style.value.bottom = '0'
      style.value.left = '0'

      isVertical.value = true
      break
    case 'left':
      style.value.cursor = 'col-resize'
      style.value.width = '3px'
      style.value.height = '100%'
      style.value.top = '0'
      style.value.left = '0'

      isVertical.value = false
      break
    case 'right':
      style.value.cursor = 'col-resize'
      style.value.width = '3px'
      style.value.height = '100%'
      style.value.top = '0'
      style.value.right = '0'

      isVertical.value = false
      break
    default:
      break
  }
})

/**
 * 开始拖拽尺寸组件
 * @param event
 */
function startMove (event: PointerEvent) {
  const { pageY, pageX } = event
  pointerId.value = event.pointerId

  if (isVertical.value) {
    startPos.value = pageY
  } else {
    startPos.value = pageX
  }
  isMoving.value = true
  if (event.target) {
    ;(event.target as Element).setPointerCapture(pointerId.value)
  }

  // 让当前组件中的尺寸，在拖拽前同步成父级组件传入的最新尺寸
  size.value = props.value
}

/**
 * 拖拽尺寸组件过程中重新计算尺寸
 * @param event
 */
function resizing (event: PointerEvent) {
  if (!isMoving.value) return

  const { pageX, pageY } = event
  // 计算移动的偏移量
  let offset = 0
  switch (props.direction) {
    case 'top':
      offset = startPos.value - pageY
      break
    case 'bottom':
      offset = pageY - startPos.value
      break
    case 'left':
      offset = startPos.value - pageX
      break
    case 'right':
      offset = pageX - startPos.value
      break
    default:
      break
  }

  // 计算最大和最小值之间的新尺寸
  const newSize = Math.max(Math.min(size.value + offset, props.max), props.min)

  emit('move', newSize)
}

/**
 * 结束拖拽调整尺寸
 * @param event
 */
function stopMove (event: PointerEvent) {
  if (!isMoving.value) return
  if (event.target) {
    ;(event.target as Element).releasePointerCapture(pointerId.value)
  }

  isMoving.value = false
  startPos.value = 0
}
</script>

<template>
  <div
    class="resize-layer"
    :style="style"
    @pointerdown="startMove"
    @pointermove="resizing"
    @pointerup="stopMove"
  />
</template>

<style scoped lang="less">
.resize-layer {
  position: absolute;
  z-index: 1000;
  width: auto;
  height: auto;
  cursor: default;
}
</style>
