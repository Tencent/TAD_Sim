<script setup lang="ts">
import { trafficLightConfig } from '@/config/trafficLight'
</script>

<template>
  <div class="object-list">
    <ul class="object-ul">
      <li
        v-for="lightOption of trafficLightConfig"
        :key="lightOption.name"
        class="object-li"
      >
        <div v-my-drag="lightOption" class="icon-box" draggable="false">
          <img :src="lightOption.iconUrl" class="icon" draggable="false">
        </div>
        <span class="title">{{ lightOption.showName }}</span>
      </li>
    </ul>
  </div>
</template>

<style scoped lang="less"></style>
