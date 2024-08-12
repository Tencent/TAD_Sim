<template>
  <div v-loading="loading" class="scenario-list">
    <section class="part-search">
      <el-select
        id="scenario-set-list-select"
        v-model="scenarioSetId"
        class="scenario-set-list-select"
      >
        <el-option
          v-for="item in computedScenarioSetList"
          :id="`scenario-set-list-option-${item.name}`"
          :key="item.id"
          :label="item.name"
          :value="item.id"
          :style="{ maxWidth: '300px' }"
          :title="item.name"
        />
      </el-select>
      <el-select
        id="scenario-ext-list-select"
        v-model="scenarioExt"
        class="scenario-ext-list-select"
      >
        <el-option
          id="scenario-ext-list-option-all"
          :label="$t('scenario.allFormats')"
          value=""
        />
        <el-option
          id="scenario-ext-list-option-sim"
          label=".sim"
          value="sim"
        />
        <el-option
          id="scenario-ext-list-option-xosc"
          label=".xosc"
          value="xosc"
        />
      </el-select>
      <el-input
        id="scenario-name"
        v-model="keyword"
        style="width: 200px;"
        prefix-icon="search"
        :placeholder="($t('operation.searchName'))"
        class="search-word"
      />
    </section>

    <section v-if="mode === 'list'" class="part-batch">
      <div>
        <span class="search-result-item">{{ $t('scenario.filteredResults') }}: {{ list.length }}</span>
        <span class="search-result-item">{{ $t('scenario.selected') }}: {{ selectedKeys.length }}</span>
        <span class="search-result-item">{{ $t('scenario.total') }}: {{ scenarioList.length }}</span>
      </div>
      <div class="icon-btn-wrap">
        <el-tooltip effect="dark" :content="$t('operation.import')" placement="bottom">
          <span>
            <el-icon class="icon" @click="handleImport"><Download /></el-icon>
          </span>
        </el-tooltip>
        <el-tooltip effect="dark" :content="$t('operation.export')" placement="bottom">
          <span>
            <el-icon
              class="icon"
              :class="{ disabled: !selectedKeys.length || exporting }"
              @click="handleExport"
            ><Upload /></el-icon>
          </span>
        </el-tooltip>
        <el-tooltip effect="dark" :content="$t('operation.delete')" placement="bottom">
          <span>
            <el-icon
              class="icon"
              :class="{ disabled: !noPresetKeys.length || exporting }"
              @click="handleDelete"
            >
              <Delete />
            </el-icon>
          </span>
        </el-tooltip>
      </div>
    </section>

    <section class="part-details" style="height: 400px">
      <el-auto-resizer>
        <template #default="{ height, width }">
          <el-table-v2
            :columns="getColumns()"
            :data="list"
            :width="width"
            :height="height"
            fixed
          />
        </template>
      </el-auto-resizer>
    </section>

    <section v-if="mode === 'list'" class="part-buttons">
      <el-button id="scenario-add-dailog-close" @click="$emit('close')">
        {{ $t('operation.close') }}
      </el-button>
    </section>
    <section v-else class="part-buttons">
      <el-button id="scenario-add-cancel" @click="$emit('close')">
        {{
          $t('operation.cancel')
        }}
      </el-button>
      <el-button id="scenario-add-confirm" @click="handleConfirm">
        {{
          $t('operation.add')
        }}
      </el-button>
    </section>

    <ScenarioImportConfirm
      v-if="scenarioImportConfirm.visible"
      :name="scenarioImportConfirm.filename"
      :show-apply-all="scenarioImportConfirm.showApplyAll"
      :window-type="1"
      :disable-replace="scenarioImportConfirm.isPresetScenario"
      @confirm="handleImportConfirm"
      @close="scenarioImportConfirm = { visible: false, data: null }"
      @apply-all="onApplyAll"
    />

    <ScenarioExportConfirm
      v-if="scenarioExportConfirm.visible"
      :exts="computedExtList"
      @confirm="handleExportConfirm"
      @close="scenarioExportConfirm = { visible: false, data: null }"
    />
  </div>
</template>

