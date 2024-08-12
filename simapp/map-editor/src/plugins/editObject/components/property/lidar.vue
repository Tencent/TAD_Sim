<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import { cloneDeep, isEqual } from 'lodash'
import { useObjectInteractionStore } from '../../store/interaction'
import InputNumber from '@/components/common/inputNumber.vue'
import { getLidarParams } from '@/config/sensor'
import { globalConfig } from '@/utils/preset'
import { useObjectStore } from '@/stores/object'
import { getAllRsuId } from '@/stores/object/common'
// @ts-expect-error
import ChoiceSvg from '@/assets/icons/choice.svg?component'
// @ts-expect-error
import SettingSvg from '@/assets/icons/setting.svg?component'
import { useFileStore } from '@/stores/file'

const objectStore = useObjectStore()
const fileStore = useFileStore()
const interactionStore = useObjectInteractionStore()
// 如果是云端环境，则传感器设备可选择已有传感器配置
// 如果是单机环境，由于没有已配置好的传感器属性，需要展示对应的具体属性面板
const { isCloud } = globalConfig
// 通信单元的列表
const rsuList: Ref<Array<string>> = ref([])
const lidarConfig = ref(getLidarParams())

// 当前选中的激光雷达
const currentLidar = computed(() => {
  const { currentSensor } = interactionStore
  if (currentSensor && currentSensor.name === 'Lidar') {
    return currentSensor
  }

  return null
})
watch(
  currentLidar,
  (val) => {
    if (!val) return

    // 由于是引用类型，最好是深拷贝一份再做修改
    rsuList.value = getAllRsuId()
    lidarConfig.value = cloneDeep(val.deviceParams)
  },
  {
    immediate: true,
    deep: true,
  },
)

// 监听激光雷达传感器参数的更新
watch(
  lidarConfig,
  (val) => {
    if (!currentLidar.value) return
    // 对比当前修改的传感器属性，跟物体自身的属性是否完全相同
    const compareRes = isEqual(val, currentLidar.value.deviceParams)
    if (compareRes) return

    // 更新传感器设备属性
    objectStore.updateSensorDeviceParams({
      objectId: currentLidar.value.id,
      deviceParams: val,
    })
  },
  {
    deep: true,
    immediate: true,
  },
)

// 恢复默认的传感器设备参数
function resetDefault () {
  if (!currentLidar.value) return
  const defaultParams = getLidarParams()
  // 对比当前值跟默认值是否完全相同
  const compareRes = isEqual(defaultParams, currentLidar.value.deviceParams)
  if (compareRes) return

  // 用默认值更新设备属性
  objectStore.updateSensorDeviceParams({
    objectId: currentLidar.value.id,
    deviceParams: defaultParams,
  })
}

// 切换三维场景中选中传感器关联的通信单元可用性
function toggleSelectRsuEnabled () {
  interactionStore.setSelectRsuEnabled(!interactionStore.canSelectRsu)
}

// 跳转云端v2x模型管理页面
function openV2XManagePage () {
  fileStore.openV2XManagerPage()
}
</script>

<template>
  <div class="property-container">
    <template v-if="isCloud">
      <!-- 云端版，选择已经配置好的传感器配置 -->
      <div class="property-cell config">
        <label class="label">激光雷达设备</label>
        <div class="content">
          <el-select
            v-model="lidarConfig.V2X_idx"
            :style="{ width: '100px' }"
            clearable
            placement="bottom"
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="idx of fileStore.v2xDeviceList.lidar"
              :key="idx"
              :label="idx"
              :value="idx"
            />
          </el-select>
          <span class="config-icon" @click="openV2XManagePage">
            <SettingSvg />
          </span>
        </div>
      </div>
      <div class="property-cell scene-pick-cell">
        <label class="label">关联通信单元</label>
        <div class="content">
          <el-select
            v-model="lidarConfig.BelongRSU"
            :style="{ width: '100px' }"
            placement="bottom"
            clearable
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="rsuId of rsuList"
              :key="rsuId"
              :label="`RSU ${rsuId}`"
              :value="rsuId"
            />
          </el-select>
          <span
            class="choice"
            :class="{ enabled: interactionStore.canSelectRsu }"
            @click="toggleSelectRsuEnabled"
          >
            <span class="mask" />
            <ChoiceSvg />
          </span>
        </div>
      </div>
    </template>
    <template v-else>
      <!-- 单机版，按照之前的逻辑列出所有的传感器属性 -->
      <div class="property-cell scene-pick-cell">
        <label class="label">关联通信单元</label>
        <div class="content">
          <el-select
            v-model="lidarConfig.BelongRSU"
            :style="{ width: '100px' }"
            placement="bottom"
            clearable
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="rsuId of rsuList"
              :key="rsuId"
              :label="`RSU ${rsuId}`"
              :value="rsuId"
            />
          </el-select>
          <span
            class="choice"
            :class="{ enabled: interactionStore.canSelectRsu }"
            @click="toggleSelectRsuEnabled"
          >
            <span class="mask" />
            <ChoiceSvg />
          </span>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">线数</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.RayNum"
            :disabled="false"
            :precision="0"
            :max="320"
            :min="1"
            unit=""
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">半径范围</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.Radius"
            :disabled="false"
            :precision="1"
            :max="1000"
            :min="0.1"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">水平分辨率</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.HorizontalRes"
            :disabled="false"
            :precision="1"
            :max="360"
            :min="0.1"
            unit=""
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">上仰角</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.FovUp"
            :disabled="false"
            :precision="1"
            :max="90"
            :min="0.1"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">下仰角</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.FovDown"
            :disabled="false"
            :precision="1"
            :max="90"
            :min="0.1"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">起点角度</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.FovStart"
            :disabled="false"
            :precision="1"
            :max="360"
            :min="0"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">结束角度</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.FovEnd"
            :disabled="false"
            :precision="1"
            :max="360"
            :min="0"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">遮挡最小比例</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.Completeness"
            :disabled="false"
            :precision="0"
            :max="90"
            :min="10"
            unit="%"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">最小光数</label>
        <div class="content">
          <InputNumber
            v-model="lidarConfig.MinHitNum"
            :disabled="false"
            :precision="0"
            :max="10000"
            :min="1"
            unit=""
          />
        </div>
      </div>
      <div class="property-cell">
        <el-button @click="resetDefault">
          恢复默认
        </el-button>
      </div>
    </template>
  </div>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
