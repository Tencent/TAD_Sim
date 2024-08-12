<script setup lang="ts">
// 正交和透视相机的控制
// @ts-nocheck
import { computed, onMounted, ref } from 'vue'
import NP from 'number-precision'
import DropDownSvg from './viewIcon/drop-down.svg?component'
import TopCameraSvg from './viewIcon/perspective-top.svg?component'
import FreeCameraSvg from './viewIcon/perspective-free.svg?component'
import ZoomInSvg from './viewIcon/zoom-in.svg?component'
import ZoomOutSvg from './viewIcon/zoom-out.svg?component'
import i18n from '@/locales'
import root3d from '@/main3d'
import { useConfig3dStore } from '@/stores/config3d'
import { usePluginStore } from '@/stores/plugin'
import { warningMessage } from '@/utils/common'
import { getPermission } from '@/utils/permission'

const config3dStore = useConfig3dStore()
const pluginStore = usePluginStore()

const viewTypes = [
  { id: 'O', label: 'desc.topView', icon: TopCameraSvg },
  { id: 'P', label: 'desc.perspectiveView', icon: FreeCameraSvg },
]

const shownViewTypes = viewTypes.filter((item) => {
  if (item.id === 'O') {
    return getPermission('action.mapEditor.cameraType.top.enable')
  }
  if (item.id === 'P') {
    return getPermission('action.mapEditor.cameraType.perspective.enable')
  }
  return true
})

const defaultView = shownViewTypes[0] || {}
const activeView = ref(defaultView)

const toggleViewDropdown = ref()
const zoomPercent = computed(() => {
  // 保留两位小数
  return `${NP.round(config3dStore.zoom * 100, 0)}%`
})
// 是否是顶视角
const isTopView = computed(() => {
  return activeView.value.id === 'O'
})

onMounted(() => {
  if (defaultView.id !== 'O') toggleViewType(defaultView)
})

function toggleViewType (view) {
  activeView.value = view
  if (view.id === 'P') toggleToPerspectiveView()
  if (view.id === 'O') toggleToTopView()
}
function toggleToTopView () {
  const res = config3dStore.toggleCameraTypeOnly('O')
  toggleViewDropdown.value.handleClose()
  isOpen.value = false
  if (!res) return
  root3d.updateCamera({
    type: 'O',
  })
}
function toggleToPerspectiveView () {
  if (pluginStore.currentPlugin === 'editRoad') {
    warningMessage({
      content: i18n.global.t(
        'desc.tips.cannotToggleToPerspectiveViewInEditRoadMode',
      ),
    })
    return
  }
  const res = config3dStore.toggleCameraTypeOnly('P')
  toggleViewDropdown.value.handleClose()
  isOpen.value = false
  if (!res) return
  root3d.updateCamera({
    type: 'P',
  })
}
const ratio = 8 / 7
function handleZoomOut () {
  const newZoom = NP.times(config3dStore.zoom, 1 / ratio)
  config3dStore.updateCameraZoom(newZoom)
  root3d.updateOrthographicCameraZoom(newZoom)
}
function handleZoomIn () {
  const newZoom = NP.times(config3dStore.zoom, ratio)
  config3dStore.updateCameraZoom(newZoom)
  root3d.updateOrthographicCameraZoom(newZoom)
}

const isOpen = ref(false)
</script>

<template>
  <ul class="view-ul">
    <li v-if="shownViewTypes.length" class="view-li">
      <el-dropdown
        ref="toggleViewDropdown"
        trigger="click"
        :hide-on-click="false"
        popper-class="toggle-view-popper"
      >
        <div class="view-li-wrap">
          <div class="view-li-icon view-icon">
            <TopCameraSvg v-if="isTopView" class="camera-icon" />
            <FreeCameraSvg v-else class="camera-icon" />
            <DropDownSvg class="drop-down-icon" />
          </div>
          <div class="view-li-text view-title">
            {{ $t(activeView.label) }}
          </div>
        </div>
        <template #dropdown>
          <el-dropdown-menu>
            <template v-for="item in shownViewTypes" :key="item.id">
              <el-dropdown-item
                :class="{ active: activeView.id === item.id }"
                @click="toggleViewType(item)"
              >
                <component :is="item.icon" />
                <span> {{ $t(item.label) }}</span>
              </el-dropdown-item>
            </template>
          </el-dropdown-menu>
        </template>
      </el-dropdown>
    </li>
    <BaseAuth :perm="['action.mapEditor.camera.zoom.enable']">
      <li v-if="isTopView" class="view-li">
        <div class="view-li-wrap">
          <div class="view-li-icon zoom-item">
            <span @click="handleZoomOut"><ZoomOutSvg /></span>
            <span @click="handleZoomIn"><ZoomInSvg /></span>
          </div>
          <div class="view-li-text zoom-desc">
            {{ zoomPercent }}
          </div>
        </div>
      </li>
    </BaseAuth>
  </ul>
</template>

<style scoped lang="less">
.view-ul {
  display: flex;
  align-items: center;
  flex-wrap: nowrap;
  .view-li {
    margin: 0 8px;
    align-items: center;
    color: var(--text-color);
    display: flex;
    flex-direction: column;
    justify-content: center;
    height: 100%;
    line-height: 18px;

    :deep(.el-dropdown) {
      color: inherit;
      line-height: inherit;
    }

    .view-li-wrap {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
    }

    .view-li-icon {
      height: 18px;
      display: flex;
      align-items: center;
      gap: 0 4px;
    }

    .view-title {
      margin-top: 2px;
    }
  }
}
</style>
