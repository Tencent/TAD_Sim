<script setup lang="ts">
// 只执行逻辑，不渲染内容
import { computed, onMounted, onUnmounted, watch } from 'vue'
import { useHelperStore } from '@/stores/helper'
import { usePluginStore } from '@/stores/plugin'
import { useRoadInteractionStore } from '@/plugins/editRoad/store/interaction'
import { useJunctionInteractionStore } from '@/plugins/editJunction/store/interaction'
import { useLaneWidthInteractionStore } from '@/plugins/editLaneWidth/store/interaction'
import { useLaneNumberInteractionStore } from '@/plugins/editLaneNumber/store/interaction'
import { useLaneAttrInteractionStore } from '@/plugins/editLaneAttr/store/interaction'
import { useBoundaryInteractionStore } from '@/plugins/editBoundary/store/interaction'
import { useSignalControlInteractionStore } from '@/plugins/editSignalControl/store/interaction'
import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import { useObjectInteractionStore } from '@/plugins/editObject/store/interaction'
import { useCircleRoadInteraction } from '@/plugins/editCircleRoad/store/interaction'
import { useCrgInteractionStore } from '@/plugins/editCrg/store/interaction'
import { useElevationInteractionStore } from '@/plugins/editElevation/store/interaction'
import ee, { CustomEventType } from '@/utils/event'
import { useObjectStore } from '@/stores/object'
import root3d from '@/main3d/index'

import { disposeRoadEndEdge } from '@/plugins/editJunction/main3d/draw'
import { warningMessage } from '@/utils/common'
import i18n from '@/locales'

const helperStore = useHelperStore()
const roadInteractionStore = useRoadInteractionStore()
const junctionInteractionStore = useJunctionInteractionStore()
const laneWidthInteractionStore = useLaneWidthInteractionStore()
const laneNumberInteractionStore = useLaneNumberInteractionStore()
const laneAttrInteractionStore = useLaneAttrInteractionStore()
const boundaryInteractionStore = useBoundaryInteractionStore()
const objectInteractionStore = useObjectInteractionStore()
const signalControlInteractionStore = useSignalControlInteractionStore()
const linkInteractionStore = useLinkInteractionStore()
const circleRoadInteractionStore = useCircleRoadInteraction()
const crgInteractionStore = useCrgInteractionStore()
const elevationInteractionStore = useElevationInteractionStore()
const pluginStore = usePluginStore()
const objectStore = useObjectStore()

// 当前的插件
const currentPlugin = computed(() => {
  return pluginStore.currentPlugin
})

