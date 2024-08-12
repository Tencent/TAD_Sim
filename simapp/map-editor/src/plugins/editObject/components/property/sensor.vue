<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useObjectInteractionStore } from '../../store/interaction'
import Camera from './camera.vue'
import Rsu from './rsu.vue'
import Radar from './radar.vue'
import Lidar from './lidar.vue'
import InputNumber from '@/components/common/inputNumber.vue'
import { fixedPrecision } from '@/utils/common3d'
import { getPoleConfig } from '@/config/pole'
import { useObjectStore } from '@/stores/object'
import { Constant } from '@/utils/business'
import { getObject } from '@/utils/mapCache'

const interactionStore = useObjectInteractionStore()
const objectStore = useObjectStore()
const activeNames = ref(['position', 'rotation', 'device'])

const onVerticalPole = ref(false)
// 传感器在杆上的局部坐标
// 高度和水平距离
const height = ref('')
const distance = ref('')
// 在横杆和竖杆上放置的最大最小值
const minHeight = ref(0)
const maxHeight = ref(0)
const minDistance = ref(0)
const maxDistance = ref(0)
// 如果在竖杆上，局部坐标系下的角度【朝着来车方向，角度为 0】
const angle = ref('')
const sensorName = ref('')

const precision = Constant.precision

const currentSensor = computed(() => {
  return interactionStore.currentSensor
})

// 根据当前传感器是否放置在竖杆上，调整变量状态
watch(
  currentSensor,
  (val) => {
    if (!val) return

    if (val.onVerticalPole) {
      // 竖杆上
      onVerticalPole.value = true
      minHeight.value = 0
      const poleData = getObject(val.poleId)
      if (poleData) {
        const poleConfig = getPoleConfig(poleData.name)
        if (poleConfig) {
          maxHeight.value = poleConfig.style.height
        }
      }
      angle.value = String(fixedPrecision(val.angle))
    } else {
      // 横杆上
      const poleData = getObject(val.poleId)
      if (poleData) {
        const poleConfig = getPoleConfig(poleData.name)
        if (poleConfig) {
          const { style } = poleConfig
          onVerticalPole.value = false
          minDistance.value = style.vRadius
          maxDistance.value = style.length + style.vRadius
        }
      }
    }
    // 在杆上的水平和竖直距离，是基于标志牌在杆的局部坐标系下的坐标
    // y 指高度
    height.value = String(fixedPrecision(val.position.y))
    // z 指距离竖杆圆心的距离
    distance.value = String(fixedPrecision(val.position.z))

    // 传感器的名称
    sensorName.value = val.name
  },
  {
    immediate: true,
    deep: true,
  },
)

// 更新竖杆上的高度
watch(height, (val) => {
  if (!currentSensor.value) return
  const { id: sensorId, position } = currentSensor.value
  // 高度取 y
  const { y: originHeight } = position
  const _height = Number(val)
  if (fixedPrecision(originHeight, precision) === _height) return
  objectStore.moveSensorByHeightAndDist({
    objectId: sensorId,
    height: _height,
  })
})

// 更新横杆上的水平距离
watch(distance, (val) => {
  if (!currentSensor.value) return
  const { id: sensorId, position } = currentSensor.value
  // 水平距离取 z
  const { z: originDistance } = position
  const _distance = Number(val)
  if (fixedPrecision(originDistance, precision) === _distance) return
  objectStore.moveSensorByHeightAndDist({
    objectId: sensorId,
    distance: _distance,
  })
})

// 更新竖杆上标志牌的角度（绕竖杆的角度）
watch(angle, (val) => {
  if (!currentSensor.value) return
  const { id: sensorId, angle } = currentSensor.value
  const _angle = Number(val)
  if (fixedPrecision(angle, precision) === _angle) return
  objectStore.rotateSensor({
    objectId: sensorId,
    angle: _angle,
  })
})

// 快速切换朝向
function toggleDirection () {
  if (!currentSensor.value) return
  const { id, position } = currentSensor.value

  // positionX 如果大于0，说明物体处于正向，切换就需要调整为反向
  // positionX 如果小于0，说明物体处于反向，切换就需要调整为正向
  const currentIsForward = position.x > 0
  objectStore.toggleTrafficLightDirection({
    objectId: id,
    isForward: !currentIsForward,
  })
}
</script>

<template>
  <template v-if="currentSensor">
    <div class="property-title">
      {{ $t('desc.editObject.currentSensorProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.sensorId') }}</label>
        <div class="content">
          {{ currentSensor.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.sensorName') }}</label>
        <div class="content">
          {{ currentSensor.showName }}
        </div>
      </div>
      <el-form class="property-form" label-width="70" @submit.enter.prevent>
        <el-collapse v-model="activeNames">
          <el-collapse-item
            :title="$t('desc.commonRoad.positionOnPole')"
            name="position"
          >
            <el-form-item
              :label="$t('desc.editObject.verticalDistance')"
              class="param-cell"
            >
              <template v-if="onVerticalPole">
                <!-- 竖杆上的物体垂直高度，可修改 -->
                <InputNumber
                  v-model="height"
                  :disabled="false"
                  :precision="precision"
                  :min="minHeight"
                  :max="maxHeight"
                  unit="m"
                />
              </template>
              <template v-else>
                <!-- 横杆上的物体垂直高度，不可修改 -->
                <InputNumber
                  v-model="height"
                  :disabled="true"
                  :precision="precision"
                  unit="m"
                />
              </template>
            </el-form-item>
            <el-form-item
              :label="$t('desc.editObject.horizontalDistance')"
              class="param-cell"
            >
              <template v-if="onVerticalPole">
                <!-- 竖杆上的物体水平距离，不可修改 -->
                <InputNumber
                  v-model="distance"
                  :disabled="true"
                  :precision="precision"
                  unit="m"
                />
              </template>
              <template v-else>
                <!-- 横杆上的物体水平距离，不可修改 -->
                <InputNumber
                  v-model="distance"
                  :disabled="false"
                  :precision="precision"
                  :min="minDistance"
                  :max="maxDistance"
                  unit="m"
                />
              </template>
            </el-form-item>
            <!-- 只有位于横杆上的物体才会展示 -->
            <template v-if="!currentSensor.onVerticalPole">
              <el-button
                :style="{ marginBottom: '10px' }"
                @click="toggleDirection"
              >
                {{ $t('actions.object.toggleOrientation') }}
              </el-button>
            </template>
          </el-collapse-item>
          <template v-if="onVerticalPole">
            <el-collapse-item
              :title="$t('desc.commonRoad.angleOnPole')"
              name="rotation"
            >
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
        </el-collapse>
      </el-form>

      <el-form class="property-form" label-width="70" @submit.enter.prevent>
        <el-collapse v-model="activeNames">
          <el-collapse-item title="设备" name="device">
            <!-- 根据传感器的名称来展示对应的模型参数配置面板 -->
            <template v-if="sensorName === 'Camera'">
              <Camera />
            </template>
            <template v-else-if="sensorName === 'Millimeter_Wave_Radar'">
              <Radar />
            </template>
            <template v-else-if="sensorName === 'Lidar'">
              <Lidar />
            </template>
            <template v-else-if="sensorName === 'RSU'">
              <Rsu />
            </template>
            <template v-else />
          </el-collapse-item>
        </el-collapse>
      </el-form>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
