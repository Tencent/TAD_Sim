<template>
  <div class="module-global-management">
    <el-tabs v-model="activeTab">
      <el-tab-pane :label="$t('module.GroupConfig')" name="set">
        <ModuleGlobalSet
          @close="$emit('close')"
        />
      </el-tab-pane>
      <el-tab-pane :label="$t('module.moduleSettings')" name="setting">
        <ModuleSetting
          :global="true"
          @close="$emit('close')"
        />
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import ModuleGlobalSet from './module-global-set.vue'
import ModuleSetting from './module-setting.vue'

export default {
  name: 'ModuleGlobalManagement',
  components: {
    ModuleGlobalSet,
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

.module-global-management {
  width: 600px;
  height: auto;
  padding: 23px;
}
</style>
