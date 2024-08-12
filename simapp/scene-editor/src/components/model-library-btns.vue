<script setup lang="ts">
// 模型库的模型上面的按钮
import type { CatalogModel } from 'models-manager/src/catalogs/class.ts'
import { CatalogCategory } from 'models-manager/src/catalogs/class.ts'
import { computed } from 'vue'
import store from '@/store'
import { delModel } from '@/api/models.ts'
import i18n from '@/locales'
import { getModelProperty } from '@/common/utils.ts'
import { messageBoxConfirm } from '@/plugins/message-box'

const props = defineProps<{
  data: CatalogModel
}>()

function toEditModel () {
  const interfaceCategory = getModelProperty(props.data, 'interfaceCategory')
  store.commit('mission/startMission', {
    name: 'AddModel',
    props: {
      interfaceCategory,
      mode: 'edit',
      variable: props.data.variable,
    },
  })
}

function copyModel () {
  const interfaceCategory = getModelProperty(props.data, 'interfaceCategory')
  store.commit('mission/startMission', {
    name: 'AddModel',
    props: {
      interfaceCategory,
      mode: 'copy',
      variable: props.data.variable,
    },
  })
}

async function deleteModel () {
  await messageBoxConfirm(i18n.t('tips.deleteModelConfirm'))

  // 在当前场景中删除使用了该 Model 的 object
  if (store.getters['scenario/presentScenario']) {
    const objects = store.state.scenario.objects

    objects.forEach((obj: { name: string, type: string, carType: string, obstacleType: string, subType: string }) => {
      // 主车 object 上存储自定义模型名称的字段为 name，
      // 障碍物 object 上存储自定义模型名称的字段为 obstacleType，交通车上存储自定义模型名称的字段为 carType，其他 object 上存储自定义模型名称的字段为 subType
      let propertyName: 'carType' | 'obstacleType' | 'name' | 'subType'

      switch (obj.type) {
        case 'obstacle':
          propertyName = 'obstacleType'
          break
        case 'planner':
          propertyName = 'name'
          break
        case 'car':
          propertyName = 'carType'
          break
        default:
          propertyName = 'subType'
      }
      if (obj[propertyName] === props.data.variable) {
        store.dispatch('scenario/removeObject', obj)
      }
    })
  }

  await delModel(props.data.variable, props.data.catalogCategory)
  await store.dispatch('planners/getAllCatalogs')
}

const isPreset = getModelProperty(props.data, 'preset')

const btnVisible = computed(() => {
  const {
    catalogCategory,
    catalogSubCategory,
  } = props.data
  return !((catalogCategory === CatalogCategory.EGO && getModelProperty(props.data, 'preset')) || catalogSubCategory === 'combination')
})
</script>

<template>
  <div v-if="btnVisible && !isPreset" class="model-library-btns">
    <BaseAuth :perm="['action.importCustomModels.modify.enable']">
      <el-icon v-if="!isPreset">
        <EditPen @click="toEditModel" />
      </el-icon>
    </BaseAuth>
    <BaseAuth :perm="['action.importCustomModels.clone.enable']">
      <el-icon v-if="false">
        <CopyDocument @click="copyModel" />
      </el-icon>
    </BaseAuth>
    <BaseAuth :perm="['action.importCustomModels.delete.enable']">
      <el-icon v-if="!isPreset">
        <Delete @click="deleteModel" />
      </el-icon>
    </BaseAuth>
  </div>
</template>

<style scoped lang="less">
.model-library-btns {
  position: absolute;
  z-index: 5;
  bottom: 0;
  left: 0;
  width: 100%;
  display: flex;
  justify-content: space-around;
  align-items: center;
  height: 20px;
  background-color: rgba(19, 19, 19, 0.40);

  .el-icon {
    cursor: pointer;

    &:hover {
      color: #fff;
    }
  }
}
</style>
