<script setup lang="ts">
// @ts-nocheck
import { type Ref, computed, onMounted, ref } from 'vue'
import { cloneDeep } from 'lodash'
import type Node from 'element-plus/es/components/tree/src/model/node'
import SimButton from '../common/button.vue'
import { useHelperStore } from '@/stores/helper'
import { useFileStore } from '@/stores/file'
import { getCrgDetailInCloud, getMapCrgListInCloud } from '@/services'
import { useRoadStore } from '@/stores/road'
import { useCrgInteractionStore } from '@/plugins/editCrg/store/interaction'
import { warningMessage } from '@/utils/common'
import i18n from '@/locales'

interface Tree {
  id: string
  rootId?: string
  fileKey?: string // crg 对应的文件路径（包含 crg 文件名称）
  name: string // crg 名称
  key: string
  hasChildren: boolean
  leaf: boolean
  children?: Array<Tree>
  isCrgFile: boolean
}
interface ICrgOption {
  id: string
  name: string
  version: string
}
const props = {
  children: 'children',
  label: 'name',
  isLeaf: 'isCrgFile',
  class: (data: Tree) => {
    // 给 crg 配置的叶子节点添加类标识
    if (data.isCrgFile) {
      return 'is-target-node'
    }
    return null
  },
}

const tree = ref()
const targetCrg: Ref<ICrgOption> = ref({
  id: '',
  name: '',
  fileKey: '',
  version: '',
})
const crgList = ref([])
const versions = ref([])
const fileStore = useFileStore()
const helperStore = useHelperStore()
const roadStore = useRoadStore()
const crgInteractionStore = useCrgInteractionStore()
const isLoading = ref(false)

// 地图未保存弹窗，由于没有固定的触发交互，判断当前弹窗可见时，禁用键盘热键
const visible = computed(() => {
  if (helperStore.selectCrgDialogShow) {
    // 弹窗展示后，禁用热键
    helperStore.closeShortcutKeys()
  }
  return helperStore.selectCrgDialogShow
})

onMounted(async () => {
  crgList.value = cloneDeep(fileStore.crgList)
})

async function loadNode (node: Node, resolve) {
  if (node.level === 0) {
    return resolve(crgList.value)
  }
  if (node.level === 1) {
    return resolve(node.data.children)
  }

  if (node.level > 1) {
    const mapGroupId = Number(node.data.id)

    // 只有叶子节点，才能保存 crg 文件
    if (node.data.leaf === true && node.data.isCrgFile === false) {
      const crgFileList = []
      // 请求当前 crgGroup 下的所有 crg 列表
      try {
        const { data } = await getMapCrgListInCloud(mapGroupId)
        if (data.message !== 'ok') {
          throw new Error(`get map crg list error. groupId is ${mapGroupId}`)
        }
        const fileList = data.data.records || []
        for (let i = 0; i < fileList.length; i++) {
          const { id, name, rootId = -1, fileKey = '' } = fileList[i]
          crgFileList.push({
            id: String(id),
            rootId: String(rootId),
            fileKey,
            name,
            key: `${name}_${id}`,
            hasChildren: false,
            children: [],
            leaf: true,
            isCrgFile: true,
          })
        }
      } catch (err) {
        console.log(err)
      }
      return resolve(crgFileList)
    } else {
      // 通过接口请求加载对应的叶子节点
      const children = await fileStore.syncCrgList(mapGroupId)
      return resolve(children)
    }
  }
}

// 点击取消按钮
function handleClose () {
  if (isLoading.value) return

  reset()
}
// 点击确认按钮
function handleConfirm () {
  if (isLoading.value) return

  if (!targetCrg.value.id || !targetCrg.value.name) {
    warningMessage({
      content: i18n.global.t('desc.editCrg.noRoadSurfaceDataAssociated'),
    })
    return
  }
  if (!targetCrg.value.version) {
    warningMessage({
      content: i18n.global.t('desc.editCrg.noFileVersionAssociated'),
    })
    return
  }

  if (!crgInteractionStore.roadId) return

  // 更新当前选中道路的关联的 openCrg 文件
  roadStore.addCrg({
    roadId: crgInteractionStore.roadId,
    file: targetCrg.value.fileKey,
    id: targetCrg.value.id,
    version: targetCrg.value.version,
    showName: targetCrg.value.name,
  })

  reset()
}

function reset () {
  // 重置状态
  targetCrg.value = {
    id: '',
    name: '',
    version: '',
  }
  crgList.value = []
  versions.value = []

  // 恢复快捷键的操作
  helperStore.openShortcutKeys()

  // 关闭弹窗
  helperStore.toggleDialogStatus('crg', false)
}

