<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useObjectInteractionStore } from '../../store/interaction'
import { getRoad } from '@/utils/mapCache'
import InputNumber from '@/components/common/inputNumber.vue'
import { Constant } from '@/utils/business'
import i18n from '@/locales'
import { fixedPrecision } from '@/utils/common3d'
import { useObjectStore } from '@/stores/object'
import { getOtherConfig } from '@/config/other'

const objectStore = useObjectStore()
const interactionStore = useObjectInteractionStore()

const activeNames = ref(['position', 'rotation', 'size'])

// 是否处于道路中
const onRoad = ref(true)
// 放置在道路上时对应的属性
// st 坐标
const posS = ref('')
const posT = ref('')
// 通过道路长度来调整最大的 s 值
const maxS = ref(0)
const maxT = Constant.maxTValue
// 沿st坐标系的偏航角
const yaw = ref('')

// 放置在路口中时对应的属性
const posX = ref('')
const posY = ref('')
// 世界坐标系下的角度
const angle = ref('')

// 长宽高 size
const length = ref('')
const width = ref('')
const height = ref('')
// 是否支持调节长宽高
const adjustLengthDisabled = ref(false)
const adjustWidthDisabled = ref(false)
const adjustHeightDisabled = ref(false)
// 调整尺寸的模式
const adjustSizeMode = ref('')
const maxLength = ref(20)
const isRatioSize = ref(false)
const currentRatio = ref(1)
// 对于天桥物体，调整对应的跨度
const isBridge = ref(false)
const span = ref('')

const precision = Constant.precision
const ratioPrecision = 1
const currentOther = computed(() => {
  return interactionStore.currentOther
})

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
  currentOther,
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
      yaw.value = String(fixedPrecision(val.yaw))
      onRoad.value = true
    } else {
      posX.value = String(fixedPrecision(val.position.z))
      posY.value = String(fixedPrecision(val.position.x))
      angle.value = String(fixedPrecision(val.angle))
      onRoad.value = false
    }

    isBridge.value = false

    // 人行天桥，且跨度有效
    if (name === 'PedestrianBridge' && val.span) {
      isBridge.value = true
      span.value = String(fixedPrecision(val.span))
      return
    }

    // 通过 name 属性获取对应的配置
    const config = getOtherConfig(name)
    if (config) {
      const { adjustSize } = config
      if (adjustSize) {
        adjustSizeMode.value = adjustSize
        width.value = String(val.width)
        length.value = String(val.length)
        height.value = String(val.height)
        if (adjustSize === 'l') {
          // 只能调节长度
          adjustLengthDisabled.value = false
          adjustWidthDisabled.value = true
          adjustHeightDisabled.value = true
          isRatioSize.value = false
          return
        } else if (adjustSize === 'lw') {
          // 只能调节长和宽
          adjustLengthDisabled.value = false
          adjustWidthDisabled.value = false
          adjustHeightDisabled.value = true
          isRatioSize.value = false
          return
        } else if (adjustSize === 'lwh') {
          // 长宽高每个属性都能单独调节
          adjustLengthDisabled.value = false
          adjustWidthDisabled.value = false
          adjustHeightDisabled.value = false
          isRatioSize.value = false
          return
        } else if (adjustSize === 'ratio3') {
          // 只能同比例调整尺寸
          adjustLengthDisabled.value = true
          adjustWidthDisabled.value = true
          adjustHeightDisabled.value = true

          // 通过模型的原始尺寸和当前尺寸计算比例
          const { length, basicLength } = val
          currentRatio.value = fixedPrecision(
            length / basicLength,
            ratioPrecision,
          )

          // 比例调节
          isRatioSize.value = true
          return
        }
      }
    }

    // 不支持尺寸调节的模型
    adjustSizeMode.value = ''
    width.value = String(val.width)
    length.value = String(val.length)
    height.value = String(val.height)
    adjustLengthDisabled.value = true
    adjustWidthDisabled.value = true
    adjustHeightDisabled.value = true
    isRatioSize.value = false
    currentRatio.value = 1
  },
  {
    immediate: true,
    deep: true,
  },
)
// 【道路】参考线坐标系下的属性更新
watch(posS, (val) => {
  if (!currentOther.value) return
  const { id: otherId, s, t } = currentOther.value
  const _s = Number(val)
  if (fixedPrecision(s, precision) === _s) return
  objectStore.moveOtherByST({
    objectId: otherId,
    s: _s,
    t,
    saveRecord: true,
  })
})
watch(posT, (val) => {
  if (!currentOther.value) return
  const { id: otherId, s, t } = currentOther.value
  const _t = Number(val)
  if (fixedPrecision(t, precision) === _t) return
  objectStore.moveOtherByST({
    objectId: otherId,
    s,
    t: _t,
    saveRecord: true,
  })
})
watch(yaw, (val) => {
  if (!currentOther.value) return
  const { id: otherId, yaw } = currentOther.value
  const _yaw = Number(val)
  if (fixedPrecision(yaw, precision) === _yaw) return
  objectStore.rotateOther({
    objectId: otherId,
    yaw: _yaw,
  })
})

