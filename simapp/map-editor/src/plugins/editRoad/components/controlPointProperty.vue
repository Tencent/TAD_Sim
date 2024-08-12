<script setup lang="ts">
/* global biz */
import { computed, onMounted, onUnmounted, ref, watch } from 'vue'
import { Vector3 } from 'three'
import { ElMessage } from 'element-plus'
import { useEditRoadStore } from '../store'
import { useRoadInteractionStore } from '../store/interaction'
import InputNumber from '@/components/common/inputNumber.vue'
import { useRoadStore } from '@/stores/road'
import { fixedPrecision } from '@/utils/common3d'
import { Constant } from '@/utils/business'
import ee, { CustomEventType } from '@/utils/event'
import i18n from '@/locales'

const activeNames = ref(['position'])
// 当前选中控制点的坐标
const posX = ref('')
const posY = ref('')
const posZ = ref('')

// 控制点之间的最大间隔
const maxInterval = 10000

// 当前是否处于框选多个控制点的状态
const isSelectMultiple = ref(false)

const precision = Constant.precision

const roadStore = useRoadStore()
const editRoadStore = useEditRoadStore()
const roadInteractionStore = useRoadInteractionStore()

const selectedRefPoint = computed(() => {
  return roadInteractionStore.selectedRefPoint
})

const selectedAdjacentRefPoints = computed(() => {
  return roadInteractionStore.selectedAdjacentRefPoints
})

const extremalPos = computed(() => {
  const res = { xMax: 0, xMin: 0, yMax: 0, yMin: 0 }

  if (!selectedRefPoint.value) return res

  if (!selectedAdjacentRefPoints.value.length) {
    return {
      xMax: Number.POSITIVE_INFINITY,
      xMin: Number.NEGATIVE_INFINITY,
      yMax: Number.POSITIVE_INFINITY,
      yMin: Number.NEGATIVE_INFINITY,
    }
  }

  selectedAdjacentRefPoints.value.forEach((point) => {
    if (!point) return
    // 屏幕的坐标和实际的坐标需要做转换
    // point.z 对应 posX, point.x 对应 posY, point.y 对应 PosZ

    const anchor = { x: fixedPrecision(point.z), y: fixedPrecision(point.x) }

    const { max: xMax, min: xMin } = getExtremalX(
      anchor,
      fixedPrecision(selectedRefPoint.value!.x),
    )
    const { max: yMax, min: yMin } = getExtremalY(
      anchor,
      fixedPrecision(selectedRefPoint.value!.z),
    )

    if (xMax > res.xMax) res.xMax = xMax
    if (xMin < res.xMin) res.xMin = xMin
    if (yMax > res.yMax) res.yMax = yMax
    if (yMin < res.yMin) res.yMin = yMin
  })

  return res
})

window.extremalPos = extremalPos

function getExtremalX (anchor: common.vec2, y: number) {
  const xDiff = Math.sqrt(maxInterval ** 2 - (anchor.y - y) ** 2)
  return { max: anchor.x + xDiff, min: anchor.x - xDiff }
}

function getExtremalY (anchor: common.vec2, x: number) {
  const yDiff = Math.sqrt(maxInterval ** 2 - (anchor.x - x) ** 2)
  return { max: anchor.y + yDiff, min: anchor.y - yDiff }
}

function callExceedExtremalPosTip () {
  ElMessage.warning({
    message: i18n.global.t(
      'actions.controlPoint.adjacentIntervalCantExceedLimit',
      { interval: maxInterval },
    ),
  })
}

// 当前参考点对应控制点集，控制的道路 id
const controlRoadIds = computed(() => {
  if (!selectedRefPoint.value) return ''
  const cp = editRoadStore.getControlPointById(selectedRefPoint.value.parentId)
  if (!cp) return ''
  const ids = cp.roadId.join(', ')
  return ids
})

const controlRoadLength = computed(() => {
  if (!selectedRefPoint.value) return ''
  const cp = editRoadStore.getControlPointById(selectedRefPoint.value.parentId)
  if (!cp) return ''
  const [forwardRoadId] = cp.roadId
  if (!forwardRoadId) return ''
  const forwardRoad = roadStore.getRoadById(forwardRoadId)
  if (!forwardRoad) return ''
  return `${forwardRoad.length} m`
})

