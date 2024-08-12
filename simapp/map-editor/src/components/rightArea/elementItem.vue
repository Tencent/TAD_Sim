<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import ArrowSvg from '@/assets/icons/arrow.svg?component'
import { useHelperStore } from '@/stores/helper'
import { useRoadInteractionStore } from '@/plugins/editRoad/store/interaction'
import { useJunctionStore } from '@/stores/junction'
import { useObjectStore } from '@/stores/object'
import { useJunctionInteractionStore } from '@/plugins/editJunction/store/interaction'
import { useObjectInteractionStore } from '@/plugins/editObject/store/interaction'

const props = defineProps({
  isActive: {
    type: Boolean,
    required: true,
  },
  icon: {
    type: Object,
    required: true,
  },
  name: {
    type: String,
    required: true,
  },
  hasChild: {
    type: Boolean,
    default: false,
  },
  id: {
    type: String,
    required: true,
  },
})

const helperStore = useHelperStore()
const roadInteractionStore = useRoadInteractionStore()
const junctionStore = useJunctionStore()
const junctionInteractionStore = useJunctionInteractionStore()
const objectStore = useObjectStore()
const objectInteractionStore = useObjectInteractionStore()

const isExpand = ref(true)
const childrenHeight = ref('auto')
const isSelected = ref(false)

// 当前选中的元素
const currentSelected = computed(() => {
  return helperStore.selectedElement
})

watch(
  currentSelected,
  (val) => {
    // 判断当前选中的元素是否是自身
    if (val === props.id) {
      isSelected.value = true
    } else {
      isSelected.value = false
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

function toggleExpand () {
  isExpand.value = !isExpand.value
  if (isExpand.value) {
    childrenHeight.value = 'auto'
  } else {
    childrenHeight.value = '0px'
  }
}

// 元素选中
function onItemClick (id: string, event: MouseEvent) {
  event.preventDefault()

  // 设置当前元素为选中状态
  helperStore.setSelectedElement({
    element: id,
    focus: true, // 选中需要调整视角
    emitEvent: true, // 触发编辑模式中的元素选中
  })
}

// 鼠标右键弹出删除菜单框，点击可以确认删除元素
function onContextMenu (id: string, event: MouseEvent) {
  helperStore.showContextMenu({
    element: id,
    left: event.pageX,
    top: event.pageY,
  })
}
</script>

<template>
  <div class="element-item">
    <div
      class="item-info"
      :class="{ selected: isSelected }"
      @click.stop="onItemClick(props.id, $event)"
      @contextmenu.prevent.stop="onContextMenu(props.id, $event)"
    >
      <component :is="icon" class="element-icon" />
      <label class="element-name">{{ props.name }}</label>
      <span
        v-show="props.hasChild"
        class="arrow-container"
        @click.stop="toggleExpand"
      >
        <ArrowSvg class="arrow-icon" :class="{ folded: !isExpand }" />
      </span>
    </div>
    <div class="children-content" :style="{ height: childrenHeight }">
      <slot />
    </div>
  </div>
</template>

<style scoped lang="less">
.element-item {
  padding-left: 12px;

  .item-info {
    min-height: 2em;
    position: relative;
    display: flex;
    align-items: center;

    &.selected {
      background-color: var(--enable-select-background);
    }

    .element-icon {
      flex: none;
    }
    .element-name {
      flex: auto;
      margin-left: 6px;
    }

    .arrow-container {
      display: inline-block;
      width: 12px;
      height: 12px;
      margin-right: 4px;
      .arrow-icon {
        flex: none;
        cursor: pointer;
        transform: rotateZ(0deg);

        &.folded {
          transform: rotateZ(-90deg);
        }
      }
    }
  }
  .children-content {
    overflow: hidden;
  }
}
</style>
