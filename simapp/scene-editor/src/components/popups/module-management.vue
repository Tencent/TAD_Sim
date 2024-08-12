<template>
  <div class="module-management">
    <el-tabs v-model="activeTab">
      <el-tab-pane :label="$t('module.GroupConfig')" name="set">
        <ModuleSet
          @close="$emit('close')"
        />
      </el-tab-pane>
      <el-tab-pane :label="$t('module.moduleSettings')" name="setting">
        <ModuleSetting
          @close="$emit('close')"
        />
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import ModuleSet from './module-set.vue'
import ModuleSetting from './module-setting.vue'

export default {
  name: 'ModuleManagement',
  components: {
    ModuleSet,
    ModuleSetting,
  },
  data () {
    return {
      activeTab: 'set',
    }
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .module-management {
    width: 600px;
    height: auto;
    padding: 23px;
  }
</style>
