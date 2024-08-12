<template>
  <div>
    <el-dialog
      class="pb-config-choose-dialog"
      :model-value="visible"
      header="添加数据"
      append-to-body
      width="700px"
      :close-on-click-modal="false"
      @close="handleClose"
    >
      <div class="search-bar">
        <el-select v-model="search.topic" class="topic-input" style="width:100px">
          <el-option
            v-for="op in shownTopicList"
            :key="op.value"
            :value="op.value"
            :label="op.label"
          />
        </el-select>
        <el-input
          v-model="search.field"
          class="search-input"
          prefix-icon="search"
          :style="{ width: '200px' }"
          :placeholder="$t('player.searchField')"
        />
      </div>
      <div class="table-data-wrap">
        <span class="search-result-item">{{ $t('scenario.filteredResults') }}: {{ shownList.length }}</span>
        <span class="search-result-item">{{ $t('scenario.selected') }}: {{ chosen.length }}</span>
        <span class="search-result-item">{{ $t('scenario.total') }}: {{ list.length }}</span>
      </div>
      <div class="table-wrap">
        <el-table
          ref="table"
          :data="shownList"
          width="100%"
          height="50vh"
          @selection-change="handleChosenChange"
        >
          <el-table-column type="selection" width="36" />
          <el-table-column prop="field" :label="$t('player.field')" show-overflow-tooltip />
          <el-table-column prop="topic" :label="$t('player.topic')" show-overflow-tooltip width="90px" />
          <el-table-column
            prop="paths"
            :label="$t('player.formPaths')"
            :formatter="row => row.paths.join('.')"
            show-overflow-tooltip
          />
          <el-table-column prop="description" :label="$t('player.description')" show-overflow-tooltip />
          <el-table-column :label="$t('operation.operation')" width="70px">
            <template #default="{ row }">
              <div class="button-group">
                <el-button
                  link
                  :disabled="row.preset"
                  @click="handleModify(row)"
                >
                  <el-icon class="el-icon-edit" :title="$t('operation.modify')">
                    <edit />
                  </el-icon>
                </el-button>
                <el-button
                  link
                  :disabled="row.preset"
                  @click="handleDelete(row)"
                >
                  <el-icon class="el-icon-delete" :title="$t('operation.delete')">
                    <delete />
                  </el-icon>
                </el-button>
              </div>
            </template>
          </el-table-column>
        </el-table>
      </div>
      <div class="form-buttons">
        <el-button class="btn-add" @click="handleAdd">
          + 添加字段
        </el-button>
        <el-button @click="handleClose">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button @click="handleConfirm">
          {{ $t('operation.ok') }}
        </el-button>
      </div>
    </el-dialog>
    <PbConfigSettingDialog ref="PbConfigSettingDialog" />
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import PbConfigSettingDialog from './pb-config-setting-dialog.vue'
import { createPbConfig, deletePbConfig, getPbConfigDetail, updatePbConfig } from '@/api/pb-config'

