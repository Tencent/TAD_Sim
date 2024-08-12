<template>
  <el-dialog
    :model-value="true"
    :header="$t('operation.addScene')"
    append-to-body
    :close-on-click-modal="false"
    @close="$emit('close')"
  >
    <el-tabs v-model="sceneType" class="tap-center">
      <el-tab-pane label="WorldSim场景" name="sim" :disabled="availableSceneType === 'simrec'">
        <ScenarioList
          v-if="sceneType === 'sim'"
          class="scenario-add"
          mode="select"
          :selected="selected"
          :scene-type="sceneType"
          @close="$emit('close')"
          @confirm="handleConfirm"
          @available-scene-type-change="onSceneTypeChange"
        />
      </el-tab-pane>
      <el-tab-pane label="LogSim场景" name="simrec" :disabled="availableSceneType === 'sim'">
        <ScenarioList
          v-if="sceneType === 'simrec'"
          class="scenario-add"
          mode="select"
          :selected="selected"
          :scene-type="sceneType"
          @close="$emit('close')"
          @confirm="handleConfirm"
          @available-scene-type-change="onSceneTypeChange"
        />
      </el-tab-pane>
    </el-tabs>
  </el-dialog>
</template>

<script>
import ScenarioList from './scenario-list.vue'

export default {
  name: 'ScenarioAdd',
  components: {
    ScenarioList,
  },
  props: {
    selected: {
      type: Array,
      default () {
        return []
      },
    },
    canSwitchSceneType: {
      type: Boolean,
      default: true,
    },

    initSceneType: {
      type: String,
      default: 'sim',
    },

    initAvailableSceneType: {
      type: String,
      default: '',
    },
  },
  data () {
    return {
      sceneType: this.initSceneType,
      availableSceneType: this.initAvailableSceneType,
    }
  },
  computed: {
  },
  methods: {
    handleConfirm (payload) {
      this.$emit('confirm', payload)
    },
    onSceneTypeChange (payload) {
      if (this.canSwitchSceneType) {
        this.availableSceneType = payload
      }
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .scenario-add {
    width: 600px;
    height: auto;
    padding: 23px;
  }

  .tap-center :deep(.el-tabs__nav-wrap) {
    display: flex;
    justify-content: center;
  }

  .tap-center :deep(.el-tabs__item.is-top.is-disabled) {
    color: @global-font-color;
    &:hover {
      cursor: not-allowed
    };
  }
</style>
