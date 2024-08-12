<script setup lang="ts">
import { computed } from 'vue'
import { useElevationInteractionStore } from '../store/interaction'
import { useRoadStore } from '@/stores/road'

const roadStore = useRoadStore()
const roadInteractionStore = useElevationInteractionStore()

const selectedRoad = computed(() => {
  const { roadId } = roadInteractionStore
  if (roadId) {
    const road = roadStore.getRoadById(roadInteractionStore.roadId)
    if (!road) return null
    return road
  }

  return null
})
// 当前道路连通的路口 id
const linkJunctionIds = computed(() => {
  if (!selectedRoad.value) return ''
  const ids = selectedRoad.value.linkJunction.join(', ')
  return ids
})
</script>

<template>
  <template v-if="selectedRoad">
    <!-- 偏展示信息类，没有表单输入的，使用自定义的属性列表结构 -->
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
        <label class="label">{{ $t('desc.commonRoad.roadLength') }}</label>
        <div class="content">
          {{ selectedRoad.length }} m
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editRoad.linkJunctionId') }}</label>
        <div class="content">
          {{ linkJunctionIds }}
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
