<template>
  <el-dialog
    header="旧版传感器数据转换器"
    append-to-body
    :close-on-click-modal="false"
    :close-on-press-escape="false"
    @close="$emit('update:visible', false)"
  >
    <div class="old-sensor-converter">
      <p class="tips">
        <span class="red">实验中的功能</span><br>
        本转换器可转换旧版 TADSim 中的传感器数据。<br>
        一套传感器配置可以转换成一套主车配置。<br>
        如果想转换某个场景里的传感器配置，请先加载那个场景，再回到本界面。
      </p>
      <ul class="sensor-group-list">
        <li
          v-for="(group, index) of sensorGroups"
          :key="index"
          class="sensor-group"
          @click="createByGroup(index)"
        >
          <template v-if="index === 0">
            场景配置
          </template>
          <template v-else-if="index === 5">
            全局配置
          </template>
          <template v-else>
            预设配置{{ index }}
          </template>
        </li>
      </ul>
      <el-dialog
        header="生成主车"
        :model-value="createPlannerDialogVisible"
        append-to-body
        :close-on-click-modal="false"
        :close-on-press-escape="false"
        @close="createPlannerDialogVisible = false"
      >
        <div class="create-planner-by-sensor-group">
          <el-form class="create-planner-form" inline label-width="150">
            <el-form-item>
              <template #label>
                源主车：
                <el-popover trigger="hover" content="新主车将会基于这个主车创建">
                  <template #reference>
                    <el-icon class="el-icon-info">
                      <Warning />
                    </el-icon>
                  </template>
                </el-popover>
              </template>
              <PlannerSelector :name="sourcePlannerName" @change="selectPlanner($event)" />
            </el-form-item>
            <el-form-item :error="nameErrorMessage">
              <template #label>
                新主车名称：
                <el-popover trigger="hover" content="最多输入10个字符">
                  <template #reference>
                    <el-icon class="el-icon-info">
                      <Warning />
                    </el-icon>
                  </template>
                </el-popover>
              </template>
              <el-input v-model="newPlannerName" maxlength="10" />
            </el-form-item>
            <el-form-item label="转换范围">
              <el-radio-group v-model="transRange">
                <el-radio :label="0" :value="0">
                  全部传感器
                </el-radio>
                <el-radio :label="1" :value="1">
                  仅勾选的
                </el-radio>
              </el-radio-group>
            </el-form-item>
          </el-form>
          <div class="btns">
            <el-button size="small" :disabled="!valid" @click="createPlanner">
              确定
            </el-button>
            <el-button size="small" @click="backToMain">
              取消
            </el-button>
          </div>
        </div>
      </el-dialog>
    </div>
  </el-dialog>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { ElMessage } from 'element-plus'
import PlannerSelector from './planner-selector.vue'
import { sensorTypeMap } from '@/store/modules/sensor/constants'

let sensorIndex = 1

