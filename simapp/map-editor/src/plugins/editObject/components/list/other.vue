<script setup lang="ts">
// @ts-expect-error
import { ref } from 'vue'
import ArrowSvg from '@/assets/icons/arrow.svg?component'
import { otherConfig } from '@/config/other'

type OtherType =
  | 'roadDamageOrDefect'
  | 'trafficManagement'
  | 'vegetation'
  | 'structure'

const itemExpands = ref({
  roadDamageOrDefect: false,
  trafficManagement: false,
  vegetation: false,
  structure: false,
})
function toggleExpand (type: OtherType) {
  itemExpands.value[type] = !itemExpands.value[type]
}
</script>

<template>
  <div class="object-list">
    <div v-for="config of otherConfig" :key="config.type">
      <div class="item-tab" @click="toggleExpand(config.type as OtherType)">
        <label class="tab-name">{{ config.name }}</label>
        <ArrowSvg
          class="arrow-icon"
          :class="{ folded: !itemExpands[config.type as OtherType] }"
        />
      </div>
      <ul
        class="object-ul"
        :class="{ folded: !itemExpands[config.type as OtherType] }"
      >
        <li v-for="option of config.list" :key="option.name" class="object-li">
          <div v-my-drag="option" class="icon-box" draggable="false">
            <img :src="option.iconUrl" class="icon" draggable="false">
          </div>
          <span class="title">{{ option.showName }}</span>
        </li>
      </ul>
    </div>
  </div>
</template>

<style scoped lang="less"></style>