<script lang="jsx">
import { filter, forEach, without } from 'lodash-es'
import { mapActions, mapMutations, mapState } from 'vuex'
import { h } from 'vue'
import { ElButton, ElCheckbox, ElIcon } from 'element-plus'
import { Lock } from '@element-plus/icons-vue'
import { errorHandler } from '@/common/errorHandler'
import { dirname, filename } from '@/common/utils'
import dict from '@/common/dict'
import ScenarioImportConfirm from '@/components/popups/scenario-import-confirm.vue'
import ScenarioExportConfirm from '@/components/popups/scenario-export-confirm.vue'
import { messageBoxInfo } from '@/plugins/message-box'

const { electron: { path, dialog } } = window

function isScenarioNameExist (list, name) {
  let ret = false
  ret = list.some(item => item.filename === name)
  return ret
}

let requestPromise = null
let resolveFunc = null

let scenarioImportArray = null

export default {
  name: 'ScenarioList',
  components: {
    ScenarioImportConfirm,
    ScenarioExportConfirm,
  },
  props: {
    mode: {
      type: String,
      default: 'list', // list | select
    },
    selected: {
      type: Array,
      default () {
        return []
      },
    },
    sceneType: {
      type: String, // 是sim还是simrec
      default: 'sim',
    },
  },
  data () {
    return {
      loading: false,
      scenarioSetId: 0,
      scenarioExt: '',
      keyword: '',
      selectedKeys: [],
      selectedExts: [],
      scenarioImportConfirm: {
        visible: false,
        data: null,
        filename,
        applyAll: false,
        applyAllTypes: [], // replace, overwrite
        showApplyAll: false,
        isPresetScenario: false,
      },
      scenarioExportConfirm: {
        visible: false,
        data: null,
      },
    }
  },
  computed: {
    ...mapState('scenario', [
      'scenarioList',
    ]),
    ...mapState('progressInfo', [
      'progressInfo',
    ]),
    ...mapState('scenario-set', [
      'scenarioSetList',
      'exporting',
    ]),
    computedScenarioSetList () {
      return [
        { id: 0, name: this.$t('scenario.allSets') },
        ...this.scenarioSetList,
      ]
    },
    list () {
      return this.scenarioList.filter((item) => {
        if (this.scenarioSetId === 0) {
          return true
        } else {
          return item.set.includes(this.scenarioSetId)
        }
      }).filter((item) => {
        if (this.scenarioExt === '') {
          return true
        } else {
          return item.type === this.scenarioExt
        }
      }).filter((item) => {
        return item.filename.includes(this.keyword)
      }).filter((item) => {
        return this.sceneType === 'simrec' ? item.traffictype === 'simrec' : item.traffictype !== 'simrec'
      })
    },
    computedExtList () {
      const matched = this.selectedExts.includes('xosc')
      const exts = ['xosc']
      if (!matched) {
        exts.unshift('sim')
      }
      return exts
    },
    noPresetKeys () {
      const noPreset = this.scenarioList.filter(item => this.selectedKeys.includes(item.id) && item.preset === '0')
      const ids = noPreset.map(item => item.id)
      return ids
    },
  },
  watch: {
    list (val) {
      console.log(val)
      this.$nextTick(() => {
        this.setSelected()
      })
    },
  },
  async created () {
    this.loading = true
    try {
      await this.getScenarioList()
    } catch (e) {
      await errorHandler(e)
    }
    this.loading = false
  },
  methods: {
    ...mapMutations('progressInfo', [
      'updateProgressInfo',
    ]),
    ...mapMutations('scenario-set', {
      updateScenarioSet: 'updateState',
    }),
    ...mapActions('scenario', [
      'getScenarioList',
      'openScenario',
      'copyScenario',
      'importScenarios',
      'exportScenarios',
      'delScenarios',
      'importScenariosSub',
    ]),
    getColumns () {
      const columns = [
        {
          key: 'selection',
          dataKey: 'selection',
          width: 36,
          cellRenderer: ({ rowData }) => {
            return (
              <ElCheckbox
                modelValue={this.selectedKeys.includes(rowData.id)}
                onChange={(value) => {
                  this.handleCheckChange(rowData, value)
                }}
                indeterminate={false}
                disabled={!this.canSelect(rowData)}
              />
            )
          },
          headerCellRenderer: () => {
            const onChange = (value) => {
              if (value) {
                this.selectedKeys = this.list.map(v => v.id)
              } else {
                this.selectedKeys = [...this.selected]
              }
              this.handleTypeChange()
            }
            const allSelected = this.selectedKeys.length !== 0 && this.selectedKeys.length === this.list.length
            const containsChecked = this.selectedKeys.length > 0 && this.selectedKeys.length !== this.list.length
            return h(
              ElCheckbox,
              {
                modelValue: allSelected,
                onChange: value => onChange(value),
                indeterminate: containsChecked && !allSelected,
              },
            )
          },
        },
        {
          key: 'filename',
          dataKey: 'filename',
          title: this.$t('name'),
          width: 180,
          cellRenderer: ({ rowData }) => (
            <ElTooltip content={rowData.filename}>
              <div class="scenario-item">
                {rowData.filename}
              </div>
            </ElTooltip>
          ),
          // h(
          //   'div',
          //   {
          //     class: 'scenario-item',
          //   },
          //   rowData.filename
          // )
        },
        {
          key: 'preset',
          width: 20,
          cellRenderer: ({ rowData }) => {
            return rowData.preset !== '0' ? <ElIcon><Lock /></ElIcon> : ''
          },
        },
        {
          key: 'info',
          dataKey: 'info',
          title: this.$t('scenario.workingCondition'),
          width: 130,
        },
        {
          key: 'map',
          dataKey: 'map',
          title: this.$t('scenario.map'),
          width: 130,
        },

      ]
      if (this.sceneType === 'simrec') {
        columns.push({
          key: 'dataSource',
          dataKey: 'dataSource',
          title: this.$t('scenario.dataSource'),
          width: 70,
        })
      }
      if (this.mode === 'list' && this.sceneType !== 'simrec') {
        columns.push({
          key: 'id',
          title: this.$t('operation.open'),
          width: 70,
          cellRenderer: ({ rowData }) => (
            <ElButton
              link
              class={[rowData.traffictype === 'simrec' ? 'table-operations-disabled' : 'table-operations']}
              disabled={rowData.traffictype === 'simrec'}
              onClick={() => this.handleOpen(rowData)}
            >
              {this.$t('operation.open')}
            </ElButton>
          ),
        })
      }
      return columns
    },
    handleCheckChange (data, value) {
      if (value && !this.selectedKeys.includes(data.id)) {
        this.selectedKeys.push(data.id)
      } else {
        this.selectedKeys = [...this.selectedKeys].filter(v => v !== data.id)
      }
      this.handleTypeChange()
    },
    handleTypeChange () {
      this.selectedExts = this.selectedKeys.map(item => item.type)
      if (this.selectedKeys.length > 0) {
        this.$emit('available-scene-type-change', this.sceneType)
      } else {
        this.$emit('available-scene-type-change', '')
      }
    },
    canSelect (row) {
      return !this.selected.includes(row.id)
    },
    setSelected () {
      forEach(this.list, (item) => {
        const included = !this.canSelect(item)
        if (included) {
          if (!this.selectedKeys.includes(item.id)) {
            this.selectedKeys.push(item.id)
          }
        }
      })
    },

    startProgressTask (mode = 0) {
      this.updateProgressInfo({
        mode,
        status: true,
        progress: 0,
      })
    },

    stopProgressTask () {
      const mockData = false
      if (mockData) {
        setTimeout(() => {
          this.updateProgressInfo({
            status: false,
          })
        }, 10000)
      } else {
        this.updateProgressInfo({
          status: false,
        })
      }
    },

    async handleOpen (scenario) {
      this.loading = true
      const result = await this.openScenario(scenario)
      this.loading = false
      if (result) {
        this.$emit('close')
      }
    },
    async handleCopy (scenario) {
      this.loading = true
      await this.copyScenario(scenario)
      this.loading = false
    },
    async handleImport () {
      // this.loading = true
      if (this.progressInfo.status) {
        messageBoxInfo(this.$t('tips.importingOrExporting'))
        return
      }

      try {
        let { filePaths: sources } = await dialog.showOpenDialog({
          title: this.$t('tips.selectScenarioToImport'),
          properties: ['openFile', 'multiSelections'],
          filters: [
            { name: `XOSC ${this.$t('menu.file')}`, extensions: ['xosc'] },
            { name: `SIM ${this.$t('menu.file')}`, extensions: ['sim'] },
          ],
        })
        if (sources && sources.length) {
          if (this.scenarioList.length + sources.length > dict.maxScenarioCount) {
            this.$errorBox(this.$t('tips.scenarioExceededCannotImport', { maxScenarioCount: dict.maxScenarioCount }))
          } else {
            let [dir] = sources
            dir = dirname(dir, path.sep)
            sources = sources.map((source) => {
              return filename(source, path.sep)
            })
            this.scenarioImportConfirm.applyAll = false // 每次导入初始都是不勾选全部应用
            this.scenarioImportConfirm.applyAllTypes.splice(0)
            this.scenarioImportConfirm.showApplyAll = sources.length > 1

            scenarioImportArray = []

            while (sources.length > 0) {
              // this.loading = true
              const oneSource = sources.shift()
              const exist = isScenarioNameExist(this.scenarioList, oneSource)
              const isPreset = this.isPresetScenario(oneSource)

              if (exist) { // 是否重名
                if (!this.scenarioImportConfirm.applyAllTypes[+isPreset]) { // 如果不是应用全部
                  this.scenarioImportConfirm.visible = true

                  this.scenarioImportConfirm.data = {
                    dir,
                    sources: [oneSource],
                  }
                  this.scenarioImportConfirm.filename = oneSource
                  this.scenarioImportConfirm.isPresetScenario = isPreset

                  requestPromise = new Promise((resolve, reject) => {
                    resolveFunc = resolve
                  })
                  await requestPromise
                } else { // 应用全部
                  if (this.scenarioImportConfirm.applyAllTypes[+isPreset] === 2) { // 跳过
                    continue
                  }
                  const sourceInfo = {
                    name: oneSource,
                    replaceFlag: this.scenarioImportConfirm.applyAllTypes[+isPreset] === 3,
                  }

                  scenarioImportArray.push(sourceInfo)
                }
              } else {
                // this.loading = true
                const sourceInfo = {
                  name: oneSource,
                  replaceFlag: true,
                }

                scenarioImportArray.push(sourceInfo)
              }
            }

            if (scenarioImportArray.length > 0) {
              // 收集完处理方式之后，统一发送接口
              this.startProgressTask()

              let category
              switch (this.sceneType) {
                case 'sim':
                  category = 'WORLD_SIM'
                  break
                case 'simrec':
                  category = 'LOG_SIM'
                  break
                default:
                  category = 'WORLD_SIM'
              }

              await this.importScenariosSub({ // todo type dir
                type: 'files',
                dir,
                category,
                sources: scenarioImportArray,
              })

              this.stopProgressTask()
            }

            scenarioImportArray = null
          }
        }
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
    },
    async handleExport () {
      if (!this.selectedKeys.length) return
      this.loading = true
      try {
        const { filePaths } = await dialog.showOpenDialog({
          title: this.$t('tips.selectDirectoryForExportingScenarios'),
          properties: ['openDirectory'],
        })
        const [dest] = filePaths
        if (dest) {
          this.scenarioExportConfirm.visible = true
          this.scenarioExportConfirm.data = {
            dest,
          }
        }
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
    },
    async handleDelete () {
      if (!this.selectedKeys.length) return
      if (!this.noPresetKeys.length) return
      this.loading = true

      try {
        await this.$confirmBox(this.$t('tips.whetherDeleteSelectedScenario'))
      } catch (e) {
        this.loading = false
        return
      }

      try {
        const totalNum = this.selectedKeys.length
        const noPresetNum = this.noPresetKeys.length
        const presetNum = totalNum - noPresetNum
        const oSelectedKeys = this.selectedKeys.slice()
        // 只能删除非预设场景
        await this.delScenarios(this.noPresetKeys)
        const tip = presetNum > 0 ?
          `${this.$t('tips.deleteDataSuccess', { dataNum: noPresetNum })}, ${this.$t('tips.dataCantDeleted', { dataNum: presetNum })}` :
          this.$t('tips.deleteDataSuccess', { dataNum: noPresetNum })
        this.$message({
          message: tip,
          type: 'success',
        })
        this.selectedKeys = without(oSelectedKeys, ...this.noPresetKeys)
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
    },
    handleConfirm () {
      const payload = filter(this.selectedKeys, (item) => {
        return this.canSelect({ id: item })
      })
      this.$emit('confirm', payload)
      this.$emit('close')
    },
    tableSelectionChange (selection) {
      this.selectedKeys = selection.map(item => item.id)
      this.selectedExts = selection.map(item => item.type)
      if (selection.length > 0) {
        this.$emit('available-scene-type-change', this.sceneType)
      } else {
        this.$emit('available-scene-type-change', '')
      }
    },
    setRowClassName ({ row }) {
      return this.selectedKeys.findIndex(key => row.id === key) > -1 ? 'selected-row' : ''
    },
    async handleImportConfirm ({ type = 0 } = {}) {
      // this.loading = true
      this.scenarioImportConfirm.visible = false
      const isPreset = +this.scenarioImportConfirm.isPresetScenario
      if (this.scenarioImportConfirm.applyAll) {
        // this.scenarioImportConfirm.applyAllType = type // 全部应用的类型
        this.scenarioImportConfirm.applyAllTypes[isPreset] = type
      }
      if (type === 2) {
        this.loading = false
        resolveFunc(true)
        return
      }
      try {
        if (this.scenarioImportConfirm.data) {
          const { sources } = this.scenarioImportConfirm.data
          const sourcesFlag = sources.map((item) => {
            return {
              name: item,
              replaceFlag: type === 3,
            }
          })
          // let result = await this.importScenariosSub({ // todo type dir
          //   type: 'files',
          //   dir,
          //   sources: sourcesFlag,
          // })
          scenarioImportArray = scenarioImportArray.concat(sourcesFlag)
          resolveFunc(true)
        }
      } catch (error) {
        await errorHandler(error)
      }
      this.loading = false
      resolveFunc(false)
    },
    async handleExportConfirm (ext) {
      this.loading = true
      this.updateScenarioSet({ exporting: true })
      this.scenarioExportConfirm.visible = false
      try {
        if (this.scenarioExportConfirm.data) {
          const { dest } = this.scenarioExportConfirm.data
          this.startProgressTask(1)
          await this.exportScenarios({
            sources: this.selectedKeys,
            dest,
            export_type: ext === -1 ? '': ext,
          })
          this.stopProgressTask()
        }
      } catch (error) {
        await errorHandler(error)
      }
      this.loading = false
      this.updateScenarioSet({ exporting: false })
    },

    onApplyAll ({ value }) {
      this.scenarioImportConfirm.applyAll = value
    },
    isPresetScenario (scenarioName) {
      const item = this.scenarioList.filter(item => item.filename === scenarioName)
      return !!(item && item[0] && item[0].preset !== '0')
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.scenario-list {
  :deep(.scenario-item) {
    width: 100%;
    text-overflow: ellipsis;
    overflow: hidden;
    white-space: nowrap;

    i {
      margin: 0 10px;
    }
  }

  .part-search {
    text-align: center;
    margin-top: 10px;
    margin-bottom: 23px;

    .scenario-set-list-select {
      width: 200px;
    }

    .scenario-ext-list-select {
      width: 105px;
    }
  }

  .part-batch {
    height: 30px;
    line-height: 30px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-top: -13px;
    margin-bottom: 10px;

    .search-result-item {
      font-size: 13px;
      color: @global-font-color;
      margin-right: 20px;
    }

    .icon {
      color: @global-font-color;
      font-size: 16px;
      margin: 0 6px;
      cursor: pointer;

      &:hover {
        color: white;
      }

      &.disabled {
        pointer-events: none;
        opacity: 0.5;
      }
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

  .el-table, .el-table-v2 {
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
      color: @active-font-color;
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

      .table-operations-disabled {
        color: @disabled-color;
        border-style: none
      }
    }

    :deep(.selected-row) {
      color: @active-font-color;
    }
  }
}
</style>
