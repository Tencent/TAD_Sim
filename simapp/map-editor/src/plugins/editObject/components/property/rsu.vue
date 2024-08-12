<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import { cloneDeep, get, isEqual } from 'lodash'
import { useObjectInteractionStore } from '../../store/interaction'
import InputNumber from '@/components/common/inputNumber.vue'
import { getRsuParams } from '@/config/sensor'
import { globalConfig } from '@/utils/preset'
import { useObjectStore } from '@/stores/object'
// @ts-expect-error
import ChoiceSvg from '@/assets/icons/choice.svg?component'
// @ts-expect-error
import SettingSvg from '@/assets/icons/setting.svg?component'
import { useJunctionStore } from '@/stores/junction'
import { useFileStore } from '@/stores/file'

const objectStore = useObjectStore()
const fileStore = useFileStore()
const interactionStore = useObjectInteractionStore()
const junctionStore = useJunctionStore()
const dialog = get(window, 'electron.dialog')
// 如果是云端环境，则传感器设备可选择已有传感器配置
// 如果是单机环境，由于没有已配置好的传感器属性，需要展示对应的具体属性面板
const { isCloud } = globalConfig
const rsuConfig = ref(getRsuParams())
// 当前地图中可关联的路口
const junctionIds: Ref<Array<string>> = ref([])

// 当前选中的通信单元
const currentRSU = computed(() => {
  const { currentSensor } = interactionStore
  if (currentSensor && currentSensor.name === 'RSU') {
    return currentSensor
  }

  return null
})

watch(
  currentRSU,
  (val) => {
    if (!val) return
    // 由于是引用类型，最好是深拷贝一份再做修改
    junctionIds.value = cloneDeep(junctionStore.ids)
    // 校验当前通信单元关联的路口是否有效
    const validJunctionIds: Array<string> = []
    val.deviceParams.JunctionIDs.forEach((id: string) => {
      if (junctionIds.value.includes(id)) {
        validJunctionIds.push(id)
      }
    })

    // 更新当前展示的配置
    rsuConfig.value = cloneDeep({
      ...val.deviceParams,
      JunctionIDs: validJunctionIds,
    })
  },
  {
    immediate: true,
    deep: true,
  },
)

// 监听通信单元传感器参数的更新
watch(
  rsuConfig,
  (val) => {
    if (!currentRSU.value) return
    // 对比当前修改的传感器属性，跟物体自身的属性是否完全相同
    const compareRes = isEqual(val, currentRSU.value.deviceParams)
    // 对比 rsu 关联的路口是否存在更新，如果存在更新，则需要触发对应的路口元素高亮效果的渲染
    const compareJunctionIdRes = isEqual(
      val.JunctionIDs,
      currentRSU.value.deviceParams.JunctionIDs,
    )
    if (compareRes) return

    // 更新传感器设备属性
    objectStore.updateSensorDeviceParams({
      objectId: currentRSU.value.id,
      deviceParams: val,
      // 比较结果为 false，才需要更新路口的渲染效果
      isUpdateRsuJunctionIds: !compareJunctionIdRes,
    })
  },
  {
    deep: true,
    immediate: true,
  },
)

// 恢复默认的传感器设备参数
function resetDefault () {
  if (!currentRSU.value) return
  const defaultParams = getRsuParams()
  // 对比当前值跟默认值是否完全相同
  const compareRes = isEqual(defaultParams, currentRSU.value.deviceParams)
  if (compareRes) return

  // 用默认值更新设备属性
  objectStore.updateSensorDeviceParams({
    objectId: currentRSU.value.id,
    deviceParams: defaultParams,
  })
}

// 切换三维场景中选中关联路口的可用性
function toggleSelectJunctionEnabled () {
  interactionStore.setSelectJunctionEnabled(!interactionStore.canSelectJunction)
}

// 跳转云端v2x模型管理页面
function openV2XManagePage () {
  fileStore.openV2XManagerPage()
}

async function chooseJsonFilePath () {
  if (!dialog) return
  try {
    const { filePaths } = await dialog.showOpenDialog({
      title: '请选择json文件',
      properties: ['openFile'],
      filters: [
        {
          name: 'json 文件',
          extensions: ['json'],
        },
      ],
    })

    if (filePaths.length < 0) return

    const jsonFilePath = filePaths[0]
    rsuConfig.value = {
      ...rsuConfig.value,
      PreMSG: jsonFilePath,
    }
  } catch (err) {
    console.log(`select json file error.`, err)
  }
}
</script>

