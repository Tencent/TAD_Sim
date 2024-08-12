<script setup lang="ts">
import { type Ref, computed, nextTick, onUnmounted, ref, watch } from 'vue'
import { Delete, Plus } from '@element-plus/icons-vue'
import type { UploadFile } from 'element-plus'
import { warningMessage } from '@/utils/common'
import { useModelCacheStore } from '@/stores/object/modelCache'
import i18n from '@/locales'

const props = defineProps({
  modelOption: {
    default: {
      id: '',
      fileName: '',
      path: '',
    },
  },
  snapshotFile: {
    type: File,
    default: null,
  },
  // 二次编辑的自定义模型名称
  editModelName: {
    type: String,
    default: '',
  },
})

const emit = defineEmits(['confirm', 'cancel'])

const modelCacheStore = useModelCacheStore()

// 图片组件上传的文件列表
const fileList: Ref<Array<UploadFile>> = ref([])
// 展示出来的图片 url
const targetImgUrl = ref('')
const limitType = 'png'
const limitFileType = `image/${limitType}`
const limitSize = 300 // 暂定 300kb

const uploaderRef = ref<HTMLElement>()

// 默认暂时不允许上传缩略图
const uploadThumbDisabled = ref(true)
const cursorStyle = computed(() =>
  uploadThumbDisabled.value ? 'not-allowed' : 'pointer',
)

// 向父组件暴露方法
defineExpose({ handleRemove })

onUnmounted(() => {
  if (targetImgUrl.value) {
    // 在组件销毁后主动释放内存
    try {
      URL.revokeObjectURL(targetImgUrl.value)
    } catch (err) {
      //
    }
    targetImgUrl.value = ''
  }
})

watch(
  () => props.editModelName,
  (val) => {
    // 二次编辑的地图名称
    if (!val) return
    const originConfig = modelCacheStore.getCustomModelConfigByName(val)
    if (!originConfig) return
    // 将缩略图设置成已经配置好的缩略图
    const { thumbUrl } = originConfig
    nextTick(() => {
      targetImgUrl.value = thumbUrl
    })
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  () => props.modelOption,
  (val) => {
    // 如果已经上传模型文件，则可以上传缩略图
    // 即 fileName 属性不存在，disabled 为真
    uploadThumbDisabled.value = !val.fileName
    if (!val.fileName && targetImgUrl.value) {
      // 如果模型文件不存在，但存在上传的缩略图，需要清空
      handleRemove()
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

// 如果存在截图，则使用截图作为缩略图展示
watch(
  () => props.snapshotFile,
  (val) => {
    if (!val) return
    // 如果通过三维场景截取缩略图进行展示
    targetImgUrl.value = URL.createObjectURL(val)
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
      targetImgUrl.value = ''
      return
    }
    if (val.length > 1) {
      fileList.value.shift()
      return
    }
    const file = val[val.length - 1]
    if (!file || !file.raw) {
      clear()
      return
    }
    // 格式校验
    if (file.raw.type !== limitFileType) {
      clear()
      warningMessage({
        content: i18n.global.t('desc.uploadModel.fileFormatNotMatch'),
      })
      return
    }
    // 文件大小校验
    if (file.raw.size / 1024 > limitSize) {
      clear()
      warningMessage({
        content: `${i18n.global.t(
          'desc.uploadModel.fileSizeExceed',
        )} ${limitSize} Kb`,
      })
      return
    }

    // 将 raw 对象转换成 url 支持 img 标签展示
    targetImgUrl.value = URL.createObjectURL(file.raw)

    // 触发外层确认事件
    emit('confirm', file.raw)
  },
  {
    immediate: true,
    deep: true,
  },
)

// 删除已上传的缩略图
function handleRemove () {
  // 如果缩略图不存在，则直接返回
  if (!targetImgUrl.value) return

  clear()
  emit('cancel', '')
}

// 清空当前组件状态
function clear () {
  targetImgUrl.value = ''
  fileList.value = []
}

function handleClickImgContainer () {
  uploaderRef.value?.$el.querySelector('.upload-area').click()
}
</script>

<template>
  <div class="thumb-upload-container">
    <div
      v-if="targetImgUrl"
      class="img-container"
      @click="handleClickImgContainer"
    >
      <img class="preview-img" :src="targetImgUrl" alt="">
      <span class="img-mask">
        <el-icon @click.stop="handleRemove"><Delete /></el-icon>
      </span>
    </div>
    <el-upload
      ref="uploaderRef"
      v-model:file-list="fileList"
      action="#"
      class="thumb-uploader"
      :style="{ display: targetImgUrl ? 'none' : 'block' }"
      :multiple="false"
      :auto-upload="false"
      :show-file-list="false"
      :disabled="uploadThumbDisabled"
    >
      <!-- 触发上传操作 -->
      <template #trigger>
        <div class="upload-area">
          <el-icon class="upload-icon">
            <Plus />
          </el-icon>
        </div>
      </template>
    </el-upload>
    <div class="tip">
      {{ $t('desc.uploadModel.thumbnailComponentTips') }}
    </div>
  </div>
</template>

<style lang="less">
.thumb-upload-container {
  --size: 80px;
  position: relative;

  .img-container {
    width: var(--size);
    height: var(--size);
    border-radius: 2px;
    border: var(--input-border);
    box-sizing: border-box;
    position: relative;
    margin-bottom: 10px;

    .preview-img {
      width: 100%;
      height: 100%;
      display: inline-block;
      border-radius: 2px;
    }
    .img-mask {
      position: absolute;
      width: 100%;
      height: 100%;
      left: 0;
      top: 0;
      cursor: default;
      display: inline-flex;
      justify-content: center;
      align-items: center;
      color: #fff;
      opacity: 0;
      font-size: 20px;
      background-color: rgba(0, 0, 0, 0.5);
      &:hover {
        opacity: 1;
      }
      .el-icon {
        --color: inherit;
        height: 1em;
        width: 1em;
        line-height: 1em;
        display: inline-flex;
        justify-content: center;
        align-items: center;
        position: relative;
        font-size: inherit;
        cursor: pointer;
      }
    }
  }

  .thumb-uploader {
    .el-upload {
      cursor: v-bind(cursorStyle);
    }

    margin-bottom: 10px;
    .el-upload--picture-card {
      width: var(--size);
      height: var(--size);
    }
    .upload-area {
      width: var(--size);
      height: var(--size);
      background-color: var(--main-dark-color);
      border: var(--input-border);
      border-radius: 2px;
      display: flex;
      align-items: center;
      justify-content: center;
      box-sizing: border-box;

      .upload-icon {
        width: 14px;
        height: 14px;
        color: #666666;
      }
    }
  }
  .tip {
    font-size: 11px;
    margin-top: 4px;
  }
}
</style>
