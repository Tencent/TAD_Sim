<script setup lang="ts">
// @ts-nocheck
import { computed } from 'vue'
import { MathUtils } from 'three'
import CompassPointerSvg from './viewIcon/compass_pointer.svg?component'
import CompassNorthSvg from './viewIcon/compass_north.svg?component'
import { useConfig3dStore } from '@/stores/config3d'
import { fixedPrecision, halfPI } from '@/utils/common3d'
import root3d from '@/main3d'

const config3dStore = useConfig3dStore()

const compassAngle = computed(() => {
  // 将弧度制转换成角度制度
  // 指北针的角度，跟相机偏移量的角度有 90° 的偏差
  return fixedPrecision(MathUtils.radToDeg(config3dStore.cameraAngle + halfPI))
})

// 点击指北针，将视角重置成默认视角
function resetView () {
  root3d.resetPerspectiveCameraAngle()
}
</script>

<template>
  <div
    class="compass"
    :style="{
      transform: `rotateZ(${compassAngle}deg)`,
    }"
    @click="resetView"
  >
    <CompassPointerSvg />
    <CompassNorthSvg />
  </div>
</template>

<style scoped lang="less">
.compass {
  position: absolute;
  bottom: 10px;
  left: 10px;
  background-color: rgba(0, 0, 0, 0.5);
  width: 36px;
  height: 36px;
  border-radius: 18px;
  cursor: pointer;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
}
</style>
