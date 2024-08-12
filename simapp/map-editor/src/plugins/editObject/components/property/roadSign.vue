<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useObjectInteractionStore } from '../../store/interaction'
import InputNumber from '@/components/common/inputNumber.vue'
import { Constant } from '@/utils/business'
import { fixedPrecision } from '@/utils/common3d'
import { getRoad } from '@/utils/mapCache'
import { useObjectStore } from '@/stores/object'
import i18n from '@/locales'
import { getRoadSignConfig } from '@/config/roadSign'
import { parseLateralDecelerationLength } from '@/stores/object/customRoadSign/lateralDecelerationMarking'
import { parseSemicircleLineLength } from '@/stores/object/customRoadSign/semicircleLine'

const interactionStore = useObjectInteractionStore()
const objectStore = useObjectStore()

const activeNames = ref(['position', 'rotation', 'size'])

// 是否处于道路中
const onRoad = ref(true)
// 道路中需要用到的属性
// st 坐标
const posS = ref('')
const posT = ref('')
// 根据道路的长度，来调整最大的 s 坐标值
const minS = ref(0)
const maxS = ref(0)
const maxT = Constant.maxTValue
// 是否可以在道路上旋转角度
const rotateInRoad = ref(false)
// 是否可以在路口中旋转角度
const rotateInJunction = ref(false)
// 沿 st 坐标系的偏航角
const yaw = ref('')
// 路口中需要用到的属性
const posX = ref('')
const posY = ref('')
const angle = ref('')
// 尺寸
const length = ref('')
const width = ref('')
// 调整尺寸的模式
const adjustSizeMode = ref('')
const adjustLength = ref(true)
const adjustWidth = ref(true)
const defaultMaxLength = 20
const defaultMinLength = 0.001
// 长和宽的最大最小值
const maxLength = ref(defaultMaxLength)
const minLength = ref(defaultMinLength)
const maxWidth = ref(defaultMaxLength)
const minWidth = ref(defaultMinLength)

const precision = Constant.precision
const currentRoadSign = computed(() => {
  return interactionStore.currentRoadSign
})

// 位置状态栏标签
const positionTabName = computed(() => {
  return onRoad.value ?
    i18n.global.t('desc.commonRoad.stCoordinatePosition') :
    i18n.global.t('desc.commonRoad.globalCoordinatePosition')
})
const angleTabName = computed(() => {
  return onRoad.value ?
    i18n.global.t('desc.commonRoad.stCoordinateAngle') :
    i18n.global.t('desc.commonRoad.globalCoordinateAngle')
})

