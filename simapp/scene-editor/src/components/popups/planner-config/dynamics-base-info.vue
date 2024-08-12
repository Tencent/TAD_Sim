<template>
  <el-form
    ref="form"
    class="property-form dynamics-base-info"
    label-width="90px"
    :model="currentPlanner"
    @submit.prevent
  >
    <el-form-item class="param-cell" label="参数ID" prop="dynamic">
      <el-input
        :style="{ width: '110px' }"
        :model-value="currentPlanner.catalogParams[0].properties.dynamic"
        :disabled="true"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="参数文件路径" prop="dynamic_file_path">
      <el-input
        type="textarea"
        :style="{ width: '110px' }"
        :model-value="dynamic_file_path"
        :disabled="true"
      />
    </el-form-item>
  </el-form>
</template>

<script>
import { mapState } from 'vuex'

const { electron } = window
const { app, path } = electron
const { userDataPath = '' } = app || {}
const { sep = '\\' } = path || {}

export default {
  name: 'DynamicsBaseInfo',
  data () {
    return {}
  },
  computed: {
    ...mapState('planners', ['currentPlanner']),
    dynamic_file_path () {
      const id = this.currentPlanner.catalogParams[0].properties.dynamic
      const dynamicDirPath = ['data', 'scenario', 'Catalogs', 'Dynamics'].join(sep)
      const dynamicFileName = `dynamic_${id}.json`
      return `${userDataPath}${dynamicDirPath}${sep}${dynamicFileName}`
    },
  },
}
</script>

<style scoped>
.dynamics-base-info {
  padding: 0 20px;
}
</style>
