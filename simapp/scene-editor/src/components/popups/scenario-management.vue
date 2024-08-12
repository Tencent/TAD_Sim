<template>
  <div class="scenario-management">
    <el-tabs v-model="activeTab">
      <el-tab-pane :label="$t('scenario.myScene')" name="list">
        <el-tabs v-model="activeSceneType" class="tap-center">
          <el-tab-pane label="WorldSim场景" name="sim">
            <ScenarioList
              v-if="activeTab === 'list' && activeSceneType === 'sim'"
              scene-type="sim"
              @close="$emit('close')"
            />
          </el-tab-pane>
          <el-tab-pane label="LogSim场景" name="simrec">
            <ScenarioList
              v-if="activeTab === 'list' && activeSceneType === 'simrec'"
              scene-type="simrec"
              @close="$emit('close')"
            />
          </el-tab-pane>
        </el-tabs>
      </el-tab-pane>
      <el-tab-pane :label="$t('scenario.sets')" name="set">
        <ScenarioSet
          v-if="activeTab === 'set'"
          @close="$emit('close')"
        />
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import ScenarioSet from './scenario-set.vue'
import ScenarioList from './scenario-list.vue'

export default {
  name: 'ScenarioManagement',
  components: {
    ScenarioSet,
    ScenarioList,
  },
  data () {
    return {
      activeTab: 'list',
      activeSceneType: 'sim',
    }
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .tap-center :deep(.el-tabs__nav-wrap) {
    display: flex;
    justify-content: center;
  }

  .scenario-management {
    width: 600px;
    height: auto;
    padding: 23px;
  }
</style>
