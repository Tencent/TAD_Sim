<script setup lang="ts">
import { MathUtils } from 'three'
import { computed } from 'vue'
import { useCircleRoadInteraction } from '../store/interaction'
import { PI, fixedPrecision } from '@/utils/common3d'

const interactionStore = useCircleRoadInteraction()
const selectedRoad = computed(() => {
  return interactionStore.currentRoad
})
// 圆弧半径
const arcRadius = computed(() => {
  if (!selectedRoad.value) return ''
  const { isCircleRoad, circleOption } = selectedRoad.value
  if (!isCircleRoad) return ''
  const { radius } = circleOption || {}
  return String(fixedPrecision(radius as number))
})

// 圆弧角度
const arcAngle = computed(() => {
  if (!selectedRoad.value) return ''
  const { isCircleRoad, circleOption } = selectedRoad.value
  if (!isCircleRoad) return ''
  const {
    startAngle = 0,
    endAngle = 0,
    // 计算角度需要判断是否是
    isClockwise = false,
  } = circleOption || {}
  let deltaAngle = 0
  let _startAngle = (startAngle + PI * 2) % (PI * 2)
  let _endAngle = (endAngle + PI * 2) % (PI * 2)
  if (isClockwise) {
    // 顺时针绘制，起始角度应该大于截止角度
    if (_startAngle < _endAngle) {
      _startAngle += PI * 2
    }
    deltaAngle = _startAngle - _endAngle
  } else {
    // 逆时针绘制，起始角度应该小于截止角度
    if (_endAngle < _startAngle) {
      _endAngle += PI * 2
    }
    deltaAngle = _endAngle - _startAngle
  }

  return String(fixedPrecision(MathUtils.radToDeg(deltaAngle), 0))
})
</script>

<template>
  <template v-if="selectedRoad">
    <!-- 偏展示信息类，没有表单输入的，使用自定义的属性列表结构 -->
    <div class="property-title">
      {{ $t('desc.commonRoad.currentRoadProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadId') }}</label>
        <div class="content">
          {{ selectedRoad.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadLength') }}</label>
        <div class="content">
          {{ selectedRoad.length }} m
        </div>
      </div>
      <div v-if="arcRadius" class="property-cell">
        <label class="label">{{ $t('desc.editCircleRoad.arcRadius') }}</label>
        <div class="content">
          {{ arcRadius }} m
        </div>
      </div>
      <div v-if="arcAngle" class="property-cell">
        <label class="label">{{ $t('desc.editCircleRoad.arcAngle') }}</label>
        <div class="content">
          {{ arcAngle }} °
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
