<script setup lang="ts">
/* global biz */
import { computed } from 'vue'
import { getJunction } from '@/utils/mapCache'
import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import LaneInfo from '@/plugins/editLink/components/property/laneInfo.vue'

const linkInteractionStore = useLinkInteractionStore()

const selectedJunction = computed(() => {
  const { currentJunctionId } = linkInteractionStore
  const junction = getJunction(currentJunctionId)
  if (!junction) return null
  return junction
})

const selectedJunctionLink = computed(() => {
  if (!selectedJunction.value) return null
  const { currentLaneLinkId } = linkInteractionStore
  if (!currentLaneLinkId) return null
  const link = selectedJunction.value.laneLinks.find(
    (item: biz.ILaneLink) => item.id === currentLaneLinkId,
  )
  if (!link) return null
  return link
})
</script>

<template>
  <template v-if="selectedJunctionLink">
    <div class="property-title">
      {{ $t('desc.editLink.currentJunctionLinkProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editLink.laneLinkId') }}</label>
        <div class="content">
          {{ selectedJunctionLink.id }}
        </div>
      </div>
      <div v-if="selectedJunction" class="property-cell">
        <label class="label">{{
          $t('desc.editLink.belongingJunctionId')
        }}</label>
        <div class="content">
          {{ selectedJunction.id }}
        </div>
      </div>
      <LaneInfo
        :title="$t('desc.editLink.enteringLaneProperty')"
        :road-id="selectedJunctionLink.trid"
        :section-id="selectedJunctionLink.tsid"
        :lane-id="selectedJunctionLink.tid"
      />
      <LaneInfo
        :title="$t('desc.editLink.leavingLaneProperty')"
        :road-id="selectedJunctionLink.frid"
        :section-id="selectedJunctionLink.fsid"
        :lane-id="selectedJunctionLink.fid"
      />
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
