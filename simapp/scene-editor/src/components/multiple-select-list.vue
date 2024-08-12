<template>
  <div ref="wrapper" class="multiple-select-list">
    <ul>
      <li
        v-for="(item, index) of data"
        :key="index"
        :class="liClass"
        @click="select($event, item, index)"
        @click.right.stop="contextMenuHandler($event, item, index)"
      >
        <slot v-bind="{ row: item, $index: index }" />
      </li>
    </ul>
    <ul
      v-show="contextMenuVisible"
      ref="contextMenu"
      class="list-context-menu"
      :style="contextMenuPosition"
    >
      <li
        v-for="item of menus"
        :key="item.key"
        class="list-context-item"
        @click="$emit('command', item.key)"
      >
        {{ item.name }}
      </li>
    </ul>
  </div>
</template>

<script>
export default {
  name: 'MultipleSelectList',
  model: {
    event: 'change',
  },
  props: {
    data: {
      type: Array,
      required: true,
    },
    liClass: {
      type: String,
      default: '',
    },
    value: {
      type: Array,
      required: true,
    },
    rowKey: {
      type: String,
      // 如果是 $index 则以index为索引
      default: '$index',
    },
    menus: {
      type: Array,
      default: () => [],
    },
  },
  data () {
    return {
      lastIndex: 0,
      contextMenuVisible: false,
      contextMenuPosition: {
        top: '0',
        left: '0',
      },
    }
  },
  watch: {
    data () {
      this.lastIndex = 0
    },
  },
  mounted () {
    document.addEventListener('click', this.hideContextMenu)
  },
  beforeUnmount () {
    document.removeEventListener('click', this.hideContextMenu)
  },
  methods: {
    select ($event, item, index) {
      const {
        data,
        value,
        rowKey,
        lastIndex,
      } = this
      let idx
      if (rowKey === '$index') {
        idx = index
      } else {
        idx = item[rowKey]
      }
      let result = []
      if ($event.shiftKey) {
        const [min, max] = [index, lastIndex].sort()
        if (rowKey === '$index') {
          for (let i = min; i < max + 1; i++) {
            result.push(i)
          }
        } else {
          result = data.slice(min, max + 1).map(d => d[rowKey])
        }
      } else {
        this.lastIndex = index
        if ($event.ctrlKey) {
          if (value.includes(idx)) {
            result = value.filter(id => id !== idx)
          } else {
            result = [...value, idx]
          }
        } else {
          result.push(idx)
        }
      }
      this.$emit('change', result)
    },
    contextMenuHandler ($event, item, index) {
      const {
        $refs: { wrapper },
        value,
        rowKey,
      } = this

      let idx
      if (rowKey === '$index') {
        idx = index
      } else {
        idx = item[rowKey]
      }

      if ($event.shiftKey) {
        this.select($event, item, index)
      } else if ($event.ctrlKey) {
        if (!value.includes(idx)) {
          this.$emit('change', [...value, idx])
        }
      } else {
        if (value.length !== 1 || value[0] !== idx) {
          this.$emit('change', [idx])
        }
      }
      this.contextMenuVisible = true
      // wrapper is contextMenu's offsetParent
      const { top, left } = wrapper.getBoundingClientRect()
      const x = $event.pageX - left + 2
      const y = $event.pageY - top + 2
      this.contextMenuPosition.top = `${y}px`
      this.contextMenuPosition.left = `${x}px`
    },
    hideContextMenu () {
      this.contextMenuVisible = false
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .multiple-select-list {
    user-select: none;
    position: relative;
    z-index: 1;
  }

  .list-context-menu {
    position: absolute;
    top: 0;
    left: 0;
    z-index: 2;

    .list-context-item {
      line-height: 28px;
      padding: 0 12px;
      width: 60px;
      font-size: 14px;
      background-color: @hover-bg;
      color: @global-font-color;
      cursor: pointer;

      &:hover {
        color: @active-font-color;
      }
    }
  }
</style>
