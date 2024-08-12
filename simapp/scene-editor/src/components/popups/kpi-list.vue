<template>
  <div v-loading="loading" class="kpi-list">
    <section class="part-search">
      <el-select
        id="kpi-category-list-select"
        v-model="categoryId"
      >
        <el-option
          v-for="item in specialKpiCategoryOptions"
          :id="`kpi-category-list-option-${item.name}`"
          :key="item.id"
          :label="$t(item.name)"
          :value="item.id"
        />
      </el-select>
      <el-input
        id="kpi-name"
        v-model="keyword"
        style="width: 200px;"
        prefix-icon="search"
        :placeholder="($t('operation.searchName'))"
        class="search-word"
      />
    </section>

    <section class="part-details">
      <el-table
        id="kpi-list-table"
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
            <span>{{ $kpiAlias(scope.row) }}</span>
          </template>
        </el-table-column>
        <el-table-column
          :label="$t('classification')"
          class-name="table-row"
          width="120"
          show-overflow-tooltip
        >
          <template #default="scope">
            {{ getCategoryTxt(scope.row) }}
          </template>
        </el-table-column>
      </el-table>
    </section>

    <section class="part-buttons">
      <el-button id="kpi-list-cancel" class="dialog-cancel" @click="$emit('close')">
        {{
          $t('operation.cancel')
        }}
      </el-button>
      <el-button id="kpi-list-confirm" class="dialog-cancel" @click="handleConfirm">
        {{
          $t('operation.add')
        }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { filter, find, forEach } from 'lodash-es'
import { mapActions, mapState } from 'vuex'
import { errorHandler } from '@/common/errorHandler'
import KpiMixin from '@/components/popups/kpi-form-new/mixin'

export default {
  name: 'KpiList',
  mixins: [
    KpiMixin,
  ],
  props: {
    selected: {
      type: Array,
      default () {
        return []
      },
    },
  },
  data () {
    return {
      loading: false,
      keyword: '',
      selectedList: [],
      categoryId: -1,
    }
  },
  computed: {
    ...mapState('kpi', [
      'kpiList',
    ]),
    specialKpiCategoryOptions () {
      return this.kpiCategoryOptions.concat([{ id: '未分类', name: 'kpi.noCategory' }])
    },
    sortedKpiList () {
      const kpiList = []
      kpiList.push(...this.kpiList)
      return kpiList
    },
    list () {
      const list = this.sortedKpiList.filter((item) => {
        if (this.categoryId === -1) {
          return true
        } else if (this.categoryId === '未分类') {
          const labels = item.category?.labels || []
          return labels.length === 0
        } else {
          const labels = item.category?.labels || []
          return labels.includes(this.categoryId)
        }
      }).filter((item) => {
        if (this.keyword) {
          const kpiName = this.$kpiAlias(item)
          return kpiName.toLowerCase().includes(this.keyword.toLowerCase())
        } else {
          return true
        }
      })
      console.log('list', list)
      return list
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
      await this.getKpiList()
    } catch (e) {
      await errorHandler(e)
    }
    this.loading = false
  },
  methods: {
    ...mapActions('kpi', [
      'getKpiList',
    ]),
    canSelect (row) {
      return !find(this.selected, { name: row.name })
    },
    setSelected () {
      forEach(this.list, (item) => {
        const included = !this.canSelect(item)
        if (included) {
          this.$refs.table.toggleRowSelection(item, true)
        }
      })
    },
    handleConfirm () {
      const payload = filter(this.selectedList, (item) => {
        return this.canSelect(item)
      })
      this.$emit('confirm', payload)
      this.$emit('close')
    },
    tableSelectionChange (selection) {
      this.selectedList = selection
    },
    setRowClassName ({ row }) {
      return this.selectedList.findIndex(item => item.name === row.name) > -1 ? 'selected-row' : ''
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.kpi-list {
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
      td {
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
