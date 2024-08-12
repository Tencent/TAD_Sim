<template>
  <div v-loading="loading" class="module-list">
    <section class="part-search">
      <el-select
        id="module-category-list-select"
        v-model="categoryId"
      >
        <el-option
          v-for="item in computedCategoryList"
          :id="`module-category-list-option-${item.name}`"
          :key="item.id"
          :label="$t(item.name)"
          :value="item.id"
        />
      </el-select>
      <el-input
        id="module-name"
        v-model="keyword"
        style="width: 200px;"
        prefix-icon="search"
        :placeholder="($t('operation.searchName'))"
        class="search-word"
      />
    </section>

    <section class="part-details">
      <el-table
        id="module-list-table"
        ref="table"
        size="small"
        :data="list"
        height="300"
        row-key="name"
        :row-class-name="setRowClassName"
        @selection-change="tableSelectionChange"
      >
        <el-table-column
          type="selection"
          label-class-name="select-all"
          width="36"
          reserve-selection
          :selectable="canSelect"
        />
        <el-table-column
          :label="$t('name')"
          class-name="table-row"
          show-overflow-tooltip
        >
          <template #default="scope">
            <span>{{ scope.row.name }}</span>
            <span v-if="isPreset(scope.row.name)" class="preset-icon">{{ $t('preset') }} </span>
          </template>
        </el-table-column>
        <el-table-column
          :label="$t('classification')"
          class-name="table-row"
          width="120"
          show-overflow-tooltip
        >
          <template #default="scope">
            {{ $t(category[scope.row.category]) }}
          </template>
        </el-table-column>
      </el-table>
    </section>

    <section class="part-buttons">
      <el-button id="module-list-cancel" class="dialog-cancel" @click="$emit('close')">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button id="module-list-confirm" class="dialog-cancel" @click="handleConfirm">
        {{ $t('operation.add') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import dict from '@/common/dict'
import { errorHandler } from '@/common/errorHandler'

const { electron } = window

export default {
  name: 'ModuleList',
  props: {
    selected: {
      type: Array,
      default () {
        return []
      },
    },
    // false主车算法，true全局模块
    global: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      loading: false,
      keyword: '',
      selectedKeys: [],
      categoryId: -1,
    }
  },
  computed: {
    ...mapState('sim-modules', {
      moduleList: 'modules',
    }),
    categoryList () {
      if (this.global) return dict.moduleGlobalCategoryList
      return dict.moduleCategoryList
    },
    category () {
      return this.categoryList.reduce((map, item) => {
        return { ...map, [item.id]: item.name }
      }, {})
    },
    computedCategoryList () {
      return [
        { id: -1, name: 'indicator.AllClass' },
        ...this.categoryList,
      ]
    },
    sortedModuleList () {
      const moduleList = []
      _.forEach(this.categoryList, (category) => {
        _.forEach(this.moduleList, (item) => {
          if (item.category === category.id) {
            moduleList.push(item)
          }
        })
      })
      return moduleList
    },
    list () {
      return this.sortedModuleList.filter((item) => {
        if (this.categoryId === -1) {
          return true
        } else {
          return this.categoryId === item.category
        }
      }).filter((item) => {
        return item.name.includes(this.keyword)
      })
    },
  },
  watch: {
    list () {
      this.$nextTick(() => {
        this.setSelected()
      })
    },
  },
  async created () {
    this.loading = true
    try {
      this.getModuleList()
    } catch (e) {
      await errorHandler(e)
    }
    this.loading = false
  },
  methods: {
    ...mapActions('sim-modules', [
      'getModuleList',
    ]),
    isPreset (name) {
      return electron.modules.isPreset(name)
    },
    canSelect (row) {
      return !_.includes(this.selected, row.name)
    },
    setSelected () {
      _.forEach(this.list, (item) => {
        const included = !this.canSelect(item)
        if (included) {
          this.$refs.table.toggleRowSelection(item, true)
        }
      })
    },
    handleConfirm () {
      const payload = _.filter(this.selectedKeys, (item) => {
        return this.canSelect({ name: item })
      })
      this.$emit('confirm', payload)
      this.$emit('close')
    },
    tableSelectionChange (selection) {
      this.selectedKeys = selection.map(item => item.name)
    },
    setRowClassName ({ row }) {
      return this.selectedKeys.findIndex(key => row.name === key) > -1 ? 'selected-row' : ''
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .module-list {
    .part-search {
      text-align: center;
      margin-top: 10px;
      margin-bottom: 23px;

      .el-select {
        width: 200px;
      }
    }

    .part-details {
      background-color: #111;
    }

    .part-buttons {
      margin-top: 10px;
      height: 40px;
      line-height: 40px;
      text-align: right;
    }

    .preset-icon {
      margin-left: 10px;
      padding: 0 2px;
      font-size: 10px;
      color: @disabled-color;
      border: 1px solid @disabled-color;
      border-radius: 1px;
    }

    .el-table {
      color: @disabled-color;

      :deep(.sim-button--text) {
        padding: 3px 8px 3px 0;

        &:last-child {
          padding-right: 0;
        }
      }

      :deep(.el-table-column--selection) {
        text-align: center;
      }

      :deep(.table-operations) {
        color: @disabled-color;
      }

      :deep(.el-table__row) {
        td{
          color: @title-font-color;
        }
        &:hover {
          .table-operations {
            color: #fff;
          }
        }
      }

      :deep(.selected-row) {
        color: @active-font-color;
      }
    }
  }
</style>
