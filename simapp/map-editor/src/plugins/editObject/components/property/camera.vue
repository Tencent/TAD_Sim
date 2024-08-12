<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import { cloneDeep, isEqual } from 'lodash'
import { useObjectInteractionStore } from '../../store/interaction'
import InputNumber from '@/components/common/inputNumber.vue'
import { getCameraParams } from '@/config/sensor'
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
const cameraConfig = ref(getCameraParams())
// 通信单元的列表
const rsuList: Ref<Array<string>> = ref([])

// 当前选中的摄像头
const currentCamera = computed(() => {
  const { currentSensor } = interactionStore
  if (currentSensor && currentSensor.name === 'Camera') {
    return currentSensor
  }

  return null
})

watch(
  currentCamera,
  (val) => {
    if (!val) return

    // 由于是引用类型，最好是深拷贝一份再做修改
    rsuList.value = getAllRsuId()
    cameraConfig.value = cloneDeep(val.deviceParams)
  },
  {
    immediate: true,
    deep: true,
  },
)

// 监听摄像头传感器参数的更新
watch(
  cameraConfig,
  (val) => {
    if (!currentCamera.value) return
    // 对比当前修改的传感器属性，跟物体自身的属性是否完全相同
    const compareRes = isEqual(val, currentCamera.value.deviceParams)
    if (compareRes) return

    // 更新传感器设备属性
    objectStore.updateSensorDeviceParams({
      objectId: currentCamera.value.id,
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
  if (!currentCamera.value) return
  const defaultParams = getCameraParams()
  // 对比当前值跟默认值是否完全相同
  const compareRes = isEqual(defaultParams, currentCamera.value.deviceParams)
  if (compareRes) return

  // 用默认值更新设备属性
  objectStore.updateSensorDeviceParams({
    objectId: currentCamera.value.id,
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
        <label class="label">摄像头设备</label>
        <div class="content">
          <el-select
            v-model="cameraConfig.V2X_idx"
            :style="{ width: '100px' }"
            clearable
            placement="bottom"
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="idx of fileStore.v2xDeviceList.camera"
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
            v-model="cameraConfig.BelongRSU"
            :style="{ width: '100px' }"
            clearable
            placement="bottom"
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
            v-model="cameraConfig.BelongRSU"
            :style="{ width: '100px' }"
            clearable
            placement="bottom"
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
        <label class="label">频率</label>
        <div class="content">
          <InputNumber
            v-model="cameraConfig.Frequency"
            :disabled="false"
            :precision="0"
            :max="1000"
            :min="0"
            unit="Hz"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">水平分辨率</label>
        <div class="content">
          <InputNumber
            v-model="cameraConfig.ResHorizontal"
            :disabled="false"
            :precision="0"
            :max="10000"
            :min="10"
            unit=""
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">垂直分辨率</label>
        <div class="content">
          <InputNumber
            v-model="cameraConfig.ResVertical"
            :disabled="false"
            :precision="0"
            :max="10000"
            :min="10"
            unit=""
          />
        </div>
      </div>
      <div class="property-cell vertical">
        <label class="label">畸变参数</label>
        <div class="content">
          <div class="matrix">
            <div class="item">
              <InputNumber
                v-model="cameraConfig.Distortion.k1"
                :disabled="false"
                :precision="3"
                :max="10"
                :min="0"
                unit=""
              />
              <span>k1</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.Distortion.k2"
                :disabled="false"
                :precision="3"
                :max="10"
                :min="0"
                unit=""
              />
              <span>k2</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.Distortion.k3"
                :disabled="false"
                :precision="3"
                :max="10"
                :min="0"
                unit=""
              />
              <span>k3</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.Distortion.p1"
                :disabled="false"
                :precision="3"
                :max="10"
                :min="0"
                unit=""
              />
              <span>p1</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.Distortion.p2"
                :disabled="false"
                :precision="3"
                :max="10"
                :min="0"
                unit=""
              />
              <span>p2</span>
            </div>
          </div>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">内参形式</label>
        <div class="content">
          <el-select
            v-model="cameraConfig.IntrinsicType"
            :style="{ width: '100px' }"
            placement="bottom"
          >
            <el-option label="矩阵" value="0" />
            <el-option label="FOV" value="1" />
            <el-option label="感光器" value="2" />
          </el-select>
        </div>
      </div>
      <div
        v-if="cameraConfig.IntrinsicType === '0'"
        class="property-cell vertical"
      >
        <label class="label">内参矩阵</label>
        <div class="content">
          <div class="matrix">
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.fx"
                :disabled="false"
                :precision="3"
                :max="2000"
                :min="0"
                unit=""
              />
              <span>fx</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.skew"
                :disabled="false"
                :precision="3"
                :max="2000"
                :min="0"
                unit=""
              />
              <span>skew</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.cx"
                :disabled="false"
                :precision="3"
                :max="2000"
                :min="0"
                unit=""
              />
              <span>cx</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.param4"
                :disabled="true"
                :precision="0"
                :max="10"
                :min="0"
                unit=""
              />
              <span />
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.fy"
                :disabled="false"
                :precision="3"
                :max="2000"
                :min="0"
                unit=""
              />
              <span>fy</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.cy"
                :disabled="false"
                :precision="3"
                :max="2000"
                :min="0"
                unit=""
              />
              <span>cy</span>
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.param7"
                :disabled="true"
                :precision="0"
                :max="10"
                :min="0"
                unit=""
              />
              <span />
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.param8"
                :disabled="true"
                :precision="0"
                :max="10"
                :min="0"
                unit=""
              />
              <span />
            </div>
            <div class="item">
              <InputNumber
                v-model="cameraConfig.IntrinsicMat.param9"
                :disabled="true"
                :precision="0"
                :max="10"
                :min="0"
                unit=""
              />
              <span />
            </div>
          </div>
        </div>
      </div>
      <template v-if="cameraConfig.IntrinsicType === '1'">
        <div class="property-cell">
          <label class="label">水平FOV</label>
          <div class="content">
            <InputNumber
              v-model="cameraConfig.FovHorizontal"
              :disabled="false"
              :precision="1"
              :max="170"
              :min="0"
              unit="°"
            />
          </div>
        </div>
        <div class="property-cell">
          <label class="label">垂直FOV</label>
          <div class="content">
            <InputNumber
              v-model="cameraConfig.FovVertical"
              :disabled="false"
              :precision="1"
              :max="170"
              :min="0"
              unit="°"
            />
          </div>
        </div>
      </template>
      <template v-if="cameraConfig.IntrinsicType === '2'">
        <div class="property-cell">
          <label class="label">感光器宽度</label>
          <div class="content">
            <InputNumber
              v-model="cameraConfig.CcdWidth"
              :disabled="false"
              :precision="1"
              :max="10000"
              :min="0.1"
              unit="mm"
            />
          </div>
        </div>
        <div class="property-cell">
          <label class="label">感光器高度</label>
          <div class="content">
            <InputNumber
              v-model="cameraConfig.CcdHeight"
              :disabled="false"
              :precision="1"
              :max="10000"
              :min="0.1"
              unit="mm"
            />
          </div>
        </div>
        <div class="property-cell">
          <label class="label">镜头焦距</label>
          <div class="content">
            <InputNumber
              v-model="cameraConfig.CcdFocal"
              :disabled="false"
              :precision="1"
              :max="10000"
              :min="0.1"
              unit="mm"
            />
          </div>
        </div>
      </template>
      <div class="property-cell">
        <label class="label">感知距离</label>
        <div class="content">
          <InputNumber
            v-model="cameraConfig.MaxDistance"
            :disabled="false"
            :precision="0"
            :max="500"
            :min="10"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">遮挡最小比例</label>
        <div class="content">
          <InputNumber
            v-model="cameraConfig.Completeness"
            :disabled="false"
            :precision="0"
            :max="90"
            :min="10"
            unit="%"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">最小像素面积</label>
        <div class="content">
          <InputNumber
            v-model="cameraConfig.MinArea"
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
