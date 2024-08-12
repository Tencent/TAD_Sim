<script setup lang="ts">
import {
  computed,
} from 'vue'
import { usePluginStore } from '@/stores/plugin'

// 【TODO】由于动态加载模块在 build 后找不到对应的模块，暂时先手动读取
import ControlPointProperty from '@/plugins/editRoad/components/controlPointProperty.vue'
import RoadProperty from '@/plugins/editRoad/components/roadProperty.vue'
import JunctionProperty from '@/plugins/editJunction/components/junctionProperty.vue'
import RoadEdgeProperty from '@/plugins/editJunction/components/roadEdgeProperty.vue'
import LaneAttrProperty from '@/plugins/editLaneAttr/components/laneAttrProperty.vue'
import LaneWidthProperty from '@/plugins/editLaneWidth/components/laneWidthProperty.vue'
import LaneNumberProperty from '@/plugins/editLaneNumber/components/laneNumberProperty.vue'
import BoundaryProperty from '@/plugins/editBoundary/components/boundaryProperty.vue'
import HtPointProperty from '@/plugins/editLink/components/htPointProperty.vue'
import LinkProperty from '@/plugins/editLink/components/linkProperty.vue'
import ObjectProperty from '@/plugins/editObject/components/objectProperty.vue'
import ElevationRoadProperty from '@/plugins/editElevation/components/roadProperty.vue'
import SignalControlProperty from '@/plugins/editSignalControl/components/signalControlProperty.vue'
import CircleRoadProperty from '@/plugins/editCircleRoad/components/circleRoadProperty.vue'
import CrgProperty from '@/plugins/editCrg/components/crgProperty.vue'

const pluginStore = usePluginStore()

// const pluginList = getAllPluginConfig()
const pluginList = {
  editRoad: [RoadProperty, ControlPointProperty],
  editJunction: [JunctionProperty, RoadEdgeProperty],
  editLaneAttr: [LaneAttrProperty],
  editLaneWidth: [LaneWidthProperty],
  editLaneNumber: [LaneNumberProperty],
  editBoundary: [BoundaryProperty],
  editLink: [HtPointProperty, LinkProperty],
  editObject: [ObjectProperty],
  editElevation: [ElevationRoadProperty],
  editSignalControl: [SignalControlProperty],
  editCircleRoad: [CircleRoadProperty],
  editCrg: [CrgProperty],
}

// const pluginProperties: Ref<any> = shallowRef({})
// const currentPluginProperty = shallowRef([])
const currentPlugin = computed(() => {
  return pluginStore.currentPlugin
})

// // 遍历已加载的插件配置，判断是否有定制化的插件组件
// pluginList.forEach((plugin) => {
//   if (
//     !plugin?.componentPaths?.property ||
//     plugin?.componentPaths?.property.length < 1
//   ) {
//     return
//   }

//   pluginProperties.value[plugin.name] = []
//   // 由于在属性面板容器组件中，所以判断是否有定制化的插件属性面板组件
//   const { property: propertyFileNames } = plugin.componentPaths
//   propertyFileNames.forEach((fileName) => {
//     const relativePath = `${plugin.name}/components/${fileName}`
//     pluginProperties.value[plugin.name].push(
//       // 异步加载插件组件，由于 import() 暂不支持完全的变量路径（es6 提案中），需要使用 "字符串+变量" 的形式才能够正常加载
//       defineAsyncComponent(
//         // 保持跟 src 目录相差 2 个层级，用 ../../ 进行相对引用。@ 不支持
//         () => import(/* @vite-ignore */ `../../plugins/${relativePath}`)
//       )
//     )
//   })
// })

// watch(currentPlugin, (newVal: string) => {
//   if (!newVal) return
//   // 根据当前激活的插件，调整属性面板展示的定制化插件组件
//   currentPluginProperty.value = pluginProperties.value[newVal]
// })

// 计算当前插件模式下是否有对应的定制化组件
const hasPluginProperty = computed(() => {
  if (!currentPlugin.value) return false
  // if (!currentPluginProperty.value) return false
  // if (currentPluginProperty.value.length < 1) return false
  if (!pluginList[currentPlugin.value]) return false
  return true
})
</script>

<template>
  <div class="property-container">
    <div class="title">
      {{ $t('desc.property') }}
    </div>
    <div class="properties">
      <template v-if="hasPluginProperty">
        <!-- <component
          v-for="(pluginComponent, index) in currentPluginProperty"
          :key="index"
          :is="pluginComponent"
        /> -->
        <component
          :is="pluginComponent"
          v-for="(pluginComponent, index) in pluginList[currentPlugin]"
          :key="index"
        />
      </template>
    </div>
  </div>
</template>

<style scoped lang="less">
.property-container {
  flex: 1 1 300px;
  overflow-x: hidden;
  --title-height: 24px;
  --padding: 12px;
  position: relative;
  .title {
    height: var(--title-height);
    line-height: var(--title-height);
    padding: 0 var(--padding);
    background-color: var(--main-dark-color);
  }

  .properties {
    position: absolute;
    top: var(--title-height);
    bottom: 0;
    right: 0;
    left: 0;
    overflow: auto;
  }
}
</style>