watch(
  selectedRefPoint,
  (val) => {
    if (!val) return
    if (roadInteractionStore.moreRefPoints.length > 1) {
      isSelectMultiple.value = true
    } else {
      isSelectMultiple.value = false
    }

    // 展示的坐标跟实际的坐标需要做转换
    posX.value = String(fixedPrecision(val.z))
    posY.value = String(fixedPrecision(val.x))
    posZ.value = String(fixedPrecision(val.y))
  },
  {
    // 参考点的改变，以及参考点坐标的改变，都会触发更新
    immediate: true,
    deep: true,
  },
)

// 只监听 x 和 y 坐标的数值变化
watch(posX, (val) => {
  if (!selectedRefPoint.value) return
  const { id, parentId, x, y, z } = selectedRefPoint.value
  // 展示的 x 坐标变换，对应三维场景中元素 z 坐标的更新
  const _z = Number(val)
  // 判断精度调整后的数值是否相等
  if (fixedPrecision(z, precision) === _z) return
  const newPos = new Vector3(x, y, _z)
  editRoadStore.moveControlPoint(id, parentId, newPos)
})
watch(posY, (val) => {
  if (!selectedRefPoint.value) return
  const { id, parentId, x, y, z } = selectedRefPoint.value
  // 展示的 y 坐标变换，对应三维场景中元素 x 坐标的更新
  const _x = Number(val)
  // 判断精度调整后的数值是否相等
  if (fixedPrecision(x, precision) === _x) return
  const newPos = new Vector3(_x, y, z)
  editRoadStore.moveControlPoint(id, parentId, newPos)
})

// 更新
function startSelectMultiple () {
  isSelectMultiple.value = true
}
function endSelectMultiple (data: Array<biz.IRefPoint>) {
  // 如果存在2个及以上的选中控制点
  if (data.length > 1) {
    isSelectMultiple.value = true
    return
  }

  isSelectMultiple.value = false
}

onMounted(() => {
  ee.on(CustomEventType.controlPoint.hover, startSelectMultiple)
  ee.on(CustomEventType.controlPoint.select, endSelectMultiple)
})

onUnmounted(() => {
  ee.off(CustomEventType.controlPoint.hover, startSelectMultiple)
  ee.off(CustomEventType.controlPoint.select, endSelectMultiple)
})
</script>

<template>
  <template v-if="!isSelectMultiple && selectedRefPoint">
    <div class="property-title">
      {{ $t('desc.editRoad.currentControlPointProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editRoad.controlRoadId') }}</label>
        <div class="content">
          {{ controlRoadIds }}
        </div>
      </div>
      <div v-if="controlRoadLength" class="property-cell">
        <label class="label">{{ $t('desc.editRoad.controlRoadLength') }}</label>
        <div class="content">
          {{ controlRoadLength }}
        </div>
      </div>
    </div>
    <el-form class="property-form" label-width="60" @submit.enter.prevent>
      <el-collapse v-model="activeNames">
        <el-collapse-item
          :title="$t('desc.commonRoad.position')"
          name="position"
        >
          <el-form-item label="X" class="param-cell">
            <InputNumber
              v-model="posX"
              :disabled="false"
              :precision="precision"
              unit="m"
              :min="extremalPos.xMin"
              :max="extremalPos.xMax"
              @exceed-max="callExceedExtremalPosTip"
              @exceed-min="callExceedExtremalPosTip"
            />
          </el-form-item>
          <el-form-item label="Y" class="param-cell">
            <InputNumber
              v-model="posY"
              :disabled="false"
              :precision="precision"
              unit="m"
              :min="extremalPos.yMin"
              :max="extremalPos.yMax"
              @exceed-max="callExceedExtremalPosTip"
              @exceed-min="callExceedExtremalPosTip"
            />
          </el-form-item>
          <el-form-item label="Z" class="param-cell">
            <InputNumber
              v-model="posZ"
              :disabled="true"
              :precision="precision"
              unit="m"
              :min="-maxInterval"
              :max="maxInterval"
            />
          </el-form-item>
        </el-collapse-item>
      </el-collapse>
    </el-form>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
