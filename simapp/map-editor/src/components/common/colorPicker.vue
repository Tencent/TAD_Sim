<script setup lang="ts">
import { delay } from 'lodash'
import { ref, watch } from 'vue'

const props = defineProps({
  displayColor: {
    type: String,
    default: '',
  },
  predefine: {
    type: Object,
    default () {
      return {}
    },
  },
})
const emit = defineEmits(['color-change'])
const visible = ref(false)
const currentColor = ref('')
watch(
  () => props.displayColor,
  (val) => {
    if (props.predefine[val]) {
      currentColor.value = props.predefine[val].string
    }
  },
  // 立即将更新的属性，同步到样式中
  { immediate: true },
)

function pickColor (colorName: string) {
  emit('color-change', colorName)
}
function onButtonBlur () {
  // 将窗口关闭的延时调整到 200ms，保证 ubuntu 环境能够稳定切换颜色
  delay(() => {
    visible.value = false
  }, 200)
}
</script>

<template>
  <el-popover
    popper-class="color-picker"
    trigger="click"
    placement="bottom"
    :visible="visible"
    transition="el-zoom-in-top"
    :popper-style="{ 'width': '92px', 'min-width': '92px' }"
  >
    <ul class="color-ul">
      <li
        v-for="colorOption of props.predefine"
        :key="colorOption.name"
        class="color-li"
        :style="{ 'background-color': colorOption.string }"
        @click="pickColor(colorOption.name)"
      />
    </ul>
    <template #reference>
      <el-button
        class="current-color"
        :style="{ 'background-color': currentColor }"
        @click="visible = true"
        @blur="onButtonBlur"
      />
    </template>
  </el-popover>
</template>

<style scoped lang="less">
.color-picker {
  .color-ul {
    line-height: 0;
    .color-li {
      display: inline-block;
      width: 80px;
      height: 24px;
      box-sizing: border-box;
      cursor: pointer;

      &:hover {
        border: 1px solid #16d1f3;
      }
    }
  }
}
.property-container {
  .el-button.current-color {
    width: 80px;
    height: 24px;
  }
}
</style>