watch(
  currentRoadSign,
  (val) => {
    if (!val) return

    const { roadId, name } = val
    if (roadId) {
      const roadData = getRoad(roadId)
      if (roadData) {
        switch (name) {
          case 'Crosswalk_Line': {
            // 如果是人行横道，允许放置到道路外一定的范围
            const offset = Constant.crosswalkSize.offset + val.width / 2
            minS.value = -offset
            maxS.value = Number(roadData.length) + offset
            break
          }
          case 'Crosswalk_with_Left_and_Right_Side': {
            // 行人左右分道的人行横道，允许放置到道路外一定范围
            const offset = Constant.crosswalkWithSideSize.offset + val.width / 2
            minS.value = -offset
            maxS.value = Number(roadData.length) + offset
            break
          }
          case 'Road_Guide_Lane_Line':
          case 'Variable_Direction_Lane_Line':
            // 导向车道线，最大长度为道路长度
            maxLength.value = roadData.length
            minLength.value = defaultMinLength
            minS.value = 0
            maxS.value = Number(roadData.length)
            break
          case 'Longitudinal_Deceleration_Marking':
            // 纵向减速标线
            maxLength.value = roadData.length
            minLength.value = Constant.longitudinalDecelerationSize.length.min
            minS.value = 0
            maxS.value = Number(roadData.length)
            break
          case 'Lateral_Deceleration_Marking':
            // 横向减速标线
            maxLength.value = roadData.length
            minLength.value = Constant.lateralDecelerationSize.length.min
            minS.value = 0
            maxS.value = Number(roadData.length)
            break
          case 'White_Semicircle_Line_Vehicle_Distance_Confirmation':
            // 白色半圆状车距确认线
            maxLength.value = roadData.length
            minLength.value = Constant.semicircleLineSize.length.min
            minS.value = 0
            maxS.value = Number(roadData.length)
            break
          default:
            minS.value = 0
            maxS.value = Number(roadData.length)
            break
        }
      }
      const config = getRoadSignConfig(name)
      if (config) {
        const { freeMoveInRoad = false } = config
        rotateInRoad.value = freeMoveInRoad
      }
      posS.value = String(fixedPrecision(val.s))
      posT.value = String(fixedPrecision(val.t))
      yaw.value = String(fixedPrecision(val.yaw))
      onRoad.value = true
    } else {
      posX.value = String(fixedPrecision(val.position.z))
      posY.value = String(fixedPrecision(val.position.x))
      angle.value = String(fixedPrecision(val.angle))
      onRoad.value = false
      rotateInJunction.value = true
    }

    // 通过 name 属性获取对应的配置
    const config = getRoadSignConfig(name)
    if (config) {
      const { adjustSize } = config
      width.value = String(val.width)
      length.value = String(val.length)
      // 尺寸相关的配置
      if (adjustSize) {
        adjustSizeMode.value = adjustSize
        adjustLength.value = true
        adjustWidth.value = true
        // 部分路面标识的长宽比较大，长度可能超过20m，需要手动调整最大的长度为25m
        switch (name) {
          case 'Non_Motor_Vehicle_Line':
            maxLength.value = 25
            minLength.value = defaultMinLength
            maxWidth.value = 25
            minWidth.value = defaultMinLength
            break
          case 'Non_Motor_Vehicle_Area':
            // 路口中的非机动车禁驶区标记
            maxLength.value = Constant.nonMotorVehicleSize.max
            minLength.value = Constant.nonMotorVehicleSize.min
            maxWidth.value = Constant.nonMotorVehicleSize.max
            minWidth.value = Constant.nonMotorVehicleSize.min
            break
          case 'Crosswalk_Line':
            // 人行横道
            // 不可旋转角度
            rotateInRoad.value = false
            maxLength.value = Constant.crosswalkSize.length.max
            minLength.value = Constant.crosswalkSize.length.min
            maxWidth.value = Constant.crosswalkSize.width.max
            minWidth.value = Constant.crosswalkSize.width.min
            break
          case 'Crosswalk_with_Left_and_Right_Side':
            // 行人左右分道的人行横道
            // 不可旋转角度
            rotateInRoad.value = false
            maxLength.value = Constant.crosswalkWithSideSize.length.max
            minLength.value = Constant.crosswalkWithSideSize.length.min
            maxWidth.value = Constant.crosswalkWithSideSize.width.max
            minWidth.value = Constant.crosswalkWithSideSize.width.min
            break
          case 'Road_Guide_Lane_Line':
          case 'Variable_Direction_Lane_Line':
          case 'Longitudinal_Deceleration_Marking':
          case 'Lateral_Deceleration_Marking':
          case 'White_Semicircle_Line_Vehicle_Distance_Confirmation':
            // 导向车道线、可变导线车道线
            // 纵、横向减速标线
            // 白色半圆状车距确认线
            rotateInRoad.value = false
            // 线宽不可编辑
            adjustWidth.value = false
            maxWidth.value = defaultMaxLength
            minWidth.value = defaultMinLength
            break
          default:
            // 其他默认为 20
            maxLength.value = defaultMaxLength
            minLength.value = defaultMinLength
            maxWidth.value = defaultMaxLength
            minWidth.value = defaultMinLength
            break
        }
      } else {
        adjustSizeMode.value = ''
        adjustLength.value = false
        adjustWidth.value = false
        maxLength.value = defaultMaxLength
        minLength.value = defaultMinLength
      }
    }

    if (name === 'Intersection_Guide_Line') {
      // 路口导向线【路口中】
      // 不可旋转
      rotateInJunction.value = false
      adjustSizeMode.value = ''
      // 设置为一个比较大的值
      maxLength.value = 200
      minLength.value = defaultMinLength
      maxWidth.value = defaultMaxLength
      minWidth.value = defaultMinLength
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

// 【道路】参考线坐标系下的属性更新
watch(posS, (val) => {
  if (!currentRoadSign.value) return
  // 基于 st 坐标来计算世界坐标系下的空间坐标
  const { id: roadSignId, s, t } = currentRoadSign.value
  const _s = Number(val)
  // 判断精度调整后的数值是否相等
  if (fixedPrecision(s, precision) === _s) return
  // 调用数据层更新 s 坐标的方法
  objectStore.moveRoadSignByST({
    objectId: roadSignId,
    s: _s,
    t,
    saveRecord: true,
  })
})
watch(posT, (val) => {
  if (!currentRoadSign.value) return
  const { id: roadSignId, s, t } = currentRoadSign.value
  const _t = Number(val)
  if (fixedPrecision(t, precision) === _t) return
  objectStore.moveRoadSignByST({
    objectId: roadSignId,
    s,
    t: _t,
    saveRecord: true,
  })
})
watch(yaw, (val) => {
  if (!currentRoadSign.value) return
  const { id: roadSignId, yaw } = currentRoadSign.value
  const _yaw = Number(val)
  if (fixedPrecision(yaw, precision) === _yaw) return
  objectStore.rotateRoadSign({
    objectId: roadSignId,
    yaw: _yaw,
  })
})

// 【路口】世界坐标系下的属性更新
watch(angle, (val) => {
  if (!currentRoadSign.value) return
  const { id: roadSignId, angle } = currentRoadSign.value
  const _angle = Number(val)
  if (fixedPrecision(angle, precision) === _angle) return
  objectStore.rotateRoadSign({
    objectId: roadSignId,
    angle: _angle,
  })
})

// 尺寸的更新
watch(width, (val) => {
  if (!currentRoadSign.value) return
  // 如果无法调整尺寸，则返回
  if (!adjustSizeMode.value) return

  const { id: roadSignId, width: lastWidth, length } = currentRoadSign.value
  let _width = Number(val)
  let _length = length

  if (adjustSizeMode.value === 'ratio2') {
    // 等比例更新尺寸
    const ratio = length / lastWidth
    _length = fixedPrecision(ratio * _width)
    // 长和宽存在最大长度的限制，如果超过限制需要调整对应的数值
    if (_length > maxLength.value) {
      _length = maxLength.value
      _width = fixedPrecision(_length / ratio)
    } else if (_length < defaultMinLength) {
      _length = defaultMinLength
      _width = fixedPrecision(length / ratio)
    }
  }
  // 如果数据不合理则返回
  if (
    Number.isNaN(lastWidth) ||
    Number.isNaN(length) ||
    Number.isNaN(_length) ||
    Number.isNaN(_width)
  ) {
    return
  }

  // 如果更新后的数字完全一样，则不触发数据层的更新
  if (
    fixedPrecision(length, precision) === _length &&
    fixedPrecision(lastWidth, precision) === _width
  ) {
    width.value = String(_width)
    return
  }

  objectStore.updateRoadSignSize({
    objectId: roadSignId,
    length: _length,
    width: _width,
  })
})
watch(length, (val) => {
  if (!currentRoadSign.value) return
  // 如果无法调整尺寸，则返回
  if (!adjustSizeMode.value) return

  const {
    id: roadSignId,
    width,
    length: lastLength,
    name,
  } = currentRoadSign.value
  let _length = Number(val)
  let _width = width

  if (name === 'Lateral_Deceleration_Marking') {
    // 纵向减速标线，长度只能是固定值
    const fixedLength = parseLateralDecelerationLength(_length)
    // 如果更新后的尺寸，跟原来的尺寸一样，则无须重复更新
    if (lastLength === fixedLength) {
      length.value = String(lastLength)
      return
    }
  } else if (name === 'White_Semicircle_Line_Vehicle_Distance_Confirmation') {
    // 白色半圆状车距确认线，长度只能是固定值
    const fixedLength = parseSemicircleLineLength(_length)
    if (lastLength === fixedLength) {
      length.value = String(lastLength)
      return
    }
  }

  if (adjustSizeMode.value === 'ratio2') {
    // 等比例更新尺寸
    const ratio = width / lastLength
    _width = fixedPrecision(ratio * _length)
    // 长和宽存在最大长度的限制，如果超过限制需要调整对应的数值
    if (_width > maxLength.value) {
      _width = maxLength.value
      _length = fixedPrecision(_width / ratio)
    } else if (_width < defaultMinLength) {
      // 如果长度小于最小的值，需要调整对应的数值
      _width = defaultMinLength
      _length = fixedPrecision(_width / ratio)
    }
  }

  // 如果数据不合理则返回
  if (
    Number.isNaN(lastLength) ||
    Number.isNaN(width) ||
    Number.isNaN(_length) ||
    Number.isNaN(_width)
  ) {
    return
  }

  // 如果更新后的数字完全一样，则不触发数据层的更新
  if (
    fixedPrecision(lastLength, precision) === _length &&
    fixedPrecision(width, precision) === _width
  ) {
    length.value = String(_length)
    return
  }

  objectStore.updateRoadSignSize({
    objectId: roadSignId,
    length: _length,
    width: _width,
  })
})
</script>

<template>
  <template v-if="currentRoadSign">
    <div class="property-title">
      {{ $t('desc.editObject.currentRoadSignProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.roadSignId') }}</label>
        <div class="content">
          {{ currentRoadSign.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.roadSignName') }}</label>
        <div class="content">
          {{ currentRoadSign.showName }}
        </div>
      </div>
    </div>
    <el-form class="property-form" label-width="60" @submit.enter.prevent>
      <el-collapse v-model="activeNames">
        <el-collapse-item :title="positionTabName" name="position">
          <template v-if="onRoad">
            <!-- 在道路上，使用 st 坐标 -->
            <el-form-item label="s" class="param-cell">
              <InputNumber
                v-model="posS"
                :disabled="false"
                :precision="precision"
                :min="minS"
                :max="maxS"
                unit="m"
              />
            </el-form-item>
            <!-- 道路上的路面标识，不支持通过属性面板更改 t 值 -->
            <el-form-item label="t" class="param-cell">
              <InputNumber
                v-model="posT"
                :disabled="!rotateInRoad"
                :precision="precision"
                :max="maxT"
                unit="m"
              />
            </el-form-item>
          </template>
          <template v-else>
            <!-- 在路口中，使用 xy 坐标【不提供输入框更新位置】 -->
            <el-form-item label="x" class="param-cell">
              <InputNumber
                v-model="posX"
                :disabled="true"
                :precision="precision"
                unit="m"
              />
            </el-form-item>
            <el-form-item label="y" class="param-cell">
              <InputNumber
                v-model="posY"
                :disabled="true"
                :precision="precision"
                unit="m"
              />
            </el-form-item>
          </template>
        </el-collapse-item>
        <el-collapse-item :title="angleTabName" name="rotation">
          <template v-if="onRoad">
            <!-- 指的是沿着道路方向的偏航角 -->
            <el-form-item :label="$t('desc.commonRoad.yaw')" class="param-cell">
              <InputNumber
                v-model="yaw"
                :disabled="!rotateInRoad"
                :precision="precision"
                :max="180"
                :min="-180"
                :angle="true"
                unit="°"
              />
            </el-form-item>
          </template>
          <template v-else>
            <!-- 绝对坐标系下的角度 -->
            <el-form-item
              :label="$t('desc.commonRoad.angle')"
              class="param-cell"
            >
              <InputNumber
                v-model="angle"
                :disabled="!rotateInJunction"
                :precision="precision"
                :max="180"
                :min="-180"
                :angle="true"
                unit="°"
              />
            </el-form-item>
          </template>
        </el-collapse-item>
        <el-collapse-item :title="$t('desc.commonRoad.size')" name="size">
          <el-form-item
            :label="$t('desc.commonRoad.length')"
            class="param-cell"
          >
            <InputNumber
              v-model="length"
              :disabled="!adjustLength"
              :precision="precision"
              :max="maxLength"
              :min="minLength"
              unit="m"
            />
          </el-form-item>
          <el-form-item :label="$t('desc.commonRoad.width')" class="param-cell">
            <InputNumber
              v-model="width"
              :disabled="!adjustWidth"
              :precision="precision"
              :max="maxWidth"
              :min="minWidth"
              unit="m"
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
