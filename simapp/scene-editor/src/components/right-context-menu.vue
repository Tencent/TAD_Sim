<template>
  <el-popover
    ref="popover"
    v-model:visible="visible"
    trigger="manual"
    width="100px"
    placement="bottom"
    popper-class="context-menu-popover"
  >
    <template #reference>
      <span
        ref="trigger"
        v-bind="$attrs"
        @contextmenu.prevent="handleContextMenu"
      >
        <slot />
      </span>
    </template>
    <div ref="contextMenu" class="context-menu">
      <ul v-if="visible" class="context-menu-list">
        <li
          v-for="op in options"
          :key="op.id"
          class="context-menu-item"
          :class="{ 'has-children': op.children && op.children.length }"
        >
          <slot name="menu-item" :item="op">
            <div class="menu-content" :class="{ disabled: op.disabled }" @click="handleClickItem(op)">
              <span class="name">{{ op.label }}</span>
              <el-icon v-if="op.children && op.children.length" class="el-icon-arrow-right" style="margin-left: auto">
                <arrow-right />
              </el-icon>
            </div>
          </slot>
          <ul
            v-if="op.children && op.children.length"
            class="context-menu-list sub-menu"
          >
            <li v-for="subOp in op.children" :key="subOp.id" class="context-menu-item sub-menu">
              <slot name="sub-menu-item" :item="subOp">
                <div class="menu-content" :class="{ disabled: subOp.disabled }" @click="handleClickItem(subOp)">
                  <span class="name">{{ subOp.label }}</span>
                </div>
              </slot>
            </li>
          </ul>
        </li>
      </ul>
    </div>
  </el-popover>
</template>

<script>
export default {
  props: {
    options: {
      type: Array,
      default: () => [],
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      visible: false,
    }
  },
  mounted () {
    window.addEventListener('click', this.clickOutside)
    window.addEventListener('contextmenu', this.clickOutside)
  },
  beforeUnmount () {
    window.removeEventListener('click', this.clickOutside)
    window.removeEventListener('contextmenu', this.clickOutside)
  },
  methods: {
    // 右键，显示菜单
    handleContextMenu () {
      if (this.disabled) return
      this.visible = true
    },
    // 点击菜单项
    handleClickItem (item) {
      this.$emit('command', item)
      this.visible = false
    },
    // 若点击按钮或菜单以外的地方，则隐藏菜单
    clickOutside (e) {
      const $menu = this.$refs.contextMenu
      const $button = this.$refs.trigger
      if ($menu && $button) {
        const matchButton = this.domContains($button, e.target) // 触发按钮
        const matchMenu = this.domContains($menu, e.target) // 右键菜单
        if (!(matchButton || matchMenu)) {
          this.visible = false
        }
      }
    },
    // 判断 $trigger 是否在 $contains 内
    domContains ($contains, $trigger) {
      let $dom = $trigger
      while ($dom && $dom !== document) {
        if ($dom === $contains) return true
        $dom = $dom.parentNode
      }
      return false
    },
  },
}
</script>

<style lang="less">
@import "@/assets/less/mixins";

.el-popper.el-popover.context-menu-popover {
  --el-popover-padding: 0 0;
  font-size: 12px;
  min-width: auto;

  &[x-placement^=bottom] {
    margin-top: 2px;
  }

  &[x-placement^=bottom] .popper__arrow::after {
    border-bottom-color: @list-bd;
  }
}

.context-menu-list {
  margin: 0 0;
  padding: 4px 0;
  list-style: none;
  color: @global-font-color;

  &.sub-menu {
    position: absolute;
    left: 100%;
    top: 0;
    margin-left: -2px;
    opacity: 0;
    pointer-events: none;
    width: 80px;
    background: var(--el-bg-color-overlay);
    border: 1px solid var(--el-border-color-light);
  }
}

.context-menu-item {
  position: relative;
  padding: 4px 12px;

  &:hover {
    background: @hover-bg;
    color: @active-font-color;
  }

  &:hover > .context-menu-list.sub-menu {
    opacity: 1;
    pointer-events: auto;
  }
}

.menu-content {
  display: flex;
  align-items: center;
  cursor: pointer;

  &.disabled {
    color: @disabled-color;
    pointer-events: none;
  }

  & > .name {
    flex-grow: 1;
    .text-overflow();
  }

  & > .el-icon-arrow-right {
    position: relative;
    left: 4px;
  }
}
</style>
