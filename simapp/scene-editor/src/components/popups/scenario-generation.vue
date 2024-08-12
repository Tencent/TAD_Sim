<template>
  <el-tabs type="border-card">
    <el-tab-pane :label="$t('menu.sceneGenerations')">
      <scenarioGeneration v-bind="$attrs" />
    </el-tab-pane>
    <el-tab-pane :label="$t('menu.sceneSemantic')">
      <scenarioSemantic v-bind="$attrs" />
    </el-tab-pane>
  </el-tabs>
</template>

<script>
import scenarioSemantic from './scenario-semantic.vue'
import scenarioGeneration from './scenario-generation-batch.vue'

export default {
  name: 'scenario-generation',
  components: {
    scenarioSemantic,
    scenarioGeneration,
  },
  data () {
    return { }
  },
  methods: {
  },
}
</script>

<style scoped lang="less">
  .el-tabs--border-card {
    border: none;
  }

</style>
