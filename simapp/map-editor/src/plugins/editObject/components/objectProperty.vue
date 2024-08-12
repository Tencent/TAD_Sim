<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useObjectInteractionStore } from '../store/interaction'
import PoleProperty from './property/pole.vue'
import RoadSignProperty from './property/roadSign.vue'
import ParkingSpaceProperty from './property/parkingSpace.vue'
import SignalBoardProperty from './property/signalBoard.vue'
import TrafficLightProperty from './property/trafficLight.vue'
import OtherProperty from './property/other.vue'
import SensorProperty from './property/sensor.vue'
import CustomModelProperty from './property/customModel.vue'
import { getObject } from '@/utils/mapCache'

const interactionStore = useObjectInteractionStore()

const currentRoadId = ref('')
const currentJunctionId = ref('')
const currentPoleId = ref('')
const mainType = ref('')

const currentObject = computed(() => {
  return interactionStore.currentObject
})

// 监听当前选中的物体 id 和类型
watch(
  currentObject,
  (val) => {
    if (!val) {
      currentRoadId.value = ''
      currentJunctionId.value = ''
      currentPoleId.value = ''
      return
    }

    let { roadId = '', junctionId = '' } = val
    // @ts-expect-error
    const { poleId = '' } = val
    // 基于当前选中的物体，找到对应关联的父级元素
    if (poleId) {
      // 如果存在 poleId, 说明该物体是杆容器中的元素，从 pole 对象往上找关联的道路或者路口
      const pole = getObject(poleId)
      if (pole) {
        roadId = pole.roadId || ''
        junctionId = pole.junctionId || ''
      }
    }
    currentPoleId.value = poleId
    currentRoadId.value = roadId
    currentJunctionId.value = junctionId
    mainType.value = val.mainType
  },
  {
    immediate: true,
    deep: true,
  },
)
</script>

<template>
  <!-- 当前物体关联的道路、路口、杆父容器，都作为共同的属性展示 -->
  <template v-if="currentRoadId">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentRoadProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadId') }}</label>
        <div class="content">
          {{ currentRoadId }}
        </div>
      </div>
    </div>
  </template>
  <template v-if="currentJunctionId">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentJunctionProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.junctionId') }}</label>
        <div class="content">
          {{ currentJunctionId }}
        </div>
      </div>
    </div>
  </template>
  <template v-if="currentPoleId">
    <div class="property-title">
      {{ $t('desc.editObject.currentPoleProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.poleId') }}</label>
        <div class="content">
          {{ currentPoleId }}
        </div>
      </div>
    </div>
  </template>
  <template v-if="mainType === 'pole'">
    <PoleProperty />
  </template>
  <template v-else-if="mainType === 'roadSign'">
    <RoadSignProperty />
  </template>
  <template v-else-if="mainType === 'parkingSpace'">
    <ParkingSpaceProperty />
  </template>
  <template v-else-if="mainType === 'trafficLight'">
    <TrafficLightProperty />
  </template>
  <template v-else-if="mainType === 'signalBoard'">
    <SignalBoardProperty />
  </template>
  <template v-else-if="mainType === 'sensor'">
    <SensorProperty />
  </template>
  <template v-else-if="mainType === 'other'">
    <OtherProperty />
  </template>
  <template v-else-if="mainType === 'customModel'">
    <CustomModelProperty />
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
