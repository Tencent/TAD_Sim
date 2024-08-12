<script setup lang="ts">
/* global biz */
// 道路参考线横截面编辑面板
import { type Ref, computed, onMounted, onUnmounted, ref, watch } from 'vue'
import { cloneDeep } from 'lodash'
import InputNumber from '../common/inputNumber.vue'
import root2d from '@/main2d/index'
import { useElevationInteractionStore } from '@/plugins/editElevation/store/interaction'
import { useEditRoadStore } from '@/plugins/editRoad/store'
import { getRoad } from '@/utils/mapCache'
import { useRoadStore } from '@/stores/road'
import { fixedPrecision } from '@/utils/common3d'
import { Constant } from '@/utils/business'

const props = defineProps({
  // 当前组高程组件是否处于激活状态
  isActive: {
    type: Boolean,
    default: false,
  },
  width: {
    type: Number,
    default: 0,
  },
  height: {
    type: Number,
    default: 0,
  },
})

interface IPointProperty {
  s: string
  h: string
}

const minWidth = 500
const minHeight = 300

const interactionStore = useElevationInteractionStore()
const editRoadStore = useEditRoadStore()
const roadStore = useRoadStore()
const elevationContainerRef = ref()
const containerWidth = ref(0)
const containerHeight = ref(0)
const areaWidth = ref(0)
const areaHeight = ref(0)
const showCanvas = ref(false)
// 当前选中的二维面板控制点
const currentPoint: Ref<IPointProperty | null> = ref(null)
const precision = 3
// 给控制点属性面板预留120px的高度
const propertyHeight = 120

// 当前选中的道路
const currentRoad = computed(() => {
  return interactionStore.selectedRoad
})

// 当前是否已经组件加载完毕
const isMounted = ref(false)

