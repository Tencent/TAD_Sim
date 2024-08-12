<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import { Delete, Download, Lock, Search, Upload } from '@element-plus/icons-vue'
import { difference, get } from 'lodash'
import SimButton from '../common/button.vue'
import DeleteMapDialog from './deleteMapDialog.vue'
import ImportMapDialog from './importMapDialog.vue'
import { useHelperStore } from '@/stores/helper'
import { useFileStore } from '@/stores/file'
import {
  errorMessage,
  fullScreenLoading,
  successMessage,
  warningMessage,
} from '@/utils/common'
import i18n from '@/locales'
import { deleteHadmap, exportHadmap, importHadmap } from '@/services'
import { getDirname, getFileSize, getFilename } from '@/utils/tools'

interface IMapFileItem {
  name: string
  createTime: string
  id: number
  preset: string
  size: string
}
interface IImportMap {
  name: string
  replaceFlag?: boolean
  isPreset?: boolean
}
const helperStore = useHelperStore()
const fileStore = useFileStore()
// 搜索的关键词
const keyword = ref('')
const table = ref()
// 是否选中全部地图
const isSelectedAll = ref(false)
// 删除地图的二次确认弹窗显示
const deleteMapDialogVisible = ref(false)
// 导入地图的二次确认弹窗显示
const importMapDialogVisible = ref(false)
// 当前选中表格中的选项
const selectedMap: Ref<Array<IMapFileItem>> = ref([])
// 正在导入的地图文件属性
const importDir = ref('')
const importSameMaps: Ref<Array<IImportMap>> = ref([])
const importDiffMaps: Ref<Array<IImportMap>> = ref([])

const dialog = get(window, 'electron.dialog')
const pathSep = get(window, 'electron.path.sep')

const visible = computed(() => {
  if (helperStore.mapFileDialogShow) {
    // 弹窗展示后，禁用热键
    helperStore.closeShortcutKeys()
  }
  return helperStore.mapFileDialogShow
})

const mapList = computed(() => {
  // 通过关键词来筛选列表
  if (keyword.value) {
    return fileStore.mapList.filter(mapOption =>
      mapOption.name.includes(keyword.value),
    )
  }

  return fileStore.mapList
})
// 当前是否可以导出地图
const exportDisabled = computed(() => {
  return selectedMap.value.length < 1
})
// 当前是否可以删除地图
const deleteDisabled = computed(() => {
  // 基于已选中的地图项，筛选出非预设的地图
  const noPresetMap = selectedMap.value.filter(
    mapInfo => mapInfo.preset === '0',
  )
  return noPresetMap.length < 1
})

watch(keyword, (val) => {
  const _val = val.trim()
  keyword.value = _val
})

