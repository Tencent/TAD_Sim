<script setup lang="ts">
// @ts-nocheck
import { get } from 'lodash'
import { computed, nextTick, onMounted, ref, watch } from 'vue'
import SimButton from '../common/button.vue'
import InfoSvg from './icon/excmark.svg?component'
import i18n from '@/locales'
import { getMapCatalogueInCloud, getMapList } from '@/services'
import { useFileStore } from '@/stores/file'
import { useHelperStore } from '@/stores/helper'
import { MapType } from '@/utils/business'
import { globalConfig } from '@/utils/preset'
import { calcStrLength, isNameInvalidated } from '@/utils/tools'
import { getPermission } from '@/utils/permission'

const props = defineProps({
  isSaveAs: {
    type: Boolean,
    default: false,
  },
})
const fileSuffixList = [
  { value: '.xodr v1.4', label: 'xodr v1.4' },
  { value: '.xodr v1.5', label: 'xodr v1.5' },
  { value: '.sqlite', lable: '.sqlite' },
]
const shownFileSuffixList = fileSuffixList.filter((item) => {
  if (item.value === '.xodr v1.4') {
    return getPermission('action.mapEditor.fileType.openDrive1.4.show')
  }
  if (item.value === '.xodr v1.5') {
    return getPermission('action.mapEditor.fileType.openDrive1.5.show')
  }
  if (item.value === '.sqlite') {
    return getPermission('action.mapEditor.fileType.sqlite.show')
  }
  return true
})
const defaultFileSuffix = shownFileSuffixList[0]?.value

const { isCloud, projectName } = globalConfig
const helperStore = useHelperStore()
const fileStore = useFileStore()
const dialogTitle = ref(i18n.global.t('menu.save'))
const mapNameInputRef = ref()
const newMapName = ref('')
// 默认为 xodr 格式
const fileSuffix = ref(defaultFileSuffix)
const validateContent = ref('')
// 云端版，保存的地图目录验证信息
const validateMapGroupContent = ref('')
// 是否处于验证名称合规性过程中(由于可能需要获取地图列表接口，存在pending时间)
const isValidating = ref(false)

const visible = computed(() => {
  if (props.isSaveAs) {
    helperStore.saveAsMapDialogShow && onShow()
    return helperStore.saveAsMapDialogShow
  } else {
    helperStore.saveMapDialogShow && onShow()
    return helperStore.saveMapDialogShow
  }
})
const isLoading = computed(() => {
  return fileStore.isLoading
})

// 地图分组的列表
const mapCatalogueList = ref([])
// 是否展示地图目录
const showMapCatalogue = ref(false)
// 当前选中的地图目录 id
const mapGroupId = ref('')

// 递归获取地图目录的层级
function recursionMapGroup (data) {
  if (!data || data.length < 1) return null
  const arr = []
  for (let i = 0; i < data.length; i++) {
    const { id, name, childMapAggregate, leaf } = data[i]
    const children = recursionMapGroup(childMapAggregate)

    const option = {
      value: String(id),
      label: name,
    }
    if (children && children.length > 0) {
      option.children = children
    }

    if ((!children || children.length < 1) && Number(leaf) === 0) {
      // 如果没有叶子节点，且当前节点 leaf 属性为 0，则不展示（不能作为新建地图关联的地图目录）
      continue
    }

    arr.push(option)
  }
  return arr
}

// 选中的地图目录
onMounted(async () => {
  // 云端版在保存地图弹窗中，需要取消自动保存地图的计时器
  if (isCloud) {
    fileStore.clearAutoSaveTimer()
  }
})

watch(
  () => props.isSaveAs,
  (val) => {
    if (val) {
      dialogTitle.value = i18n.global.t('menu.saveAs')
      if (fileStore.openedFile) {
        // 如果是另存为，且存在已打开的地图，则将地图名称显示出来
        const nameWithoutSuffix = fileStore.openedFile
          .replace(/\.xodr$/i, '')
          .replace(/\.sqlite$/i, '')
        newMapName.value = nameWithoutSuffix
      }
    } else {
      dialogTitle.value = i18n.global.t('menu.save')
    }
  },
  {
    immediate: true,
  },
)

// 弹窗展示后的回调
function onShow () {
  // 禁用热键
  helperStore.closeShortcutKeys()
  // input 框获取焦点
  nextTick(() => {
    mapNameInputRef.value.focus()
  })
}

