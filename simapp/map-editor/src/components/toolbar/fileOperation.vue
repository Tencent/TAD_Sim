<script setup lang="ts">
// @ts-nocheck
import { debounce } from 'lodash'
import { computed } from 'vue'
import OpenSvg from './fileIcon/open.svg?component'
import NewSvg from './fileIcon/new.svg?component'
import SaveSvg from './fileIcon/save.svg?component'
import { useFileStore } from '@/stores/file'
import { globalConfig } from '@/utils/preset'
import root3d from '@/main3d/index'
import { pickUpMapElementData } from '@/services/saveParser'
import { useHistoryStore } from '@/stores/history'
import { warningMessage } from '@/utils/common'
import i18n from '@/locales'
import { useModelCacheStore } from '@/stores/object/modelCache'

const { exposeModule, isCloud } = globalConfig
const { file: exposeFileEntry } = exposeModule
const {
  open: enableOpen,
  create: enableCreate,
  save: enableSave,
  saveAs: enableSaveAs,
} = exposeFileEntry

const fileStore = useFileStore()
const historyStore = useHistoryStore()
const modelCacheStore = useModelCacheStore()

// 根据操作记录，来判断当前是否有diff数据，即是否能保存
const hasDiff = computed(() => {
  return historyStore.getDiffRecord().length > 0
})

// enableSave 是配置层面的是否能保存，而 canSave 是根据当前是否有操作记录来判断
const canSave = computed(() => {
  // modelCacheStore.hasDeletedObjWhenDeletingCustomModel 表示是否通过左侧列表中【我的模型】【删除自定义模型】删除了【场景中的元素】
  return hasDiff.value || modelCacheStore.hasDeletedObjWhenDeletingCustomModel
})

async function handleOpen () {
  fileStore.openMapList()
}

function handleCreateNew () {
  fileStore.createNew()
}

// 通过防抖避免连续点击频发触发
const handleSave = debounce(() => {
  // 如果没有有效的操作记录，则不会触发
  if (!canSave.value) return

  // 如果处于loading状态，则不能重复执行保存操作
  if (fileStore.isLoading) return

  // 不允许直接保存预设地图
  if (fileStore.isPreset) {
    warningMessage({
      content: i18n.global.t('desc.tips.cannotSavePresetMapFileDirectly'),
    })
    return
  }

  fileStore.saveMap()
}, 300)

// 另存为
async function handleSaveAs () {
  fileStore.saveMap(true)
}

window.testSaveMap = () => {
  testSave()
}

async function testSave () {
  const data = await pickUpMapElementData()
  console.log('save map data:', data)
}

window.testLoadMap = () => {
  testLoad()
}

function testLoad () {
  fileStore.loadMap('test')
}

function exportObjFile () {
  root3d.testExporter()
}
</script>

<template>
  <ul class="file-btn-ul">
    <li v-if="enableOpen && !isCloud" class="file-btn-li" @click="handleOpen">
      <OpenSvg class="icon" />
      <label class="label">{{ $t('menu.open') }}</label>
    </li>
    <li
      v-if="enableCreate && !isCloud"
      class="file-btn-li"
      @click="handleCreateNew"
    >
      <NewSvg class="icon" />
      <label class="label">{{ $t('menu.new') }}</label>
    </li>
    <li
      v-if="enableSave"
      class="file-btn-li"
      :class="{ disabled: !canSave }"
      @click="handleSave"
    >
      <SaveSvg class="icon" />
      <label class="label">{{ $t('menu.save') }}</label>
    </li>
    <!-- 另存为 -->
    <li
      v-if="enableSaveAs && isCloud"
      class="file-btn-li"
      @click="handleSaveAs"
    >
      <SaveSvg class="icon" />
      <label class="label">{{ $t('menu.saveAs') }}</label>
    </li>
    <!-- <li v-if="enableSave" class="file-btn-li" @click="testSave">
      <SaveSvg class="icon" />
      <label class="label">测试保存物体</label>
    </li>
    <li v-if="enableSave" class="file-btn-li" @click="testLoad">
      <SaveSvg class="icon" />
      <label class="label">测试加载物体</label>
    </li> -->
    <!-- <li class="file-btn-li" @click="exportObjFile">
      <SaveSvg class="icon" />
      <label class="label">导出Obj</label>
    </li> -->
  </ul>
</template>

<style scoped lang="less">
.file-btn-ul {
  display: flex;
  align-items: center;
  flex-wrap: nowrap;
  margin-left: 10px;

  .file-btn-li {
    margin: 0 8px;
    display: flex;
    flex-direction: column;
    align-items: center;
    cursor: pointer;
    --disable-color: #6f6f6f;

    &:hover {
      color: var(--active-color);
    }
    &:active {
      color: var(--active-color);
    }

    &.disabled {
      cursor: auto;
      color: var(--disable-color);

      .label {
        cursor: auto;
      }
    }

    .icon {
      width: 20px;
      height: 20px;
    }
    .label {
      font-size: 12px;
      line-height: 1.6em;
      cursor: pointer;
    }
  }
}
</style>