<template>
  <div class="property-container">
    <template v-if="isCloud">
      <!-- 云端版，选择已经配置好的传感器配置 -->
      <div class="property-cell config">
        <label class="label">通信单元设备</label>
        <div class="content">
          <el-select
            v-model="rsuConfig.V2X_idx"
            :style="{ width: '110px' }"
            placement="bottom"
            clearable
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="idx of fileStore.v2xDeviceList.rsu"
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
      <div class="property-cell multiple-select scene-pick-cell">
        <label class="label">关联路口</label>
        <div class="content">
          <el-select
            v-model="rsuConfig.JunctionIDs"
            :style="{ width: '110px' }"
            placement="bottom"
            multiple
            class="multiple"
            clearable
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="id of junctionIds"
              :key="id"
              :label="`路口 ${id}`"
              :value="id"
            />
          </el-select>
          <span
            class="choice"
            :class="{ enabled: interactionStore.canSelectJunction }"
            @click="toggleSelectJunctionEnabled"
          >
            <span class="mask" />
            <ChoiceSvg />
          </span>
        </div>
      </div>
    </template>
    <template v-else>
      <div class="property-cell multiple-select scene-pick-cell">
        <label class="label">关联路口</label>
        <div class="content">
          <el-select
            v-model="rsuConfig.JunctionIDs"
            :style="{ width: '110px' }"
            placement="bottom"
            multiple
            class="multiple"
            clearable
            :placeholder="$t('actions.tips.select')"
          >
            <el-option
              v-for="id of junctionIds"
              :key="id"
              :label="`路口 ${id}`"
              :value="id"
            />
          </el-select>
          <span
            class="choice"
            :class="{ enabled: interactionStore.canSelectJunction }"
            @click="toggleSelectJunctionEnabled"
          >
            <span class="mask" />
            <ChoiceSvg />
          </span>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">SPAT发送频率</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.FrequencySPAT"
            :disabled="false"
            :precision="0"
            :max="100"
            :min="0"
            unit="Hz"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">RSM发送频率</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.FrequencyRSM"
            :disabled="false"
            :precision="0"
            :max="100"
            :min="0"
            unit="Hz"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">RSI发送频率</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.FrequencyRSI"
            :disabled="false"
            :precision="0"
            :max="100"
            :min="0"
            unit="Hz"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">MAP发送频率</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.FrequencyMAP"
            :disabled="false"
            :precision="0"
            :max="100"
            :min="0"
            unit="Hz"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">通信距离</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.DistanceCommu"
            :disabled="false"
            :precision="0"
            :max="3000"
            :min="10"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">频段</label>
        <div class="content">
          <el-select
            v-model="rsuConfig.Band"
            :style="{ width: '110px' }"
            placement="bottom"
          >
            <el-option label="5905-5915" value="5905-5915" />
            <el-option label="5915-5925" value="5915-5925" />
            <el-option label="5905-5925" value="5905-5925" />
          </el-select>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">传输速率</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.Mbps"
            :disabled="false"
            :precision="1"
            :max="3000"
            :min="0.1"
            unit="Mbps"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">最大延时</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.MaxDelay"
            :disabled="false"
            :precision="0"
            :max="1000"
            :min="0"
            unit="ms"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">地图半径</label>
        <div class="content">
          <InputNumber
            v-model="rsuConfig.MapRadius"
            :disabled="false"
            :precision="0"
            :max="2000"
            :min="10"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <label class="label">关键事件立即触发</label>
        <div class="content">
          <el-radio-group v-model="rsuConfig.TriggerImmediately">
            <el-radio label="True" size="small">
              开启
            </el-radio>
            <el-radio label="False" size="small">
              禁用
            </el-radio>
          </el-radio-group>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">拥堵调节</label>
        <div class="content">
          <el-radio-group v-model="rsuConfig.CongestionRegulation">
            <el-radio label="True" size="small">
              开启
            </el-radio>
            <el-radio label="False" size="small">
              禁用
            </el-radio>
          </el-radio-group>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">云协同(V2N)</label>
        <div class="content">
          <el-radio-group v-model="rsuConfig.V2N">
            <el-radio label="True" size="small">
              开启
            </el-radio>
            <el-radio label="False" size="small">
              禁用
            </el-radio>
          </el-radio-group>
        </div>
      </div>
      <div class="property-cell">
        <label class="label">车路协同(V2I)</label>
        <div class="content">
          <el-radio-group v-model="rsuConfig.V2I">
            <el-radio label="True" size="small">
              开启
            </el-radio>
            <el-radio label="False" size="small">
              禁用
            </el-radio>
          </el-radio-group>
        </div>
      </div>
      <div class="property-cell config">
        <label class="label">消息预设</label>
        <div class="content">
          <el-input
            v-model="rsuConfig.PreMSG"
            class="path-input disabled"
            disabled
          />
          <span class="config-icon" @click="chooseJsonFilePath">
            <SettingSvg />
          </span>
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

.path-input {
  display: inline-block;
  box-sizing: border-box;
  vertical-align: top;
  width: 100px;

  &.disabled {
    :deep(.el-input__wrapper) {
      background-color: var(--input-disabled-background);
      border-color: var(--input-disabled-border-color);
    }
  }

  :deep(.el-input__wrapper) {
    border-radius: 0;
    background-color: var(--input-background);
    border: var(--input-border);
    box-shadow: none;
    box-sizing: border-box;
    padding: 0 6px;
    height: 30px;

    .el-input__inner {
      color: var(--text-color);
    }
  }
}
</style>
