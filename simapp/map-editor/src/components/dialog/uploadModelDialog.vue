<script setup lang="ts">
import { type Ref, computed, provide, ref, watch } from 'vue'
import md5 from 'crypto-js/md5'
import { get } from 'lodash'
import { WarningFilled } from '@element-plus/icons-vue'
import SimButton from '../common/button.vue'
import ThumbUploader from './uploadModelComponent/thumbUploader.vue'
import FileUploader from './uploadModelComponent/fileUploader.vue'
import ModelPreview from './uploadModelComponent/modelPreview.vue'
import { useHelperStore } from '@/stores/helper'
import { calcStrLength, isCustomModelNameValidated } from '@/utils/tools'
import { useModelCacheStore } from '@/stores/object/modelCache'
import { saveCustomModel, uploadModelThumb } from '@/services'
// @ts-expect-error
import { isCustomModelInUsed } from '@/stores/object/common'
import { confirm, errorMessage, successMessage } from '@/utils/common'
import i18n from '@/locales'
import ee, { CustomEventType } from '@/utils/event'

interface IModelOption {
  id: string // 临时目录 id
  fileName: string
  rawFileName: string
  missingFiles: string[] // 缺失贴图文件
  path: string
  width: number
  length: number
  height: number
  posX: number
  posY: number
  posZ: number
}
interface IThumbOption {
  uploadedFile: File | null // 通过图片组件上传的本地缩略图
  snapshotFile: File | null // 通过截取三维场景提供的缩略图
}

const props = defineProps({
  modelValue: {
    type: Boolean,
    default: true,
  },
  customModelName: {
    type: String,
    default: '',
  },
})

const isModify = computed(() => {
  return !!props.customModelName
})

provide('isModify', isModify.value)

// 对话框默认宽度 440px，当模型预览时宽度变大为 690px
const defaultWidth = '440px'
const extendWidth = '690px'

const helperStore = useHelperStore()
const modelCacheStore = useModelCacheStore()

// 确认导入 loading
const isLoading = ref(false)
const dialogWidth = ref(defaultWidth)
const showPreviewModel = ref(true)
// 模型名称
const showName = ref('')
// 模型名称输入框的提示
const showNameTips = {
  none: i18n.global.t('desc.uploadModel.modelNameCannotBeEmpty'),
  type: i18n.global.t('desc.uploadModel.modelNameSupportFormat'),
  length: i18n.global.t('desc.uploadModel.modelNameSupportLength'),
  conflict: i18n.global.t('desc.uploadModel.modelNameDuplicated'),
}
const showNameTip = ref('')
// 三维模型参数
const modelOption: Ref<IModelOption> = ref({
  id: '',
  fileName: '', // 解析后的 fileName，如上传 xx.zip , 该参数表示会解析出内部模型文件的名称
  rawFileName: '', // 解析前的 fileName
  missingFiles: [],
  path: '',
  width: 0,
  length: 0,
  height: 0,
  posX: 0,
  posY: 0,
  posZ: 0,
})
// 上传三维模型的提示
const modelTip = ref('')
const thumbOption: Ref<IThumbOption> = ref({
  uploadedFile: null,
  snapshotFile: null,
})
// 当前二次编辑的模型名称（如果存在，则不允许更新作为唯一标识变量的模型名称）
const editModelName = ref('')
const thumbUpdated = ref(false)
const thumbRef = ref(null)
const modelPreviewRef = ref(null)

const visible = computed(() => {
  if (helperStore.uploadModelDialogShow) {
    // 弹窗展示后，禁用热键
    helperStore.closeShortcutKeys()
  }
  return helperStore.uploadModelDialogShow
})

