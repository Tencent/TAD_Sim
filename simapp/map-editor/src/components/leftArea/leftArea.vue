<script setup lang="ts">
import {
  computed,
} from 'vue'
import { usePluginStore } from '@/stores/plugin'

// 【TODO】由于动态加载模块在 build 后找不到对应的模块，暂时先手动读取
import ObjectList from '@/plugins/editObject/components/objectList.vue'
import RoadType from '@/plugins/editRoad/components/roadType.vue'

const pluginStore = usePluginStore()
const pluginList = {
  editRoad: [RoadType],
  editObject: [ObjectList],
}
const currentPlugin = computed(() => {
  return pluginStore.currentPlugin
})

// 计算当前插件模式下是否有对应的定制化组件
const hasPluginComponent = computed(() => {
  if (!currentPlugin.value) return false
  if (!pluginList[currentPlugin.value]) return false
  return true
})

// const pluginList = getAllPluginConfig()
// const pluginStore = usePluginStore()
// const currentPlugin = computed(() => {
//   return pluginStore.currentPlugin
// })
// const pluginComponents: Ref<any> = shallowRef({})
// const currentPluginComponents = shallowRef([])

// pluginList.forEach((plugin) => {
//   if (
//     !plugin?.componentPaths?.leftArea ||
//     plugin?.componentPaths?.leftArea.length < 1
//   ) {
//     return
//   }

//   pluginComponents.value[plugin.name] = []
//   const { leftArea: componentFileNames } = plugin.componentPaths
//   componentFileNames.forEach((fileName) => {
//     const relativePath = `${plugin.name}/components/${fileName}`
//     pluginComponents.value[plugin.name].push(
//       defineAsyncComponent(
//         // 保持跟 src 目录相差 2 个层级，用 ../../ 进行相对引用。@ 不支持
//         () => import(/* @vite-ignore */ `../../plugins/${relativePath}`)
//       )
//     )
//   })
// })

// watch(currentPlugin, (newVal: string) => {
//   if (!newVal) return
//   currentPluginComponents.value = pluginComponents.value[newVal]
// })
// const hasPluginComponent = computed(() => {
//   if (!currentPlugin.value) return false
//   if (!currentPluginComponents.value) return false
//   if (currentPluginComponents.value.length < 1) return false
//   return true
// })
</script>

<template>
  <div class="left-area">
    <!-- <template v-if="hasPluginComponent">
      <component
        v-for="(pluginComponent, index) in currentPluginComponents"
        :key="index"
        :is="pluginComponent"
      />
    </template> -->
    <template v-if="hasPluginComponent">
      <component
        :is="pluginComponent"
        v-for="(pluginComponent, index) in pluginList[currentPlugin]"
        :key="index"
      />
    </template>
  </div>
</template>

<style scoped lang="less">
.left-area {
  height: 100%;
  overflow-y: auto;
}
</style>