function handleInput () {
  // 如果存在验证反馈的提示，则在输入后清空提示
  if (validateContent.value) {
    validateContent.value = ''
  }
}
function handleChange () {
  // 如果存在下拉菜单未选中内容的提示，则在选中后清空提示
  if (validateMapGroupContent.value) {
    validateMapGroupContent.value = ''
  }
}

function handleClose () {
  if (isLoading.value) return

  // 退出时重置验证信息
  validateContent.value = ''
  validateMapGroupContent.value = ''
  isValidating.value = false

  helperStore.openShortcutKeys()
  if (props.isSaveAs) {
    helperStore.toggleDialogStatus('saveAsMap', false)
  } else {
    helperStore.toggleDialogStatus('saveMap', false)
  }

  // 在关闭保存弹窗以后，重启云端自动保存地图的计时器
  if (isCloud) {
    fileStore.initAutoSaveTimer()
  }
}

// 保存地图
async function handleSaveName () {
  if (isLoading.value) return
  if (isValidating.value) return

  isValidating.value = true

  const nameWithoutSuffix = newMapName.value.trim()

  // 提交时，地图名称不能为空
  if (!nameWithoutSuffix) {
    validateContent.value = i18n.global.t('desc.tips.mapNameIsEmpty')
    isValidating.value = false
    return
  }

  // 最多支持 160 个字符
  const maxStrNumber = 160

  if (isCloud) {
    // 如果是云端版，限制最大字符数，且支持中英文和特殊符号
    if (calcStrLength(nameWithoutSuffix) > maxStrNumber) {
      validateContent.value = i18n.global.t('desc.tips.mapNameIsTooLong')
      isValidating.value = false
      return
    }
  } else {
    // 如果是单机版，使用如下的名称验证
    // 名称字符规范验证
    if (isNameInvalidated(nameWithoutSuffix)) {
      validateContent.value = i18n.global.t('desc.tips.mapNameIsInvalidated')
      isValidating.value = false
      return
    }
    // 名称长度验证
    if (calcStrLength(nameWithoutSuffix) > maxStrNumber) {
      validateContent.value = i18n.global.t('desc.tips.mapNameIsTooLong')
      isValidating.value = false
      return
    }
  }

  const _suffix = fileSuffix.value.split(' ')[0]

  // 保存的地图名称需要带上文件后缀
  const nameWithSuffix = nameWithoutSuffix + _suffix

  // 地图类型，默认是 opendrive 1.4 版本
  let mapType = MapType.xodr14
  if (fileSuffix.value === '.sqlite') {
    mapType = MapType.sqlite
  } else if (fileSuffix.value.includes('1.5')) {
    mapType = MapType.xodr15
  }

  // 校验是否跟当前打开的地图重名
  if (nameWithSuffix === fileStore.openedFile) {
    validateContent.value = i18n.global.t(
      'desc.tips.aMapFileWithTheSameNameExists',
    )
    isValidating.value = false
    return
  }

  // 如果是云平台，不需要判断是否存在重复的地图文件名
  if (isCloud) {
    // 判断如果需要选中地图目录分组
    if (showMapCatalogue.value) {
      if (!mapGroupId.value) {
        // 如果没有选中的地图分组
        validateMapGroupContent.value = i18n.global.t(
          'desc.tips.mapCatalogueCannotBeEmpty',
        )
        isValidating.value = false
        return
      }
    }

    // 云端版保存
    await fileStore.saveAllMapInCloud({
      mapName: nameWithSuffix,
      mapGroupId: mapGroupId.value,
      isSaveAs: props.isSaveAs,
      mapType,
    })
    // 保存后关闭窗口
    handleClose()
    return
  }

  // 判断是否有跟当前地图列表中的已有地图重名
  try {
    const mapListRes = await getMapList()
    if (!mapListRes || get(mapListRes, 'data.message') !== 'ok') {
      throw new Error(
        `api get hadmap list error. error code: ${get(mapListRes, 'data.code')}`,
      )
    }
    const latestMapList = get(mapListRes, 'data.data')
    for (const mapInfo of latestMapList) {
      const { name: mapFileName } = mapInfo
      if (nameWithSuffix === mapFileName) {
        // 如果输入的名称，跟地图列表中已有的地图重名
        validateContent.value = i18n.global.t(
          'desc.tips.aMapFileWithTheSameNameExists',
        )
        isValidating.value = false
        return
      }
    }
  } catch (err) {
    console.log('get map list error (saving map): ', err)
  }

  // 单机版保存
  // 需要输入新的地图名称的保存，统一走 create + save 的逻辑
  await fileStore.createAndSaveMap({
    mapName: nameWithSuffix,
    isSaveAs: props.isSaveAs,
    mapType,
  })

  // 保存后关闭窗口
  handleClose()
}
</script>

