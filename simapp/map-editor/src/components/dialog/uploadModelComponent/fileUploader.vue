<script setup lang="ts">
import { type Ref, nextTick, ref, watch } from 'vue'
import { genFileId } from 'element-plus'
import type { UploadFile, UploadRawFile } from 'element-plus'
import { Close, Document } from '@element-plus/icons-vue'
import { get } from 'lodash'
import { errorMessage, warningMessage } from '@/utils/common'
import { uploadCustomModel } from '@/services'
import { useModelCacheStore } from '@/stores/object/modelCache'
import i18n from '@/locales'

interface IFile {
  name: string
  rawName: string
  path: string
  id: string
}

const props = defineProps({
  // 是否展示预览模型
  showModel: {
    type: Boolean,
    default: false,
  },
  editModelName: {
    type: String,
    default: '',
  },
})

// 暂定 5 mb
const emit = defineEmits(['confirm', 'cancel', 'toggle-preview'])
const modelCacheStore = useModelCacheStore()
const fileList: Ref<Array<UploadFile | UploadRawFile>> = ref([])
const targetFile: Ref<IFile | null> = ref(null)
const limitSize = 5// 当前操作系统分隔符
const sep = get(window, 'electron.path.sep') || '\\'

// 基于 fileUploader 上传的文件路径，调用后端接口保存到临时目录下
async function handleUploadModelFile (filePath: string) {
  if (!filePath) return

  try {
    const res = await uploadCustomModel(filePath)
    if (!res || !get(res, 'data')) {
      throw new Error(
        `api upload cache custom model error. error info: ${get(res, 'data')}`,
      )
    }
    const { filepath = [], id } = res.data
    // 遍历返回的文件列表，提取以 .fbx 结尾的模型文件路径
    let fbxModelPath = ''
    for (const _path of filepath) {
      if (fbxModelPath) continue

      if (/.fbx$/i.test(_path)) {
        fbxModelPath = _path
      }
    }

    if (!fbxModelPath) return

    // 提取 fbx 文件名
    const pathSegment = fbxModelPath.split(sep)
    const modelName = pathSegment[pathSegment.length - 1]
    // 将模型路径整合成 webgl loader 能够解析加载的路径
    const replacePath = pathSegment.join('/')
    // TODO 考虑 ubuntu 环境下分隔符区别的问题！

    targetFile.value = {
      name: modelName,
      rawName: fileList.value[0].name,
      path: replacePath,
      id,
    }

    // 触发外层确认事件
    emit('confirm', targetFile.value)
  } catch (err) {
    console.log(err)
    errorMessage({
      content: i18n.global.t('desc.uploadModel.failedToUploadCustomModelFile'),
    })
  }
}

