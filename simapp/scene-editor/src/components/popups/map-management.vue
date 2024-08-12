<template>
  <div v-loading="loading" class="map-management">
    <section class="part-search">
      <el-input
        v-model="searchWord"
        style="width: 200px;"
        prefix-icon="search"
        :placeholder="($t('operation.searchName'))"
        class="search-word"
      />
    </section>
    <section class="part-batch">
      <el-checkbox class="select-all" :model-value="allCheck" @input="toggleSelectAll">
        {{ $t('operation.selectAll') }}
      </el-checkbox>
      <div>
        <el-tooltip effect="dark" :content="$t('operation.import')" placement="bottom">
          <span>
            <el-icon class="icon el-icon-download" @click="handleImport"><download /></el-icon>
          </span>
        </el-tooltip>
        <el-tooltip effect="dark" :content="$t('operation.export')" placement="bottom">
          <span>
            <el-icon
              class="icon el-icon-upload2"
              :class="{ disabled: !selectedKeys.length || exporting }" @click="handleExport"
            ><upload /></el-icon>
          </span>
        </el-tooltip>
        <el-tooltip v-if="true" effect="dark" :content="$t('operation.delete')" placement="bottom">
          <span>
            <el-icon
              class="icon el-icon-delete"
              :class="{ disabled: !noPresetKeys.length || exporting }" @click="handleDelete"
            ><delete /></el-icon>
          </span>
        </el-tooltip>
      </div>
    </section>
    <section class="part-details">
      <el-table
        ref="table"
        size="small"
        :data="list"
        height="320"
        row-key="id"
        :row-class-name="setRowClassName"
        @selection-change="tableSelectionChange"
        @row-click="rowClickHandler"
      >
        <el-table-column type="selection" label-class-name="select-all" width="36" />
        <el-table-column
          :label="$t('name')"
          prop="name"
          class-name="table-row"
          show-overflow-tooltip
          min-width="160"
        >
          <template #default="scope">
            <div class="map-item">
              <span>{{ scope.row.name }}</span>
            </div>
          </template>
        </el-table-column>
        <el-table-column width="20">
          <template #default="scope">
            <!-- 预设地图带锁 -->
            <el-icon v-if="scope.row.preset !== '0'" class="el-icon-lock">
              <lock />
            </el-icon>
          </template>
        </el-table-column>
        <el-table-column
          :label="$t('menu.file')"
          prop="name"
          class-name="table-row"
          min-width="160"
          show-overflow-tooltip
        />
        <el-table-column
          :label="$t('scenario.size')"
          prop="size"
          class-name="table-row"
          width="80"
          show-overflow-tooltip
        >
          <template #default="scope">
            {{ getSize(scope.row.size) }}
          </template>
        </el-table-column>
      </el-table>
    </section>
    <section class="part-buttons">
      <el-button class="dialog-cancel" @click="$emit('close')">
        {{ $t('operation.close') }}
      </el-button>
    </section>

    <ScenarioImportConfirm
      v-if="scenarioImportConfirm.visible"
      :name="scenarioImportConfirm.filename"
      :window-type="2"
      :show-apply-all="scenarioImportConfirm.showApplyAll"
      :disable-replace="scenarioImportConfirm.isPresetMap"
      @confirm="handleImportConfirm"
      @close="scenarioImportConfirm = { visible: false, data: null }"
      @apply-all="onApplyAll"
    />
  </div>
</template>

<script>
import { mapActions, mapMutations, mapState } from 'vuex'
import { errorHandler } from '@/common/errorHandler'
import { dirname, filename, getFileSize } from '@/common/utils'
import ScenarioImportConfirm from '@/components/popups/scenario-import-confirm.vue'
import { messageBoxInfo } from '@/plugins/message-box'

const { electron: { path, dialog } } = window

function isMapNameExist (list, name) {
  let ret = false
  ret = list.some(item => item.name === name)
  return ret
}

let requestPromise = null
let resolveFunc = null