export default {
  components: {
    PbConfigSettingDialog,
  },
  data () {
    return {
      visible: false,
      chosen: [],
      maxLength: 6,
      callbacks: {},
      search: {
        topic: '',
        field: '',
      },
    }
  },
  computed: {
    ...mapState('pb-config', ['topicList']),
    ...mapState('pb-config', { list: 'pbConfigList' }),
    shownTopicList () {
      return [
        { value: '', label: '全部Topic' },
        ...this.topicList,
      ]
    },
    shownList () {
      const { topic, field } = this.search
      let shownList = this.list
      if (topic) shownList = shownList.filter(e => e.topic === topic)
      if (field) shownList = shownList.filter(e => e.field.includes(field))
      return shownList
    },
  },
  watch: {
    shownList () {
      this.refreshTableSelection()
    },
  },
  methods: {
    ...mapActions('pb-config', [
      'refreshTopicList',
      'refreshPbConfigList',
    ]),
    async openDialog (opts) {
      const { chosen = [], success, fail } = opts
      this.chosen = _.cloneDeep(chosen)
      this.refreshTopicList()
      this.refreshPbConfigList()
      this.callbacks = { success, fail }
      this.visible = true
    },
    refreshTableSelection () {
      const tableRef = this.$refs.table
      if (!tableRef || !this.chosen.length) return
      const selectedIds = this.chosen.map(e => e.field)
      tableRef.clearSelection()
      this.$nextTick(() => {
        this.list.forEach((row) => {
          const selected = selectedIds.includes(row.field)
          selected && tableRef.toggleRowSelection(row, true)
        })
      })
    },
    handleFilter (key, value) {
      this.search[key] = value
    },
    handleChosenChange (selection) {
      const unselectedIds = _.differenceBy(this.list, selection, 'field')
      let selectionBuffer = _.unionBy(this.chosen, selection, 'field')
      selectionBuffer = _.differenceBy(selectionBuffer, unselectedIds, 'field')
      this.chosen = selectionBuffer
    },
    async handleModify (pbConfig) {
      const $dialog = this.$refs.PbConfigSettingDialog
      if (!$dialog) return
      let remotePbConfig
      try {
        remotePbConfig = await getPbConfigDetail(pbConfig.field)
        delete remotePbConfig.source_field
      } catch (err) {
        this.$message.error(`${this.$t('tips.error')}: ${err.message}`)
        return
      }
      $dialog.openDialog({
        action: 'modify',
        list: this.list,
        data: remotePbConfig,
        success: async ({ confirm, data }) => {
          if (!confirm) return
          console.log('form', data)
          try {
            await updatePbConfig(pbConfig.field, data)
            this.$message.success('成功')
            this.refreshPbConfigList()
          } catch (err) {
            this.$message.error(`${this.$t('tips.error')}: ${err.message}`)
          }
        },
      })
    },
    async handleDelete (pbConfig) {
      try {
        await this.$confirmBox('确定要删除此配置？')
      } catch (e) {
        return
      }
      try {
        await deletePbConfig(pbConfig.field)
        this.chosen = this.chosen.filter(e => e.field !== pbConfig.field)
        this.$message.success('成功')
        this.refreshPbConfigList()
      } catch (err) {
        this.$message.error(`${this.$t('tips.error')}: ${err.message}`)
      }
    },
    handleAdd () {
      const $dialog = this.$refs.PbConfigSettingDialog
      if (!$dialog) return
      $dialog.openDialog({
        action: 'create',
        list: this.list,
        data: {},
        success: async ({ confirm, data }) => {
          if (!confirm) return
          console.log('form', data)
          try {
            await createPbConfig(data)
            this.$message.success('成功')
            this.refreshPbConfigList()
          } catch (err) {
            this.$message.error(`${this.$t('tips.error')}: ${err.message}`)
          }
        },
      })
    },
    handleClose () {
      this.callbacks.success({ confirm: false, data: undefined })
      this.visible = false
    },
    handleConfirm () {
      if (this.chosen.length > this.maxLength) {
        this.$message.error(`选中数量不可超过${this.maxLength}个`)
        return
      }
      this.callbacks.success({ confirm: true, data: this.chosen })
      this.visible = false
    },
  },
}
</script>

<style lang="less" scoped>
.search-bar {
  display: flex;
  align-items: center;
  padding: 16px 20px;

  .topic-input {
    margin-right: auto;
  }
}

.table-data-wrap {
  padding: 0 20px 10px;
  color: #B9B9B9;

  & > :not(:last-child) {
    margin-right: 30px;
  }
}

.table-wrap {
  padding: 0 20px;

  :deep(.el-button.is-disabled.el-button--text) {
    border: none;

    &:hover {
      background: none;
    }
  }
}

.form-buttons {
  display: flex;
  align-items: center;
  padding: 16px 20px;

  .btn-add {
    margin-right: auto;
  }
}
</style>
