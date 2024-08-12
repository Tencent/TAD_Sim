<script setup lang="ts">
import { computed, h, onMounted, provide, ref, watch } from 'vue'
import { cloneDeep, set } from 'lodash-es'
import { ElButton, ElForm, ElMessage } from 'element-plus'
import type {
  CarModel,
  CatalogModel,
  CatalogSubCategory,
  Dimensions,
  EgoModel,
  ElectronFile,
  InterfaceCategory,
  ObstacleModel,
  PedestrianModel,
} from 'models-manager/src/catalogs/class.ts'
import {
  CatalogCategory,
  SubCategoryMap,
  categoryMap,
} from 'models-manager/src/catalogs/class.ts'
import { createCatalogModel } from 'models-manager/src/catalogs/utils.ts'
import type { FormItemRule } from 'element-plus/es/components/form/src/types'
import type { Arrayable } from 'element-plus/es/utils'
import ModelView from './model-view.vue'
import EgoForm from './model-detail/ego.vue'
import CarForm from './model-detail/car.vue'
import PedestrianForm from './model-detail/pedestrain.vue'
import { uploadModelZip, uploadThumbnail } from '@/api/models.ts'
import store from '@/store'
import {
  filePathToURL,
  fileURLToPath,
  formatModel3d,
  genHashAndModelIdFromName,
  getModel3dURL,
  getModelDisplayName,
  getModelIcon,
  getModelInterfaceCatalog,
  setModel3d,
  setModelDisplayName,
  setModelProperty,
} from '@/common/utils.ts'
import { errorHandler } from '@/common/errorHandler'
import { messageBoxConfirm } from '@/plugins/message-box.ts'

const props = defineProps<{
  interfaceCategory: InterfaceCategory
  mode: 'add' | 'edit' | 'copy'
  variable?: string
}>()

const emits = defineEmits<{
  (e: 'close'): void
}>()

const isAdd = props.mode === 'add'
const isEdit = props.mode === 'edit'
const isCopy = props.mode === 'copy'

provide('mode', props.mode)

function findModelInStore (interfaceCategory: InterfaceCategory, variable: string): CatalogModel {
  const catalogCategory = categoryMap[interfaceCategory]
  const {
    state: {
      planners: {
        plannerList,
      },
      catalogs: {
        pedestrianList,
        obstacleList,
        vehicleList,
      },
    },
  } = store
  let model: CatalogModel | undefined
  switch (catalogCategory) {
    case CatalogCategory.EGO:
      model = (plannerList as EgoModel[]).find(m => m.variable === variable)
      break
    case CatalogCategory.PEDESTRIAN:
      model = (pedestrianList as PedestrianModel[]).find(m => m.variable === variable)
      break
    case CatalogCategory.CAR:
      model = (vehicleList as CarModel[]).find(m => m.variable === variable)
      break
    case CatalogCategory.OBSTACLE:
      model = (obstacleList as ObstacleModel[]).find(m => m.variable === variable)
      break
  }
  if (!model) {
    throw new Error(`未找到模型定义：${interfaceCategory} ${variable}`)
  }
  return model
}

// edit和copy获取现有模型，add新建模型
let initialModel
if (isAdd) {
  initialModel = createCatalogModel(props.interfaceCategory)
} else if (isEdit) {
  initialModel = findModelInStore(props.interfaceCategory, props.variable!)
} else if (isCopy) {
  initialModel = cloneDeep(findModelInStore(props.interfaceCategory, props.variable!))
  setModelProperty(initialModel, 'preset', false)
}

// 模型实体
const model = ref<CatalogModel>(initialModel!)

function updateModel (path: string, value: any) {
  set(model.value, path, value)
}

let initialName = ''
// 编辑和复制时，获取初始name
if (isEdit || isCopy) {
  initialName = getModelDisplayName(model.value)
}
// 用户名称
const modelName = ref<string>(initialName)

// 实时更新variable和modelId
watch(modelName, (value) => {
  if (!value) {
    model.value.variable = ''
    if (model.value.catalogCategory !== CatalogCategory.EGO) {
      if (model.value.catalogCategory === CatalogCategory.CAR) {
        model.value.catalogParams[0].properties.modelId = -1
      } else {
        model.value.catalogParams.properties.modelId = -1
      }
    }
    return
  }
  const {
    hash: hashValue,
    modelId,
  } = genHashAndModelIdFromName(value)

  setModelDisplayName(model.value, value)
  setModelProperty(model.value, 'modelId', modelId)
  model.value.variable = `user_${model.value.catalogCategory}_${hashValue}`
})

// 模型加载后的错误显示
const errorFiles = ref<string[]>([])

