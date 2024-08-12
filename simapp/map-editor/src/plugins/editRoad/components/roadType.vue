<script setup lang="ts">
import { computed } from 'vue'
import { RoadTypeConfig } from '@/config/roadType'
import { useConfig3dStore } from '@/stores/config3d'
import { getPermission } from '@/utils/permission'

const config3dStore = useConfig3dStore()

// 当前绘制的道路类型
const currentRoadType = computed(() => {
  return config3dStore.roadType
})

// 显示中的道路类型列表
const shownList = computed(() => {
  return RoadTypeConfig.filter((item) => {
    // 匝道
    if (item.name === 'ramp') {
      return getPermission('action.mapEditor.road.ramp.enable')
    }
    // 隧道
    if (item.name === 'tunnel') {
      return getPermission('action.mapEditor.road.tunnel.enable')
    }
    // 其他
    return true
  })
})

// 切换当前绘制的道路类型
function changeRoadType (type: string) {
  // 如果类型一致，则无须切换
  if (currentRoadType.value === type) return

  config3dStore.updateRoadType(type)
}
</script>

<template>
  <div class="container">
    <div class="list-title">
      {{ $t('desc.editRoad.roadType') }}
    </div>
    <ul class="type-ul">
      <li
        v-for="roadType of shownList"
        :key="roadType.name"
        class="type-li"
        :class="{ active: currentRoadType === roadType.name }"
        @click="changeRoadType(roadType.name)"
      >
        <div class="icon-box" draggable="false">
          <img :src="roadType.iconUrl" class="icon" draggable="false">
        </div>
        <span class="title">{{ roadType.showName }}</span>
      </li>
    </ul>
  </div>
</template>

<style scoped lang="less">
.list-title {
  height: 24px;
  line-height: 24px;
  padding: 0 12px;
  color: var(--text-color);
}
.type-ul {
  display: flex;
  justify-content: flex-start;
  flex-wrap: wrap;
  overflow: hidden;
  height: auto;

  .type-li {
    display: inline-block;
    width: 80px;
    margin: 6px;

    .icon-box {
      width: 80px;
      height: 80px;
      box-sizing: border-box;
      border: 2px solid var(--object-tab-divider-color);
      display: flex;
      align-items: center;
      justify-content: center;
      .icon {
        max-width: 76px;
        max-height: 76px;
      }
    }
    .title {
      display: block;
      text-align: center;
      font-size: var(--font-size);
      padding-top: 6px;
    }

    &:hover,
    &.active {
      .icon-box {
        border-color: var(--active-color);
      }
      .title {
        color: var(--active-color);
      }
    }
  }
}
</style>