<template>
  <el-dialog
    v-model="visible"
    :v-loading="true"
    :width="showMapCatalogue ? '340px' : '260px'"
    class="enter-name-dialog"
    :close-on-click-modal="false"
    :show-close="false"
    :destroy-on-close="true"
    :close-on-press-escape="false"
    align-center
  >
    <div class="title">
      <el-icon size="24">
        <InfoSvg />
      </el-icon>
      <span>{{ dialogTitle }}</span>
    </div>
    <template v-if="!showMapCatalogue">
      <div class="sub-title">
        {{ $t('actions.mapFile.pleaseEnterMapName') }}
      </div>
      <div class="input-part">
        <el-input
          ref="mapNameInputRef"
          v-model="newMapName"
          :disabled="isLoading"
          @input="handleInput"
        >
          <template v-if="shownFileSuffixList.length" #append>
            <el-select
              v-model="fileSuffix"
              :disabled="isLoading"
              :style="{
                width: '100px',
              }"
            >
              <template v-for="item in shownFileSuffixList" :key="item.value">
                <el-option :label="item.label" :value="item.value" />
              </template>
            </el-select>
          </template>
        </el-input>
        <div class="validate-info">
          {{ validateContent }}
        </div>
      </div>
    </template>
    <template v-else>
      <div class="row">
        <div class="label">
          {{ $t('desc.mapName') }}
        </div>
        <div class="input-part" :style="{ width: '240px' }">
          <el-input
            ref="mapNameInputRef"
            v-model="newMapName"
            :disabled="isLoading"
            @input="handleInput"
          >
            <template v-if="shownFileSuffixList.length" #append>
              <el-select
                v-model="fileSuffix"
                :disabled="isLoading"
                :style="{
                  width: '100px',
                }"
              >
                <template v-for="item in shownFileSuffixList" :key="item.value">
                  <el-option :label="item.label" :value="item.value" />
                </template>
              </el-select>
            </template>
          </el-input>
          <div class="validate-info">
            {{ validateContent }}
          </div>
        </div>
      </div>
      <div class="row">
        <div class="label">
          {{ $t('desc.mapCatalogue') }}
        </div>
        <div class="input-part">
          <el-tree-select
            v-model="mapGroupId"
            :style="{ width: '100%' }"
            :placeholder="$t('desc.tips.pleaseSelectAMapCatalogue')"
            :data="mapCatalogueList"
            :render-after-expand="false"
            @change="handleChange"
          />
          <div class="validate-info">
            {{ validateMapGroupContent }}
          </div>
        </div>
      </div>
    </template>
    <div class="button-part">
      <SimButton @click="handleClose">
        {{ $t('desc.tips.cancel') }}
      </SimButton>
      <SimButton
        class="primary"
        :loading="isLoading"
        @click="handleSaveName"
      >
        {{ $t('desc.tips.confirm') }}
      </SimButton>
    </div>
  </el-dialog>
</template>

<style lang="less">
.el-dialog.enter-name-dialog {
  .el-dialog__header {
    display: none;
  }
  .el-dialog__body {
    background-color: var(--dialog-area-color);
    padding: 20px 16px;
    .title {
      text-align: center;
      font-size: 18px;
      height: 24px;
      line-height: 24px;
      color: var(--text-color);
      margin-bottom: 14px;
      .el-icon {
        vertical-align: bottom;
        margin-right: 8px;
      }
    }
    .sub-title {
      color: var(--text-color);
      text-align: center;
      margin-bottom: 14px;
    }
    .row {
      display: flex;

      .label {
        color: var(--text-color);
        width: 68px;
        text-align: left;
        height: 26px;
        line-height: 26px;
      }
      .input-part {
        margin-bottom: 0px;
        flex-grow: 1;
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
      .validate-info {
        height: 20px;
        line-height: 20px;
        font-size: var(--font-size);
        color: #cc4343;
        padding-top: 4px;
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
</style>
