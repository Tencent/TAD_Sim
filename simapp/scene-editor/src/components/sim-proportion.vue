<template>
  <div class="sim-proportion">
    <ul ref="track" class="list">
      <li
        v-for="(item, index) in items"
        :key="item.id"
        class="item"
        :style="{ 'background-color': colors[index % colors.length], 'width': `${item.weight * 100 / total}%` }"
      >
        <el-tooltip placement="top" :content="item.name" :enterable="false" :disabled="item.weight > 10">
          <span>{{ item.name }}</span>
        </el-tooltip>
        <div
          v-if="index !== items.length - 1"
          class="slider"
          @mousedown="dragStart($event, item, index)"
        />
      </li>
    </ul>
  </div>
</template>

<script>
let moveStartX = 0
let vm = null
let leftItem = null
let rightItem = null
let leftItemOldWeight = 0
let rightItemOldWeight = 0
let mouseupHandler = null
const colors = ['#364a8a', '#2c758d', '#2b8646', '#9d8333']

function isDragStart () {
  return leftItem !== null && rightItem !== null
}

function mousemove (evt) {
  if (isDragStart() && evt.which === 1) {
    const distanceX = evt.pageX - moveStartX
    let distanceWeight = Math.round((distanceX / vm.$refs.track.clientWidth) * vm.total / vm.step) * vm.step
    if (distanceWeight < 0 - leftItemOldWeight) {
      distanceWeight = 0 - leftItemOldWeight
    }
    if (distanceWeight > rightItemOldWeight) {
      distanceWeight = rightItemOldWeight
    }
    // 调整权重
    leftItem.weight = leftItemOldWeight + distanceWeight
    rightItem.weight = rightItemOldWeight - distanceWeight
  }
}

function mouseup (evt) {
  if (isDragStart() && evt.which === 1) {
    moveStartX = 0
    vm = null
    leftItem = null
    rightItem = null
    leftItemOldWeight = 0
    rightItemOldWeight = 0
    mouseupHandler()
  }
}

/**
 * @module proportion
 * @desc 比例调整组件
 */
export default {
  name: 'SimProportion',
  props: {
    method: {
      type: String,
      default: 'next',
      validator (val) {
        return val === 'next' || val === 'last'
      },
    },
    step: {
      type: Number,
      default: 1,
    },
    items: {
      type: Array,
      default: () => [],
    },
    colors: {
      type: Array,
      default: () => colors,
    },
  },
  data () {
    return {
      zIndex: 1,
    }
  },
  computed: {
    total () {
      return this.items.reduce((prev, item) => prev + item.weight, 0)
    },
  },
  created () {
    // 绑定在body上的作用是当鼠标move和up时触发时不在down的元素上时依然能拖动
    document.body.addEventListener('mousemove', mousemove)
    document.body.addEventListener('mouseup', mouseup)
  },
  beforeUnmount () {
    // 销毁时需要移除事件，否则存在内存泄漏
    document.body.removeEventListener('mouseup', mouseup)
    document.body.removeEventListener('mousemove', mousemove)
  },
  methods: {
    dragStart (evt, item, index) {
      // 鼠标左键
      if (evt.which === 1) {
        const isNext = this.method === 'next'
        const items = this.items
        moveStartX = evt.pageX
        vm = this
        leftItem = item
        rightItem = isNext ? items[index + 1] : items[items.length - 1]
        leftItemOldWeight = leftItem.weight
        rightItemOldWeight = rightItem.weight

        const cursor = evt.target.style.cursor
        evt.target.style.cursor = 'grabbing'
        evt.target.style.zIndex = this.zIndex++
        mouseupHandler = function () {
          evt.target.style.cursor = cursor
        }
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

@slider-height: 20px;
.sim-proportion {
  display: inline-block;
  width: 100%;
  height: @slider-height;

  .list {
    width: 100%;
    height: @slider-height;
    list-style: none;
    position: relative;

    &:after,
    &::after {
      content: '';
      display: block;
      clear: both;
    }

    .item {
      float: left;
      color: white;
      height: @slider-height;
      position: relative;
      font-size: 12px;
      font-weight: 300;

      span {
        display: block;
        height: @slider-height;
        line-height: @slider-height;
        text-align: center;
        .text-overflow;
      }

      .slider {
        @slider-width: 12px;
        position: absolute;
        width: @slider-width;
        height: @slider-width/2;
        right: 0;
        margin-right: -@slider-width/2;
        top: 100%;
        margin-top: @slider-width/2;
        cursor: grab;
        user-select: none;

        background-color: @global-font-color;
        z-index: 0;

        &:hover,
        &:focus,
        &:active {
          background-color: @active-font-color;
          z-index: 1;

          &:after,
          &::after {
            border-bottom-color: @active-font-color;
          }
        }

        &:after,
        &::after {
          content: '';
          display: block;
          width: 0;
          height: 0;
          margin-top: -@slider-width;
          border: solid transparent @slider-width/2;
          border-bottom-color: @global-font-color;
        }
      }
    }
  }
}
</style>
