// 自定义模型缓存数据
import { defineStore } from 'pinia'
import { get } from 'lodash'
import { useObjectStore } from './index'
import { deleteCustomModel, getCustomModelList } from '@/services/index'
import { getObject, setObject } from '@/utils/mapCache'
import { fixedPrecision } from '@/utils/common3d'
import { errorMessage, successMessage } from '@/utils/common'
import i18n from '@/locales/index'

// 从后端返回的自定义模型数据结构
interface ICommonCustomModel {
  name: string
  rawFileName: string
  variable: string
  model3d: string
  boundingBox: {
    center: {
      x: number
      y: number
      z: number
    }
    dimensions: {
      length: number
      width: number
      height: number
    }
  }
}

// 自定义模型的数据结构
export interface ICustomModelConfig {
  name: string
  rawFileName: string
  variable: string
  fbxModelName: string
  thumbUrl: string
  modelUrl: string
  posX: number
  posY: number
  posZ: number
  length: number
  width: number
  height: number
  placementAreaName: Array<string> // 支持放置的区域
  isCustomModel: boolean // 自定义模型标识
}

interface IState {
  timestamp: number
  customModelList: Array<ICustomModelConfig>
  deletedObjId: string[]
}

export const useModelCacheStore = defineStore('modelCache', {
  state: (): IState => {
    return {
      timestamp: 0,
      customModelList: [],
      deletedObjId: [], // deleteLocalCustomModel 操作中删除地图场景中的物体（用于判断地图 isDirty 的一个参数）
    }
  },
  getters: {
    currentList: (state): Array<ICustomModelConfig> => {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      return state.customModelList
    },
    hasDeletedObjWhenDeletingCustomModel: (state): boolean => {
      return !!state.deletedObjId.length
    },
  },
  actions: {
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 通过 name 属性查找对应的自定义模型配置
    getCustomModelConfigByName (name: string) {
      for (const option of this.customModelList) {
        if (option.name === name) {
          return option
        }
      }
      return null
    },
    // 更新某一个自定义模型配置
    updateCustomModelConfig (modelName: string) {
      // 由于重新加载一遍自定义模型配置后，配置跟 state 中存储的存在差别
      // 利用旧的数据状态，主要是【尺寸】信息获取缩放比例
      const objectStore = useObjectStore()
      const matchIds: Array<string> = []
      for (const id of objectStore.ids) {
        const objectData = getObject(id) as biz.ICustomModel
        if (!objectData) continue
        // 如果不是修改配置的目标自定义模型，则不需要调整
        if (objectData.name !== modelName) continue

        // 获取旧数据的尺寸信息
        const { width, length, basicLength, basicWidth } = objectData
        // 旧数据的缩放比例，保留一位小数
        const ratio = fixedPrecision(
          (width / basicWidth + length / basicLength) / 2,
          1,
        )

        // 获取新的配置
        const newConfig = this.getCustomModelConfigByName(modelName)
        if (!newConfig) return

        const {
          width: newBasicWidth,
          length: newBasicLength,
          height: newBasicHeight,
        } = newConfig

        objectData.basicLength = fixedPrecision(newBasicLength)
        objectData.basicWidth = fixedPrecision(newBasicWidth)
        objectData.basicHeight = fixedPrecision(newBasicHeight)
        // 基于尺寸比例计算实际的尺寸
        objectData.length = fixedPrecision(newBasicLength * ratio)
        objectData.width = fixedPrecision(newBasicWidth * ratio)
        objectData.height = fixedPrecision(newBasicHeight * ratio)

        // 强制更新（虽然是引用类型自动更新）
        setObject(objectData.id, objectData)

        matchIds.push(id)
      }

      this.updateTimestamp()

      // 返回需要更新的模型id集合
      return matchIds
    },
    // 请求接口同步所有自定义模型列表
    async syncLocalCustomModel (modelName?: string) {
      try {
        // 调用接口同步自定义模型列表
        const getRes = await getCustomModelList()

        if (!getRes) {
          throw new Error(
            `api get custom model list error. error info: ${get(
              getRes,
              'data',
            )}`,
          )
        }

        if (!get(getRes, 'data')) {
          getRes.data = []
        }

        const originModelList = (getRes.data as Array<ICommonCustomModel>) || []

        // 将本地的自定义模型列表清空
        this.customModelList.length = 0

        for (const option of originModelList) {
          const { name, rawFileName, variable, model3d, boundingBox } = option
          const { center, dimensions } = boundingBox
          const { x: posX, y: posY, z: posZ } = center
          const { length, width, height } = dimensions
          // 将路径分隔符统一成 webgl loader 可以加载的静态资源路径
          const validModelUrl = model3d.replaceAll('\\', '/')
          const pathSegment = validModelUrl.split('/')
          const fbxModelName = pathSegment[pathSegment.length - 1]
          // // 缩略图跟模型文件同名，只不过后缀为 .png
          // let thumbUrl = validModelUrl.replace(/.fbx$/i, '.png')
          // 缩略图为模型名称加上 thumbnail_ 前缀
          const thumbName = `thumbnail_${fbxModelName}`.replace(
            /.fbx$/i,
            '.png',
          )
          pathSegment[pathSegment.length - 1] = thumbName
          let thumbUrl = pathSegment.join('/')
          pathSegment.length = 0
          // 如果是因为修改某个自定义模型配置触发的重新获取模型列表，针对修改的模型缩略图添加标识重新加载
          if (name === modelName) {
            thumbUrl += `?${Math.random()}`
          }
          // 组装成前端的数据结构
          const _option: ICustomModelConfig = {
            name,
            rawFileName,
            variable,
            fbxModelName,
            thumbUrl,
            modelUrl: validModelUrl,
            posX,
            posY,
            posZ,
            length,
            width,
            height,
            isCustomModel: true,
            // 默认支持放置在道路和路口中
            placementAreaName: [
              'lane',
              'junction',
              'roadPlacementArea',
              'junctionPlacementArea',
            ],
          }
          this.customModelList.push(_option)
        }

        this.updateTimestamp()
      } catch (err) {
        console.log(err)
        errorMessage({
          content: i18n.global.t('desc.uploadModel.failedToGetCustomModelList'),
        })
      }
    },
    // 删除本地的自定义模型
    async deleteLocalCustomModel (option: ICustomModelConfig, index: number) {
      try {
        // 调用接口删除对应的模型
        const deleteRes = await deleteCustomModel(option.variable)
        if (!deleteRes || get(deleteRes, 'data.err') !== 0) {
          throw new Error(
            `delete custom model api error, error info: ${get(
              deleteRes,
              'data',
            )}`,
          )
        }

        // 根据接口返回的结果，判断是重新请求一遍全量获取接口，还是直接将缓存维护的列表手动删除
        const removeModelConfig = this.customModelList.splice(index, 1)[0]
        // 获取当前场景中所有对应的自定义模型数据
        const objectStore = useObjectStore()
        // 使用源引用对象的拷贝元素 id 集合
        const _ids = [...objectStore.ids]
        for (const _id of _ids) {
          const objectData = getObject(_id)
          if (!objectData) continue

          const { name, mainType } = objectData
          if (mainType === 'customModel' && name === removeModelConfig.name) {
            // 将对应的模型从数据层和渲染层中删除，不保留操作记录（即删除对应自定义模型配置后，无法通过撤销恢复）
            objectStore.removeCustomModel(_id, false)
            this.deletedObjId.push(_id)
          }
        }

        this.updateTimestamp()

        successMessage({
          content: i18n.global.t(
            'desc.uploadModel.deleteCustomModelSuccessfully',
          ),
        })
      } catch (err) {
        console.log(err)
        errorMessage({
          content: i18n.global.t('desc.uploadModel.failedToDeleteCustomModel'),
        })
      }
    },
  },
})
