<template>
  <el-form
    ref="form"
    class="property-form planner-name"
    label-width="80px"
    :model="currentPlanner"
    @submit.prevent
  >
    <el-form-item class="param-cell" label="配置名称" prop="catalogParams[0].properties.alias" :rules="nameRules">
      <el-input
        :style="{ width: '110px' }"
        :model-value="currentPlanner.catalogParams[0].properties.alias"
        :disabled="!isNew"
        @input="updatePlanner({ 'catalogParams[0].properties.alias': $event.trim() })"
      />
    </el-form-item>
  </el-form>
</template>

<script>
import { mapActions, mapMutations, mapState } from 'vuex'
import { genHashAndModelIdFromName } from '@/common/utils'

export default {
  name: 'PlannerName',
  data () {
    return {
      nameRules: [
        {
          validator: (rule, value, callback) => {
            this.validatePlanner().then(() => {
              callback()
            }).catch(callback)
          },
        },
      ],
    }
  },
  computed: {
    ...mapState('planners', ['currentPlanner', 'isNew']),
  },
  methods: {
    ...mapMutations('planners', ['updatePlanner']),
    ...mapActions('planners', ['validatePlanner']),
    updatePlannerName ($event) {
      const newName = $event.trim()
      const {
        hash: hashValue,
      } = genHashAndModelIdFromName(newName)
      this.updatePlanner({
        'catalogParams[0].properties.alias': newName,
        // 更新主车名字的同时也要更新变量名
        'variable': `user_ego_${hashValue}`,
      })
    },
  },
}
</script>

<style scoped>
.planner-name {
  padding: 0 20px;
}
</style>
