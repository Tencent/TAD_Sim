<script setup lang="ts">
/* global biz */
import { type Ref, ref, watch } from 'vue'
import { useLaneNumberInteractionStore } from '../store/interaction'
import { useRoadStore } from '@/stores/road'

const roadStore = useRoadStore()
const laneNumberInteraction = useLaneNumberInteractionStore()

const selectedRoad: Ref<biz.IRoad | null> = ref(null)
const selectedSection: Ref<biz.ISection | null> = ref(null)
const selectedLane: Ref<biz.ILane | null> = ref(null)
watch(
  () => laneNumberInteraction,
  (val) => {
    const { roadId, sectionId, laneId } = val
    if (!roadId || !sectionId || !laneId) {
      selectedRoad.value = null
      selectedSection.value = null
      selectedLane.value = null
      return
    }
    const queryLaneRes = roadStore.getLaneById(roadId, sectionId, laneId)
    if (!queryLaneRes) return
    selectedRoad.value = queryLaneRes.road
    selectedSection.value = queryLaneRes.section
    selectedLane.value = queryLaneRes.lane
  },
  {
    immediate: true,
    deep: true,
  },
)

// 点击按钮触发数据层面新增车道逻辑
function addLane () {
  const { roadId, sectionId, laneId } = laneNumberInteraction
  if (!roadId || !sectionId || !laneId) return

  roadStore.addLane({ roadId, sectionId, laneId })
}

// 点击按钮触发数据层面删除当前车道逻辑
function removeLane () {
  const { roadId, sectionId, laneId } = laneNumberInteraction
  if (!roadId || !sectionId || !laneId) return

  roadStore.removeLane({
    roadId,
    sectionId,
    laneId,
  })
}
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
        <el-button @click="addLane">
          {{
            $t('desc.editLaneNumber.addSameTypeLane')
          }}
        </el-button>
      </div>
      <div class="property-cell">
        <el-button @click="removeLane">
          {{
            $t('desc.editLaneNumber.removeCurrentLane')
          }}
        </el-button>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
