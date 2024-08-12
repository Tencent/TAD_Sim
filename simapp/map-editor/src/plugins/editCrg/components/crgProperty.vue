<script setup lang="ts">
/* global biz */
import { type Ref, computed, ref, watch } from 'vue'
import { cloneDeep, get, isEqual } from 'lodash'
import { useCrgInteractionStore } from '../store/interaction'
import { useRoadStore } from '@/stores/road'
import InputNumber from '@/components/common/inputNumber.vue'
import DeleteSvg from '@/assets/icons/delete.svg'
import { globalConfig } from '@/utils/preset'
import i18n from '@/locales'
import { useHelperStore } from '@/stores/helper'

interface ICrgOption extends biz.IOpenCrg {
  // 展示的 crg 配置记录当前归属的道路
  roadId: string
}

const { isCloud } = globalConfig

const roadStore = useRoadStore()
const interactionStore = useCrgInteractionStore()
const helperStore = useHelperStore()
const table = ref()
const activeNames = ref(['roadSurface'])
const crgList: Ref<Array<biz.IOpenCrg>> = ref([])
const currentCrgConfig: Ref<ICrgOption | null> = ref(null)

// 单机版 electron 选择本地文件
const dialog = get(window, 'electron.dialog')

// 当前选中的道路
const currentRoad = computed(() => {
  return interactionStore.selectedRoad
})

