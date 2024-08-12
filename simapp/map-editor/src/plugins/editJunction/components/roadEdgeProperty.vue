<script setup lang="ts">
import { computed } from 'vue'
import { useJunctionInteractionStore } from '../store/interaction'

const junctionInteractionStore = useJunctionInteractionStore()
const selectedSingleEndEdge = computed(() => {
  const { roadEndEdges } = junctionInteractionStore
  if (!roadEndEdges || roadEndEdges.length !== 1) return null

  const [endEdge] = roadEndEdges
  return endEdge
})
</script>

<template>
  <template v-if="selectedSingleEndEdge">
    <div class="property-title">
      {{ $t('desc.editJunction.currentRoadHelperEndProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{
          $t('desc.editJunction.belongingRoadId')
        }}</label>
        <div class="content">
          {{ selectedSingleEndEdge.split('_')[0] }}
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
