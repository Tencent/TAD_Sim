<script setup lang="ts">
/* global common */
// @ts-nocheck
import { computed, onMounted, ref } from 'vue'
import CurveRoadSvg from './pluginIcon/CurveRoad2.svg?component'
import JunctionSvg from './pluginIcon/CrossRoad2.svg?component'
import ObjectSvg from './pluginIcon/addobject.svg?component'
import LaneSvg from './pluginIcon/lane.svg?component'
import LaneWidthSvg from './pluginIcon/lane-width.svg?component'
import AddLaneSvg from './pluginIcon/add-lane.svg?component'
import LaneBoundarySvg from './pluginIcon/lane-line.svg?component'
import RoadHeightSvg from './pluginIcon/road-height.svg?component'
import TrafficLightSvg from './pluginIcon/traffic-light.svg?component'
import EditLinkSvg from './pluginIcon/link-edit.svg?component'
import CircleRoadSvg from './pluginIcon/CircleRoad2.svg?component'
import DropDownSvg from './viewIcon/drop-down.svg?component'
import CrgSvg from './pluginIcon/crg.svg?component'

import { usePluginStore } from '@/stores/plugin'
import { getAllPluginConfig } from '@/utils/tools'
import { useConfig3dStore } from '@/stores/config3d'
import ee, { CustomEventType } from '@/utils/event'

const config3dStore = useConfig3dStore()
const pluginList = getAllPluginConfig()
// 组件的展示顺序
const pluginOrder = {
  editRoad: 1,
  editJunction: 2,
  editCircleRoad: 3,
  editLaneAttr: 4,
  editBoundary: 7,
  editLaneWidth: 5,
  editLaneNumber: 6,
  editObject: 8,
  editLink: 9,
  editSection: 20,
  editElevation: 10,
  editSignalControl: 11,
  editCrg: 12,
}
// 按照插件的展示顺序来排序
const sortedPluginList = pluginList.sort(sortPluginByOrder)

function sortPluginByOrder (
  pluginA: common.pluginConfig,
  pluginB: common.pluginConfig,
) {
  const orderA = pluginOrder[pluginA.name]
  const orderB = pluginOrder[pluginB.name]

  if (orderA > orderB) {
    return 1
  } else if (orderA < orderB) {
    return -1
  } else {
    return 0
  }
}

// 插件 icon 集合
const pluginIconComponents = {
  CurveRoadSvg,
  JunctionSvg,
  ObjectSvg,
  LaneSvg,
  LaneWidthSvg,
  AddLaneSvg,
  LaneBoundarySvg,
  RoadHeightSvg,
  TrafficLightSvg,
  EditLinkSvg,
  CircleRoadSvg,
  CrgSvg,
}

const pluginStore = usePluginStore()

const currentPlugin = computed(() => {
  return pluginStore.currentPlugin
})

// 切换编辑模式的插件
function togglePluginMode (pluginName: string) {
  pluginStore.activate(pluginName)
  if (pluginName === 'editCircleRoad') {
    toggleReserveJunctionDropdown.value[0].handleOpen()
    isOpen.value = true
  }
}

const toggleReserveJunctionDropdown = ref()
const isOpen = ref(false)
// 预留 1-4 个路口的环形道路
const reserveJunction = [1, 2, 3, 4]
const currentReserveJunction = computed(() => {
  return config3dStore.reserveJunction
})

// 在下拉菜单框切换环形道路预留的路口数
function toggleJunctionNumber (num: number) {
  isOpen.value = false
  // 如果一样则不响应
  if (num === config3dStore.reserveJunction) return

  config3dStore.updateReserveJunction(num)
}

onMounted(() => {
  // 手动触发下拉菜单栏的关闭
  ee.on(CustomEventType.window.pointerdown, () => {
    if (isOpen.value) {
      toggleReserveJunctionDropdown.value[0].handleClose()
      isOpen.value = false
    }
  })
})
</script>

<template>
  <ul class="plugin-ul">
    <li
      v-for="plugin in sortedPluginList"
      :key="plugin.name"
      class="plugin-li"
      :class="{ active: currentPlugin && currentPlugin === plugin.name }"
      @click="togglePluginMode(plugin.name)"
    >
      <template v-if="plugin.name === 'editCircleRoad'">
        <div class="icon-box">
          <component
            :is="pluginIconComponents[plugin.iconComponentName]"
            class="icon"
          />
          <DropDownSvg class="drop-down-icon" />
        </div>
        <label class="label">{{ $t(`plugins.${plugin.iconDesc}`) }}</label>
        <el-dropdown
          ref="toggleReserveJunctionDropdown"
          popper-class="toggle-view-popper"
        >
          <div class="empty-holder" />
          <template #dropdown>
            <el-dropdown-menu>
              <el-dropdown-item
                v-for="number in reserveJunction"
                :key="number"
                :class="{ active: currentReserveJunction === number }"
                @click="toggleJunctionNumber(number)"
              >
                <span>环形道路({{ number }}路口)</span>
              </el-dropdown-item>
            </el-dropdown-menu>
          </template>
        </el-dropdown>
      </template>
      <template v-else>
        <component
          :is="pluginIconComponents[plugin.iconComponentName]"
          class="icon"
        />
        <label class="label">{{ $t(`plugins.${plugin.iconDesc}`) }}</label>
      </template>
    </li>
  </ul>
</template>

<style scoped lang="less">
.plugin-ul {
  display: flex;
  align-items: center;
  flex-wrap: nowrap;

  .plugin-li {
    margin: 0 8px;
    display: flex;
    flex-direction: column;
    align-items: center;
    cursor: pointer;
    color: var(--text-color);

    &.active {
      color: var(--active-color);
    }

    &:hover {
      color: var(--active-color);
    }
    &:active {
      color: var(--active-color);
    }

    .icon {
      height: 20px;
    }
    .icon-box {
      height: 20px;
      line-height: 20px;
      text-align: center;
      position: relative;

      .drop-down-icon {
        position: absolute;
        top: 6px;
        right: -14px;
      }
    }
    .empty-holder {
      width: 100%;
      height: 0;
    }
    .label {
      font-size: 12px;
      line-height: 1.6em;
      cursor: pointer;
      text-align: center;
    }
  }
}
</style>
