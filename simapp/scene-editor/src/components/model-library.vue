<template>
  <div class="model-library-body">
    <div class="model-library-wrap">
      <el-collapse v-if="models.length">
        <el-collapse-item :title="tCategory('planner')">
          <template #title>
            <div class="model-planner-title">
              <span class="model-planner-title-name">{{ tCategory('planner') }}</span>
              <el-icon class="model-planner-config">
                <Tools @click.stop="toConfigPlanner" />
              </el-icon>
            </div>
          </template>
          <div
            v-for="item in plannerList"
            :id="`model-planner-${item.variable}`"
            :key="`planner.${item.variable}`"
            class="model-item"
            :title="$getModelName(item)"
          >
            <div class="model-image">
              <img
                :id="`planner.${item.variable}`"
                v-my-drag="`planner.${item.variable}`"
                :alt="$getModelName(item)"
                :src="$getModelIcon(item, true)"
                draggable="false"
              >
              <ModelLibraryBtns :data="item" />
            </div>
            <div class="model-item-title">
              {{ $getModelName(item) }}
            </div>
          </div>
          <BaseAuth :perm="['router.importCustomModels.show']">
            <AddModelButton class="model-item" @click="addModel('ego')" />
          </BaseAuth>
        </el-collapse-item>
        <el-collapse-item v-for="type in models" :id="`model-${type}`" :key="type" :title="tCategory(type)">
          <div
            v-for="item in modelList[type]"
            :id="`model-${type}-${item.variable}`"
            :key="`${type}.${item.variable}`"
            class="model-item"
            :title="$getModelName(item)"
          >
            <div class="model-image">
              <img
                :id="`${type}.${item.variable}`"
                v-my-drag="`${type}.${item.variable}`"
                :alt="$getModelName(item)"
                :src="$getModelIcon(item, true)"
                draggable="false"
              >
              <ModelLibraryBtns :data="item" />
            </div>
            <div class="model-item-title">
              {{ $getModelName(item) }}
            </div>
          </div>
          <BaseAuth :perm="['router.importCustomModels.show']">
            <AddModelButton class="model-item" @click="addModel(type)" />
          </BaseAuth>
        </el-collapse-item>
      </el-collapse>
    </div>
    <div v-show="isPlaying" class="mask" />
  </div>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import AddModelButton from './add-model-button.vue'
import mockElectron from '@/api/mock-electron'
import ModelLibraryBtns from '@/components/model-library-btns.vue'
import { getModelDisplayName } from '@/common/utils'
import { getPermission } from '@/utils/permission'

const {
  electron: {
    tadsim: {
      isNeedToUpgrade,
    },
  } = mockElectron,
} = window

export default {
  name: 'ModelLibrary',
  components: {
    ModelLibraryBtns,
    AddModelButton,
  },
  data () {
    return {
      targetId: '',
    }
  },
  computed: {
    ...mapState('stage', [
      'clientRect',
    ]),
    ...mapGetters('scenario', [
      'isPlaying',
      'isModifying',
    ]),
    ...mapState('planners', [
      'plannerList',
    ]),
    ...mapGetters('catalogs', {
      modelList: 'models',
    }),
    models () {
      const canView = type => getPermission(`action.modelsLibrary.${type}.view.enable`)
      const origin = [
        'car',
        'moto',
        'bike',
        'pedestrian',
        'animal',
        'machine',
        'obstacle', // 'dynamicObstacle'
      ]
      return origin.filter(e => canView(e))
    },
  },
  async created () {
    await this.getAllSensors()
    if (isNeedToUpgrade) {
      await this.updateAllSensors()
    }
    await this.getAllDynamics()
    // 挪到stage.vue里面了await this.getAllCatalogs()
  },
  methods: {
    ...mapMutations('mission', ['startMission']),
    ...mapActions('planners', [
      'getAllSensors',
      'getAllDynamics',
      'updateAllSensors',
    ]),
    tCategory (name) {
      let key = 'model.'
      if (name === 'bike') {
        key = 'modelCategory.'
      }
      return this.$t(key + name)
    },
    $getModelName: getModelDisplayName,
    toConfigPlanner () {
      this.startMission('PlannerConfig')
    },
    addModel (interfaceCategory) {
      this.startMission({
        name: 'AddModel',
        props: {
          interfaceCategory,
          mode: 'add',
        },
      })
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.model-library-body {
  box-sizing: border-box;
  height: 100%;
  padding: 5px 0;
  user-select: none;
  position: relative;
}

.mask {
  position: absolute;
  z-index: 100;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: rgba(0, 0, 0, .3);
}

.model-library-wrap {
  height: 100%;
  overflow: auto;
}

.model-planner-title {
  flex: 1;
  display: flex;
  align-items: center;

  .model-planner-title-name {
    flex: 1;
    text-align: left;
  }

  .model-planner-config {
    margin-right: 8px;

    &:hover {
      color: @active-font-color;
    }
  }
}

.model-item {
  display: inline-block;
  width: 80px;
  margin: 10px;
}

.model-image {
  display: flex;
  width: 80px;
  height: 80px;
  box-sizing: border-box;
  align-items: center;
  justify-content: center;
  border: 2px solid transparent;
  background-color: @hover-bg;
  position: relative;

  .model-library-btns {
    display: none;
  }
}

.model-item img {
  max-width: 76px;
  max-height: 76px;
}

.model-item-title {
  .text-overflow;
  padding-top: 10px;
  font-size: 12px;
  line-height: 1.5em;
  color: @title-font-color;
  text-align: center;
}

.model-item:hover {
  .model-image {
    border-color: @active-font-color;

    .model-library-btns {
      display: flex;
    }
  }

  .model-item-title {
    color: @active-font-color;
  }
}

.model-item:nth-child(even) {
  padding-left: 12px;
}

:deep(.el-collapse-item .el-collapse-item__header) {
  color: @title-font-color;
}
</style>
