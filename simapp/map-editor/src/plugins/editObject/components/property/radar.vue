<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import { cloneDeep, isEqual } from 'lodash'
import { useObjectInteractionStore } from '../../store/interaction'
import { getRadarParams } from '@/config/sensor'
import InputNumber from '@/components/common/inputNumber.vue'
import { globalConfig } from '@/utils/preset'
import { getAllRsuId } from '@/stores/object/common'
import { useObjectStore } from '@/stores/object'
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
const radarConfig = ref(getRadarParams())
// 通信单元的列表
const rsuList: Ref<Array<string>> = ref([])

const F0_GHzIs77 = computed(() => {
  return radarConfig.value.F0_GHz === '77'
})

// 当前选中的毫米波雷达
const currentRadar = computed(() => {
  const { currentSensor } = interactionStore
  if (currentSensor && currentSensor.name === 'Millimeter_Wave_Radar') {
    return currentSensor
  }

  return null
})

watch(
  currentRadar,
  (val) => {
    if (!val) return

    // 由于是引用类型，最好是深拷贝一份再做修改
    rsuList.value = getAllRsuId()
    radarConfig.value = cloneDeep(val.deviceParams)
  },
  {
    immediate: true,
    deep: true,
  },
)

// 监听毫米波雷达传感器参数的更新
watch(
  radarConfig,
  (val) => {
    if (!currentRadar.value) return
    // 对比当前修改的传感器属性，跟物体自身的属性是否完全相同
    const compareRes = isEqual(val, currentRadar.value.deviceParams)
    if (compareRes) return

    // 更新传感器设备属性
    objectStore.updateSensorDeviceParams({
      objectId: currentRadar.value.id,
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
  if (!currentRadar.value) return
  const defaultParams = getRadarParams()
  // 对比当前值跟默认值是否完全相同
  const compareRes = isEqual(defaultParams, currentRadar.value.deviceParams)
  if (compareRes) return

  // 用默认值更新设备属性
  objectStore.updateSensorDeviceParams({
    objectId: currentRadar.value.id,
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
        <label class="label">毫米波雷达设备</label>
        <div class="content">
          <el-select
            v-model="radarConfig.V2X_idx"
            :style="{ width: '100px' }"
            placement="bottom"
            clearable
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="idx of fileStore.v2xDeviceList.radar"
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
            v-model="radarConfig.BelongRSU"
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
      <div class="property-cell scene-pick-cell">
        <label class="label">关联通信单元</label>
        <div class="content">
          <el-select
            v-model="radarConfig.BelongRSU"
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
        <label class="label">雷达载频</label>
        <div class="content">
          <el-select
            v-model="radarConfig.F0_GHz"
            :style="{ width: '100px' }"
            placement="bottom"
          >
            <el-option label="77" value="77" />
            <el-option label="24" value="24" />
          </el-select>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">发射功率</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.Pt_dBm"
            :disabled="false"
            :precision="1"
            :max="10"
            :min="5"
            unit="dBm"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">发射天线增益</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.Gt_dBi"
            :disabled="false"
            :precision="1"
            :max="15"
            :min="10"
            unit="dBi"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">接收天线增益</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.Gr_dBi"
            :disabled="false"
            :precision="1"
            :max="15"
            :min="10"
            unit="dBi"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">等效噪声温度</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.Ts_K"
            :disabled="false"
            :precision="1"
            :max="310"
            :min="290"
            unit="K"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">接收机噪声系数</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.Fn_dB"
            :disabled="false"
            :precision="1"
            :max="45"
            :min="0"
            unit="dB"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">系统衰减常数</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.L0_dB"
            :disabled="false"
            :precision="1"
            :max="F0_GHzIs77 ? 10 : 30"
            :min="F0_GHzIs77 ? 5 : 15"
            unit="dB"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">最小可检测信噪比</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.SNR_min_dB"
            :disabled="false"
            :precision="1"
            :max="22"
            :min="15"
            unit="dB"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">目标延迟参数</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.Delay"
            :disabled="false"
            :precision="1"
            :max="5000"
            :min="0"
            unit="ms"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">垂直FOV</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.FovVertical"
            :disabled="false"
            :precision="1"
            :max="160"
            :min="0"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">水平FOV</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.FovHorizontal"
            :disabled="false"
            :precision="1"
            :max="160"
            :min="0"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">水平角度分辨率</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.ResHorizontal"
            :disabled="false"
            :precision="1"
            :max="10"
            :min="1"
            unit="°"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">垂直角度分辨率</label>
        <div class="content">
          <InputNumber
            v-model="radarConfig.ResVertical"
            :disabled="false"
            :precision="1"
            :max="10"
            :min="1"
            unit="°"
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
