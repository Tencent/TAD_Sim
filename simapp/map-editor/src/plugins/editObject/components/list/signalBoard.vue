<script setup lang="ts">
// @ts-expect-error
import { ref } from 'vue'
import ArrowSvg from '@/assets/icons/arrow.svg?component'
import { signalBoardConfig } from '@/config/signalBoard'

type SignalBoardType =
  | 'indicatorSignalBoard'
  | 'warningSignalBoard'
  | 'prohibitionSignalBoard'

const itemExpands = ref({
  indicatorSignalBoard: false,
  warningSignalBoard: false,
  prohibitionSignalBoard: false,
})
function toggleExpand (type: SignalBoardType) {
  itemExpands.value[type] = !itemExpands.value[type]
}
</script>

<template>
  <div class="object-list">
    <div v-for="config of signalBoardConfig" :key="config.type">
      <div
        class="item-tab"
        @click="toggleExpand(config.type as SignalBoardType)"
      >
        <label class="tab-name">{{ config.name }}</label>
        <ArrowSvg
          class="arrow-icon"
          :class="{ folded: !itemExpands[config.type as SignalBoardType] }"
        />
      </div>
      <ul
        class="object-ul"
        :class="{ folded: !itemExpands[config.type as SignalBoardType] }"
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