export default {
  name: 'OldSensorConverter',
  components: { PlannerSelector },
  data () {
    return {
      currentGroupIndex: -1,
      createPlannerDialogVisible: false,
      newPlannerName: '',
      sourcePlannerName: '',
      sourcePlannerIndex: -1,
      transRange: 0, // 0: 全部, 1: 仅生效的
    }
  },
  computed: {
    ...mapState('sensor', [
      'sensorGroups',
    ]),
    ...mapState('planners', [
      'plannerList',
      'currentPlanner',
    ]),
    ...mapGetters('planners', {
      sensorsMap: 'sensorGroups',
    }),
    valid () {
      const {
        sourcePlannerIndex,
      } = this
      if (sourcePlannerIndex === -1) {
        return false
      }
      return this.nameValid
    },
    nameValid () {
      const newName = this.newPlannerName.trim()
      if (!newName) {
        return false
      }
      return !this.plannerList.some(p => this.$getModelName(p) === newName)
    },
    nameErrorMessage () {
      if (!this.nameValid) {
        if (this.newPlannerName.trim()) {
          return '该主车名称已存在'
        }
      }
      return null
    },
  },
  created () {
    this.getSensors()
  },
  methods: {
    ...mapActions('sensor', [
      'getSensors',
    ]),
    ...mapActions('planners', [
      'copyPlanner',
      'copySensor',
      'modifySensor',
      'addSensorToPlanner',
      'savePlanner',
      'createSensor',
    ]),
    ...mapMutations('planners', [
      'deletePlannerSensor',
      'selectSensor',
      'changeBaseInfo',
      'changeAdvancedInfo',
      'updatePlanner',
    ]),
    createByGroup (index) {
      this.currentGroupIndex = index
      this.createPlannerDialogVisible = true
    },
    selectPlanner (planner) {
      const index = this.plannerList.findIndex(p => p === planner)
      this.sourcePlannerName = planner.variable
      this.sourcePlannerIndex = index
    },
    backToMain () {
      this.sourcePlannerName = ''
      this.newPlannerName = ''
      this.sourcePlannerIndex = -1
      this.createPlannerDialogVisible = false
      this.transRange = 0
      this.currentGroupIndex = -1
    },
    validName (name) {
      const newName = name.trim()
      if (!newName) {
        return false
      }
      return !this.plannerList.some(p => this.$getModelName(p) === newName)
    },
    async createPlanner () {
      const {
        newPlannerName,
        sourcePlannerIndex,
        currentGroupIndex,
        sensorGroups,
        transRange,
        sensorsMap,
      } = this
      const newName = newPlannerName.trim()
      let sensors = sensorGroups[currentGroupIndex]
      sensorIndex = 1
      if (transRange === 1) {
        sensors = sensors.filter(s => s.enabled)
      }
      await this.copyPlanner(sourcePlannerIndex)
      this.updatePlanner({ 'catalogParams[0].properties.alias': newName })
      const oldSensors = this.currentPlanner.sensor.group
      if (oldSensors.length) {
        const ids = oldSensors.map((s, i) => i)
        this.deletePlannerSensor(ids)
      }
      for (let i = 0; i < sensors.length; i++) {
        const s = sensors[i]
        const target = sensorsMap[sensorTypeMap[s.type]][0]
        const newIdx = await this.copySensor(target.idx)
        await this.createSensor({ idx: newIdx, name: `${newName}上的${s.type}${sensorIndex}` })
        this.addSensorToPlanner(newIdx)
        // 此处是选择index，不是idx, 所以恒选最后一个
        this.selectSensor([this.currentPlanner.sensor.group.length - 1]);
        ['Device', 'InstallSlot'].forEach((key) => {
          this.changeBaseInfo({ paramName: key, value: s[key] })
        })
        Object.entries(s.baseInfo).forEach(([key, data]) => {
          this.changeBaseInfo({ paramName: key, value: data.value })
        })
        Object.entries(s.advancedInfo).forEach(([key, data]) => {
          if (key === 'InsideParamGroup') {
            this.changeAdvancedInfo({ paramName: 'IntrinsicParamType', value: data.value })
          } else {
            this.changeAdvancedInfo({ paramName: key, value: data.value })
          }
        })
        sensorIndex++
      }
      await this.savePlanner()
      ElMessage.success('转换成功！')
      this.backToMain()
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins.less";

.old-sensor-converter {
  width: 400px;
  height: 300px;
  padding: 15px;

  .tips {
    font-size: 14px;
    color: @global-font-color;
  }

  .sensor-group-list {
    padding: 10px 0;

    .sensor-group {
      margin-bottom: 10px;
      cursor: pointer;

      &:hover {
        color: @active-font-color;
      }
    }
  }
}

.create-planner-by-sensor-group {
  width: 300px;
  height: 200px;
  padding: 20px;
  display: flex;
  flex-direction: column;

  .create-planner-form {
    flex: 1;
  }
}

.red {
  color: #ff3333;
}

.btns {
  display: flex;
  justify-content: flex-end;
}
</style>