// 监听各个编辑模式下元素的选中状态更新，如果有更新，则触发结构树的选中
// 曲线道路编辑模式
watch(
  () => roadInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editRoad') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 路口编辑模式
watch(
  () => junctionInteractionStore.junctionId,
  (val) => {
    if (currentPlugin.value !== 'editJunction') return
    if (val) {
      helperStore.setSelectedElement({
        element: `junction-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 环形道路编辑模式
watch(
  () => circleRoadInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editCircleRoad') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 车道属性编辑模式
watch(
  () => laneAttrInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editLaneAttr') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 车道宽度编辑模式
watch(
  () => laneWidthInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editLaneWidth') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 车道数量编辑模式
watch(
  () => laneNumberInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editLaneNumber') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 车道边界线编辑模式
watch(
  () => boundaryInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editBoundary') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 物体编辑模式
watch(
  () => [
    objectInteractionStore.poleId,
    objectInteractionStore.roadSignId,
    objectInteractionStore.signalBoardId,
    objectInteractionStore.trafficLightId,
    objectInteractionStore.sensorId,
    objectInteractionStore.otherId,
    objectInteractionStore.customModelId,
  ],
  (val) => {
    const [
      poleId,
      roadSignId,
      signalBoardId,
      trafficLightId,
      sensorId,
      otherId,
      customModelId,
    ] = val

    if (currentPlugin.value !== 'editObject') return

    // 先取消选中
    helperStore.clearSelected()

    let element = ''

    if (poleId) {
      element = `pole-${poleId}`
    } else if (roadSignId) {
      element = `roadSign-${roadSignId}`
    } else if (signalBoardId) {
      element = `signalBoard-${signalBoardId}`
    } else if (trafficLightId) {
      element = `trafficLight-${trafficLightId}`
    } else if (sensorId) {
      element = `sensor-${sensorId}`
    } else if (otherId) {
      element = `other-${otherId}`
    } else if (customModelId) {
      element = `customModel-${customModelId}`
    }

    if (element) {
      helperStore.setSelectedElement({
        element,
      })
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

// 连接线编辑模式
watch(
  () => linkInteractionStore.junctionId,
  (val) => {
    if (currentPlugin.value !== 'editLink') return
    if (val) {
      helperStore.setSelectedElement({
        element: `junction-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 高程编辑模式
watch(
  () => elevationInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editElevation') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// 信控编辑模式
watch(
  () => signalControlInteractionStore.trafficLightId,
  (val) => {
    if (currentPlugin.value !== 'editSignalControl') return
    if (val) {
      helperStore.setSelectedElement({
        element: `trafficLight-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)
// crg 路面形貌编辑模式
watch(
  () => crgInteractionStore.roadId,
  (val) => {
    if (currentPlugin.value !== 'editCrg') return
    if (val) {
      helperStore.setSelectedElement({
        element: `road-${val}`,
      })
    } else {
      helperStore.clearSelected()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

// 结构树的元素选中，触发各个编辑模式的元素选中
function selectElementInPlugin (element: string) {
  const [name, id] = element.split('-')

  switch (currentPlugin.value) {
    case 'editRoad': {
      if (name === 'road') {
        roadInteractionStore.selectRoad(id)
      } else {
        roadInteractionStore.unselectExceptSomeone()
      }
      break
    }
    case 'editJunction': {
      if (name === 'junction') {
        junctionInteractionStore.unselectJunction()
        junctionInteractionStore.selectJunction(id)
      } else {
        junctionInteractionStore.unselectJunction()
      }
      break
    }
    case 'editCircleRoad': {
      if (name === 'road') {
        circleRoadInteractionStore.selectRoad(id)
      } else {
        circleRoadInteractionStore.unselectRoad()
      }
      break
    }
    case 'editLaneAttr': {
      break
    }
    case 'editLaneNumber': {
      break
    }
    case 'editLaneWidth': {
      break
    }
    case 'editBoundary': {
      break
    }
    case 'editObject': {
      if (name === 'pole') {
        objectInteractionStore.selectPole(id)
      } else if (name === 'roadSign') {
        objectInteractionStore.selectRoadSign(id)
      } else if (name === 'trafficLight') {
        objectInteractionStore.selectTrafficLight(id)
      } else if (name === 'signalBoard') {
        objectInteractionStore.selectSignalBoard(id)
      } else if (name === 'sensor') {
        objectInteractionStore.selectSensor(id)
      } else if (name === 'other') {
        objectInteractionStore.selectOther(id)
      } else if (name === 'customModel') {
        objectInteractionStore.selectCustomModel(id)
      } else {
        objectInteractionStore.unselectExceptSomeone()
      }
      break
    }
    case 'editLink': {
      if (name === 'junction') {
        linkInteractionStore.selectJunction(id)
      } else {
        linkInteractionStore.unSelectAll()
      }
      break
    }
    case 'editCrg': {
      if (name === 'road') {
        crgInteractionStore.selectRoad(id)
      } else {
        crgInteractionStore.unselectRoad()
      }
      break
    }
    case 'editSignalControl': {
      if (name === 'trafficLight') {
        // 信控选择信号灯
        signalControlInteractionStore.selectTrafficLight(id)
      } else {
        signalControlInteractionStore.unselectTrafficLight()
      }
      break
    }
    case 'editElevation': {
      if (name === 'road') {
        elevationInteractionStore.selectRoad(id)
      } else {
        elevationInteractionStore.unselectRoad()
      }
      break
    }
    default:
      break
  }
}

// 删除元素
function removeElement (element: string) {
  const [elementName, elementId] = element.split('-')
  const isSelectElement = helperStore.selectedElement === element

  // 禁止在部分编辑模式下删除地图元素
  if (currentPlugin.value === 'editObject') {
    if (elementName === 'road' || elementName === 'junction') {
      // 禁止在物体编辑模式下，删除道路或路口
      let tip = ''
      if (elementName === 'road') {
        tip = i18n.global.t(
          'actions.tips.notSupportedDeleteRoadInObjectEditingMode',
        )
      } else if (elementName === 'junction') {
        tip = i18n.global.t(
          'actions.tips.notSupportedDeleteJunctionInObjectEditingMode',
        )
      }
      if (tip) {
        warningMessage({
          content: tip,
        })
      }
      return
    }
  } else if (currentPlugin.value === 'editSignalControl') {
    if (elementName === 'road' || elementName === 'junction') {
      // 禁止在信控编辑模式下，删除道路或路口
      let tip = ''
      if (elementName === 'road') {
        tip = i18n.global.t(
          'actions.tips.notSupportedDeleteRoadInSignalControlEditingMode',
        )
      } else if (elementName === 'junction') {
        tip = i18n.global.t(
          'actions.tips.notSupportedDeleteJunctionInSignalControlEditingMode',
        )
      }
      if (tip) {
        warningMessage({
          content: tip,
        })
      }
      return
    }
  }

  // 判断在不同的编辑模式下，删除元素的处理逻辑
  if (elementName === 'road') {
    roadInteractionStore.removeCurrentRoad(elementId)
    if (isSelectElement) {
      roadInteractionStore.unselectExceptSomeone()
    }

    // 在不同编辑模式下的辅助元素也需要联动删除
    if (currentPlugin.value === 'editJunction') {
      const parent = root3d.proxyScene.getObjectByName('editJunctionContainer')
      if (parent) {
        // 删除道路首尾的辅助交互元素
        disposeRoadEndEdge({
          parent,
          roadId: elementId,
        })
      }
      // 取消路口和辅助元素的选中
      junctionInteractionStore.unselectJunction()
    } else if (currentPlugin.value === 'editCircleRoad') {
      // 删除道路以后，取消道路的选中效果，能够隐藏道路的辅助交互元素
      circleRoadInteractionStore.unselectRoad()
    } else if (currentPlugin.value === 'editLaneAttr') {
      // 取消车道的高亮效果
      laneAttrInteractionStore.unselectAll()
    } else if (currentPlugin.value === 'editLaneWidth') {
      // 取消车道和车道边界线的高亮效果
      laneWidthInteractionStore.unselectAll()
    } else if (currentPlugin.value === 'editLaneNumber') {
      laneNumberInteractionStore.unselectAll()
    } else if (currentPlugin.value === 'editBoundary') {
      boundaryInteractionStore.unselectAll()
    } else if (currentPlugin.value === 'editLink') {
      linkInteractionStore.unSelectAll()
    } else if (currentPlugin.value === 'editElevation') {
      elevationInteractionStore.unselectRoad()
    } else if (currentPlugin.value === 'editCrg') {
      crgInteractionStore.unselectRoad()
    }
  } else if (elementName === 'junction') {
    junctionInteractionStore.removeCurrentSelectedJunction(elementId)
    if (isSelectElement) {
      junctionInteractionStore.unselectJunction()
    }

    // 在连接线编辑模式下，取消选中
    if (currentPlugin.value === 'editLink') {
      linkInteractionStore.unSelectAll()
    }
  } else {
    if (elementName === 'pole') {
      objectStore.removePole(elementId, true)
    } else if (elementName === 'roadSign') {
      objectStore.removeRoadSign(elementId, true)
    } else if (elementName === 'trafficLight') {
      objectStore.removeTrafficLight(elementId, true)

      // 如果在信控编辑模式，删除信号灯，需要取消信号灯的选中
      if (currentPlugin.value === 'editSignalControl') {
        signalControlInteractionStore.unselectTrafficLight()
      }
    } else if (elementName === 'signalBoard') {
      objectStore.removeSignalBoard(elementId, true)
    } else if (elementName === 'sensor') {
      objectStore.removeSensor(elementId, true)
    } else if (elementName === 'other') {
      objectStore.removeOther(elementId, true)
    } else if (elementName === 'customModel') {
      objectStore.removeCustomModel(elementId, true)
    }

    if (isSelectElement) {
      objectInteractionStore.unselectExceptSomeone()
    }
  }

  root3d.core.render()
}

onMounted(() => {
  ee.on(CustomEventType.element.select, selectElementInPlugin)
  ee.on(CustomEventType.element.remove, removeElement)
})
onUnmounted(() => {
  ee.off(CustomEventType.element.select, selectElementInPlugin)
  ee.off(CustomEventType.element.remove, removeElement)
})
</script>

<template>
  <slot />
</template>
