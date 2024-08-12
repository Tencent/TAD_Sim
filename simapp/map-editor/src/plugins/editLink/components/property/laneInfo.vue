<script setup lang="ts">
const props = defineProps({
  title: {
    type: String,
    default: '',
  },
  roadId: {
    type: String,
    default: '',
  },
  sectionId: {
    type: String,
    default: '',
  },
  laneId: {
    type: String,
    default: '',
  },
})
</script>

<template>
  <div class="property-title">
    {{ props.title }}
  </div>
  <div class="property-container">
    <div class="property-cell">
      <label class="label">{{ $t('desc.editLink.belongingRoadId') }}</label>
      <div class="content">
        {{ props.roadId }}
      </div>
    </div>
    <div class="property-cell">
      <label class="label">{{ $t('desc.editLink.belongingSectionId') }}</label>
      <div class="content">
        {{ props.sectionId }}
      </div>
    </div>
    <div class="property-cell">
      <label class="label">{{ $t('desc.commonRoad.laneId') }}</label>
      <div class="content">
        {{ props.laneId }}
      </div>
    </div>
  </div>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