function onFbxLoaded (result: { loadedFiles: string[], errorFiles: string[], dimensions: Dimensions }) {
  errorFiles.value = result.errorFiles
  form.value!.validateField('model3d')
  const { dimensions } = result
  // 模型加载后自动填入模型尺寸
  if (dimensions) {
    const popup = ElMessage({
      type: 'warning',
      duration: 5000,
      message: h('div', [
        h('p', '检测到模型尺寸，是否自动填入？'),
        // 确认填入
        h(
          ElButton,
          {
            onClick () {
              const {
                catalogCategory,
              } = model.value
              let path = 'catalogParams.boundingBox'
              if (catalogCategory === CatalogCategory.CAR || catalogCategory === CatalogCategory.EGO) {
                path = 'catalogParams[0].boundingBox'
              }
              updateModel(`${path}.dimensions.width`, dimensions.width)
              updateModel(`${path}.dimensions.length`, dimensions.length)
              updateModel(`${path}.dimensions.height`, dimensions.height)
              updateModel(`${path}.center.z`, dimensions.height / 2)
              popup.close()
            },
            type: 'primary',
          },
          () => '确认',
        ),
        // 不填入
        h(
          ElButton,
          {
            onClick () {
              popup.close()
            },
          },
          () => '取消',
        ),
      ]),
      customClass: 'sim-info-1',
      center: true,
    })
  }
}

const form = ref<InstanceType<typeof ElForm>>()
// 上传模型的错误提示
const uploadModelErrorMessage = ref('')
const uploadThumbErrorMessage = ref('')
const uploadFileName = ref('')
// 表单校验规则
const nameRules: Arrayable<FormItemRule> = [
  {
    required: true,
    message: '请输入模型名称。',
    trigger: 'blur',
  },
  {
    trigger: 'blur',
    validator (_rule, _value, callback) {
      // 编辑模式下不能编辑名称，直接校验通过
      if (isEdit) return callback()
      const {
        variable: modelVariable,
        catalogParams,
        // catalogCategory,
      } = model.value
      const {
        state: {
          planners: {
            plannerList,
          },
          catalogs: {
            obstacleList,
            pedestrianList,
            vehicleList,
          },
        },
      } = store
      const param = Array.isArray(catalogParams) ? catalogParams[0] : catalogParams

      // 查找是否有variable和modelId相同的模型
      const isExistInPlanners = !!(plannerList as EgoModel[]).find((planner) => {
        return planner.variable === modelVariable
      })
      const isExistInCars = !!(vehicleList as CarModel[]).find((vehicle) => {
        return (
          vehicle.variable === modelVariable ||
          vehicle.catalogParams[0].properties.modelId === param.properties.modelId
        )
      })
      const isExistInObstacles = !!(obstacleList as ObstacleModel[]).find((obstacle) => {
        return (
          obstacle.variable === modelVariable ||
          obstacle.catalogParams.properties.modelId === param.properties.modelId
        )
      })
      const isExistInPedestrians = !!(pedestrianList as PedestrianModel[]).find((pedestrian) => {
        return (
          pedestrian.variable === modelVariable ||
          pedestrian.catalogParams.properties.modelId === param.properties.modelId
        )
      })
      const isExist = isExistInPlanners || isExistInCars || isExistInObstacles || isExistInPedestrians
      if (isExist) {
        return callback(new Error('模型内部ID已存在，请重新输入。'))
      }
      callback()
    },
  },
]
const modelPathRules: Arrayable<FormItemRule> = [
  {
    validator (_rule, _value, callback) {
      uploadModelErrorMessage.value = ''
      // 新建模式下uploadId为-1代表没有上传模型文件
      if (isAdd && uploadId.value === '-1') {
        callback(new Error('请上传模型文件。'))
        return
      }
      if (errorFiles.value.length) {
        const paths = errorFiles.value.map(url => url.split('/').pop())
        callback(new Error(`部分贴图文件未找到: ${paths.join(', ')}`))
      }
      callback()
    },
  },
]
// 用户没有上传缩略图时自动截一张图
// const thumbnailRules: Arrayable<FormItemRule> = [
//   {
//     validator(_rule, _value, callback) {
//       uploadThumbErrorMessage.value = ''
//       if (thumbnail.value) {
//         callback()
//       } else {
//         callback(new Error('请上传模型缩略图。'))
//       }
//     }
//   }
// ]

// 模型次级类型的选项
const modelCategoryMap = computed<Array<CatalogSubCategory>>(() => {
  return SubCategoryMap[getModelInterfaceCatalog(model.value)]
})