// 【路口】世界坐标系下的属性更新
watch(angle, (val) => {
  if (!currentOther.value) return
  const { id: otherId, angle } = currentOther.value
  const _angle = Number(val)
  if (fixedPrecision(angle, precision) === _angle) return
  objectStore.rotateOther({
    objectId: otherId,
    angle: _angle,
  })
})

// 尺寸的更新
watch(length, (val) => {
  if (!currentOther.value) return
  if (!adjustSizeMode.value || adjustSizeMode.value === 'ratio3') return

  const { id: otherId, length, width } = currentOther.value
  const _length = Number(val)
  if (fixedPrecision(length, precision) === _length) return

  objectStore.updateOtherSize({
    objectId: otherId,
    length: _length,
    width,
  })
})
watch(width, (val) => {
  if (!currentOther.value) return
  if (!adjustSizeMode.value || adjustSizeMode.value === 'ratio3') return

  const { id: otherId, length, width } = currentOther.value
  const _width = Number(val)
  if (fixedPrecision(width, precision) === _width) return

  objectStore.updateOtherSize({
    objectId: otherId,
    length,
    width: _width,
  })
})
watch(height, (val) => {
  if (!currentOther.value) return
  if (!adjustSizeMode.value || adjustSizeMode.value === 'ratio3') return

  const { id: otherId, length, width, height } = currentOther.value
  const _height = Number(val)
  if (fixedPrecision(height, precision) === _height) return

  objectStore.updateOtherSize({
    objectId: otherId,
    length,
    width,
    height: _height,
  })
})
// 通过比例调整整体模型的尺寸
watch(currentRatio, (val) => {
  if (!currentOther.value) return
  if (!adjustSizeMode.value) return

  const {
    id: otherId,
    length,
    basicLength,
    basicWidth,
    basicHeight,
  } = currentOther.value
  const _ratio = Number(val)
  const lastRatio = fixedPrecision(length / basicLength, ratioPrecision)
  if (lastRatio === _ratio) return

  const _length = basicLength * _ratio
  const _width = basicWidth * _ratio
  const _height = basicHeight * _ratio

  objectStore.updateOtherSize({
    objectId: otherId,
    length: _length,
    width: _width,
    height: _height,
  })
})
// 更新人行天桥的跨度
watch(span, (val) => {
  if (!currentOther.value) return
  if (currentOther.value.name !== 'PedestrianBridge') return
  if (!currentOther.value.span) return

  const { id: bridgeId, span } = currentOther.value
  const _span = Number(val)
  if (fixedPrecision(_span) === fixedPrecision(span)) return

  // 更新天桥的跨度
  objectStore.updateBridgeSpan({
    objectId: bridgeId,
    span: _span,
  })
})
</script>