// 清空所有选中的地图
function clearSelectedMap () {
  selectedMap.value = []
}
// 点击某一行，则 toggle 该行的选中状态
function handleRowClick (row: IMapFileItem) {
  table.value.toggleRowSelection(row)
}
function handleTableSelectionChange (selection: Array<IMapFileItem>) {
  selectedMap.value = selection
  // 如果选中的数量不是全部，则需要取消全选状态
  isSelectedAll.value = selection.length === mapList.value.length
}
// 切换全选地图的状态
function toggleSelectAll () {
  table.value.toggleAllSelection()
}
function handleClose () {
  helperStore.toggleDialogStatus('mapList', false)
  // 关闭时清空展示的数据列表，因为地图内容较多时，重新打开 dialog 有延时
  fileStore.clearMapList()
  // 清空关键词
  keyword.value = ''
  // 清空选中
  clearSelectedMap()

  // 关闭弹窗后，恢复三维场景热键的使用
  helperStore.openShortcutKeys()
}
// 加载地图文件
async function loadMap (row: IMapFileItem) {
  const { name: mapFileName } = row

  const loadingInstance = fullScreenLoading(
    `${i18n.global.t('desc.tips.loading')}`,
  )

  console.time('load and save done')
  // 加载地图
  const res = await fileStore.loadMap(mapFileName)
  console.timeEnd('load and save done')
  loadingInstance.close()

  if (res) {
    // 打开成功，则关闭弹窗
    handleClose()
    successMessage({
      content: i18n.global.t('desc.tips.openMapSuccessfully'),
    })
  } else {
    // TODO 打开地图失败，提示
    errorMessage({
      content: i18n.global.t('desc.tips.failedToOpenMap'),
    })
  }
}
// -------- 导入地图 start --------
// 导入地图
async function importMap () {
  if (!dialog) return

  let loadingInstance
  try {
    const { filePaths } = await dialog.showOpenDialog({
      title: i18n.global.t('actions.file.selectAMapToImport'),
      properties: ['openFile', 'multiSelections'],
      // 支持 xodr 和 sqlite 地图文件的导入
      filters: [
        {
          name: `XODR ${i18n.global.t('desc.tips.file')}`,
          extensions: ['xodr'],
        },
        {
          name: `SQLite ${i18n.global.t('desc.tips.file')}`,
          extensions: ['sqlite'],
        },
      ],
    })

    // 如果取消导入，则 filePaths 为空数组
    if (filePaths.length < 1) return

    const dirname = getDirname(filePaths[0], pathSep)
    // 导入地图的目录
    importDir.value = dirname
    const mapNames = filePaths.map((filePath: string) =>
      getFilename(filePath, pathSep),
    )

    // 判断当前导入的地图文件名，是否有跟地图列表中的文件同名
    // 通过弹窗二次确认来筛选同名地图
    const sameNames: Array<string> = []
    const sameNameMaps: Array<IImportMap> = []
    mapNames.forEach((name: string) => {
      for (const mapInfo of fileStore.mapList) {
        if (mapInfo.name === name) {
          sameNames.push(name)
          sameNameMaps.push({
            name,
            replaceFlag: false,
            // 判断是否跟预设的地图重名
            isPreset: mapInfo.preset !== '0',
          })
          break
        }
      }
    })

    // 获取不同名的地图名称
    const diffNames = difference(mapNames, sameNames)
    importDiffMaps.value = diffNames.map(name => ({
      name,
      replaceFlag: false,
    }))

    if (sameNameMaps.length < 1) {
      // 如果没有重名的地图，则直接将地图文件保存

      // 实际开始调用导入接口时，设置全屏 loading
      loadingInstance = fullScreenLoading(i18n.global.t('desc.tips.importing'))

      // 调用导入地图的接口
      const importRes = await importHadmap({
        dir: dirname,
        sources: importDiffMaps.value,
      })
      if (!importRes || get(importRes, 'data.message') !== 'ok') {
        throw new Error(
          `api export hadmap error. error code: ${get(importRes, 'data.code')}`,
        )
      }

      // 导入成功提示
      successMessage({
        content: i18n.global.t('desc.tips.importMapSuccessfully'),
      })

      // 更新地图列表
      await fileStore.loadMapList()
    } else {
      // 如果有重名的地图，需要二次确认弹窗
      importMapDialogVisible.value = true
      // 默认也是先配置不替换
      importSameMaps.value = sameNameMaps
      return
    }
  } catch (err) {
    // 导入失败提示
    errorMessage({
      content: i18n.global.t('desc.tips.failedToImportMap'),
    })

    console.log(`import hadmap error: `, err)
  }

  if (loadingInstance) {
    loadingInstance.close()
  }

  // 无论是否保存成功，都需要清空导入地图时的临时变量状态
  importDir.value = ''
  importSameMaps.value = []
  importDiffMaps.value = []
  clearSelectedMap()
}

