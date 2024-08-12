<script setup lang="ts">
import { computed } from 'vue'
import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import i18n from '@/locales'

const linkInteractionStore = useLinkInteractionStore()

const selectedPoint = computed(() => {
  const { currentHtTPointFlag } = linkInteractionStore
  return currentHtTPointFlag && currentHtTPointFlag.split('_')
})

const selectedPointRoadId = computed(() => {
  return selectedPoint.value[0]
})

const selectedPointSectionId = computed(() => {
  return selectedPoint.value[1]
})

const selectedPointLaneId = computed(() => {
  return selectedPoint.value[2]
})

const selectedPointDirection = computed(() => {
  return selectedPoint.value[3] === 'start' ?
    i18n.global.t('desc.editLink.enteringLaneDirection') :
    i18n.global.t('desc.editLink.leavingLaneDirection')
})
</script>

<template>
  <template v-if="selectedPoint">
    <div class="property-title">
      {{ $t('desc.editLink.currentHTPointProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadId') }}</label>
        <div class="content">
          {{ selectedPointRoadId }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.sectionId') }}</label>
        <div class="content">
          {{ selectedPointSectionId }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.laneId') }}</label>
        <div class="content">
          {{ selectedPointLaneId }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editLink.orientation') }}</label>
        <div class="content">
          {{ selectedPointDirection }}
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
