<template>
  <div v-loading="loading" class="map-management">
    <section class="part-search">
      <el-input
        v-model="searchWord"
        style="width: 200px;"
        prefix-icon="el-icon-search"
        :placeholder="($t('operation.searchName'))"
        class="search-word"
      />
    </section>
    <section class="part-details">
      <el-table
        ref="table"
        size="mini"
        :data="list"
        height="320"
        row-key="id"
        @selection-change="tableSelectionChange"
        @row-click="rowClickHandler"
      >
        <el-table-column type="selection" label-class-name="select-all" width="36" />
        <el-table-column
          :label="$t('name')"
          prop="name"
          class-name="table-row"
          show-overflow-tooltip
        >
          <template #default="scope">
            <div class="map-item">
              <span>{{ scope.row.name }}</span>
            </div>
          </template>
        </el-table-column>
        <el-table-column
          :label="$t('scenario.size')"
          prop="size"
          class-name="table-row"
          width="80"
          show-overflow-tooltip
        >
          <template #default="scope">{{ getSize(scope.row.size) }}</template>
        </el-table-column>
      </el-table>
    </section>
    <section class="part-buttons">
      <el-button class="dialog-cancel" @click="$emit('close')">{{ $t('operation.cancel') }}</el-button>
      <el-button class="dialog-ok" @click="handleOK">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { mapActions, mapMutations, mapState } from 'vuex'
import { getFileSize } from '@/common/utils'

export default {
  name: 'map-select',
  props: {
    selectedMap: {
      type: Array,
      default: () => [],
    },
    mapVisible: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      searchWord: '',
      selectedKeys: [],
      loading: false,
    }
  },
  computed: {
    ...mapState('map', [
      'mapList',
      'exporting',
    ]),
    list () {
      return this.mapList.filter(map => map.name.includes(this.searchWord))
    },
  },
  watch: {
    mapVisible: {
      handler (val) {
        console.log(val)
        this.$nextTick(() => {
          this.$refs.table.clearSelection()
          this.selectedMap.forEach(item => {
            this.$refs.table.toggleRowSelection(item, true)
          })
        })
      },
      deep: true,
      immediate: true,
    },
  },
  created () {
    this.loading = true
    this.getMapList()
      .finally(() => { this.loading = false })
  },
  methods: {
    ...mapMutations('map', {
      updateMapState: 'updateState',
    }),
    ...mapActions('map', [
      'getMapList',
    ]),
    getSize: getFileSize,
    tableSelectionChange (selection) {
      this.selectedKeys = [...selection]
    },
    setRowClassName ({ row }) {
      return this.selectedKeys.findIndex(key => row.name === key.name) > -1 ? 'selected-row' : ''
    },
    rowClickHandler (row) {
      this.$refs.table.toggleRowSelection(row)
    },
    handleOK () {
      if (this.selectedKeys.length > 10) {
        this.$message.warning(this.$t('tips.mapMaxChosed'))
        return
      }
      this.$emit('handleConfirm', this.selectedKeys)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .map-management {
    width: 500px;
    height: auto;
    padding: 23px;

    .map-item {
      display: flex;
      align-items: center;
      span {
        text-overflow: ellipsis;
        overflow: hidden;
        white-space: nowrap;
      }
      i {
        margin: 0 10px;
      }
    }

    .part-search {
      margin-bottom: 20px;
      text-align: center;
    }

    .part-buttons {
      margin-top: 10px;
      display: flex;
      justify-content: flex-end;
      gap: 10px;
      height: 40px;
      line-height: 40px;
    }

    .el-table {
      color: @disabled-color;

      /deep/ .cell {
        padding: 0 4px;
      }
      :deep(.sim-button--text) {
        padding: 3px 8px 3px 0;

        &:last-child {
          padding-right: 0;
        }
      }

      :deep(.el-table-column--selection) {
        text-align: right;
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
