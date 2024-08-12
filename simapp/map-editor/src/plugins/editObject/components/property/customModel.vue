<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useObjectInteractionStore } from '../../store/interaction'
import { getRoad } from '@/utils/mapCache'
import InputNumber from '@/components/common/inputNumber.vue'
import { Constant } from '@/utils/business'
import i18n from '@/locales'
import { fixedPrecision } from '@/utils/common3d'
import { useObjectStore } from '@/stores/object'
import { useModelCacheStore } from '@/stores/object/modelCache'

const objectStore = useObjectStore()
const modelCacheStore = useModelCacheStore()
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

// 调整尺寸的模式
const isRatioSize = ref(false)
const currentRatio = ref(1)

const precision = Constant.precision
const ratioPrecision = 1 // 倍数支持 1 个小数点

const currentCustomModel = computed(() => {
  return interactionStore.currentCustomModel
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
  currentCustomModel,
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

    // 通过 name 属性获取对应的配置
    const config = modelCacheStore.getCustomModelConfigByName(name)
    if (config) {
      // 自定义模型仅支持比例调节尺寸
      // 通过模型的原始尺寸和当前尺寸计算比例
      const { length, basicLength } = val
      currentRatio.value = fixedPrecision(length / basicLength, ratioPrecision)

      // 比例调节
      isRatioSize.value = true
      return
    }

    // 不支持尺寸调节的模型
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
  if (!currentCustomModel.value) return
  const { id: modelId, s, t } = currentCustomModel.value
  const _s = Number(val)
  if (fixedPrecision(s, precision) === _s) return
  objectStore.moveCustomModelByST({
    objectId: modelId,
    s: _s,
    t,
    saveRecord: true,
  })
})
watch(posT, (val) => {
  if (!currentCustomModel.value) return
  const { id: modelId, s, t } = currentCustomModel.value
  const _t = Number(val)
  if (fixedPrecision(t, precision) === _t) return
  objectStore.moveCustomModelByST({
    objectId: modelId,
    s,
    t: _t,
    saveRecord: true,
  })
})
watch(yaw, (val) => {
  if (!currentCustomModel.value) return
  const { id: modelId, yaw } = currentCustomModel.value
  const _yaw = Number(val)
  if (fixedPrecision(yaw, precision) === _yaw) return
  objectStore.rotateCustomModel({
    objectId: modelId,
    yaw: _yaw,
  })
})

// 【路口】世界坐标系下的属性更新
watch(angle, (val) => {
  if (!currentCustomModel.value) return
  const { id: modelId, angle } = currentCustomModel.value
  const _angle = Number(val)
  if (fixedPrecision(angle, precision) === _angle) return
  objectStore.rotateCustomModel({
    objectId: modelId,
    angle: _angle,
  })
})

// 通过比例调整整体模型的尺寸
watch(currentRatio, (val) => {
  if (!currentCustomModel.value) return

  const {
    id: modelId,
    length,
    basicLength,
    basicWidth,
    basicHeight,
  } = currentCustomModel.value
  const _ratio = Number(val)
  const lastRatio = fixedPrecision(length / basicLength, ratioPrecision)
  if (lastRatio === _ratio) return

  const _length = basicLength * _ratio
  const _width = basicWidth * _ratio
  const _height = basicHeight * _ratio

  objectStore.updateCustomModelSize({
    objectId: modelId,
    length: _length,
    width: _width,
    height: _height,
  })
})
</script>

<template>
  <template v-if="currentCustomModel">
    <div class="property-title">
      {{ $t('desc.editObject.currentCustomModelProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.objectId') }}</label>
        <div class="content">
          {{ currentCustomModel.id }}
        </div>
      </div>
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.objectName') }}</label>
        <div class="content">
          {{ currentCustomModel.showName }}
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
                :min="0"
                :max="maxS"
                unit="m"
              />
            </el-form-item>
            <el-form-item label="t" class="param-cell">
              <InputNumber
                v-model="posT"
                :disabled="false"
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
                :disabled="false"
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
                :disabled="false"
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
          <template v-if="isRatioSize">
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
        </el-collapse-item>
      </el-collapse>
    </el-form>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
