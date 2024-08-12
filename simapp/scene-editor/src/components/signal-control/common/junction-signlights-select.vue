<template>
  <div class="junction-signlights-select" :class="{ disabled }">
    <TreeFilters
      ref="tree"
      :label="$t('result')"
      :data="roadLightTree"
      :with-root-node="false"
      :confirm-text="$t('operation.ok')"
      :tree-options="{
        'node-key': 'id',
        'default-expand-all': true,
      }"
      :disabled="disabled"
      @filter-change="handleTreeChange"
    >
      <template #reference>
        <div class="select-value">
          <template v-if="value.length">
            <el-tag
              v-for="tag in tags"
              :key="tag.id"
              size="small"
              closable
              disable-transitions
              @close="handleTagClose(tag)"
            >
              {{ tag.label }}
            </el-tag>
          </template>
          <template v-else>
            <span class="no-data">暂无</span>
          </template>
        </div>
      </template>
    </TreeFilters>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'
import TreeFilters from '@/components/tree-filters.vue'

export default {
  components: {
    TreeFilters,
  },
  props: {
    value: {
      type: Array,
      default: () => [],
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {}
  },
  computed: {
    ...mapGetters('signalControl', [
      'roadLightTree',
    ]),
    tags () {
      return this.value.map(id => ({ id, label: `light ${id}` }))
    },
  },
  watch: {
    value () {
      this.triggerDefaultKeys()
    },
    roadLightTree () {
      this.triggerDefaultKeys()
    },
  },
  mounted () {
    this.triggerDefaultKeys()
  },
  methods: {
    triggerDefaultKeys () {
      const $tree = this.$refs.tree?.$refs?.tree
      if (!$tree) return
      $tree.setCheckedKeys(this.value || [])
      this.$nextTick(() => {
        this.$refs.tree.checkedNodes = $tree.getCheckedNodes() || []
      })
    },
    handleTreeChange (selected) {
      const ids = selected.filter(item => !item.children).map(item => item.id)
      this.$emit('changed', ids, selected)
    },
    handleTagClose (tag) {
      const newValue = this.value.filter(id => id !== tag.id)
      const ids = newValue.map(item => item.id)
      this.$emit('changed', ids, newValue)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.junction-signlights-select {
  &.disabled {
    .select-value {
      cursor: not-allowed;
    }
  }
}

.select-value {
  background-color: #0f0f0f;
  border: 1px solid #404040;
  padding: 4px 6px;
  width: 6em;
  height: 32px;
  box-sizing: border-box;
  display: flex;
  align-items: center;
  overflow: hidden;
  color: #aaaaaa;
  font-size: 10px;
  cursor: pointer;

  :deep(.el-tag) {
    background-color: #212121;
    border-color: #303030;
    color: #b0b0b0;
  }

  :deep(.el-tag__close) {
    color: #b0b0b0;

    &:hover {
      color: #b0b0b0;
      background-color: #303030;
    }
  }

  .no-data {
    line-height: 22px;
  }
}
</style>
