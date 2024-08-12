<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import { useSignalControlInteractionStore } from '../store/interaction'
import LightTypePopper from './lightTypePopper.vue'
import { fixedPrecision } from '@/utils/common3d'
import { useObjectStore } from '@/stores/object'
// @ts-expect-error
import ChoiceSvg from '@/assets/icons/choice.svg?component'
import i18n from '@/locales'
import InputNumber from '@/components/common/inputNumber.vue'
import { getOptionalControlRoadAndJunction } from '@/stores/object/trafficLight'

const interactionStore = useSignalControlInteractionStore()
const objectStore = useObjectStore()
const activeNames = ref(['position', 'type', 'config'])

const onVerticalPole = ref(false)
// 交通信号灯在杆上的局部坐标
// 高度和水平距离
const height = ref('')
const distance = ref('')

const currentLight = computed(() => {
  return interactionStore.currentTrafficLight
})

// 选中的控制道路
const controlRoad: Ref<string> = ref('')
// 选中的控制路口
const controlJunction: Ref<string> = ref('')
// 跟选中的路口连通的候选道路集合
const roadIds: Ref<Array<string>> = ref([])
// 候选的路口集合
const junctionIds: Ref<Array<string>> = ref([])

let unWatchRoad: Function | null
let unWatchJunction: Function | null

// 根据当前交通信号灯是否放置在竖杆上，调整变量状态
watch(
  currentLight,
  (val) => {
    if (unWatchRoad && typeof unWatchRoad === 'function') {
      unWatchRoad()
    }
    if (unWatchJunction && typeof unWatchJunction === 'function') {
      unWatchJunction()
    }

    if (!val) return
    if (val.onVerticalPole) {
      // 竖杆上
      onVerticalPole.value = true
    } else {
      // 横杆上
      onVerticalPole.value = false
    }
    // 在杆上的水平和竖直距离，是基于标志牌在杆的局部坐标系下的坐标
    // y 指高度
    height.value = String(fixedPrecision(val.position.y))
    // z 指距离竖杆圆心的距离
    distance.value = String(fixedPrecision(val.position.z))

    // 获取当前信号灯所有可作为备选的道路和路口集合
    const { roadId: _roadIds, junctionId: _junctionIds } =
      getOptionalControlRoadAndJunction(val.id)

    // 同步已控制的道路和路口，需要转换成带有描述前缀的
    if (val.controlRoadId) {
      controlRoad.value = `${i18n.global.t('desc.road')}-${val.controlRoadId}`
    } else {
      controlRoad.value = ''
    }
    if (val.controlJunctionId) {
      controlJunction.value = `${i18n.global.t('desc.junction')}-${
        val.controlJunctionId
      }`
    } else {
      controlJunction.value = ''
    }
    // 监听控制道路的数值变化
    unWatchRoad = watch(controlRoad, (val: string) => {
      if (!currentLight.value) return
      let _roadId = ''
      if (val !== '') {
        // 将带有前缀的字段解析成具体的 id
        _roadId = val.split('-')[1]
      }
      const { controlRoadId, id: lightId } = currentLight.value
      // 判断之前的控制道路，跟新的控制道路是否内容一致
      if (controlRoadId === _roadId) return

      // 如果去重后还存在差异，说明需要更新
      objectStore.updateTrafficLightControlRoad({
        objectId: lightId,
        roadId: _roadId,
      })
    })

    // 监听控制路口的数值变化
    unWatchJunction = watch(controlJunction, (val: string) => {
      if (!currentLight.value) return
      let _junctionId = ''
      if (val !== '') {
        // 将带有前缀的字段解析成具体的 id
        _junctionId = val.split('-')[1]
      }
      const { controlJunctionId, id: lightId } = currentLight.value
      // 如果选中的 id 跟原有的 id 相同，则不更新
      if (controlJunctionId === _junctionId) return

      objectStore.updateTrafficLightControlJunction({
        objectId: lightId,
        junctionId: _junctionId,
      })
    })

    roadIds.value = _roadIds
    junctionIds.value = _junctionIds
  },
  {
    immediate: true,
    deep: true,
  },
)

// 将备选的道路，组装成下拉菜单的选项
const roadIdOptions = computed(() => {
  // 双向道路以后，左右两边属于一条道路了, 左右两边道路 id 会相同, 需要去重
  return [...new Set(roadIds.value)].map(
    id => `${i18n.global.t('desc.road')}-${id}`,
  )
})

