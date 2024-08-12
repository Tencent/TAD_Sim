<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import { debounce } from 'lodash'
import { useObjectInteractionStore } from '../../store/interaction'
import InputNumber from '@/components/common/inputNumber.vue'
import { Constant } from '@/utils/business'
import { DefaultColor, fixedPrecision } from '@/utils/common3d'
import { getRoad } from '@/utils/mapCache'
import { useObjectStore } from '@/stores/object'
import colorPicker from '@/components/common/colorPicker.vue'
import { getParkingSpaceConfig } from '@/config/parkingSpace'

const interactionStore = useObjectInteractionStore()
const objectStore = useObjectStore()

// 是否处于道路中
const onRoad = ref(true)
// 道路中需要用到的属性
// st 坐标
const posS = ref('')
const posT = ref('')
const maxS = ref(0) // 根据道路的长度，来调整最大的 s 坐标值
const maxT = Constant.maxTValue
const yaw = ref('') // 偏航角
const lineColor = ref('') // 车位颜色
const length = ref('') // 车位长度
const width = ref('') // 车位宽度
const lineWidth = ref('') // 车位线宽度
const margin = ref('') // 相邻车位间距
const count = ref('') // 车位数量
const innerAngle = ref('') // 车位内夹角

// 固定不能修改的属性
const fixedProperty: Ref<Array<'length' | 'innerAngle'>> = ref([])

const precision = Constant.precision
const currentParkingSpace = computed(() => {
  return interactionStore.currentParkingSpace
})

