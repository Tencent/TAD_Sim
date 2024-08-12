<script setup lang="ts">
import { onMounted, onUnmounted, ref } from 'vue'
import TopArea from './topArea/topArea.vue'
import Toolbar from './toolbar/toolbar.vue'
import LeftArea from './leftArea/leftArea.vue'
import RightArea from './rightArea/rightArea.vue'
import MainArea from './mainArea/mainArea.vue'
import BottomArea from './bottomArea/bottomArea.vue'
import ResizeLayer from './common/resizeLayer.vue'
import Keyboard from './common/keyboard.vue'
import MapFileDialog from './dialog/mapFileDialog.vue'
import SaveMapDialog from './dialog/saveMapDialog.vue'
import MapNotSaveDialog from './dialog/mapNotSaveDialog.vue'
import SelectCrgDialog from './dialog/selectCrgDialog.vue'
import UploadModelDialog from './dialog/uploadModelDialog.vue'
import Selected from './common/selected.vue'
import StructureContextMenu from './common/structureContextMenu.vue'
import { useFileStore } from '@/stores/file'
import { clearIdCache } from '@/utils/guid'
import { useHelperStore } from '@/stores/helper'
import { globalConfig } from '@/utils/preset'
import { LayoutConstant } from '@/utils/common'
import { useLayoutStore } from '@/stores/layout'
import ee, { CustomEventType } from '@/utils/event'

const fileStore = useFileStore()
const layoutStore = useLayoutStore()
const helperStore = useHelperStore()

const {
  leftMaxWidth,
  leftMinWidth,
  rightMaxWidth,
  rightMinWidth,
  bottomMaxHeight,
  bottomMinHeight,
} = LayoutConstant

// 给文档流中的 body 元素新增主题配色的 class
const themeClassName = 'dark-theme'
const body = window.document.querySelector('body')
body && body.setAttribute('class', themeClassName)

// 获取是否是云端环境
const { isCloud } = globalConfig
// 打开自定义模型模态框的透传参数
const customModelName = ref('')

onMounted(async () => {
  clearIdCache()

  if (!isCloud) {
    // 如果是非云端环境，则需要创建对应的 session
    // 创建 hadmap 服务的会话
    await fileStore.initSession()
  }

  // 订阅打开自定义模型模态框的参数更新
  ee.on(CustomEventType.other.updateCustomModelConfig, handleCustomModelParams)
})

onUnmounted(() => {
  // 销毁时取消事件监听
  ee.off(CustomEventType.other.updateCustomModelConfig, handleCustomModelParams)
})

// 自定义模型模态框启动参数
function handleCustomModelParams (name: string) {
  customModelName.value = name
}
</script>

<template>
  <div class="map-editor">
    <Keyboard />
    <Selected />
    <StructureContextMenu />
    <div class="top-container">
      <TopArea />
    </div>
    <div class="toolbar-container">
      <Toolbar />
    </div>
    <div class="main-container">
      <div
        class="left-area-container"
        :style="{ width: `${layoutStore.leftAreaWidth}px` }"
      >
        <ResizeLayer
          direction="right"
          :value="layoutStore.leftAreaWidth"
          :max="leftMaxWidth"
          :min="leftMinWidth"
          @move="layoutStore.resizeLeftAreaWidth"
        />
        <LeftArea />
      </div>
      <div class="center-area-container">
        <MainArea />
        <div
          class="bottom-area-container"
          :style="{ height: `${layoutStore.bottomAreaHeight}px` }"
        >
          <ResizeLayer
            direction="top"
            :value="layoutStore.bottomAreaHeight"
            :max="bottomMaxHeight"
            :min="bottomMinHeight"
            @move="layoutStore.resizeBottomAreaHeight"
          />
          <BottomArea />
        </div>
      </div>
      <div
        class="right-area-container"
        :style="{ width: `${layoutStore.rightAreaWidth}px` }"
      >
        <ResizeLayer
          direction="left"
          :value="layoutStore.rightAreaWidth"
          :max="rightMaxWidth"
          :min="rightMinWidth"
          @move="layoutStore.resizeRightAreaWidth"
        />
        <RightArea />
      </div>
      <MapFileDialog v-if="helperStore.mapFileDialogShow" />
      <SaveMapDialog v-if="helperStore.saveMapDialogShow" />
      <SaveMapDialog v-if="helperStore.saveAsMapDialogShow" :is-save-as="true" />
      <MapNotSaveDialog v-if="helperStore.mapNotSaveDialogShow" />
      <SelectCrgDialog v-if="helperStore.selectCrgDialogShow" />
      <UploadModelDialog
        v-if="helperStore.uploadModelDialogShow"
        :custom-model-name="customModelName"
      />
    </div>
  </div>
</template>

<style scoped lang="less">
.map-editor {
  position: relative;
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;

  --top-area-height: 40px;
  --toolbar-height: 60px;

  .top-container {
    height: var(--top-area-height);
    line-height: var(--top-area-height);
    position: relative;
  }

  .toolbar-container {
    height: var(--toolbar-height);
    line-height: var(--toolbar-height);
    position: relative;
    box-sizing: border-box;
    border-bottom: var(--area-border);
  }
  .main-container {
    flex: 1;
    display: flex;
    overflow: hidden;

    .left-area-container {
      position: relative;
      overflow: hidden;
      flex-grow: 0;
      flex-shrink: 0;
      box-sizing: border-box;
      border-right: var(--area-border);
    }

    .right-area-container {
      position: relative;
      display: flex;
      flex-direction: column;
      flex-grow: 0;
      flex-shrink: 0;
      box-sizing: border-box;
      border-left: var(--area-border);
    }

    .center-area-container {
      flex: 1;
      display: flex;
      flex-direction: column;
      overflow: hidden;

      .bottom-area-container {
        position: relative;
        box-sizing: border-box;
        border-top: var(--area-border);
      }
    }
  }
}
</style>
