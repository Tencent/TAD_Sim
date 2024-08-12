<template>
  <div
    ref="wrap"
    class="color-slider"
    :class="{ disabled }"
    @pointermove="moving"
    @pointerup="endMove"
  >
    <div
      v-for="(item, index) of innerData.filter(e => e.value > 0)"
      :key="index"
      class="color-cell" :class="[{ active: activeIndex === index }, item.color]"
      :style="{ flexGrow: item.value }"
      :title="item.value"
      @click="colorCellClick(index)"
    >
      {{ item.value }}
    </div>
    <div
      class="color-cell occupancy"
      :style="{ flexGrow: totalValue - colorsTotalValue }"
    >
      {{ totalValue - colorsTotalValue }}
    </div>
    <div
      v-for="(item, index) of splitArray"
      :key="`split-${index}`"
      class="color-split"
      :class="{ disabled: isSingleColor && index === 0 }"
      :style="{ left: `${item / totalValue * 100}%` }"
      @pointerdown="startMove($event, index)"
    />
  </div>
</template>

<script>
import _ from 'lodash'

// 获取索引之前的总时长
function getStartValue (colors, index) {
  return colors.reduce((re, item, i) => (i < index ? re + item.value : re), 0)
}

export default {
  name: 'ColorSlider',
  props: {
    // { value, color }
    data: {
      type: Array,
      required: true,
    },
    totalValue: {
      type: Number,
      required: true,
    },
    activeIndex: {
      type: Number,
      default: -1,
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      startX: 0,
      startY: 0,
      movingIndex: -1,
      innerData: [],
    }
  },
  computed: {
    // 最好的灯态是否为红色，若非红色，则首位不可拖拽
    lastIsRed () {
      if (this.data.length < 2) return false
      const lastColor = this.data[this.data.length - 1]
      if (lastColor) return lastColor.color === 'red'
      return false
    },
    splitArray () {
      const arr = []
      this.data.reduce((p, v) => {
        const c = p + v.value
        arr.push(c)
        return c
      }, 0)
      arr.unshift(0)
      arr.pop()
      return arr
    },
    colorsTotalValue () {
      return this.data.reduce((p, c) => p + c.value, 0)
    },
    isSingleColor () {
      return this.data.length === 1
    },
  },
  watch: {
    data () {
      this.innerData = _.cloneDeep(this.data)
    },
  },
  mounted () {
    this.innerData = _.cloneDeep(this.data)
  },
  methods: {
    colorCellClick (index) {
      if (this.disabled) return
      this.$emit('click-item', index)
    },
    startMove (event, index) {
      if (this.disabled) return
      const {
        $refs: { wrap },
      } = this
      if (!wrap.hasPointerCapture(event.pointerId)) {
        wrap.setPointerCapture(event.pointerId);
        ({ pageX: this.startX, pageY: this.startY } = event)
        this.movingIndex = index
        this.startItem = _.cloneDeep(this.innerData[index])
        this.startValue = getStartValue(this.innerData, index + 1)
      }
    },
    moving (event) {
      const {
        $refs: { wrap },
        totalValue,
        movingIndex,
      } = this
      const { pointerId, pageX } = event
      if (wrap.hasPointerCapture(pointerId)) {
        const { left, width } = wrap.getBoundingClientRect()
        if (width > 0) {
          let nowMovingValue = Math.round((pageX - left) / width * totalValue)
          nowMovingValue = Math.min(nowMovingValue, totalValue)
          this.handleSplitChange(movingIndex, nowMovingValue)
        }
      }
    },
    endMove (event) {
      const {
        $refs: { wrap },
      } = this
      if (wrap.hasPointerCapture(event.pointerId)) {
        wrap.releasePointerCapture(event.pointerId)
        this.$emit('change', this.innerData.filter(e => e.value > 0))
      }
    },
    handleSplitChange (startIndex, movingValue) {
      const copyData = _.cloneDeep(this.innerData)
      if (startIndex > 0) {
        // 拖拽非首位
        const currentItem = copyData[startIndex] // 拖拽条右侧的灯态
        const prevItem = copyData[startIndex - 1] // 拖拽条左侧的灯态
        if (this.startItem.color === currentItem.color) {
          const currentStartValue = getStartValue(copyData, startIndex) // 拖拽条左侧总长度
          let offset = movingValue - currentStartValue // 已拖动距离
          if (offset < 0) { // 向左可减少左侧灯态时长，若左侧灯态非首个则需保留1s
            const maxOffset = startIndex === 1 ? -prevItem.value : -prevItem.value + 1
            offset = Math.max(offset, maxOffset)
          } else if (offset > 0) { // 向右可增加左侧灯态时长，若右侧灯态非末尾则需保留1s
            const maxOffset = currentItem.value - 1
            offset = Math.min(offset, maxOffset)
          }
          if (offset === 0) return
          currentItem.value -= offset
          prevItem.value += offset
        }
      } else {
        // 拖拽条为首位
        const firstItem = copyData[0] // 首位灯态
        const lastItem = copyData[copyData.length - 1] // 末尾灯态
        let offset = movingValue
        if (firstItem.color !== lastItem.color) {
          // 首尾灯色不一致，向左无变化，向右增加一段尾灯颜色
          if (offset > 0) {
            const currentItem = copyData[startIndex] // 拖拽条右侧的灯态
            const tempItem = { color: lastItem.color, value: 0 } // 拖拽条左侧的灯态
            const maxValue = currentItem.value - 1
            offset = Math.min(offset, maxValue)
            currentItem.value -= offset
            tempItem.value += offset
            copyData.unshift(tempItem)
            this.startValue = 1
          }
        } else if (copyData.length > 1) {
          // 首尾灯色一致，向左则减少右侧灯态时长，向右则增加右侧灯态时长减少
          if (offset < 0) {
            const currentItem = copyData[startIndex] // 首个灯态
            const nextItem = copyData[startIndex + 1] // 第二个灯态
            const maxValue = -currentItem.value
            offset = Math.max(offset, maxValue)
            currentItem.value += offset
            nextItem.value -= offset
            this.startValue = Math.max(0, currentItem.value)
          } else if (movingValue > 0) {
            const currentItem = copyData[startIndex] // 首个灯态
            const nextItem = copyData[startIndex + 1] // 第二个灯态
            const maxValue = currentItem.value + nextItem.value - 1
            const value = Math.min(this.startValue + movingValue, maxValue)
            currentItem.value = value
            nextItem.value = maxValue + 1 - value
          }
        }
      }
      // console.log(JSON.stringify(copyData))
      this.innerData = copyData
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.color-slider {
  height: 32px;
  display: flex;
  position: relative;
  z-index: 1;
  user-select: none;

  &.disabled {
    cursor: not-allowed;

    .color-split {
      cursor: not-allowed;
    }
  }

  .color-cell {
    .text-overflow;

    box-sizing: border-box;
    flex: 0 0 0;
    height: 32px;
    line-height: 32px;
    color: #fff;
    text-align: center;
    box-shadow: inset 0 0 0 2px rgba(0, 0, 0, .2);
    &.active {
      box-shadow: inset 0 0 0 2px rgb(38, 209, 239);
    }
    &.green {
      background-color: #079D4B;
    }
    &.yellow {
      background-color: #E39400;
    }
    &.red {
      background-color: #8E0015;
    }
    &.occupancy {
      background-color: gray;
    }
  }

  .color-split {
    width: 2px;
    height: 32px;
    position: absolute;
    z-index: 2;
    cursor: col-resize;
    transform: translate(-50%, 0);

    &.disabled {
      cursor: default;
    }
  }
}
</style>
