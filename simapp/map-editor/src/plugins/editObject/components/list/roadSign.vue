<script setup lang="ts">
// @ts-expect-error
import { ref } from 'vue'
import ArrowSvg from '@/assets/icons/arrow.svg?component'
import { roadSignConfig } from '@/config/roadSign'

type RoadSignType =
  | 'indicatorRoadSign'
  | 'prohibitionRoadSign'
  | 'otherRoadSign'

const itemExpands = ref({
  indicatorRoadSign: false,
  prohibitionRoadSign: false,
  otherRoadSign: false,
})
function toggleExpand (type: RoadSignType) {
  itemExpands.value[type] = !itemExpands.value[type]
}
</script>

<template>
  <div class="object-list">
    <div v-for="config of roadSignConfig" :key="config.type">
      <div class="item-tab" @click="toggleExpand(config.type as RoadSignType)">
        <label class="tab-name">{{ config.name }}</label>
        <ArrowSvg
          class="arrow-icon"
          :class="{ folded: !itemExpands[config.type as RoadSignType] }"
        />
      </div>
      <ul
        class="object-ul"
        :class="{ folded: !itemExpands[config.type as RoadSignType] }"
      >
        <li
          v-for="rsIOption of config.list"
          :key="rsIOption.name"
          class="object-li"
        >
          <div v-my-drag="rsIOption" class="icon-box" draggable="false">
            <img :src="rsIOption.iconUrl" class="icon" draggable="false">
          </div>
          <span class="title">{{ rsIOption.showName }}</span>
        </li>
      </ul>
    </div>
  </div>
</template>

<style scoped lang="less"></style>