// 上传模型id，用于保存
const uploadId = ref<string>('-1')
// 上传后的模型文件列表
const fileList = ref<Array<string>>([])

// 上传模型文件
function uploadZip (file: File) {
  const extName = file.name.split('.').pop() || ''
  if (extName.toLowerCase() !== 'zip') {
    uploadModelErrorMessage.value = '仅支持 .zip 文件格式'
    return false
  }
  if (file.size > 1024 * 1024 * 5) {
    uploadModelErrorMessage.value = '文件大小不能超过 5MB'
    return false
  }
  uploadModelZip((file as ElectronFile).path)
    .then(({ id, filepath }) => {
      uploadId.value = id
      fileList.value = filepath
      const fbx = fileList.value.find(path => path.endsWith('.fbx'))
      if (fbx) {
        uploadFileName.value = file.name
        setModel3d(model.value, fbx)
        // 上传模型后主动删除缩略图避免错误
        delThumbnail()
      } else {
        uploadModelErrorMessage.value = '压缩包里不包含 .fbx 文件'
      }
    })
  return false
}

// 新增模型且没有上传模型文件时，不许上传缩略图
const uploadThumbnailDisabled = computed(() => {
  return isAdd && uploadId.value === '-1'
})

// 截图路径数据
const thumbnail = ref<string>('')
// 截图路径展示
const thumbnailUrl = ref<string>('')
const modelPreview = ref<typeof ModelView>()

if (isEdit || isCopy) {
  thumbnail.value = getModelIcon(model.value)
  thumbnailUrl.value = thumbnail.value
}

// 截图事件
function onSnapShot (path: string) {
  thumbnail.value = path
  thumbnailUrl.value = `${filePathToURL(path)}?t=${Date.now()}`
}

function uploadIcon (file: File) {
  const extName = file.name.split('.').pop() || ''
  if (extName.toLowerCase() !== 'png') {
    uploadThumbErrorMessage.value = '仅支持 .png 文件格式'
    return false
  }
  if (file.size > 1024 * 300) {
    uploadThumbErrorMessage.value = '文件大小不能超过 300KB'
    return false
  }
  const model3d = getModel3dURL(model.value)
  if (!model3d) return
  const pathArr = model3d.split('/')
  pathArr.pop()
  const dir = fileURLToPath(pathArr.join('/'))
  uploadThumbnail(file, dir)
    .then(({ dstpath }) => {
      thumbnail.value = dstpath
      thumbnailUrl.value = `${filePathToURL(dstpath)}?t=${Date.now()}`
    })
  return false
}

function delThumbnail () {
  thumbnail.value = ''
  thumbnailUrl.value = ''
}

function onCancel () {
  emits('close')
}

async function onSave () {
  try {
    const valid = await form.value!.validate()
    if (!valid) {
      return
    }
  } catch (e) {
    console.log(e)
    return
  }

  if (isEdit) {
    try {
      await messageBoxConfirm('更新内容将会在所有场景文件中应用，确认是否保存修改？', '编辑模型')
    } catch (e) {
      return
    }
  }

  // 没有截图，自动截图
  if (!thumbnail.value) {
    await modelPreview.value!.autoSnapShot()
  }

  let data
  // 如果model3d以file://开头，则说明是用户上传的模型或主车模型，需要更新model3d。否则是非主车预设模型文件
  if (getModel3dURL(model.value).startsWith('file://')) {
    // 上传了新模型，则以模型的variable计算模型路径
    const useModelVariable = uploadId.value !== '-1'
    data = formatModel3d(model.value, useModelVariable)
  } else {
    data = cloneDeep(model.value)
  }
  try {
    await store.dispatch('catalogs/saveModel', { id: uploadId.value, data })
    emits('close')
  } catch (e) {
    if (e instanceof Error) {
      e.message = `保存模型失败: ${e.message}`
    }
    errorHandler(e)
  }
}

onMounted(() => {
  store.dispatch('module-set/getModuleSetList')
})
</script>