// 替换二次确认操作后的处理
async function handleReplaceChange (mapOptions: Array<IImportMap>) {
  // 关闭替换导入地图二次确认弹窗
  importMapDialogVisible.value = false

  // 设置全屏 loading
  const loadingInstance = fullScreenLoading(
    i18n.global.t('desc.tips.importing'),
  )

  try {
    // 将同名地图替换导入二次确认后的结果，跟不重名地图直接导入的选项组装
    const allImportMaps = [...importDiffMaps.value, ...mapOptions]

    // 调用导入地图的接口
    const importRes = await importHadmap({
      dir: importDir.value,
      sources: allImportMaps,
    })
    if (!importRes || get(importRes, 'data.message') !== 'ok') {
      throw new Error(
        `api export hadmap error. error code: ${get(importRes, 'data.code')}`,
      )
    }

    // 导入成功提示
    successMessage({
      content: i18n.global.t('desc.tips.importMapSuccessfully'),
    })

    // 更新地图列表
    await fileStore.loadMapList()
  } catch (err) {
    // 导入失败提示
    errorMessage({
      content: i18n.global.t('desc.tips.failedToImportMap'),
    })

    console.log(`import hadmap error: `, err)
  }

  loadingInstance.close()
  // 无论是否保存成功，都需要清空导入地图时的临时变量状态
  importDir.value = ''
  importSameMaps.value = []
  importDiffMaps.value = []
  clearSelectedMap()
}
// -------- 导入地图 end --------
// -------- 导出地图 start --------
// 导出地图
async function exportMap () {
  if (exportDisabled.value) return
  if (!dialog) return

  const { filePaths } = await dialog.showOpenDialog({
    title: i18n.global.t('actions.file.selectADirectoryToExportTheMap'),
    properties: ['openDirectory'],
  })
  const [dest] = filePaths
  // 如果没有目标导出的目录，就直接返回
  if (!dest) return

  // 打开全屏的 loading
  const loadingInstance = fullScreenLoading(
    i18n.global.t('desc.tips.exporting'),
  )

  const selectedMapName = selectedMap.value.map(mapInfo => ({
    name: mapInfo.name,
  }))

  try {
    const exportRes = await exportHadmap({
      dest,
      sources: selectedMapName,
    })
    if (!exportRes || get(exportRes, 'data.message') !== 'ok') {
      throw new Error(
        `api export hadmap error. error code: ${get(exportRes, 'data.code')}`,
      )
    }

    // 导出成功提示
    successMessage({
      content: i18n.global.t('desc.tips.exportMapSuccessfully'),
    })
  } catch (err) {
    // 导出失败提示
    errorMessage({
      content: i18n.global.t('desc.tips.failedToExportMap'),
    })

    console.log(`export hadmap error: `, err)
  }

  // 无论是导出成功还是导出失败，都需要关闭全屏 loading
  loadingInstance.close()
}
// -------- 导出地图 end --------
// -------- 删除地图 start --------
// 删除地图
function deleteMap () {
  if (deleteDisabled.value) return

  // 预设地图不能删，筛选非预设的地图
  const noPresetMap = selectedMap.value.filter(
    mapInfo => mapInfo.preset === '0',
  )
  // 判断当前删除的地图列表中，是否包含当前打开的地图
  const { openedFile } = fileStore
  if (openedFile) {
    for (const mapInfo of selectedMap.value) {
      if (mapInfo.name === openedFile) {
        // 当前选中待删除的地图中，包含当前打开的地图
        warningMessage({
          content: i18n.global.t(
            'desc.tips.notSupportedDeleteCurrentOpenedMapFile',
          ),
        })
        return
      }
    }
  }

  if (noPresetMap.length < 1) return
  // 存在可删除的非预设地图，则展示删除提示弹窗二次确认
  deleteMapDialogVisible.value = true
}

// 取消删除
function cancelDelete () {
  deleteMapDialogVisible.value = false
}
// 确认删除
async function confirmDelete () {
  // 关闭确认弹窗
  deleteMapDialogVisible.value = false
  // 展示全屏的 loading
  const loadingInstance = fullScreenLoading(i18n.global.t('desc.tips.deleting'))

  try {
    // 预设地图不能删，筛选非预设的地图
    const noPresetMap = selectedMap.value
      .filter(mapInfo => mapInfo.preset === '0')
      .map(mapInfo => ({
        name: mapInfo.name,
      }))
    // 暂时先不显示删除了 x 条非预设地图
    // const noPresetNumber = noPresetMap.length

    const deleteRes = await deleteHadmap(noPresetMap)
    if (!deleteRes || get(deleteRes, 'data.message') !== 'ok') {
      throw new Error(
        `api delete hadmap error. error code: ${get(deleteRes, 'data.code')}`,
      )
    }

    // 删除成功提示
    successMessage({
      content: i18n.global.t('desc.tips.deleteMapSuccessfully'),
    })

    // 删除成功以后，更新一遍当前的用户列表
    await fileStore.loadMapList()

    // 清空选中的状态
    selectedMap.value = []
    isSelectedAll.value = false
  } catch (err) {
    // 删除失败提示
    errorMessage({
      content: i18n.global.t('desc.tips.failedToDeleteMap'),
    })

    console.log(`delete hadmap error: `, err)
  }

  loadingInstance.close()
  clearSelectedMap()
}
// -------- 删除地图 end --------
</script>

