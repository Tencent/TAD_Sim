<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref, watch } from 'vue'
import { Raycaster, Vector2 } from 'three'
import WebglContent from './webglContent.vue'
import Compass from './compass.vue'
import { useFileStore } from '@/stores/file'
import i18n from '@/locales'
import { useConfig3dStore } from '@/stores/config3d'
import { globalConfig } from '@/utils/preset'
import ee, { CustomEventType, updatePointerMoveEvent } from '@/utils/event'
import root3d from '@/main3d'

const fileStore = useFileStore()
const config3dStore = useConfig3dStore()
const pointer = new Vector2()
const raycaster = new Raycaster()

const currentMapName = ref(i18n.global.t('desc.mapFile.undefined'))
watch(
  () => fileStore.openedFile,
  (val) => {
    if (val) {
      currentMapName.value = val
    } else {
      currentMapName.value = i18n.global.t('desc.mapFile.undefined')
    }
  },
  {
    immediate: true,
  },
)

const showCompass = computed(() => {
  return config3dStore.cameraType === 'P'
})
// 鼠标射线当前指向的 xy 坐标
const coordinateX = computed(() => {
  return config3dStore.rayCoordinateX
})
const coordinateY = computed(() => {
  return config3dStore.rayCoordinateY
})

// 更新鼠标射线的坐标
function updateCoordinate (event: PointerEvent) {
  const { target, clientX, clientY } = event
  if (!target) return

  // 更新缓存的鼠标事件对象
  updatePointerMoveEvent(event)

  const rect = (target as HTMLElement).getBoundingClientRect()
  // 计算鼠标坐标投影到画布区域的相对坐标
  const { left, top, width, height } = rect

  // [-1,1]区间的画布屏幕坐标
  const x = (2 * (clientX - left)) / width - 1
  const y = 1 - (2 * (clientY - top)) / height
  pointer.x = x
  pointer.y = y
  // 将从屏幕发出的射线，跟场景中的默认地面进行相交检测
  raycaster.setFromCamera(pointer, root3d.core.mainCamera)
  const arr = raycaster.intersectObject(root3d.ground.object)
  if (arr.length > 0) {
    const { point } = arr[0]
    config3dStore.updateRayCoordinate(point.z, point.x)
  }
}

onMounted(() => {
  ee.on(CustomEventType.webglArea.pointermove, updateCoordinate)
})

onUnmounted(() => {
  ee.off(CustomEventType.webglArea.pointermove, updateCoordinate)
})
</script>

<template>
  <div class="main-area">
    <div class="top-area">
      <div class="title">
        <span>{{ $t('desc.mapName') }}</span> :
        <span>{{ currentMapName }}</span>
      </div>
      <!-- 根据配置项来控制是否展示鼠标 xy 坐标 -->
      <template v-if="globalConfig.exposeModule.function.showXYCoordinate">
        <div class="desc">
          <span>x: {{ coordinateX }}</span>
          <span>y: {{ coordinateY }}</span>
        </div>
      </template>
    </div>
    <WebglContent />
    <!-- <viewInteraction /> -->
    <!-- 根据配置项来控制是否展示指北针 -->
    <template v-if="globalConfig.exposeModule.function.showCompass">
      <!-- 指北针仅在透视视角下显示 -->
      <Compass v-if="showCompass" />
    </template>
  </div>
</template>

<style scoped lang="less">
.main-area {
  flex: 1;
  min-height: 0;
  position: relative;
  display: flex;
  flex-direction: column;

  --map-desc-height: 24px;

  .top-area {
    height: var(--map-desc-height);
    line-height: var(--map-desc-height);
    padding: 0 6px;
    background-color: var(--main-dark-color);
    opacity: 0.8;
    display: flex;
    flex-direction: row;
    justify-content: space-between;
    .desc span {
      display: inline-block;
      min-width: 60px;
      margin-right: 14px;
    }
  }
}
</style>