<template>
  <div class="add-model">
    <div class="form-content">
      <ElForm ref="form" class="model-form" :model="model">
        <el-form-item label="模型名称" prop="variable" :rules="nameRules">
          <el-input
            v-model.trim="modelName"
            placeholder="请输入模型名称"
            :disabled="isEdit"
            maxlength="20"
          />
        </el-form-item>
        <el-form-item label="模型类型">
          <el-select v-model="model.catalogSubCategory">
            <el-option
              v-for="value of modelCategoryMap"
              :key="value"
              :label="$t(`modelCategory.${value}`)"
              :value="value"
            />
          </el-select>
        </el-form-item>
        <el-form-item
          class="uploader-wrapper"
          label="上传文件"
          prop="model3d"
          :rules="modelPathRules"
          :inline-message="true"
          :error="uploadModelErrorMessage"
        >
          <div class="file-name">
            {{ uploadFileName }}
          </div>
          <el-upload
            class="model-upload-wrapper"
            drag
            action="#"
            accept=".zip"
            :show-file-list="true"
            :before-upload="uploadZip"
          >
            <div class="el-upload__text">
              点击上传&nbsp;/&nbsp;拖拽到此区域
            </div>
            <template #tip>
              <div class="el-upload__tip">
                仅支持&nbsp;.zip&nbsp;文件格式，大小不超过5MB
              </div>
            </template>
          </el-upload>
        </el-form-item>
        <el-form-item
          class="uploader-wrapper"
          label="缩略图"
          prop="thumbnail"
          :inline-message="true"
          :error="uploadThumbErrorMessage"
        >
          <el-upload
            class="thumbnail-upload-wrapper"
            :class="{ 'thumbnail-disabled': uploadThumbnailDisabled }"
            action="#"
            drag
            accept=".png"
            :show-file-list="false"
            :before-upload="uploadIcon"
            :disabled="uploadThumbnailDisabled"
          >
            <el-icon v-if="!thumbnail">
              <Plus />
            </el-icon>
            <div v-else class="thumbnail-img-wrap">
              <el-image
                :src="thumbnailUrl"
                :style="{
                  width: '100%',
                  height: '100%',
                }"
              />
              <div class="delete-thumbnail-button">
                <el-icon @click.stop="delThumbnail">
                  <Delete />
                </el-icon>
              </div>
            </div>

            <template #tip>
              <div class="el-upload__tip">
                仅支持&nbsp;.png&nbsp;文件格式，大小不超过300KB
              </div>
            </template>
          </el-upload>
        </el-form-item>
      </ElForm>
      <div class="model-preview">
        <ModelView
          ref="modelPreview"
          :data="model"
          @snap-shot="onSnapShot"
          @loaded="onFbxLoaded"
        />
        <EgoForm
          v-if="model.catalogCategory === CatalogCategory.EGO"
          :model-value="model"
          @change="updateModel"
        />
        <CarForm
          v-else-if="model.catalogCategory === CatalogCategory.CAR"
          :model-value="model"
          @change="updateModel"
        />
        <PedestrianForm
          v-else-if="(
            model.catalogCategory === CatalogCategory.PEDESTRIAN
            || model.catalogCategory === CatalogCategory.OBSTACLE
          )"
          :model-value="model"
          @change="updateModel"
        />
      </div>
    </div>
    <div class="form-buttons">
      <ElButton @click="onCancel">
        取消
      </ElButton>
      <ElButton type="primary" @click="onSave">
        确定
      </ElButton>
    </div>
  </div>
</template>

<style scoped lang="less">
@import "@/assets/less/mixins";

.add-model {
  padding: 20px;
  width: 640px;
  height: 640px;

  .form-content {
    display: flex;
    height: 614px;
  }

  .form-buttons {
    display: flex;
    justify-content: flex-end;
    height: 32px;
  }

  .model-form {
    width: 300px;

    .el-form-item {
      display: block;
    }

    .model-upload-wrapper {
      width: 100%;
    }

    .thumbnail-upload-wrapper {
      width: 150px;

      &.thumbnail-disabled {
        :deep(.el-upload) {
          &:focus {
            color: inherit;
          }

          .el-upload-dragger {
            border-color: #404040;
            cursor: not-allowed;
          }
        }
      }

      :deep(.el-upload-dragger) {
        padding: 0;
        height: 150px;
        display: flex;
        justify-content: center;
        align-items: center;
      }

      .thumbnail-img-wrap {
        position: relative;
        z-index: 1;
        width: 100%;
        height: 100%;
        cursor: default;

        .delete-thumbnail-button {
          position: absolute;
          z-index: 2;
          width: 100%;
          height: 100%;
          top: 0;
          left: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          font-size: 24px;
          background-color: rgba(0, 0, 0, .2);
          visibility: hidden;

          .el-icon {
            cursor: pointer;

            &:hover {
              color: @global-font-color;
            }
          }
        }

        &:hover .delete-thumbnail-button {
          visibility: visible;
        }
      }
    }

    .uploader-wrapper {
      :deep(.el-form-item__error) {
        margin-left: 0;
        width: 100%;
      }
    }
  }

  .file-name {
    margin: 10px 0;
  }

  .model-preview {
    width: 340px;
    margin-left: 24px;
  }
}
</style>
