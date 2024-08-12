<script setup lang="ts">
import { sensorConfig } from '@/config/sensor'
</script>

<template>
  <div class="object-list">
    <ul class="object-ul">
      <li
        v-for="sensorOption of sensorConfig"
        :key="sensorOption.name"
        class="object-li"
      >
        <div v-my-drag="sensorOption" class="icon-box" draggable="false">
          <img :src="sensorOption.iconUrl" class="icon" draggable="false">
        </div>
        <span class="title">{{ sensorOption.showName }}</span>
      </li>
    </ul>
  </div>
</template>

<style scoped lang="less"></style>