<template>
  <el-dialog
    v-model="visible"
    width="560px"
    class="map-list-dialog"
    :close-on-click-modal="false"
    :close-on-press-escape="false"
    align-center
    @close="handleClose"
  >
    <template #header>
      <!-- 使用嵌套的弹窗后，需要将自定义的弹窗名称手动在 #header 部分展示 -->
      <span>{{ $t('desc.mapFile.mapManagement') }}</span>
      <ImportMapDialog
        :visible="importMapDialogVisible"
        :same-name-maps="importSameMaps"
        @change="handleReplaceChange"
      />
      <DeleteMapDialog
        v-model="deleteMapDialogVisible"
        @cancel="cancelDelete"
        @confirm="confirmDelete"
      />
    </template>
    <div class="dialog-body">
      <div class="search-part">
        <el-input
          v-model="keyword"
          class="search"
          :placeholder="$t('desc.mapFile.searchName')"
          :prefix-icon="Search"
        />
      </div>
      <div class="batch-part">
        <el-checkbox
          v-model="isSelectedAll"
          class="select-all"
          @input="toggleSelectAll"
        >
          {{ $t('desc.mapFile.selectAll') }}
        </el-checkbox>
        <div>
          <BaseAuth :perm="['action.mapEditor.import.enable']">
            <el-tooltip :content="$t('desc.mapFile.import')" placement="bottom">
              <el-icon class="icon" @click="importMap">
                <Download />
              </el-icon>
            </el-tooltip>
          </BaseAuth>
          <BaseAuth :perm="['action.mapEditor.export.enable']">
            <el-tooltip :content="$t('desc.mapFile.export')" placement="bottom">
              <el-icon
                class="icon"
                :class="{ disabled: exportDisabled }"
                @click="exportMap"
              >
                <Upload />
              </el-icon>
            </el-tooltip>
          </BaseAuth>
          <BaseAuth :perm="['action.mapEditor.delete.enable']">
            <el-tooltip :content="$t('desc.mapFile.delete')" placement="bottom">
              <el-icon
                class="icon"
                :class="{ disabled: deleteDisabled }"
                @click="deleteMap"
              >
                <Delete />
              </el-icon>
            </el-tooltip>
          </BaseAuth>
        </div>
      </div>
      <div class="table-part">
        <el-table
          ref="table"
          size="small"
          height="320"
          :data="mapList"
          @selection-change="handleTableSelectionChange"
          @row-click="handleRowClick"
        >
          <el-table-column
            type="selection"
            label-class-name="select-all"
            width="24"
          />
          <el-table-column
            :label="$t('desc.mapFile.mapName')"
            property="name"
            show-overflow-tooltip
          >
            <template #default="scope">
              <div class="map-name">
                <span> {{ scope.row.name }} </span>
                <el-icon v-if="scope.row.preset !== '0'">
                  <Lock />
                </el-icon>
              </div>
            </template>
          </el-table-column>
          <el-table-column
            :label="$t('desc.mapFile.fileSize')"
            property="size"
            width="80"
          >
            <template #default="scope">
              <span class="file-size">{{ getFileSize(scope.row.size) }}</span>
            </template>
          </el-table-column>
          <el-table-column :label="$t('desc.mapFile.operation')" width="80">
            <template #default="scope">
              <span
                class="operation-btn"
                :underline="false"
                @click.stop="loadMap(scope.row)"
              >
                {{ $t('desc.mapFile.open') }}
              </span>
            </template>
          </el-table-column>
        </el-table>
      </div>
      <div class="button-part">
        <SimButton @click="handleClose">
          {{ $t('desc.mapFile.close') }}
        </SimButton>
      </div>
    </div>
  </el-dialog>
