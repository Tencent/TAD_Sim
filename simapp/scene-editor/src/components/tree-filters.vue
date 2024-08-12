<template>
  <el-popover
    ref="pop"
    trigger="click"
    placement="bottom-start"
    popper-class="tree-filters-pop"
    :visible-arrow="false"
    transition="el-zoom-in-top"
    v-bind="$attrs"
  >
    <template #reference>
      <slot name="reference">
        <div class="tree-filter-reference">
          <span class="tree-filters-label" :class="{ active: checkedNodes.length > 0 }">{{ label }}&nbsp;</span>
          <el-icon class="el-icon-arrow-down">
            <arrow-down />
          </el-icon>
        </div>
      </slot>
    </template>
    <div class="tree-filters">
      <el-tree
        ref="tree"
        class="tree-filters-tree"
        :data="options"
        node-key="id"
        show-checkbox
        :default-expanded-keys="['all']"
        v-bind="treeOptions"
        @check="checkHandler"
      />
      <div class="tree-filter-btns">
        <button @click="confirmHandler">
          {{ confirmText || $t('operation.filter') }}
        </button>
        <button @click="resetHandler">
          {{ $t('operation.reset') }}
        </button>
      </div>
    </div>
  </el-popover>
</template>

<script>
export default {
  name: 'TreeFilters',
  props: {
    data: {
      type: Array,
      required: true,
    },
    label: {
      type: String,
      required: true,
    },
    confirmText: {
      type: String,
      default: '',
    },
    withRootNode: {
      type: Boolean,
      default: true,
    },
    treeOptions: {
      type: Object,
      default: () => ({}),
    },
  },
  data () {
    return {
      checkedNodes: [],
    }
  },
  computed: {
    options () {
      if (!this.withRootNode) return this.data
      return [
        {
          id: 'all',
          label: this.$t('all'),
          children: this.data,
          level: 0,
        },
      ]
    },
  },
  methods: {
    checkHandler (node, { checkedNodes }) {
      this.checkedNodes = checkedNodes
    },
    confirmHandler () {
      this.$refs.pop.hide()
      this.$emit('filter-change', this.checkedNodes)
    },
    resetHandler () {
      this.$refs.pop.hide()
      this.$refs.tree.setCheckedNodes([])
      this.checkedNodes = []
      this.$emit('filter-change', this.checkedNodes)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .tree-filters-label {
    font-size: 12px;
    color: #909399;
    font-weight: bold;

    &.active {
      color: @active-font-color;
    }
  }

  .tree-filters {
    font-size: 12px;
    font-weight: normal;

    .tree-filters-tree {
      padding: 8px;
      max-height: 200px;
      overflow-x: hidden;
      overflow-y: auto;
      -webkit-overflow-scrolling: touch;
      background: transparent;
    }

    .tree-filter-btns {
      border-top: 1px solid #404040;
      padding: 8px;

      button {
        background: 0 0;
        border: none;
        color: #606266;
        cursor: pointer;
        font-size: 13px;
        padding: 0 3px;

        &.disabled {
          color: #C0C4CC;
          cursor: not-allowed;
        }

        &:hover {
          color: @active-font-color;
        }

        &:focus {
          outline: 0;
        }
      }
    }
  }
</style>

<style lang="less">
  .tree-filters-pop {
    padding: 0;
  }
</style>