<template>
  <template v-if="currentOther">
    <div class="property-title">
      {{ $t('desc.editObject.currentOtherTypeObjectProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.objectId') }}</label>
        <div class="content">
          {{ currentOther.id }}
        </div>
      </div>
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.objectName') }}</label>
        <div class="content">
          {{ currentOther.showName }}
        </div>
      </div>
    </div>
    <el-form class="property-form" label-width="60" @submit.enter.prevent>
      <el-collapse v-model="activeNames">
        <el-collapse-item :title="positionTabName" name="position">
          <BaseAuth
            :perm="[
              'action.mapEditor.models.other.bridge.props.position.enable',
              'disabled',
            ]"
          >
            <template #default="{ disabled }">
              <template v-if="onRoad">
                <!-- 在道路上，使用 st 坐标 -->
                <el-form-item label="s" class="param-cell">
                  <InputNumber
                    v-model="posS"
                    :disabled="isBridge && disabled"
                    :precision="precision"
                    :min="0"
                    :max="maxS"
                    unit="m"
                  />
                </el-form-item>
                <el-form-item label="t" class="param-cell">
                  <InputNumber
                    v-model="posT"
                    :disabled="isBridge && disabled"
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
            </template>
          </BaseAuth>
        </el-collapse-item>
        <el-collapse-item :title="angleTabName" name="rotation">
          <BaseAuth
            :perm="[
              'action.mapEditor.models.other.bridge.props.pattern.enable',
              'disabled',
            ]"
          >
            <template #default="{ disabled }">
              <template v-if="onRoad">
                <!-- 指的是沿着道路方向的偏航角 -->
                <el-form-item
                  :label="$t('desc.commonRoad.yaw')"
                  class="param-cell"
                >
                  <InputNumber
                    v-model="yaw"
                    :disabled="isBridge && disabled"
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
                    :disabled="isBridge && disabled"
                    :precision="precision"
                    :max="180"
                    :min="-180"
                    :angle="true"
                    unit="°"
                  />
                </el-form-item>
              </template>
            </template>
          </BaseAuth>
        </el-collapse-item>
        <el-collapse-item :title="$t('desc.commonRoad.size')" name="size">
          <!-- 人行天桥的跨度 -->
          <template v-if="isBridge">
            <BaseAuth
              :perm="[
                'action.mapEditor.models.other.bridge.props.pattern.enable',
                'disabled',
              ]"
            >
              <template #default="{ disabled }">
                <el-form-item
                  :label="$t('desc.editObject.span')"
                  class="param-cell"
                >
                  <InputNumber
                    v-model="span"
                    :disabled="isBridge && disabled"
                    :precision="3"
                    :max="50"
                    :min="0.001"
                    unit="m"
                  />
                </el-form-item>
              </template>
            </BaseAuth>
          </template>
          <template v-if="!isBridge && isRatioSize">
            <el-form-item
              :label="$t('desc.commonRoad.ratio')"
              class="param-cell"
            >
              <InputNumber
                v-model="currentRatio"
                :disabled="false"
                :precision="ratioPrecision"
                :max="10"
                :min="0.001"
                :unit="$t('desc.commonRoad.time')"
              />
            </el-form-item>
          </template>
          <template v-if="!isBridge && !isRatioSize">
            <el-form-item
              :label="$t('desc.commonRoad.length')"
              class="param-cell"
            >
              <InputNumber
                v-model="length"
                :disabled="adjustLengthDisabled"
                :precision="precision"
                :max="maxLength"
                :min="0.001"
                unit="m"
              />
            </el-form-item>
            <el-form-item
              :label="$t('desc.commonRoad.width')"
              class="param-cell"
            >
              <InputNumber
                v-model="width"
                :disabled="adjustWidthDisabled"
                :precision="precision"
                :max="maxLength"
                :min="0.001"
                unit="m"
              />
            </el-form-item>
            <el-form-item
              :label="$t('desc.commonRoad.height')"
              class="param-cell"
            >
              <InputNumber
                v-model="height"
                :disabled="adjustHeightDisabled"
                :precision="precision"
                :max="maxLength"
                :min="0.001"
                unit="m"
              />
            </el-form-item>
          </template>
        </el-collapse-item>
      </el-collapse>
    </el-form>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
