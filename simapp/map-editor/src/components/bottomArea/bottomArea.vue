<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref, watch } from 'vue'
import Record from './record.vue'
import Elevation from './elevation.vue'
import { useLayoutStore } from '@/stores/layout'
import { usePluginStore } from '@/stores/plugin'

const currentTab = ref('output')
const bottomAreaRef = ref()
const layoutStore = useLayoutStore()
const pluginStore = usePluginStore()
const areaWidth = ref()
const areaHeight = ref()
let resizeObserver: ResizeObserver

const currentPlugin = computed(() => {
  return pluginStore.currentPlugin
})

// 如果切换编辑模式，则默认切换到日志展示的tab
watch(
  currentPlugin,
  (val) => {
    if (!val) return
    if (val === 'editElevation') {
      currentTab.value = 'elevation'
    } else {
      currentTab.value = 'output'
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

onMounted(() => {
  resizeObserver = new ResizeObserver(updateSize)
  resizeObserver.observe(bottomAreaRef.value)
})

onUnmounted(() => {
  resizeObserver && resizeObserver.unobserve(bottomAreaRef.value)
})

function updateSize () {
  areaWidth.value = bottomAreaRef.value.clientWidth
  areaHeight.value = layoutStore.bottomAreaHeight
}
</script>

<template>
  <div ref="bottomAreaRef" class="bottom-area">
    <el-tabs v-model="currentTab">
      <el-tab-pane label="Output" name="output">
        <Record />
      </el-tab-pane>
      <!-- TODO 仅在高程编辑模式，才展示高程面板 -->
      <el-tab-pane
        v-if="pluginStore.currentPlugin === 'editElevation'"
        label="高程编辑"
        name="elevation"
      >
        <Elevation
          :is-active="currentTab === 'elevation'"
          :width="areaWidth"
          :height="areaHeight"
        />
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<style scoped lang="less">
.bottom-area {
  height: 100%;

  :deep(.el-tabs) {
    .el-tabs__header {
      margin: 0;
    }
    .el-tabs__item.is-active {
      color: var(--active-color);
    }
    .el-tabs__active-bar {
      background-color: var(--active-color);
    }
    .el-tabs__item {
      --height: 28px;

      color: var(--text-color);
      width: 72px;
      font-size: var(--font-size);
      text-align: center;
      padding: 0;
      height: var(--height);
      line-height: var(--height);

      &:hover {
        color: var(--active-color);
      }
    }
    .el-tabs__nav-wrap {
      &::after {
        content: '';
        background-color: var(--main-area-color);
      }
    }
    .el-tabs__nav {
      transition: none;
    }
    .el-tabs__active-bar {
      transition: none;
    }
  }
}
</style>