</template>

<style lang="less">
// 由于 dialog 的 class 属性，不会让元素带上局部的标识，所以在样式中暂不需要 scoped
.el-dialog.map-list-dialog {
  .el-dialog__header {
    padding: 0 18px 0 22px;
    height: 28px;
    line-height: 28px;
    background-color: var(--main-dark-color);
    display: flex;
    justify-content: space-between;
    margin-right: 0;

    .el-dialog__title {
      color: var(--dialog-title-color);
      font-size: 12px;
      line-height: inherit;
    }

    .el-dialog__headerbtn {
      width: auto;
      height: 28px;
      position: inherit;
      border: none;
    }
  }

  .el-dialog__body {
    padding: 20px 20px 16px;
    background-color: var(--main-area-color);

    .dialog-body {
      .search-part {
        text-align: center;

        .search {
          width: 200px;

          .el-input__wrapper {
            border-radius: 1px;
            height: 24px;
            border: var(--input-border);
            background-color: var(--main-dark-color);
            padding: 0 12px;

            .el-input__inner {
              font-size: var(--font-size);
              color: var(--text-color);
            }
          }
        }
      }

      .batch-part {
        display: flex;
        justify-content: space-between;
        align-content: center;
        height: 30px;
        line-height: 30px;

        .select-all {
          .el-checkbox__inner {
            background-color: var(--main-dark-color);
            border-width: 2px;
            border-color: var(--property-border-color);
            width: 12px;
            height: 12px;

            &::after {
              width: 4px;
              height: 6px;
              left: 2px;
              top: 0px;
            }
          }
          .is-checked {
            .el-checkbox__inner {
              background-color: #0491aa;
              border-color: #0491aa;
            }
          }
          .el-checkbox__label {
            font-size: var(--font-size);
            color: var(--property-title-color);
          }
          &.is-checked {
            .el-checkbox__label {
              color: var(--property-title-color);
            }
          }
        }

        .icon {
          font-size: 16px;
          margin: 0 6px;
          cursor: pointer;
          color: var(--text-color);

          &:hover {
            color: white;
          }
          &.disabled {
            cursor: default;
            color: inherit;
          }
        }
      }

      .table-part {
        margin-top: 10px;
        background-color: var(--main-dark-color);
        .el-table {
          background-color: var(--main-dark-color);
          th.el-table__cell {
            background-color: var(--main-dark-color);
            border-color: var(--property-border-color);
          }
          tr {
            background-color: var(--main-dark-color);
            .el-table__cell {
              border-color: var(--property-border-color);

              .map-name {
                color: var(--text-color);
                display: flex;
                align-items: center;
                span {
                  display: inline-block;
                  overflow: hidden;
                  white-space: nowrap;
                  text-overflow: ellipsis;
                }
                i {
                  margin: 0 10px;
                }
              }
              .file-size {
                color: var(--text-color);
              }

              .el-checkbox__inner {
                background-color: var(--main-dark-color);
                border-width: 2px;
                border-color: var(--property-border-color);
                &::after {
                  width: 4px;
                  height: 6px;
                  left: 2px;
                  top: 0px;
                }
              }
              .is-checked {
                .el-checkbox__inner {
                  background-color: #0491aa;
                  border-color: #0491aa;
                }
              }
              .operation-btn {
                color: #01aac8;
                font-size: var(--font-size);
                cursor: pointer;
              }
            }

            &:hover {
              td.el-table__cell {
                background-color: #242424;
              }
            }
          }
          .el-table__inner-wrapper::before {
            background-color: transparent;
          }
        }
        .el-table__header {
          .select-all .cell {
            visibility: hidden;
          }
        }
      }

      .button-part {
        padding-top: 12px;
        text-align: right;
      }
    }
  }
}
</style>
