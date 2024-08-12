<template>
  <el-form
    ref="form"
    class="property-form planner-params"
    label-position="left"
    disabled
    @submit.prevent
  >
    <el-collapse v-if="currentPlanner.catalogSubCategory === 'combination'">
      <template v-for="(params, index) of currentPlanner.catalogParams">
        <el-collapse-item
          v-if="index > 0"
          :key="index"
          :name="index"
          :title="vehiclePartName(index - 1)"
        >
          <PlannerParamForm :data="params" :index="index" />
        </el-collapse-item>
      </template>
    </el-collapse>
    <template v-else>
      <PlannerParamForm :style="{ padding: '0 22px' }" :data="currentPlanner.catalogParams[0]" :index="0" />
    </template>
  </el-form>
</template>

<script>
import { mapMutations, mapState } from 'vuex'
import PlannerParamForm from './planner-param-form.vue'

export default {
  name: 'PlannerParams',
  components: { PlannerParamForm },
  computed: {
    ...mapState('planners', ['currentPlanner']),
    mainParams () {
      return this.currentPlanner.catalogParams[0]
    },
  },
  methods: {
    ...mapMutations('planners', ['updatePlannerParam']),
    vehiclePartName (index) {
      if (index === 0) {
        return '车头'
      }
      return `挂车${index}`
    },
  },
}
</script>

<style scoped lang="less">
  .planner-params {
  }
</style>
