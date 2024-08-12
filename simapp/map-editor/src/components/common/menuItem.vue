<script setup lang="ts">
import { ref, watch } from 'vue'

const props = defineProps({
  title: {
    type: String,
    default: '',
  },
  hotkey: {
    type: String,
    default: '',
  },
  disabled: {
    type: Boolean,
    default: false,
  },
})
const emit = defineEmits(['trigger'])
const itemDisabled = ref(props.disabled)
watch(
  () => props.disabled,
  (val) => {
    itemDisabled.value = val
  },
  {
    immediate: true,
  },
)

function handleClick (event: MouseEvent) {
  if (itemDisabled.value) {
    // 如果当前选项是 disabled 状态，则点击事件不冒泡给上层的元素
    event.stopPropagation()
  } else {
    emit('trigger')
  }
}
</script>

<template>
  <div
    class="menu-item"
    :class="{ disabled: itemDisabled }"
    @click="handleClick"
  >
    <label class="title">{{ props.title }}</label>
    <span class="hotkey">{{ props.hotkey }}</span>
  </div>
</template>

<style scoped lang="less">
.menu-item {
  padding: 0 8px;
  color: var(--text-color);
  display: flex;
  justify-content: space-between;
  height: 30px;
  line-height: 30px;
  white-space: nowrap;
  font-size: var(--font-size);
  cursor: pointer;

  &:hover {
    background-color: var(--main-area-color);
    color: var(--active-color);
  }
  &.disabled {
    background-color: inherit;
    color: gray;
    cursor: auto;
  }
}
</style>