let scenarioImportArray = null

export default {
  name: 'MapManagement',
  components: {
    ScenarioImportConfirm,
  },
  data () {
    return {
      searchWord: '',
      selectedKeys: [],
      loading: false,
      scenarioImportConfirm: {
        visible: false,
        data: null,
        filename,
        applyAll: false,
        applyAllTypes: [], //  1 - 保留两者；2 - 取消； 3 - 替换; index 0 存的是非预设地图， index 1 存的是预设地图
        showApplyAll: false,
        isPresetMap: false,
        applyMapTypes: [], // 应用全部需要区分预设和非预设
      },
    }
  },
  computed: {
    ...mapState('map', [
      'mapList',
      'exporting',
    ]),
    ...mapState('progressInfo', [
      'progressInfo',
    ]),
    allCheck () {
      return (
        !!this.selectedKeys.length &&
        this.selectedKeys.slice().sort().toString() === this.mapList.map(item => item.name).sort().toString()
      )
    },
    list () {
      return this.mapList.filter(map => map.name.includes(this.searchWord))
    },
    noPresetKeys () {
      const noPreset = this.mapList.filter(item => this.selectedKeys.includes(item.name) && item.preset === '0')
      const ids = noPreset.map(item => item.name)
      return ids
    },
  },
  created () {
    this.loading = true
    this.getMapList()
      .finally(() => {
        this.loading = false
      })
  },

  methods: {
    ...mapMutations('progressInfo', [
      'updateProgressInfo',
    ]),
    ...mapMutations('map', {
      updateMapState: 'updateState',
    }),
    ...mapActions('map', [
      'getMapList',
      'exportMaps',
      'delMaps',
      'importMapsSub',
    ]),

    startProgressTask (mode = 10) {
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
    async handleImport () {
      // this.loading = true
      if (this.progressInfo.status) {
        messageBoxInfo(this.$t('tips.importingOrExporting'))
        return
      }
      try {
        let { filePaths: sources } = await dialog.showOpenDialog({
          title: this.$t('tips.selectMapToImport'),
          properties: ['openFile', 'multiSelections'],
          filters: [
            { name: `XODR ${this.$t('menu.file')}`, extensions: ['xodr'] },
            { name: `SQLite ${this.$t('menu.file')}`, extensions: ['sqlite'] },
            { name: `XML ${this.$t('menu.file')}`, extensions: ['xml'] },
          ],
        })
        if (sources && sources.length) {
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
            const exist = isMapNameExist(this.mapList, oneSource)
            const isPreset = this.isPresetMap(oneSource)
            if (exist) { // 如果有重名，则需要弹框询问
              // 如果没有应用全部则需要弹框
              if (!this.scenarioImportConfirm.applyAllTypes[+isPreset]) {
                this.scenarioImportConfirm.visible = true
                this.scenarioImportConfirm.data = {
                  dir,
                  sources: [oneSource],
                }
                this.scenarioImportConfirm.filename = oneSource
                this.scenarioImportConfirm.isPresetMap = isPreset

                requestPromise = new Promise((resolve) => {
                  resolveFunc = resolve
                })

                await requestPromise
              } else { // 如果选择了应用全部，则直接导入
                if (this.scenarioImportConfirm.applyAllTypes[+isPreset] === 2) { // 跳过
                  continue
                }
                const sourceInfo = {
                  name: oneSource,
                  replaceFlag: this.scenarioImportConfirm.applyAllTypes[+isPreset] === 3,
                }

                scenarioImportArray.push(sourceInfo)
              }
            } else { // 如果不重名则直接导入
              this.loading = true
              const sourceInfo = {
                name: oneSource,
                replaceFlag: true,
              }

              scenarioImportArray.push(sourceInfo)
            }
          }

          if (scenarioImportArray.length > 0) {
            // 收集完处理方式之后，统一发送接口
            this.startProgressTask(10)
            await this.importMapsSub({ // todo type dir
              type: 'files',
              dir,
              sources: scenarioImportArray,
            })

            this.stopProgressTask()
          }

          scenarioImportArray = null
        }
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
    },
    async handleExport () {
      if (!this.selectedKeys.length) return
      this.loading = true
      this.updateMapState({ exporting: true })
      try {
        const { filePaths } = await dialog.showOpenDialog({
          title: this.$t('tips.selectDirectoryForExMaps'),
          properties: ['openDirectory'],
        })
        const [dest] = filePaths
        if (dest) {
          this.startProgressTask(11)
          await this.exportMaps({
            sources: this.selectedKeys,
            dest,
          })
          this.stopProgressTask()
        }
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
      this.updateMapState({ exporting: false })
    },
    async handleDelete () {
      if (!this.selectedKeys.length) return
      if (!this.noPresetKeys.length) return
      this.loading = true
      try {
        await this.$confirmBox(this.$t('tips.whetherDeleteSelectedMap'))
      } catch (e) {
        this.loading = false
        return
      }

      try {
        const totalNum = this.selectedKeys.length
        const noPresetNum = this.noPresetKeys.length
        const presetNum = totalNum - noPresetNum

        // 只能删除非预设的地图
        await this.delMaps(this.noPresetKeys)
        const tip = presetNum > 0 ?
          `${this.$t('tips.deleteDataSuccess', { dataNum: noPresetNum })}, ${this.$t('tips.dataCantDeleted', { dataNum: presetNum })}` :
          this.$t('tips.deleteDataSuccess', { dataNum: noPresetNum })
        this.$message({
          message: tip,
          type: 'success',
        })
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
    },

    toggleSelectAll () {
      this.$refs.table.toggleAllSelection()
    },
    tableSelectionChange (selection) {
      this.selectedKeys = selection.map(item => item.name)
    },
    setRowClassName ({ row }) {
      return this.selectedKeys.findIndex(key => row.name === key) > -1 ? 'selected-row' : ''
    },
    rowClickHandler (row) {
      this.$refs.table.toggleRowSelection(row)
    },
    async handleImportConfirm ({ type = 0 } = {}) {
      // this.loading = true
      this.scenarioImportConfirm.visible = false
      const isPreset = +this.scenarioImportConfirm.isPresetMap
      if (this.scenarioImportConfirm.applyAll) {
        this.scenarioImportConfirm.applyAllTypes[isPreset] = type // 全部应用的类型
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
              replaceFlag: type === 3, // 是否替换
            }
          })
          /* let result = await this.importMapsSub({ // todo type dir
            type: 'files',
            dir,
            sources: sourcesFlag,
          }) */
          scenarioImportArray = scenarioImportArray.concat(sourcesFlag)
          resolveFunc(true)
        }
      } catch (error) {
        await errorHandler(error)
      }
      this.loading = false
      resolveFunc(false)
    },
    onApplyAll ({ value }) {
      this.scenarioImportConfirm.applyAll = value
    },

    isPresetMap (mapName) {
      const item = this.mapList.filter(item => item.name === mapName)
      return !!(item && item[0] && item[0].preset !== '0')
    },
    getSize: getFileSize,
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .map-management {
    width: 550px;
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

    .preset-icon {
      margin-left: 10px;
      padding: 0 2px;
      font-size: 10px;
      color: @disabled-color;
      border: 1px solid @disabled-color;
      border-radius: 1px;
    }

    .part-search {
      text-align: center;
    }

    .part-batch {
      height: 30px;
      line-height: 30px;
      display: flex;
      justify-content: space-between;
      align-items: center;

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
      background-color: #111111;
      min-height: 200px;
      max-height: 320px;
      overflow: auto;
      margin-top: 10px;
    }

    .part-buttons {
      margin-top: 10px;
      direction: rtl;
      height: 40px;
      line-height: 40px;
    }

    .el-table {
      color: @disabled-color;

      :deep(.select-all .el-checkbox) {
        display: none;
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
