<template>
  <div class="left-bar">
    <el-tabs v-model="activeTab">
      <template v-if="isModifying">
        <el-tab-pane :label="$t('scenario.library')" name="0">
          <LocalScenarioList />
        </el-tab-pane>
        <el-tab-pane :label="$t('model.model_library')" name="1">
          <ModelLibrary />
        </el-tab-pane>
      </template>
      <el-tab-pane v-if="isPlaying" :label="$t('player.playlist')" name="2">
        <el-tabs id="tab-center" v-model="localSceneType" class="tap-center">
          <!-- todo: 翻译 -->
          <el-tab-pane label="WorldSim场景" name="sim" class="scene-item">
            <Playlist
              v-if="localSceneType === 'sim'"
            />
          </el-tab-pane>
          <el-tab-pane label="LogSim场景" name="simrec" class="scene-item">
            <Playlist
              v-if="localSceneType === 'simrec'"
            />
          </el-tab-pane>
        </el-tabs>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import ModelLibrary from './model-library.vue'
import LocalScenarioList from './local-scenario-list.vue'
import Playlist from './playlist.vue'

export default {
  name: 'LeftBar',
  components: {
    ModelLibrary,
    LocalScenarioList,
    Playlist,
  },
  data () {
    return {
      activeTab: '0',
    }
  },
  computed: {
    ...mapState('scenario', [
      'status',
      'currentScenario',
    ]),
    ...mapState('playlist', [
      'sceneType',
    ]),
    ...mapGetters('scenario', [
      'isModifying',
      'isPlaying',
    ]),

    localSceneType: {
      get () {
        return this.sceneType
      },
      set (val) {
        this.updateState({
          sceneType: val,
        })
      },
    },
  },
  watch: {
    status () {
      this.setActiveTab()
    },
    currentScenario () {
      this.setActiveTab()
    },
  },
  async created () {
    await this.getScenarioList()
    await this.getScenarioSetList()
    await this.getPlaylist()
  },
  methods: {
    ...mapActions('scenario', [
      'getScenarioList',
    ]),
    ...mapActions('scenario-set', [
      'getScenarioSetList',
    ]),
    ...mapActions('playlist', [
      'getPlaylist',
    ]),
    ...mapMutations('playlist', ['updateState']),
    setActiveTab () {
      if (this.isModifying) {
        if (this.currentScenario) {
          this.activeTab = '1'
        } else {
          this.activeTab = '0'
        }
      } else if (this.isPlaying) {
        this.activeTab = '2'
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.left-bar {
  display: flex;
  flex-direction: column;
  width: 240px;
  min-height: 1px;
  margin-right: 1px;
  color: @title-font-color;
  background-color: @darker-bg;

  :deep(.el-tabs__header) {
    margin-bottom: 0;
  }

  :deep(.el-tabs__nav-wrap) {
    //width: 200px;
  }

  :deep(.el-tabs__item){
    color: @title-font-color;
  }

  :deep(.el-tabs__item.is-active) {
    color: @active-font-color;
  }

  :deep(.el-tabs__nav-next), :deep(.el-tabs__nav-prev) {
    line-height: 28px;
  }

  .el-tabs {
    flex: 1;
    display: flex;
    min-height: 1px;

    :deep(.el-tabs__content) {
      flex: 1;
      display: flex;
      flex-direction: column;
      min-height: 1px;

      .el-tab-pane {
        flex: 1;
        display: flex;
        flex-direction: column;
        min-height: 1px;
      }
    }
  }

  #tab-center {
    :deep(.el-tabs__header) {
      display: flex;
      justify-content: center;
      flex-direction: row;
    }
    :deep(.el-tabs__item) {
      font-size: 0.7em;
    }
  }
}
</style>