watch(
  currentRoad,
  (val) => {
    if (!val) {
      reset()
      return
    }
    if (val.crgConfig) {
      crgList.value = cloneDeep([...val.crgConfig])
      if (currentCrgConfig.value) {
        const { file, showName } = currentCrgConfig.value

        // 更新当前展示的 crg 配置的内容
        let index = -1
        // 通过 file 属性找到对应 crg 选项进行更新
        for (let i = 0; i < val.crgConfig.length; i++) {
          if (index > -1) continue
          const crgOption = val.crgConfig[i]
          // 通过 file 和 showName 完全一致来确认唯一性
          if (file === crgOption.file && showName === crgOption.showName) {
            index = i
          }
        }
        if (index > -1) {
          currentCrgConfig.value = cloneDeep({
            ...val.crgConfig[index],
            roadId: val.id,
          })
        }
      }

      // 如果前后两次选中的道路不一样，则需要重置面板 crg 配置的展示
      if (currentCrgConfig.value && currentCrgConfig.value.roadId !== val.id) {
        reset()
      }
    } else {
      crgList.value = []
      reset()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  currentCrgConfig,
  (val) => {
    if (!currentRoad.value || !currentRoad.value.crgConfig) return
    if (!val) return

    const { file, showName } = val

    let index = -1
    // 通过 file 属性找到对应 crg 选项进行更新
    for (let i = 0; i < currentRoad.value.crgConfig.length; i++) {
      if (index > -1) continue
      const crgOption = currentRoad.value.crgConfig[i]
      // 通过 file 和 showName 完全一致来确认唯一性
      if (file === crgOption.file && showName === crgOption.showName) {
        index = i
      }
    }
    if (index === -1) return
    // 判断当前更新的内容，跟之前的内容是否有差别
    const compareRes = isEqual(val, currentRoad.value.crgConfig[index])
    if (compareRes) return

    // 监听 crg 配置属性的更新，调用数据层更新 crg 的方法
    roadStore.updateCrg({
      roadId: currentRoad.value.id,
      file,
      showName,
      crgConfig: val,
    })
  },
  {
    immediate: true,
    deep: true,
  },
)

// 新增 crg 配置
async function addCrgOption () {
  if (!currentRoad.value) return

  if (isCloud) {
    // 云端版，通过模态框请求接口选取关联的 crg 文件
    helperStore.toggleDialogStatus('crg', true)
  } else {
    // 单机版，通过 electron 提供的选择本地文件能力
    if (!dialog) return
    try {
      const { filePaths } = await dialog.showOpenDialog({
        title: i18n.global.t('actions.crg.pleaseSelectCrgFile'),
        properties: ['openFile'],
        filters: [
          {
            name: i18n.global.t('desc.editCrg.crgFile'),
            extensions: ['crg'],
          },
        ],
      })
      if (filePaths.length < 0) return

      const crgFilePath = filePaths[0]
      // 从数据层支持 crg 文件的关联
      roadStore.addCrg({
        roadId: currentRoad.value.id,
        file: crgFilePath,
      })
    } catch (err) {
      console.log('select crg file error.', err)
    }
  }

  reset()
}

// 删除目标道路上的 crg 配置
function removeCrgOption (row: biz.IOpenCrg) {
  if (!currentRoad.value) return

  roadStore.removeCrg({
    roadId: currentRoad.value.id,
    file: row.file,
    showName: row.showName,
  })

  reset()
}

function reset () {
  currentCrgConfig.value = null
  if (table.value) {
    table.value.setCurrentRow()
  }
}

// 选中一个 crg 配置
function handleRowClick (row: biz.IOpenCrg) {
  if (!currentRoad.value) return
  currentCrgConfig.value = cloneDeep({ ...row, roadId: currentRoad.value.id })
  table.value.setCurrentRow(row)
}
</script>

<template>
  <template v-if="currentRoad">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentRoadProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadId') }}</label>
        <div class="content">
          {{ currentRoad.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadLength') }}</label>
        <div class="content">
          {{ currentRoad.length }} m
        </div>
      </div>
      <el-form class="property-form" label-width="70" @submit.enter.prevent>
        <el-collapse v-model="activeNames">
          <el-collapse-item
            :title="$t('desc.editCrg.roadSurface')"
            name="roadSurface"
          >
            <div
              class="property-cell"
              :style="{
                marginTop: '10px',
              }"
            >
              <label class="label">{{ $t('desc.editCrg.bindOpenCRG') }}</label>
              <div class="content">
                <el-button class="right-btn" @click="addCrgOption">
                  {{
                    $t('desc.editCrg.addData')
                  }}
                </el-button>
              </div>
            </div>
            <div v-if="crgList.length > 0" class="property-cell table-part">
              <el-table
                ref="table"
                size="small"
                :data="crgList"
                class="crg-table"
                highlight-current-row
                @row-click="handleRowClick"
              >
                <el-table-column type="index" width="20" />
                <el-table-column
                  prop="showName"
                  :label="$t('desc.editCrg.roadSurfaceDataFile')"
                >
                  <template #default="scope">
                    <el-tooltip :content="scope.row.file" placement="top">
                      <span>{{ scope.row.showName }}</span>
                    </el-tooltip>
                  </template>
                </el-table-column>
                <el-table-column
                  v-if="isCloud"
                  prop="version"
                  align="right"
                  width="50"
                  :label="$t('desc.editCrg.version')"
                />

                <el-table-column
                  :label="$t('desc.editCrg.operation')"
                  align="right"
                  width="50"
                >
                  <template #default="scope">
                    <span class="icon">
                      <DeleteSvg @click.stop="removeCrgOption(scope.row)" />
                    </span>
                  </template>
                </el-table-column>
              </el-table>
            </div>
            <template v-if="currentCrgConfig">
              <div class="property-cell">
                <label class="label">{{
                  $t('desc.editCrg.orientation')
                }}</label>
                <div class="content">
                  <el-select
                    v-model="currentCrgConfig.orientation"
                    :style="{ width: '100px' }"
                    placement="bottom"
                  >
                    <el-option
                      :label="$t('desc.editCrg.same')"
                      value="same"
                    />
                    <el-option
                      :label="$t('desc.editCrg.opposite')"
                      value="opposite"
                    />
                  </el-select>
                </div>
              </div>
              <div class="property-cell">
                <label class="label">{{ $t('desc.editCrg.mode') }}</label>
                <div class="content">
                  <el-select
                    v-model="currentCrgConfig.mode"
                    :style="{ width: '100px' }"
                    placement="bottom"
                  >
                    <el-option
                      :label="$t('desc.editCrg.attached')"
                      value="attached"
                    />
                    <el-option
                      :label="$t('desc.editCrg.attached0')"
                      value="attached0"
                    />
                    <el-option
                      :label="$t('desc.editCrg.genuine')"
                      value="genuine"
                    />
                    <el-option
                      :label="$t('desc.editCrg.global')"
                      value="global"
                    />
                  </el-select>
                </div>
              </div>
              <div class="property-cell">
                <label class="label">{{ $t('desc.editCrg.purpose') }}</label>
                <div class="content">
                  <el-select
                    v-model="currentCrgConfig.purpose"
                    :style="{ width: '100px' }"
                    placement="bottom"
                  >
                    <el-option
                      :label="$t('desc.editCrg.elevation')"
                      value="elevation"
                    />
                    <el-option
                      :label="$t('desc.editCrg.friction')"
                      value="friction"
                    />
                  </el-select>
                </div>
              </div>
              <div class="property-cell">
                <label class="label">{{ $t('desc.editCrg.sOffset') }}</label>
                <div class="content">
                  <InputNumber
                    v-model="currentCrgConfig.sOffset"
                    :disabled="false"
                    :precision="1"
                    unit="m"
                  />
                </div>
              </div>
              <div class="property-cell">
                <label class="label">{{ $t('desc.editCrg.tOffset') }}</label>
                <div class="content">
                  <InputNumber
                    v-model="currentCrgConfig.tOffset"
                    :disabled="false"
                    :precision="1"
                    unit="m"
                  />
                </div>
              </div>
              <div class="property-cell">
                <label class="label">{{ $t('desc.editCrg.zOffset') }}</label>
                <div class="content">
                  <InputNumber
                    v-model="currentCrgConfig.zOffset"
                    :disabled="false"
                    :precision="1"
                    unit="m"
                  />
                </div>
              </div>
              <div class="property-cell">
                <label class="label">{{ $t('desc.editCrg.zScale') }}</label>
                <div class="content">
                  <InputNumber
                    v-model="currentCrgConfig.zScale"
                    :disabled="false"
                    :precision="1"
                    unit=""
                  />
                </div>
              </div>
              <div class="property-cell">
                <label class="label">{{ $t('desc.editCrg.hOffset') }}</label>
                <div class="content">
                  <InputNumber
                    v-model="currentCrgConfig.hOffset"
                    :disabled="false"
                    :precision="1"
                    :max="180"
                    :min="-180"
                    unit="°"
                  />
                </div>
              </div>
            </template>
          </el-collapse-item>
        </el-collapse>
      </el-form>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
.property-container {
  .property-cell {
    .right-btn {
      float: right;
    }
  }
  .table-part {
    :deep(.el-table) {
      background-color: var(--main-dark-color);
      color: var(--text-color);
      th.el-table__cell {
        background-color: var(--main-dark-color);
        border-color: var(--property-border-color);
        color: var(--text-color);
      }

      tr {
        background-color: var(--main-dark-color);
        &:hover {
          td.el-table__cell {
            background-color: #242424;
          }
        }
        &.current-row {
          td.el-table__cell,
          &:hover td.el-table__cell {
            background-color: var(--property-border-color);
          }
        }
        .el-table__cell {
          border-color: var(--property-border-color);
        }
      }
      .el-table__inner-wrapper::before {
        background-color: transparent;
      }
      .icon {
        cursor: pointer;
        width: 14px;
        height: 14px;
        display: inline-block;
      }
    }
  }
}
</style>