watch(
  () => props.customModelName,
  (val) => {
    if (!val) return
    // 如果存在启动模态框的参数，则展示对应的自定义模型配置
    showName.value = val
    editModelName.value = val
    thumbUpdated.value = false
    // 展示模型预览
    showPreviewModel.value = !!val
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  showPreviewModel,
  (val) => {
    // 是否展示预览模型，控制模态框宽度
    if (val) {
      dialogWidth.value = extendWidth
    } else {
      dialogWidth.value = defaultWidth
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

// 监听模型名称的更新
watch(
  showName,
  (val) => {
    showNameTip.value = ''
    // 删除字符串前后的空格
    showName.value = val.trim()
  },
  {
    immediate: true,
    deep: true,
  },
)

// 取消模型文件的上传
function handleFileCancel () {
  modelOption.value = {
    id: '',
    fileName: '',
    missingFiles: [],
    path: '',
    width: 0,
    length: 0,
    height: 0,
    posX: 0,
    posY: 0,
    posZ: 0,
  }
  handleThumbCancel()
  // 如果之前有预览模型，则不展示
  // showPreviewModel.value = false
  modelTip.value = ''

  // 重置 modelPreview 内容
  if (modelPreviewRef.value) {
    modelPreviewRef.value.reset()
  }
}
// 确认模型文件的上传
function handleFileConfirm (fileOption: {
  name: string
  rawName: string
  path: string
  id: string
}) {
  const { name, path, id, rawName } = fileOption
  modelOption.value.fileName = name
  modelOption.value.rawFileName = rawName
  modelOption.value.path = path
  modelOption.value.id = id
  modelTip.value = ''
  // 如果 fbx 模型文件更新，则触发缩略图子组件清空方法
  if (thumbRef.value) {
    // @ts-expect-error
    thumbRef.value.handleRemove()
  }
  // 默认上传后就展开预览模型的区域
  handlePreviewModel(true)
}
// 取消缩略图的上传
function handleThumbCancel () {
  // 二次编辑过程中，如果触发了删除缩略图的操作，则视为缩略图更新
  if (props.customModelName) {
    thumbUpdated.value = true
  }

  thumbOption.value = {
    uploadedFile: null,
    snapshotFile: null,
  }
}
// 确认缩略图的上传
function handleThumbConfirm (file: File) {
  // 二次编辑过程中，如果触发了删除缩略图的操作，则视为缩略图更新
  if (props.customModelName) {
    thumbUpdated.value = true
  }
  // 如果通过图片组件上传，则需要将三维场景截取的缩略图清空
  thumbOption.value = {
    uploadedFile: file,
    snapshotFile: null,
  }
}

// 将三维场景中截取的缩略图数据同步
function handleSnapshot (file: File) {
  // 二次编辑过程中，如果触发了删除缩略图的操作，则视为缩略图更新
  if (props.customModelName) {
    thumbUpdated.value = true
  }
  // 如果将三维场景截图作为缩略图，则将图片上传组件的图片清空
  thumbOption.value = {
    uploadedFile: null,
    snapshotFile: file,
  }
}

// 切换三维场景模型预览的开关
function handlePreviewModel (status: boolean) {
  showPreviewModel.value = status
}

// 三维模型参数同步
function handleSyncModelParams (params: {
  width?: number
  length?: number
  height?: number
  posX?: number
  posY?: number
  posZ?: number
}) {
  const properties = Object.keys(params)
  for (const key of properties) {
    // @ts-expect-error
    modelOption.value[key] = params[key]
  }
}

function handleModelLoaded ({ errorFiles = [] }) {
  modelOption.value.missingFiles = errorFiles
  checkModel()
}

// 关闭弹窗
function handleClose () {
  helperStore.toggleDialogStatus('uploadModel', false)

  // 关闭弹窗后，恢复热键使用
  helperStore.openShortcutKeys()

  // 清空状态
  showNameTip.value = ''
  showName.value = ''
  handleFileCancel()
  handleThumbCancel()
  isLoading.value = false

  // 弹窗关闭主动清空当前正在编辑的自定义模型名称缓存
  ee.emit(CustomEventType.other.updateCustomModelConfig, '')
}

// 检查模型名称合理性
function checkShowName () {
  // 如果是二次编辑的更新，则不校验模型名称
  if (props.customModelName) {
    return true
  }

  if (!showName.value) {
    // 名称为空，展示提示内容
    showNameTip.value = showNameTips.none
    return false
  } else {
    // 名称不为空，则检查内容是否符合规范
    const validated = isCustomModelNameValidated(showName.value)

    if (!validated) {
      showNameTip.value = showNameTips.type
      return false
    }

    // 长度限制 20 个字符
    const count = calcStrLength(showName.value)
    if (count > 20) {
      showNameTip.value = showNameTips.length
      return false
    }

    // 自定义模型名称冲突
    for (const config of modelCacheStore.customModelList) {
      if (showName.value === config.name) {
        showNameTip.value = showNameTips.conflict
        return false
      }
    }
  }

  showNameTip.value = ''
  return true
}

// 检查三维模型文件的合理些
function checkModel () {
  if (!modelOption.value.path || !modelOption.value.fileName) {
    modelTip.value = i18n.global.t(
      'desc.uploadModel.pleaseUploadValidatedModelFile',
    )
    return false
  }

  // 文件缺失
  if (modelOption.value.missingFiles.length > 0) {
    const paths = modelOption.value.missingFiles.map((url: string) =>
      url.split('/').pop(),
    )
    modelTip.value =
      `${i18n.global.t('desc.uploadModel.missingTextureFile')
      }：${
      paths.join(',')}`
    return false
  }

  modelTip.value = ''
  return true
}

// 检查二次编辑的配置，是否跟之前的有变更
function checkConfigUpdated () {
  if (!props.customModelName) return false

  const originConfig = modelCacheStore.getCustomModelConfigByName(
    props.customModelName,
  )
  // 旧版配置不存在，属于配置更新
  if (!originConfig) return true

  const { modelUrl, posX, posY, posZ, width, length, height } = originConfig

  // 缩略图更新，属于配置更新了
  if (thumbUpdated.value) return true

  const {
    path,
    posX: _posX,
    posY: _posY,
    posZ: _posZ,
    width: _width,
    length: _length,
    height: _height,
  } = modelOption.value
  // 三维模型的配置更新，都属于配置更新
  if (
    modelUrl !== path ||
    posX !== _posX ||
    posY !== _posY ||
    posZ !== _posZ ||
    width !== _width ||
    length !== _length ||
    height !== _height
  ) {
    return true
  }
}

// 确认最终的导入
async function handleConfirm () {
  let isModelInUsed = false
  isLoading.value = true

  // 如果是二次编辑后的确认提交
  if (props.customModelName) {
    // 检查模型有效性
    const modelValidated = checkModel()
    if (!modelValidated) {
      isLoading.value = false
      return
    }

    // 判断是否存在自定义模型配置内容的更新，如果没有，则可以返回
    const configUpdated = checkConfigUpdated()
    if (!configUpdated) {
      // 没有更新，则关闭弹窗
      handleClose()
      return
    } else {
      // 有更新，判断地图场景中是否有使用到
      isModelInUsed = isCustomModelInUsed(props.customModelName)
      // 如果当前地图文件中有用到对应的自定义模型，则需要用户二次确认是否需要修改
      const warningMsg = isModelInUsed ?
        i18n.global.t('desc.uploadModel.confirmToModifyCustomModelInUsed') :
        i18n.global.t('desc.uploadModel.confirmToModifyCustomModel')
      const confirmUpdateRes = await confirm({
        msg: warningMsg,
        type: 'que',
        title: i18n.global.t('desc.tips.tips'),
      })

      // 如果二次确认弹框用户取消修改，则直接返回
      if (!confirmUpdateRes) {
        handleClose()
        return
      }
    }
  }

  const nameValidated = checkShowName()
  // 检查模型有效性
  const modelValidated = checkModel()

  if (!nameValidated || !modelValidated) {
    // 模型名称或者模型文件不符合要求
    isLoading.value = false
    return
  }

  // 如果缩略图存在更新，则需要判断是否用户上传了缩略图，如果没有缩略图，则应用默认三维场景截图
  if (!thumbOption.value.uploadedFile && !thumbOption.value.snapshotFile) {
    if (thumbUpdated.value || !props.customModelName) {
      if (modelPreviewRef.value) {
        // @ts-expect-error
        const defaultSnapshotFile = modelPreviewRef.value.applyDefaultSnapshot()
        if (defaultSnapshotFile) {
          thumbOption.value = {
            uploadedFile: null,
            snapshotFile: defaultSnapshotFile,
          }
        }
      }
    }
  }

  try {
    // 调用上传缩略图和保存自定义模型配置 2 个接口
    // 先上传缩略图到临时目录
    // 获取模型路径
    const sep = '/'
    const pathSegment = modelOption.value.path.split(sep)
    pathSegment.pop()
    // 模型所在的临时目录
    const modelDir = pathSegment.join(sep)
    // 调整缩略图
    const thumbFile =
      thumbOption.value.uploadedFile || thumbOption.value.snapshotFile

    // 二次编辑时，缩略图的 File 文件不一定存在
    if (thumbFile) {
      // 上传缩略图接口
      const thumbRes = await uploadModelThumb({
        file: thumbFile,
        modelDir,
      })
      if (!thumbRes || !get(thumbRes, 'data.dstpath')) {
        throw new Error(
          `api upload model thumb error, error info: ${get(thumbRes, 'data')}`,
        )
      }
    }

    // 保存模型的上一级目录
    const newModelDir = `user_mapobject_${md5(showName.value)}`
    const {
      id: cacheId,
      fileName,
      rawFileName,
      length,
      width,
      height,
      posX,
      posY,
      posZ,
    } = modelOption.value
    const validModelPath = newModelDir + sep + fileName

    // 默认使用 fbx 模型上传时返回的 id
    let saveId = cacheId
    // 如果是二次编辑，且模型返回 id 为空，则使用 -1
    if (!saveId && props.customModelName) {
      saveId = '-1'
    }

    // 保存模型配置接口
    const saveRes = await saveCustomModel({
      id: saveId,
      name: showName.value,
      variable: newModelDir, // 将目录名称作为标识变量传入（暂时没用）
      modelPath: validModelPath,
      length,
      width,
      height,
      posX,
      posY,
      posZ,
      rawFileName,
    })
    if (!saveRes || get(saveRes, 'data.err') !== 0) {
      throw new Error(
        `api save custom model error. error info: ${get(saveRes, 'data')}`,
      )
    }

    // 保存成功以后，重新加载对应的自定义模型列表
    await modelCacheStore.syncLocalCustomModel(props.customModelName)

    if (isModelInUsed) {
      // 基于自定义模型配置的更新，触发对应数据和渲染层效果的更新
      modelCacheStore.updateCustomModelConfig(props.customModelName)
    }

    isLoading.value = false

    successMessage({
      content: i18n.global.t(
        'desc.uploadModel.saveCustomModelConfigSuccessfully',
      ),
    })
  } catch (err) {
    console.log('save custom model config error: ', err)
    errorMessage({
      content: i18n.global.t('desc.uploadModel.failedToSaveCustomModelConfig'),
    })
  }

  // 关闭弹窗
  handleClose()
}
</script>

<template>
  <el-dialog
    v-model="visible"
    :v-loading="true"
    :width="dialogWidth"
    class="upload-model-dialog"
    :close-on-click-modal="false"
    :show-close="false"
    :destroy-on-close="true"
    :close-on-press-escape="false"
    align-center
  >
    <template #header>
      <!-- 使用嵌套的弹窗后，需要将自定义的弹窗名称手动在 #header 部分展示 -->
      <span>{{ $t('desc.uploadModel.importModel') }}</span>
    </template>
    <div class="dialog-body">
      <div class="form-area">
        <div class="row">
          <div class="label">
            * {{ $t('desc.uploadModel.modelName') }}
          </div>
          <div class="input-part">
            <el-input
              v-model="showName"
              class="model-name"
              :disabled="!!editModelName"
              @blur="checkShowName"
            />
            <div v-if="showNameTip" class="name-tip">
              <el-icon><WarningFilled /></el-icon>&nbsp;{{ showNameTip }}
            </div>
          </div>
        </div>
        <div class="row">
          <div class="label">
            * {{ $t('desc.uploadModel.uploadFile') }}
          </div>
          <div class="input-part">
            <FileUploader
              :edit-model-name="editModelName"
              :show-model="showPreviewModel"
              @cancel="handleFileCancel"
              @confirm="handleFileConfirm"
              @toggle-preview="handlePreviewModel"
            />
            <div v-if="modelTip" class="model-tip">
              <el-icon><WarningFilled /></el-icon>&nbsp;{{ modelTip }}
            </div>
          </div>
        </div>
        <div class="row last-row">
          <div class="label">
            {{ $t('desc.uploadModel.thumbnail') }}
          </div>
          <div class="input-part">
            <ThumbUploader
              ref="thumbRef"
              :edit-model-name="editModelName"
              :model-option="{
                id: modelOption.id,
                fileName: modelOption.fileName,
                path: modelOption.path,
              }"
              :snapshot-file="thumbOption.snapshotFile"
              @cancel="handleThumbCancel"
              @confirm="handleThumbConfirm"
            />
          </div>
        </div>
      </div>
      <div class="preview-area">
        <ModelPreview
          ref="modelPreviewRef"
          :edit-model-name="editModelName"
          :model-path="modelOption.path"
          @sync-params="handleSyncModelParams"
          @generate-snapshot="handleSnapshot"
          @loaded="handleModelLoaded"
        />
      </div>
    </div>
    <div class="button-part">
      <SimButton @click="handleClose">
        {{ $t('desc.tips.cancel') }}
      </SimButton>
      <SimButton class="primary" :loading="isLoading" @click="handleConfirm">
        {{
          $t('desc.tips.confirm')
        }}
      </SimButton>
    </div>
  </el-dialog>
</template>

<style lang="less">
.el-dialog.upload-model-dialog {
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
      display: flex;
      margin-bottom: 16px;
      overflow: hidden;

      .form-area {
        .row {
          //display: flex;
          min-height: 40px;
          font-size: 12px;
          margin-bottom: 2px;

          .label {
            width: 80px;
            padding: 4px 0 0;
            color: var(--property-title-color);
          }

          .input-part {
            position: relative;
            margin-top: 10px;
            .name-tip {
              display: flex;
              align-items: center;
              color: #cc4343;
              font-size: 11px;
              margin-top: 5px;
            }
            .model-tip {
              display: flex;
              align-items: center;
              color: #cc4343;
              font-size: 11px;
              margin-top: -9px;
            }
            .model-name {
              width: 300px;

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
              &.is-disabled {
                .el-input__wrapper {
                  background-color: var(--input-disabled-border-color);
                }
              }
            }
          }
        }
        .last-row {
          margin-bottom: 0;
        }
      }
      .preview-area {
        width: 370px;
        margin-left: 20px;
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
