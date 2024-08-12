<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useObjectInteractionStore } from '../../store/interaction'
import InputNumber from '@/components/common/inputNumber.vue'
import { Constant } from '@/utils/business'
import { fixedPrecision } from '@/utils/common3d'
import i18n from '@/locales'
import { getRoad } from '@/utils/mapCache'
import { useObjectStore } from '@/stores/object'

const interactionStore = useObjectInteractionStore()
const objectStore = useObjectStore()

const activeNames = ref(['position', 'rotation'])

// 是否处于道路中
const onRoad = ref(true)
// 放置在道路上时，对应的属性
// st 坐标
const posS = ref('')
const posT = ref('')
// 通过道路长度来调整最大的s值（s坐标的最大最小限制是必须的）
const maxS = ref(0)
const maxT = Constant.maxTValue
// 沿st坐标系的偏航角
const yaw = ref('')
// 放置在路口中时，对应的属性
// xy 坐标
const posX = ref('') // 对应 z
const posY = ref('') // 对应 x
// 世界坐标系下的角度
const angle = ref('')

const precision = Constant.precision
const currentPole = computed(() => {
  return interactionStore.currentPole
})

// 位置状态栏标签名
const positionTabName = computed(() => {
  return onRoad.value ?
    i18n.global.t('desc.commonRoad.stCoordinatePosition') :
    i18n.global.t('desc.commonRoad.globalCoordinatePosition')
})
// 角度状态栏标签名
const angleTabName = computed(() => {
  return onRoad.value ?
    i18n.global.t('desc.commonRoad.stCoordinateAngle') :
    i18n.global.t('desc.commonRoad.globalCoordinateAngle')
})

watch(
  currentPole,
  (val) => {
    if (!val) return
    const { roadId } = val
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
  },
  {
    immediate: true,
    deep: true,
  },
)

// 【道路】参考线坐标系下的属性更新
// 监听输入框对 s 值的修改
watch(posS, (val) => {
  if (!currentPole.value) return
  // 基于 st 坐标来计算世界坐标系下的空间坐标
  const { id: poleId, s, t } = currentPole.value
  const _s = Number(val)
  // 判断精度调整后的数值是否相等
  if (fixedPrecision(s, precision) === _s) return
  // 调用数据层更新 s 坐标的方法
  objectStore.movePoleByST({
    objectId: poleId,
    s: _s,
    t,
    saveRecord: true,
  })
})
// 监听输入框对 t 值的修改
watch(posT, (val) => {
  if (!currentPole.value) return
  // 基于 st 坐标来计算世界坐标系下的空间坐标
  const { id: poleId, s, t } = currentPole.value
  const _t = Number(val)
  // 判断精度调整后的数值是否相等
  if (fixedPrecision(t, precision) === _t) return
  // 调用数据层更新 s 坐标的方法
  objectStore.movePoleByST({
    objectId: poleId,
    s,
    t: _t,
    saveRecord: true,
  })
})

// 监听输入框对 yaw 值的修改
watch(yaw, (val) => {
  if (!currentPole.value) return
  const { id: poleId, yaw } = currentPole.value
  const _yaw = Number(val)
  if (fixedPrecision(yaw, precision) === _yaw) return
  objectStore.rotatePole({
    objectId: poleId,
    yaw: _yaw,
  })
})

// 【路口】世界坐标系下的属性更新
watch(angle, (val) => {
  if (!currentPole.value) return
  const { id: poleId, angle } = currentPole.value
  const _angle = Number(val)
  if (fixedPrecision(angle, precision) === _angle) return
  objectStore.rotatePole({
    objectId: poleId,
    angle: _angle,
  })
})
</script>

<template>
  <template v-if="currentPole">
    <div class="property-title">
      {{ $t('desc.editObject.currentPoleProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.poleId') }}</label>
        <div class="content">
          {{ currentPole.id }}
        </div>
      </div>
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.poleName') }}</label>
        <div class="content">
          {{ currentPole.showName }}
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
        <template v-if="!onRoad">
          <el-collapse-item :title="angleTabName" name="rotation">
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
          </el-collapse-item>
        </template>
        <!-- 沿着道路方向的偏航角【暂时隐藏道路上杆的旋转】 -->
        <!-- <template v-else>
          <el-collapse-item :title="angleTabName" name="rotation">
            <el-form-item :label="$t('desc.commonRoad.yaw')" class="param-cell">
              <input-number
                v-model="yaw"
                :disabled="false"
                :precision="precision"
                :max="180"
                :min="-180"
                :angle="true"
                unit="°"
              />
            </el-form-item>
          </el-collapse-item>
        </template> -->
      </el-collapse>
    </el-form>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
