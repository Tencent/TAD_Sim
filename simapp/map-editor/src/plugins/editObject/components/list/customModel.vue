<script setup lang="ts">
import { Delete, EditPen, Plus } from '@element-plus/icons-vue'
import { type Ref, ref, watch } from 'vue'
import { useHelperStore } from '@/stores/helper'
import { confirm } from '@/utils/common'
import i18n from '@/locales'
import {
  type ICustomModelConfig,
  useModelCacheStore,
} from '@/stores/object/modelCache'
import ee, { CustomEventType } from '@/utils/event'
import { isCustomModelInUsed } from '@/stores/object/common'

const helperStore = useHelperStore()
const modelCacheStore = useModelCacheStore()
const customModelList: Ref<Array<ICustomModelConfig>> = ref([])
watch(
  () => modelCacheStore.currentList,
  (val) => {
    customModelList.value = val
  },
  {
    immediate: true,
    deep: true,
  },
)

// 点击导入模型的入口
function handleImportCustomModel () {
  // 打开自定义导入模型的模态框
  helperStore.toggleDialogStatus('uploadModel', true)
}

// 二次编辑自定义模型
function handleEditModel (option: ICustomModelConfig) {
  // 二次编辑也需要打开模态框，展示对应自定义模型的信息
  helperStore.toggleDialogStatus('uploadModel', true)

  ee.emit(CustomEventType.other.updateCustomModelConfig, option.name)
}

// 删除自定义模型
async function handleDeleteModel (option: ICustomModelConfig, index: number) {
  const isModelInUsed = isCustomModelInUsed(option.name)

  const warningMsg = isModelInUsed ?
    i18n.global.t('desc.uploadModel.confirmToDeleteCustomModelInUsed') :
    i18n.global.t('desc.uploadModel.confirmToDeleteCustomModel')

  const confirmRes = await confirm({
    msg: warningMsg,
    type: 'que',
    title: i18n.global.t('desc.tips.tips'),
  })

  if (!confirmRes) return
  // 删除对应的模型数据
  modelCacheStore.deleteLocalCustomModel(option, index)
}
</script>

<template>
  <div class="object-list">
    <ul class="object-ul">
      <li
        v-for="(modelOption, index) of customModelList"
        :key="modelOption.name"
        class="object-li"
      >
        <div v-my-drag="modelOption" class="icon-box" draggable="false">
          <img :src="modelOption.thumbUrl" class="icon" draggable="false">
          <!-- 交互区域组织拖拽交互的冒泡 -->
          <div class="edit-area" @pointerdown.stop="">
            <BaseAuth
              :perm="['action.mapEditor.importCustomModels.modify.enable']"
            >
              <span
                class="tool-icon"
                @click.stop="handleEditModel(modelOption)"
              >
                <EditPen />
              </span>
            </BaseAuth>
            <BaseAuth
              :perm="['action.mapEditor.importCustomModels.delete.enable']"
            >
              <span
                class="tool-icon"
                @click.stop="handleDeleteModel(modelOption, index)"
              >
                <Delete />
              </span>
            </BaseAuth>
          </div>
        </div>
        <el-tooltip :content="modelOption.name">
          <span class="title custom-title">{{ modelOption.name }}</span>
        </el-tooltip>
      </li>
      <!-- 导入自定义模型的入口 -->
      <BaseAuth :perm="['action.mapEditor.importCustomModels.add.enable']">
        <li class="object-li">
          <div class="icon-box" @click="handleImportCustomModel">
            <el-icon class="import-icon">
              <Plus />
            </el-icon>
          </div>
          <span class="title">{{ $t('actions.object.importModel') }}</span>
        </li>
      </BaseAuth>
    </ul>
  </div>
</template>

<style lang="less">
.object-li .icon-box {
  position: relative;

  .edit-area {
    position: absolute;
    bottom: 0;
    width: 100%;
    height: 20px;
    background-color: rgba(0, 0, 0, 0.3);
    display: none;
    justify-content: center;
    align-items: center;

    .tool-icon {
      margin: 0 4px;
      width: 16px;
      height: 20px;
      display: inline-flex;
      justify-content: center;
      align-items: center;
      cursor: pointer;
      color: var(--property-title-color);

      svg {
        width: 14px;
        height: 14px;
      }

      &:hover {
        color: var(--text-color);
      }
    }
  }

  &:hover {
    .edit-area {
      display: flex;
    }
  }

  .import-icon {
    font-size: 20px;
  }
}

.custom-title {
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}
</style>
