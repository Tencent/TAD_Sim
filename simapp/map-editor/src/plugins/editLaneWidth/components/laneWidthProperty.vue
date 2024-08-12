<script setup lang="ts">
/* global biz */
import { type Ref, ref, watch } from 'vue'
import { useLaneWidthInteractionStore } from '../store/interaction'
import { useRoadStore } from '@/stores/road'
import InputNumber from '@/components/common/inputNumber.vue'
import { fixedPrecision } from '@/utils/common3d'
import { Constant } from '@/utils/business'

const laneWidth = ref('')
const selectedRoad: Ref<biz.IRoad | null> = ref(null)
const selectedSection: Ref<biz.ISection | null> = ref(null)
const selectedLane: Ref<biz.ILane | null> = ref(null)
const precision = Constant.precision

const roadStore = useRoadStore()
const interactionStore = useLaneWidthInteractionStore()

// 监听当前选中车道，更新属性面板组件中的状态
watch(
  () => interactionStore.selectedLaneInfo,
  (val) => {
    if (!val) {
      selectedRoad.value = null
      selectedSection.value = null
      selectedLane.value = null
      return
    }

    selectedRoad.value = val.road
    selectedSection.value = val.section
    selectedLane.value = val.lane
    laneWidth.value = String(val.lane.normalWidth)
  },
  {
    immediate: true,
    deep: true,
  },
)

// 监听车道宽度的数值变化
watch(laneWidth, (val) => {
  if (!selectedLane.value || !selectedSection.value || !selectedRoad.value) {
    return
  }
  const { id: laneId, normalWidth } = selectedLane.value
  // 选中的车道存在，那么选中的道路和 section 应该也都存在
  const { id: roadId } = selectedRoad.value
  const { id: sectionId } = selectedSection.value
  const _laneWidth = Number(val)
  // 如果数值没变就不更新
  if (_laneWidth === fixedPrecision(normalWidth, precision)) return

  // 从数据状态层面更新车道宽度
  roadStore.updateLaneWidth({
    roadId,
    sectionId,
    laneId,
    width: _laneWidth,
  })
})
</script>

<template>
  <template v-if="selectedRoad && selectedSection">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentRoadProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadId') }}</label>
        <div class="content">
          {{ selectedRoad.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.sectionId') }}</label>
        <div class="content">
          {{ selectedSection.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.laneNumber') }}</label>
        <div class="content">
          {{ selectedSection.lanes.length }}
        </div>
      </div>
    </div>
  </template>
  <template v-if="selectedLane">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentLaneProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.laneId') }}</label>
        <div class="content">
          {{ selectedLane.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editLaneWidth.laneWidth') }}</label>
        <div class="content">
          <InputNumber
            v-model="laneWidth"
            :disabled="false"
            :precision="precision"
            :max="Constant.maxLaneWidth"
            :min="Constant.minLaneWidth"
            unit="m"
          />
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
