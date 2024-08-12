<script setup lang="ts">
import { computed } from 'vue'
import { useJunctionInteractionStore } from '../store/interaction'
import { orderByAsc } from '@/utils/common3d'

const interactionStore = useJunctionInteractionStore()

// 当前选中的路口
const selectedJunction = computed(() => {
  return interactionStore.currentJunction
})

// TODO 当前属性面板中，展示的连通道路 id 并没有实时更新。
// 可能是 selectedJunction 中的 linkRoads 属性没有实时更新，后续可以作为 bug 修复
// 当前路口连接的道路
const linkRoadIds = computed(() => {
  if (!selectedJunction.value) return null
  const ids = selectedJunction.value.linkRoads.map(
    linkRoad => linkRoad.split('_')[0],
  )
  // 连接的道路按照 id 有小到大排序
  ids.sort(orderByAsc)
  // 去重
  return [...new Set(ids)].join(', ')
})
</script>

<template>
  <template v-if="selectedJunction">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentJunctionProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.junctionId') }}</label>
        <div class="content">
          {{ selectedJunction.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editJunction.linkRoadId') }}</label>
        <div class="content">
          {{ linkRoadIds }}
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