watch(
  () => props.editModelName,
  (val) => {
    if (!val) return
    const originConfig = modelCacheStore.getCustomModelConfigByName(val)
    if (!originConfig) return

    const { modelUrl, fbxModelName, rawFileName } = originConfig

    nextTick(() => {
      targetFile.value = {
        name: fbxModelName,
        rawName: rawFileName,
        path: modelUrl,
        id: '-1',
      }
      // 触发外层确认事件
      emit('confirm', targetFile.value)
    })
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  fileList,
  (val) => {
    if (val.length < 1) {
      targetFile.value = null
      return
    }

    const currentFile = val[0]
    if (!currentFile) {
      clear()
      return
    }

    let verifyRes = false
    const rawFile = (currentFile as UploadFile).raw
    let originPath = ''
    if (rawFile) {
      verifyRes = verifyFileTypeAndSize(rawFile)
      originPath = rawFile.path
    } else {
      verifyRes = verifyFileTypeAndSize(currentFile as UploadRawFile)
      originPath = currentFile.path
    }
    if (!verifyRes) {
      clear()
      return
    }

    handleUploadModelFile(originPath)
  },
  {
    immediate: true,
    deep: true,
  },
)

// 验证格式有效性
function verifyFileTypeAndSize (file: UploadRawFile) {
  if (!file) return false

  // 格式校验（满足 .fbx 或者 .zip 都视为有效）
  let isValid = false
  if (/.fbx$/i.test(file.name)) {
    isValid = true
  } else if (
    /.zip$/i.test(file.name) ||
    file.type === 'application/x-zip-compressed'
  ) {
    isValid = true
  }
  if (!isValid) {
    warningMessage({
      content: i18n.global.t('desc.uploadModel.fileFormatNotMatch'),
    })
    return false
  }

  // 文件大小校验
  if (file.size / 1024 / 1024 > limitSize) {
    warningMessage({
      content: `${i18n.global.t(
        'desc.uploadModel.fileSizeExceed',
      )} ${limitSize} Mb`,
    })
    return false
  }

  return true
}

// 如果上传的文件超过 1 个，则使用新上传的替换掉历史的
function handleExceed (files: Array<UploadRawFile>) {
  const newFile = files[0]
  const res = verifyFileTypeAndSize(newFile)
  if (!res) return

  // 先清空之前上传的文件
  fileList.value = []

  // 使用新的上传文件
  newFile.uid = genFileId()
  fileList.value = [newFile]
}

// 删除上传的模型文件
function handleRemove () {
  if (!targetFile.value) return

  clear()
  emit('cancel', null)
}

// 清空状态
function clear () {
  targetFile.value = null
  fileList.value = []
}

// 展示模型预览效果
function showPreviewModel () {
  if (props.showModel) {
    // 如果当前展示了模型，再次点击为隐藏
    emit('toggle-preview', false)
  } else {
    // 如果当前隐藏了模型，再次点击为显示
    emit('toggle-preview', true)
  }
}
</script>

<template>
  <div class="file-upload-container">
    <div v-if="targetFile" class="file-item">
      <span class="file-info">
        <el-icon><Document /></el-icon>
        <span class="file-name">{{ targetFile.rawName }}</span>
      </span>
      <el-icon class="close-icon" @click="handleRemove">
        <Close />
      </el-icon>
    </div>
    <el-upload
      v-model:file-list="fileList"
      action="#"
      class="file-uploader"
      drag
      :multiple="false"
      :auto-upload="false"
      :show-file-list="false"
      :on-exceed="handleExceed"
      :limit="1"
    >
      <div class="upload-area">
        <span style="color: #5381f8">
          {{ $t('desc.uploadModel.clickToUpload') }}
          &nbsp;
        </span>
        {{ '/' }}
        &nbsp;{{ $t('desc.uploadModel.dropFileHere') }}
      </div>
    </el-upload>
    <div class="tip">
      {{ $t('desc.uploadModel.uploadFileComponentTips') }}
    </div>
    <!--    <div v-if="targetFile" class="preview-model" @click="showPreviewModel"> -->
    <div v-if="false" class="preview-model" @click="showPreviewModel">
      {{ $t('desc.uploadModel.modelPreview') }}
    </div>
  </div>
</template>

<style lang="less">
.file-upload-container {
  position: relative;
  margin-bottom: 14px;

  .file-item {
    margin: 10px 0;
    position: relative;
    border-radius: 2px;
    font-size: 14px;
    box-sizing: border-box;
    width: 100%;
    .file-info {
      display: inline-flex;
      width: calc(100% - 30px);
      margin-left: 8px;
      align-items: center;

      .file-name {
        padding-left: 6px;
        width: 268px;
        overflow: hidden;
        text-overflow: ellipsis;
        white-space: nowrap;
      }
    }
    .close-icon {
      position: absolute;
      right: 5px;
      top: 3px;
      cursor: pointer;
      display: none;
      opacity: 0.75;

      &:hover {
        opacity: 1;
      }
    }

    &:hover {
      background-color: rgba(255, 255, 255, 0.2);
      .file-info {
        color: var(--text-color);
      }
      .close-icon {
        display: inline-flex;
        color: var(--text-color);
      }
    }
  }

  .file-uploader {
    .el-upload-dragger {
      width: 300px;
      height: 120px;
      background-color: var(--main-dark-color);
      border: var(--input-border);
      border-radius: 2px;
      .upload-area {
        height: 100%;
        display: flex;
        align-items: center;
        justify-content: center;
      }
    }

    .el-upload:focus {
      border-color: #666666;
      color: inherit;

      .el-upload-dragger {
        border-color: #666666;
      }
    }

    .el-upload-list__item {
      transition: none;
      border-radius: 2px;
      .el-upload-list__item-name {
        transition: none;
      }

      &:hover {
        background-color: rgba(255, 255, 255, 0.2);
        .el-upload-list__item-name {
          color: var(--text-color);
        }
        .el-icon {
          color: var(--text-color);
        }
      }
    }
  }
  .tip {
    font-size: 11px;
    margin-top: 10px;
    width: 216px;
  }

  .preview-model {
    position: absolute;
    right: 0;
    bottom: -6px;
    width: 86px;
    height: 24px;
    background-color: rgba(255, 255, 255, 0.06);
    color: var(--text-color);
    display: flex;
    justify-content: center;
    align-items: center;
    border-radius: 2px;
    cursor: pointer;

    &:hover {
      background-color: rgba(255, 255, 255, 0.2);
    }
  }
}
</style>