// 将备选的路口，组装成下拉菜单的选项
const junctionIdOptions = computed(() => {
  return [...new Set(junctionIds.value)].map(
    id => `${i18n.global.t('desc.junction')}-${id}`,
  )
})

// 更新信号灯的类型
function updateTrafficLightType (typeName: string) {
  if (!currentLight.value) return
  // 如果是同类型的灯，则返回
  if (currentLight.value.name === typeName) return

  objectStore.updateTrafficLightType({
    objectId: currentLight.value.id,
    typeName,
  })
}

// 切换三维场景中选中控制路口的可用性
function toggleSelectJunctionEnabled () {
  interactionStore.setSelectJunctionEnabled(!interactionStore.canSelectJunction)
}
// 切换三维场景中选中控制道路的可用性
function toggleSelectRoadEnabled () {
  interactionStore.setSelectRoadEnabled(!interactionStore.canSelectRoad)
}
</script>

<template>
  <!-- v-if 动态创建的内容有点多导致属性面板渲染慢，使用 v-show 代替 -->
  <div v-show="currentLight">
    <div class="property-title">
      {{ $t('desc.editObject.currentTrafficLightProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.trafficLightId') }}</label>
        <div class="content">
          {{ currentLight ? currentLight.id : '' }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.poleId') }}</label>
        <div class="content">
          {{ currentLight ? currentLight.poleId : '' }}
        </div>
      </div>
      <div v-if="currentLight && currentLight.roadId" class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadId') }}</label>
        <div class="content">
          {{ currentLight.roadId }}
        </div>
      </div>
      <div v-if="currentLight && currentLight.junctionId" class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.junctionId') }}</label>
        <div class="content">
          {{ currentLight.junctionId }}
        </div>
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
            <InputNumber v-model="height" :disabled="true" unit="m" />
          </el-form-item>
          <el-form-item
            :label="$t('desc.editObject.horizontalDistance')"
            class="param-cell"
          >
            <InputNumber v-model="distance" :disabled="true" unit="m" />
          </el-form-item>
        </el-collapse-item>
      </el-collapse>
      <el-collapse v-model="activeNames">
        <el-collapse-item
          :title="$t('desc.editSignalControl.lightType')"
          name="type"
        >
          <div class="property-container">
            <div class="property-cell" :style="{ padding: '16px 0' }">
              <LightTypePopper
                :light-name="currentLight ? currentLight.name : ''"
                @type-change="updateTrafficLightType"
              />
            </div>
          </div>
        </el-collapse-item>
      </el-collapse>
      <el-collapse v-model="activeNames">
        <el-collapse-item
          :title="$t('desc.editSignalControl.signalControl')"
          name="config"
        >
          <div class="property-container signal-control-container">
            <div
              v-if="junctionIdOptions.length > 0"
              class="property-cell scene-pick-cell"
            >
              <label class="label">{{
                $t('desc.editSignalControl.belongingJunction')
              }}</label>
              <div class="content">
                <el-select
                  v-model="controlJunction"
                  placement="bottom"
                  :placeholder="$t('actions.tips.select')"
                  :style="{ width: '100px' }"
                >
                  <el-option
                    v-for="(junctionId, index) in junctionIdOptions"
                    :key="index"
                    :label="junctionId"
                    :value="junctionId"
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
            <div class="property-cell multiple-select scene-pick-cell">
              <label class="label">{{
                $t('desc.editSignalControl.controlRoad')
              }}</label>
              <div class="content">
                <el-select
                  v-model="controlRoad"
                  placement="bottom"
                  :placeholder="$t('actions.tips.select')"
                  :style="{ width: '100px' }"
                >
                  <el-option
                    v-for="(roadId, index) in roadIdOptions"
                    :key="index"
                    :label="roadId"
                    :value="roadId"
                  />
                </el-select>
                <span
                  class="choice"
                  :class="{ enabled: interactionStore.canSelectRoad }"
                  @click="toggleSelectRoadEnabled"
                >
                  <span class="mask" />
                  <ChoiceSvg />
                </span>
              </div>
            </div>
          </div>
        </el-collapse-item>
      </el-collapse>
    </el-form>
  </div>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
.signal-control-container {
  :deep(.el-select) {
    width: 120px;
  }

  .property-cell {
    padding: 16px 0 0;
    .label {
      width: 80px;
    }
  }
}
</style>
