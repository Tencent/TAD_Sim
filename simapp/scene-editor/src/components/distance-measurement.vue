<template>
  <div class="distance-measurement">
    <div
      v-for="{ uuid, id, idx, type, x, y, distance, direction, angle } of points"
      :key="uuid"
      class="distance-measurement-card"
      :style="{ top: `${y}px`, left: `${x}px` }"
    >
      <span v-if="type === 'pre-start'">
        {{ $t('tips.ClickSetStart') }}
      </span>
      <span v-else-if="type === 'pre-middle'">
        {{ distance }}m<br>
        {{ $t('tips.ClickToAdd') }}
      </span>
      <span v-else-if="type === 'pre-end'">
        {{ $t('scenario.TotalLength') }}: {{ distance }}m<br>
        {{ $t('scenario.Orientation') }}: {{ direction }}°<br>
        {{ $t('tips.clickStart', { b: $t('operation.ok') }) }}, {{ $t('tips.pressToEnd', { b: $t('enter') }) }}
      </span>
      <span v-else-if="type === 'start'">
        {{ $t('scenario.StartingPoint') }}
        <el-icon
          @click="handleRemovePoint(uuid, id, idx)"
          @mouseup="onMouseUpClose"
          @mousemove="onMouseMoveClose"
          @mousedown="onMouseUpClose"
        ><Close /></el-icon>
      </span>
      <span v-else-if="type === 'middle'">
        {{ distance }}m
        <el-icon
          @click="handleRemovePoint(uuid, id, idx)"
          @mouseup="onMouseUpClose"
          @mousemove="onMouseMoveClose"
          @mousedown="onMouseUpClose"
        ><Close /></el-icon>
        <br>
        {{ $t('scenario.Orientation') }}: {{ direction }}°<br>
        {{ $t('scenario.IncludedAngle') }}: {{ angle }}°
      </span>
      <span v-else-if="type === 'end'">
        {{ $t('scenario.TotalLength') }}: {{ distance }}m
        <el-icon
          @click="handleRemovePoint(uuid, id, idx)"
          @mouseup="onMouseUpClose"
          @mousemove="onMouseMoveClose"
          @mousedown="onMouseUpClose"
        ><Close /></el-icon>
        <el-icon
          @click="handleRemoveLine(uuid, id, idx)"
          @mouseup="onMouseUpClose"
          @mousemove="onMouseMoveClose"
          @mousedown="onMouseUpClose"
        ><Delete /></el-icon>
        <br>
        {{ $t('scenario.Orientation') }}: {{ direction }}°
      </span>
    </div>
  </div>
</template>

<script>
import { editor } from '@/api/interface'

export default {
  name: 'DistanceMeasurement',
  props: {
    points: {
      type: Array,
      default () {
        return []
      },
    },
  },
  methods: {
    handleRemovePoint (uuid, id, idx) {
      editor.scenario.removeDistanceMeasurementPoint({
        uuid,
        id,
        idx,
      })
    },
    handleRemoveLine (uuid, id, idx) {
      editor.scenario.removeDistanceMeasurementLine({
        uuid,
        id,
        idx,
      })
    },

    onMouseUpClose (event) {
      event.stopPropagation()
    },

    onMouseMoveClose (event) {
      event.stopPropagation()
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .distance-measurement {
    position: relative;
    width: 100%;
    height: 100%;
    overflow: hidden;

    .distance-measurement-card {
      position: absolute;
      top: 0;
      left: 0;
      padding: 0 4px;
      font-size: 12px;
      line-height: 18px;
      color: #191919;
      background-color: rgba(255, 255, 255, 0.85);
      border-radius: 3px;

      span i {
        margin-left: 4px;
        cursor: pointer;
      }
    }
  }
</style>
