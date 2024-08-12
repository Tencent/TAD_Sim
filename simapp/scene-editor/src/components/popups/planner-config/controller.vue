<template>
  <el-form
    ref="form"
    class="property-form planner-controller"
    label-position="left"
    @submit.prevent
  >
    <el-form-item class="param-cell" label="控制器">
      <el-select
        :model-value="currentPlanner.catalogParams[0].properties.controller"
        @change="updatePlanner({ 'catalogParams[0].properties.controller': $event })"
      >
        <el-option value="none" label="None" />
        <el-option
          v-for="scheme of mainCarModuleSetList"
          :key="scheme.id"
          :value="scheme.name"
          :label="scheme.name"
        />
      </el-select>
    </el-form-item>
  </el-form>
</template>

<script>
import { mapGetters, mapMutations, mapState } from 'vuex'

export default {
  name: 'PlannerController',
  computed: {
    ...mapState('planners', ['currentPlanner']),
    ...mapGetters('module-set', ['mainCarModuleSetList']),
  },
  methods: {
    ...mapMutations('planners', ['updatePlanner']),
  },
}
</script>

<style scoped lang="less">
  .planner-controller {
    padding: 0 20px;
    .el-form-item {
      :deep(.el-form-item__label) {
        display: inline-block;
        width: 51%;
        height: initial;
      }
      :deep(.el-form-item__content) {
        width: 49%;
        float: left;
        .el-select {
          width: 100%;
        }
      }
    }
  }
</style>