watch(
  currentParkingSpace,
  (val) => {
    if (!val) return

    const { roadId, name } = val
    if (roadId) {
      const roadData = getRoad(roadId)
      if (roadData) {
        maxS.value = Number(roadData.length)
      }
      posS.value = String(fixedPrecision(val.s))
      posT.value = String(fixedPrecision(val.t))
      lineColor.value = val.color
      length.value = String(fixedPrecision(val.length))
      width.value = String(fixedPrecision(val.width))
      margin.value = String(fixedPrecision(val.margin))
      count.value = String(fixedPrecision(val.count))
      innerAngle.value = String(fixedPrecision(val.innerAngle))
      yaw.value = String(fixedPrecision(val.yaw))
      lineWidth.value = String(fixedPrecision(val.lineWidth))

      const config = getParkingSpaceConfig(name)
      if (config) {
        fixedProperty.value = config.fixedProperty
      }

      onRoad.value = true
    } else {
      onRoad.value = false
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

// 参考线坐标系下的属性更新
watch(posS, (val) => {
  if (!currentParkingSpace.value) return
  // 基于 st 坐标来计算世界坐标系下的空间坐标
  const { id: objectId, s, t } = currentParkingSpace.value
  const _s = Number(val)
  // 判断精度调整后的数值是否相等
  if (fixedPrecision(s, precision) === _s) return
  // 调用数据层更新 s 坐标的方法
  objectStore.moveParkingSpaceByST({
    objectId,
    s: _s,
    t,
    saveRecord: true,
  })
})
watch(posT, (val) => {
  if (!currentParkingSpace.value) return
  // 基于 st 坐标来计算世界坐标系下的空间坐标
  const { id: objectId, s, t } = currentParkingSpace.value
  const _t = Number(val)
  // 判断精度调整后的数值是否相等
  if (fixedPrecision(t, precision) === _t) return
  // 调用数据层更新 t 坐标的方法
  objectStore.moveParkingSpaceByST({
    objectId,
    s,
    t: _t,
    saveRecord: true,
  })
})
// 相对于参考线坐标系的偏航角
watch(yaw, (val) => {
  if (!currentParkingSpace.value) return
  const { id: objectId, yaw } = currentParkingSpace.value
  const _yaw = Number(val)
  if (fixedPrecision(yaw, precision) === _yaw) return
  objectStore.rotateParkingSpace({
    objectId,
    yaw: _yaw,
  })
})

// 更新停车位颜色
function updateParkingSpaceColor (val: string) {
  if (!currentParkingSpace.value) return
  if (!val || val === lineColor.value) return
  lineColor.value = val
  const { id: objectId } = currentParkingSpace.value
  objectStore.updateParkingSpaceProperty({
    objectId,
    type: 'color',
    value: val,
  })
}
// 单个停车位长度
watch(length, (val) => {
  if (!currentParkingSpace.value) return
  const { id: objectId, length } = currentParkingSpace.value
  const _length = Number(val)
  if (fixedPrecision(length, precision) === _length) return
  objectStore.updateParkingSpaceProperty({
    objectId,
    type: 'length',
    value: _length,
  })
})
// 单个停车位宽度
watch(width, (val) => {
  if (!currentParkingSpace.value) return
  const { id: objectId, width } = currentParkingSpace.value
  const _width = Number(val)
  if (fixedPrecision(width, precision) === _width) return
  objectStore.updateParkingSpaceProperty({
    objectId,
    type: 'width',
    value: _width,
  })
})
// 相邻车位间距
watch(margin, (val) => {
  if (!currentParkingSpace.value) return
  const { id: objectId, margin } = currentParkingSpace.value
  const _margin = Number(val)
  if (fixedPrecision(margin, precision) === _margin) return
  objectStore.updateParkingSpaceProperty({
    objectId,
    type: 'margin',
    value: _margin,
  })
})
// 停车位边线宽度
watch(lineWidth, (val) => {
  if (!currentParkingSpace.value) return
  const { id: objectId, lineWidth } = currentParkingSpace.value
  const _lineWidth = Number(val)
  if (fixedPrecision(lineWidth, precision) === _lineWidth) return
  objectStore.updateParkingSpaceProperty({
    objectId,
    type: 'lineWidth',
    value: _lineWidth,
  })
})
// 停车位重复数量
watch(count, (val) => {
  if (!currentParkingSpace.value) return
  const { id: objectId, count } = currentParkingSpace.value
  const _count = Number(val)
  if (fixedPrecision(count, precision) === _count) return
  objectStore.updateParkingSpaceProperty({
    objectId,
    type: 'count',
    value: _count,
  })
})
// 停车位内部夹角
watch(innerAngle, (val) => {
  if (!currentParkingSpace.value) return
  const { id: objectId, innerAngle } = currentParkingSpace.value
  const _innerAngle = Number(val)
  if (fixedPrecision(innerAngle, precision) === _innerAngle) return
  objectStore.updateParkingSpaceProperty({
    objectId,
    type: 'innerAngle',
    value: _innerAngle,
  })
})
// 复制停车位，防抖
const copyParkingSpace = debounce(() => {
  if (!currentParkingSpace.value) return
  const { id } = currentParkingSpace.value
  objectStore.copyParkingSpace(id)
}, 300)
</script>

<template>
  <template v-if="currentParkingSpace">
    <div class="property-title">
      {{ $t('desc.editObject.currentParkingSpaceProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.parkingSpaceId') }}</label>
        <div class="content">
          {{ currentParkingSpace.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">s</label>
        <div class="content">
          <InputNumber
            v-model="posS"
            :disabled="false"
            :precision="precision"
            :max="maxS"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">t</label>
        <div class="content">
          <InputNumber
            v-model="posT"
            :disabled="false"
            :precision="precision"
            :max="maxT"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.yaw') }}</label>
        <div class="content">
          <InputNumber
            v-model="yaw"
            :disabled="false"
            :precision="precision"
            :max="180"
            :min="-180"
            :angle="true"
            unit="°"
          />
        </div>
      </div>
      <!-- 颜色 -->
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.color') }}</label>
        <div class="content">
          <color-picker
            :display-color="lineColor"
            :predefine="DefaultColor"
            @color-change="updateParkingSpaceColor"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editParkingSpace.length') }}</label>
        <div class="content">
          <InputNumber
            v-model="length"
            :disabled="fixedProperty.includes('length')"
            :precision="precision"
            :max="50"
            :min="2.5"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editParkingSpace.width') }}</label>
        <div class="content">
          <InputNumber
            v-model="width"
            :disabled="false"
            :precision="precision"
            :max="50"
            :min="2.5"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editParkingSpace.margin') }}</label>
        <div class="content">
          <InputNumber
            v-model="margin"
            :disabled="false"
            :precision="precision"
            :max="50"
            :min="1"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editParkingSpace.lineWidth') }}</label>
        <div class="content">
          <InputNumber
            v-model="lineWidth"
            :disabled="false"
            :precision="precision"
            :max="0.5"
            :min="0.01"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editParkingSpace.number') }}</label>
        <div class="content">
          <InputNumber
            v-model="count"
            :disabled="false"
            :precision="0"
            :max="30"
            :min="1"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{
          $t('desc.editParkingSpace.innerAngle')
        }}</label>
        <div class="content">
          <InputNumber
            v-model="innerAngle"
            :disabled="fixedProperty.includes('innerAngle')"
            :precision="precision"
            :max="160"
            :min="20"
            :angle="true"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <el-button @click="copyParkingSpace">
          {{
            $t('actions.object.copyParkingSpace')
          }}
        </el-button>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
