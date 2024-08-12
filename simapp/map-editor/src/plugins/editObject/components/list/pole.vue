<script setup lang="ts">
import { poleConfig } from '@/config/pole'

const poleList = poleConfig
</script>

<template>
  <div class="object-list">
    <ul class="object-ul">
      <li
        v-for="poleOption of poleList"
        :key="poleOption.name"
        class="object-li"
      >
        <div v-my-drag="poleOption" class="icon-box" draggable="false">
          <img :src="poleOption.iconUrl" class="icon" draggable="false">
        </div>
        <span class="title">{{ poleOption.showName }}</span>
      </li>
    </ul>
  </div>
</template>

<style scoped lang="less"></style>