watch(
  [currentRoad, () => props.isActive, () => isMounted],
  ([_currentRoad, _isActive, _isMounted]) => {
    if (!_isMounted.value) return
    if (!_currentRoad || !_isActive) {
      // 清空二维元素
      showCanvas.value = false
      currentPoint.value = null
      root2d.emit('clear')
      return
    }

    showCanvas.value = true

    // 将同步的流程变成异步，保证前端标签元素渲染完成后再调用渲染二维场景的逻辑
    setTimeout(() => {
      // 渲染二维面板
      root2d.init({
        container: elevationContainerRef.value,
        width: areaWidth.value,
        height: areaHeight.value,
      })

      // 透传当前道路的高程控制点
      parseElevationContainerPoints(_currentRoad)
    }, 0)
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  () => props.width,
  (val) => {
    containerWidth.value = val
    areaWidth.value = Math.max(minWidth, containerWidth.value)
    if (showCanvas.value) {
      onResize()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
watch(
  () => props.height,
  (val) => {
    containerHeight.value = val
    areaHeight.value = Math.max(
      containerHeight.value - propertyHeight,
      minHeight,
    )
    if (showCanvas.value) {
      onResize()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

let unsubscribeRoadStore: Function | undefined

onMounted(() => {
  isMounted.value = true

  root2d.on('update', updateControlPoints)
  root2d.on('select', selectControlPoint)
  root2d.on('unselect', unselectControlPoint)
  root2d.on('add', addControlPoint)
  root2d.on('remove', removeControlPoint)
  unsubscribeRoadStore = roadStore.$onAction(({ name, after }) => {
    after(() => {
      // 如果是应用操作记录，需要重置选中的控制点
      if (name === 'applyState') {
        unselectControlPoint()
      }
    })
  })
})

onUnmounted(() => {
  isMounted.value = false

  root2d.off('update', updateControlPoints)
  root2d.off('select', selectControlPoint)
  root2d.off('unselect', unselectControlPoint)
  root2d.off('add', addControlPoint)
  root2d.off('remove', removeControlPoint)
  root2d.dispose()
  if (unsubscribeRoadStore && typeof unsubscribeRoadStore === 'function') {
    unsubscribeRoadStore()
  }
})

function onResize () {
  root2d.resize(areaWidth.value, areaHeight.value)
}

// 删除高程控制点
function removeControlPoint (index: number) {
  if (!currentRoad.value) return

  // 判断当前道路是否是双向道路，如果是双向道路，修改了其中一条道路的高程，另一条道路也需要调整高程
  const cp = editRoadStore.getControlPointByRoadId(currentRoad.value.id)
  if (!cp) return
  const [forwardRoadId] = cp.roadId

  const roadOptions = []

  const forwardRoad = getRoad(forwardRoadId)
  if (forwardRoad && forwardRoad.elevationPath) {
    const controlPoints = cloneDeep(forwardRoad.elevationPath.points)
    const removeIndex = index
    // 删除正向道路的高程控制点
    controlPoints.splice(removeIndex, 1)
    roadOptions.push({
      roadId: forwardRoadId,
      points: controlPoints,
    })
  }

  if (roadOptions.length < 1) return

  // 触发数据层道路高程的更新
  roadStore.updateRoadElevation(roadOptions, 'remove')

  // 清空控制点的属性面板
  currentPoint.value = null
}

// 新增高程控制点
function addControlPoint (params: { x: number, y: number }) {
  const { x, y } = params

  if (!currentRoad.value) return

  // 判断当前道路是否是双向道路，如果是双向道路，修改了其中一条道路的高程，另一条道路也需要调整高程
  const cp = editRoadStore.getControlPointByRoadId(currentRoad.value.id)
  if (!cp) return
  const [forwardRoadId] = cp.roadId

  const roadOptions = []

  const forwardRoad = getRoad(forwardRoadId)
  if (forwardRoad && forwardRoad.elevationPath) {
    const _x = x
    const controlPoints = cloneDeep(forwardRoad.elevationPath.points)
    let index = -1
    for (let i = 0; i < controlPoints.length; i++) {
      if (index > -1) continue
      const p = controlPoints[i]
      // 判断在哪一个控制点前，插入新增的控制点
      if (
        Number(Number(_x).toFixed(precision)) <
          Number(Number(p.x).toFixed(precision))
      ) {
        index = i
      }
    }
    if (index > -1) {
      controlPoints.splice(index, 0, {
        x: _x,
        y,
        z: 0,
      })
      roadOptions.push({
        roadId: forwardRoadId,
        points: controlPoints,
      })
    }
  }

  if (roadOptions.length < 1) return

  // 属性面板展示新增的控制点
  selectControlPoint(params)

  // 触发数据层道路高程的更新
  roadStore.updateRoadElevation(roadOptions, 'add')
}

// 选中二维画面中的控制点
function selectControlPoint (params: { x: number, y: number }) {
  // 需要在控制点的属性面板中展示当前控制点的位置和高度
  const { x, y } = params
  currentPoint.value = {
    s: String(fixedPrecision(x, precision)),
    h: String(fixedPrecision(y, precision)),
  }
}

// 取消控制点的选中
function unselectControlPoint () {
  currentPoint.value = null
}

// 二维画面中控制点位置的更新，触发数据层高程数据的更新
function updateControlPoints (params: { x: number, y: number }) {
  const { x, y } = params
  if (!currentRoad.value) return

  // 判断当前道路是否是双向道路，如果是双向道路，修改了其中一条道路的高程，另一条道路也需要调整高程
  const cp = editRoadStore.getControlPointByRoadId(currentRoad.value.id)
  if (!cp) return
  const [forwardRoadId] = cp.roadId

  const roadOptions = []

  const forwardRoad = getRoad(forwardRoadId)
  if (forwardRoad && forwardRoad.elevationPath) {
    const controlPoints = cloneDeep(forwardRoad.elevationPath.points)
    // 通过 x 坐标来判断移动的是哪一个控制点
    let index = -1
    for (let i = 0; i < controlPoints.length; i++) {
      if (index > -1) continue
      const p = controlPoints[i]
      const _x = p.x
      if (Number(x).toFixed(precision) === Number(_x).toFixed(precision)) {
        // 如果 x 坐标一致
        index = i
      }
    }
    if (index > -1) {
      controlPoints[index].y = y
      roadOptions.push({
        roadId: forwardRoadId,
        points: controlPoints,
      })
    }
  }

  if (roadOptions.length < 1) return

  // 触发数据层道路高程的更新
  roadStore.updateRoadElevation(roadOptions)
}

// 解析当前道路的高程控制点，如果没有控制点
function parseElevationContainerPoints (road: biz.IRoad) {
  if (!road) return
  const { elevationPath } = road
  const points = cloneDeep(elevationPath.points)

  root2d.updateControlPoints({
    roadId: road.id,
    points,
    isSync: true,
  })

  // 如果更新高程控制点时，存在选中的控制点，则需要同步更新控制点的属性面板
  if (currentPoint.value && currentPoint.value.s) {
    // 通过 x 坐标来判断移动的是哪一个控制点
    let index = -1
    for (let i = 0; i < points.length; i++) {
      if (index > -1) continue
      const p = points[i]
      if (
        Number(currentPoint.value.s).toFixed(precision) ===
        Number(p.x).toFixed(precision)
      ) {
        // 如果 x 坐标一致
        index = i
      }
    }
    if (index > -1) {
      currentPoint.value = {
        s: currentPoint.value.s,
        h: String(fixedPrecision(points[index].y, precision)),
      }
      return
    }
  }

  // 否则重置
  currentPoint.value = null
}

// 监听输入框控制点高度的变化
watch(
  currentPoint,
  (val) => {
    if (!val) return
    if (!currentRoad.value || !currentRoad.value.elevationPath) return
    const { points } = currentRoad.value.elevationPath
    // 通过 x 坐标来判断移动的是哪一个控制点
    let index = -1
    for (let i = 0; i < points.length; i++) {
      if (index > -1) continue
      const p = points[i]
      if (Number(val.s).toFixed(precision) === Number(p.x).toFixed(precision)) {
        // 如果 x 坐标一致
        index = i
      }
    }
    if (index === -1) return
    if (
      Number(val.h).toFixed(precision) ===
      Number(points[index].y).toFixed(precision)
    ) {
      return
    }

    // 调用数据层更新高程控制点的高度
    updateControlPoints({
      x: fixedPrecision(Number(val.s), precision),
      y: fixedPrecision(Number(val.h), precision),
    })
  },
  {
    immediate: true,
    deep: true,
  },
)
</script>

<template>
  <div
    class="elevation-wrapper"
    :style="{
      width: `${containerWidth}px`,
      height: `${containerHeight}px`,
    }"
  >
    <div v-if="showCanvas" class="title">
      {{ $t('desc.editElevation.elevationPropertyAdjustment') }}
    </div>
    <!-- 高程面板外层的容器 -->
    <div
      :style="{
        height: `${containerHeight - propertyHeight}px`,
        visibility: showCanvas ? 'visible' : 'hidden',
      }"
      class="elevation-container"
    >
      <!-- 高程面板画布的容器（可能会比外层容器的尺寸更大） -->
      <div
        ref="elevationContainerRef"
        class="elevation-area"
        :style="{
          width: `${areaWidth}px`,
          height: `${areaHeight}px`,
          visibility: showCanvas ? 'visible' : 'hidden',
        }"
      />
    </div>
    <div v-if="currentPoint" class="control-point-property">
      <div class="cell">
        <label class="label">{{ $t('desc.editElevation.location') }}</label>
        <div class="content">
          <InputNumber
            v-model="currentPoint.s"
            :disabled="true"
            :precision="precision"
            unit="m"
          />
        </div>
      </div>
      <div class="cell">
        <label class="label">{{ $t('desc.editElevation.height') }}</label>
        <div class="content">
          <InputNumber
            v-model="currentPoint.h"
            :disabled="false"
            :precision="precision"
            :max="Constant.maxElevation"
            :min="Constant.minElevation"
            unit="m"
          />
        </div>
      </div>
    </div>
    <!-- 没有选中道路时的文字提示 -->
    <div v-if="!currentRoad" class="tip-container">
      {{ $t('actions.elevation.clickToSelectTheRoad') }}
    </div>
  </div>
</template>

<style scoped lang="less">
.elevation-wrapper {
  width: 100%;
  height: 100%;
  .title {
    color: var(--text-color);
    height: 28px;
    line-height: 28px;
    padding-left: 14px;
  }
  .elevation-container {
    position: relative;
    flex: 1;
    min-height: 0;
    overflow: auto;
  }
  .control-point-property {
    margin-top: 10px;
    display: flex;
    .cell {
      display: flex;
      margin-left: 14px;
      margin-right: 30px;
      align-items: center;
      .label {
        margin-right: 14px;
      }
    }
  }
  .tip-container {
    height: 50px;
    line-height: 50px;
    text-align: center;
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
  }
}
</style>