async function handleChangeCrgFile (node: Tree) {
  // 只对 crg 文件叶子节点生效
  if (!node.isCrgFile) return
  try {
    isLoading.value = true
    const { data } = await getCrgDetailInCloud(Number(node.rootId))
    if (data.message !== 'ok') {
      throw new Error(`get crg detail error. groupId is ${node.rootId}`)
    }

    const crgDetail = data.data || []
    // number 类型的版本数组
    const _versions = crgDetail.map(option => option.version)

    // 转换成形如 v1.0 的版本号
    versions.value = _versions.map(v => `v${v}.0`)
    if (_versions[0]) {
      // 更新当前选中的 crg 文件配置
      targetCrg.value = {
        id: node.id,
        name: node.name,
        fileKey: node.fileKey,
        // 默认选择最新的版本
        version: `v${_versions[0]}.0`,
      }
    } else {
      // 更新当前选中的 crg 文件配置
      targetCrg.value = {
        id: node.id,
        name: node.name,
        fileKey: node.fileKey,
        version: '',
      }
    }

    isLoading.value = false
    return
  } catch (err) {
    console.log(err)
  }
  versions.value = []
  isLoading.value = false
}
</script>

<template>
  <el-dialog
    v-model="visible"
    width="420px"
    class="select-crg-dialog"
    :close-on-click-modal="false"
    :show-close="false"
    :destroy-on-close="true"
    :close-on-press-escape="false"
    append-to-body
    align-center
  >
    <template #header>
      <!-- 使用嵌套的弹窗后，需要将自定义的弹窗名称手动在 #header 部分展示 -->
      <span>{{ $t('desc.editCrg.roadSurface') }}</span>
    </template>
    <div class="dialog-body">
      <div>
        <div class="row">
          <div class="label">
            {{ $t('desc.editCrg.bindRoadSurfaceData') }}
          </div>
          <div class="input-part" :style="{ width: '260px' }">
            <el-input
              v-model="targetCrg.name"
              :disabled="true"
              class="file-name"
            />
            <div class="tree-container">
              <el-tree
                ref="tree"
                :props="props"
                :load="loadNode"
                lazy
                node-key="value"
                :style="{ color: 'inherit', background: 'inherit' }"
                @current-change="handleChangeCrgFile"
              />
            </div>
          </div>
        </div>
        <div v-if="versions.length > 0" class="row">
          <div class="label">
            {{ $t('desc.editCrg.bindFileVersion') }}
          </div>
          <div class="input-part">
            <el-select
              v-model="targetCrg.version"
              :placeholder="$t('actions.tips.select')"
              :style="{ width: '280px' }"
            >
              <el-option
                v-for="version of versions"
                :key="version"
                :value="version"
              />
            </el-select>
          </div>
        </div>
      </div>
      <div class="button-part">
        <SimButton @click="handleClose">
          {{
            $t('desc.tips.cancel')
          }}
        </SimButton>
        <SimButton
          class="primary"
          :loading="isLoading"
          @click="handleConfirm"
        >
          {{ $t('desc.tips.confirm') }}
        </SimButton>
      </div>
    </div>
  </el-dialog>
</template>

<style lang="less">
.el-dialog.select-crg-dialog {
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
      .row {
        display: flex;
        margin-bottom: 20px;

        .label {
          color: var(--text-color);
          width: 100px;
          text-align: left;
          height: 26px;
          line-height: 26px;
        }
        .input-part {
          margin-bottom: 0px;
          flex-grow: 1;
          .file-name {
            margin-bottom: 10px;
          }
          .tree-container {
            width: 280px;
            height: 300px;
            background-color: var(--input-background);
            color: var(--text-color);
            overflow: auto;
            .el-tree .el-tree-node__content {
              background-color: var(--input-background);
            }
            .el-tree .el-tree-node__content:hover {
              background-color: var(--object-tab-background);
            }
            .el-tree .is-current > .el-tree-node__content {
              background-color: var(--object-tab-divider-color);
            }
            // 可选的 crg 配置叶子节点的样式
            .is-target-node .el-tree-node__content .el-tree-node__label {
              position: relative;
              &::before {
                content: '';
                display: inline-block;
                width: 8px;
                height: 8px;
                background-color: var(--text-color);
                position: absolute;
                top: 6px;
                left: -12px;
                border-radius: 4px;
              }
            }
          }
        }
      }
      .input-part {
        margin-bottom: 14px;

        .el-input__wrapper {
          border-radius: 1px;
          height: 24px;
          border: var(--input-border);
          background-color: var(--main-dark-color);

          .el-input__inner {
            font-size: var(--font-size);
            color: var(--text-color);
          }
        }
      }
      .button-part {
        text-align: right;
        button {
          margin-left: 10px;
        }
      }
    }
  }
}
</style>
