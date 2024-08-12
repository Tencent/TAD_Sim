<template>
  <div class="generate-params">
    <template v-for="key in paramsObjShowItem" :key="key">
      <div class="generate-param-item">
        <el-form-item
          v-if="paramsObj[key].dispName"
          class="param-cell"
          :class="{ 'padding-horizontal-20': level < 2, 'label-top': paramsObj[key].u0Axis }"
          :label-width="
            paramsObj[key].hasOwnProperty('val') || paramsObj[key].hasOwnProperty('strVar') ? '51%' : '100%'
          "
        >
          <template #label>
            {{ paramsObj[key].dispName }}
            <template v-if="paramsObj[key].comment">
              <el-tooltip class="item" effect="dark" :content="paramsObj[key].comment" placement="top">
                <ICircleSvg class="i-circle" />
              </el-tooltip>
            </template>
          </template>
          <InputNumber
            v-if="paramsObj[key].hasOwnProperty('val')"
            v-model="paramsObj[key].val"
            :unit="['null', '[]'].includes(paramsObj[key].unit) ? '' : paramsObj[key].unit"
            @blur="notifyDirty"
          />
          <el-input
            v-else-if="paramsObj[key].hasOwnProperty('strVar')"
            v-model="paramsObj[key].strVar"
            @focus="recordStrVar(key)"
            @blur="strVarBlur(key)"
          />
          <template v-else-if="paramsObj[key].u0Axis">
            <DynamicsTableChart ref="tablechart" v-model="paramsObj[key]" :width="contentWidth" />
          </template>
        </el-form-item>
        <el-form-item
          v-else-if="(typeof paramsObj[key]) === 'string'"
          :label="key"
          class="param-cell"
          :class="level < 2 && 'padding-horizontal-20'"
        >
          <el-select
            v-model="paramsObj[key]"
            @change="handleSelectChange(key, $event)"
          >
            <el-option
              v-for="item in carTypeItemOptions[key]"
              :key="item"
              :hidden="!isShowOption(key, item)"
              :label="item"
              :value="item"
            />
          </el-select>
        </el-form-item>
        <el-collapse v-else>
          <el-collapse-item :title="key" :name="key">
            <GenerateParams
              ref="paramschart"
              v-model="paramsObj[key]"
              :car-type="carType"
              :sub-type="subType"
              :level="(level + 1)"
            />
          </el-collapse-item>
        </el-collapse>
      </div>
    </template>
  </div>
</template>

<script>
import { mapMutations, mapState } from 'vuex'
import DynamicsTableChart from './dynamics-table-chart.vue'
import InputNumber from '@/components/input-number.vue'
import ICircleSvg from '@/assets/images/i-circle.svg'
import mockElectron from '@/api/mock-electron'

const { electron: { checkPathExist } = mockElectron } = window

export default {
  name: 'GenerateParams',
  components: { InputNumber, DynamicsTableChart, ICircleSvg },
  props: {
    modelValue: {
      type: Object,
      required: true,
    },
    carType: {
      type: Object,
      required: true,
    },
    subType: {
      type: String,
      required: true,
    },
    level: {
      type: Number,
      default: 0,
    },
  },
  data () {
    return {
      paramsObj: {},
      carTypeItemOptions: {
        drivetrainType: ['FrontWheel_Drive', 'RearWheel_Drive', 'FourWheel_Drive'],
        propulsionType: ['Electric_Drive', 'ICE_Engine', 'Hybrid_Gen_P3'],
        suspensionType: ['I_I', 'I_S'],
        steerType: ['Mapped_Steer', 'Dynamic_Steer'],
      },
      contentWidth: 190,
      oldStrVar: '',
    }
  },
  computed: {
    ...mapState('planners', ['paramsWidth']),
    propulsionType () {
      return this.carType.propulsionType
    },
    drivetrainType () {
      return this.carType.drivetrainType
    },
    steerType () {
      return this.carType.steerType
    },
    paramsObjShowItem () {
      const { paramsObj, isShow } = this
      return Object.keys(paramsObj).filter(key => isShow(key))
    },
  },
  watch: {
    modelValue: {
      handler () {
        this.paramsObj = this.modelValue
      },
      deep: true,
    },
    paramsObj: {
      handler () {
        this.$emit('update:model-value', this.paramsObj)
      },
      deep: true,
    },
  },
  created () {
    this.paramsObj = this.modelValue
    this.contentWidth = this.paramsWidth - 44
  },
  mounted () {
    window.addEventListener('paramsResizeEvent', this.paramsResizeFn)
  },
  beforeUnmount () {
    window.removeEventListener('paramsResizeEvent', this.paramsResizeFn)
  },
  methods: {
    ...mapMutations('planners', ['setIsDirty']),
    strVarBlur (key) {
      const newVal = this.paramsObj[key].strVar
      let flag = true
      if (newVal === this.oldStrVar) {
        return
      }
      if (!newVal) {
        this.$message({
          showClose: true,
          message: `${this.paramsObj[key].dispName}不能为空`,
          type: 'warning',
        })
        flag = false
      }
      if (key === 'TNOTirPath') {
        if (!newVal.endsWith('.tir')) {
          this.$message({
            showClose: true,
            message: `${this.paramsObj[key].dispName}文件地址后缀必须为.tir文件`,
            type: 'warning',
          })
          flag = false
        }
        if (!checkPathExist(`./data/scenario/Catalogs/Dynamics/${newVal}`)) {
          this.$message({
            showClose: true,
            message: `${newVal}文件不存在`,
            type: 'warning',
          })
          flag = false
        }
      }
      if (!flag) {
        this.paramsObj[key].strVar = this.oldStrVar
        return
      }
      this.notifyDirty()
    },
    recordStrVar (key) {
      this.oldStrVar = this.paramsObj[key].strVar
    },
    paramsResizeFn ($event) {
      const { detail: { paramsWidth } } = $event
      this.contentWidth = paramsWidth - 44
    },
    notifyDirty () {
      this.setIsDirty(true)
    },
    isShow (key) {
      if (this.level !== 0) {
        return true
      }
      if (this.subType === 'propulsion' && ['engineParameter', 'frontMotorParameter', 'battParamter', 'rearMotorParameter', 'hybridParameter'].includes(key)) {
        return this.propulsionFilter(key)
      }
      if (this.subType === 'driveline' && ['tcu', 'tm', 'frontMotorRatio', 'frontFd', 'rearMotorRatio', 'rearFd'].includes(key)) {
        return this.drivelineFilter(key)
      }
      if (this.subType === 'ecu' && ['ecuMaxAcc', 'ecuMinAcc', 'softVcu', 'softAbs', 'softEms', 'softHcu'].includes(key)) {
        return this.ecuFilter(key)
      }
      if (['steerMapped', 'dynamicSteer'].includes(key)) {
        return this.steerFilter(key)
      }
      return true
    },
    steerFilter (key) {
      if (this.steerType === 'Mapped_Steer' && key === 'steerMapped') {
        return true
      }
      if (this.steerType === 'Dynamic_Steer' && key === 'dynamicSteer') {
        return true
      }
    },
    propulsionFilter (key) {
      const { propulsionType, drivetrainType } = this.carType
      if (propulsionType === 'Electric_Drive') {
        if (key === 'engineParameter') {
          return false
        }
        if (drivetrainType === 'FrontWheel_Drive') {
          return ['frontMotorParameter', 'battParamter'].includes(key)
        } else if (drivetrainType === 'RearWheel_Drive') {
          return ['rearMotorParameter', 'battParamter'].includes(key)
        } else if (drivetrainType === 'FourWheel_Drive') {
          return ['frontMotorParameter', 'rearMotorParameter', 'battParamter'].includes(key)
        }
      } else if (propulsionType === 'ICE_Engine') {
        return !['frontMotorParameter', 'rearMotorParameter', 'battParamter'].includes(key)
      } else if (propulsionType.slice(0, 6) === 'Hybrid') {
        if (propulsionType === 'Hybrid_Gen_P2P4' || propulsionType === 'Hybrid_Gen_P3P4') {
          return ['engineParameter', 'frontMotorParameter', 'rearMotorParameter', 'battParamter', 'hybridParameter'].includes(key)
        }
        return ['engineParameter', 'frontMotorParameter', 'battParamter', 'hybridParameter'].includes(key)
      }
      return true
    },
    drivelineFilter (key) {
      const { propulsionType, drivetrainType } = this.carType
      if (propulsionType === 'Electric_Drive') {
        if (['tcu', 'tm'].includes(key)) {
          return false
        }
        if (drivetrainType === 'FrontWheel_Drive') {
          return ['frontMotorRatio', 'frontFd'].includes(key)
        } else if (drivetrainType === 'RearWheel_Drive') {
          return ['rearMotorRatio', 'rearFd'].includes(key)
        } else if (drivetrainType === 'FourWheel_Drive') {
          return ['frontMotorRatio', 'rearMotorRatio', 'frontFd', 'rearFd'].includes(key)
        }
      } else if (propulsionType === 'ICE_Engine') {
        if (['frontMotorRatio', 'RearMotorRatio'].includes(key)) {
          return false
        }
        if (drivetrainType === 'FrontWheel_Drive') {
          return ['tcu', 'tm', 'frontFd'].includes(key)
        } else if (drivetrainType === 'RearWheel_Drive') {
          return ['tcu', 'tm', 'rearFd'].includes(key)
        } else if (drivetrainType === 'FourWheel_Drive') {
          return ['tcu', 'tm', 'frontFd', 'rearFd'].includes(key)
        }
      } else if (propulsionType.slice(0, 6) === 'Hybrid') {
        if (propulsionType === 'Hybrid_Gen_P2P4' || propulsionType === 'Hybrid_Gen_P3P4') {
          return ['tcu', 'tm', 'frontFd', 'rearFd', 'frontMotorRatio', 'rearMotorRatio'].includes(key)
        }
        return ['tcu', 'tm', 'frontFd', 'frontMotorRatio'].includes(key)
      }
      return true
    },
    ecuFilter (key) {
      const { propulsionType } = this.carType
      if (propulsionType === 'Electric_Drive') {
        return ['ecuMaxAcc', 'softVcu', 'softAbs'].includes(key)
      }
      if (propulsionType.slice(0, 6) === 'Hybrid') {
        return ['ecuMaxAcc', 'ecuMinAcc', 'softHcu', 'softAbs'].includes(key)
      }
      return ['ecuMaxAcc', 'softEms', 'softAbs'].includes(key)
    },
    isShowOption (key, op) {
      const { propulsionType } = this.carType
      if (key === 'drivetrainType') {
        if (propulsionType === 'Hybrid_Gen_P2P4' || propulsionType === 'Hybrid_Gen_P3P4') {
          return op === 'FourWheel_Drive'
        }
        if (propulsionType === 'Hybrid_Gen_P2' || propulsionType === 'Hybrid_Gen_P3') {
          return op === 'FrontWheel_Drive'
        }
      }
      return true
    },
    handleSelectChange (key, value) {
      if (key === 'propulsionType') {
        if (value === 'Hybrid_Gen_P2P4' || value === 'Hybrid_Gen_P3P4') {
          this.paramsObj.drivetrainType = 'FourWheel_Drive'
        }
        if (value === 'Hybrid_Gen_P2' || value === 'Hybrid_Gen_P3') {
          this.paramsObj.drivetrainType = 'FrontWheel_Drive'
        }
      }
      this.notifyDirty()
    },
  },
}
</script>

<style scoped lang="less">
.generate-params {
  .padding-horizontal-20 {
    padding-left: 20px;
    padding-right: 20px;
  }

  :deep(.el-table .el-button) {
    padding: 0 0;
    background: transparent;
  }

  .el-form-item {
    &.label-top {
      flex-wrap: wrap;

      :deep(.el-form-item__content) {
        flex: auto;
      }
    }

    :deep(.el-form-item__label) {
      padding-right: 8px;
      display: inline-block;

      .i-circle {
        width: 1em;
        height: 1em;
        cursor: pointer;
      }
    }

    :deep(.el-form-item__content) {
      .el-select {
        width: 100%;
      }

      .input-number {
        width: 100%;

        .el-input {
          width: 100%;
          padding: 0 5px;
        }
      }
    }
  }

  .el-collapse-item {
    :deep(.el-collapse-item__content) {
      padding: 10px 0;

      .el-collapse-item {
        .el-collapse-item__header {
          background-color: transparent;

          .el-collapse-item__arrow {
            order: -1;
            margin: 0 8px 0 0;
            vertical-align: middle;
          }
        }

        .generate-params {
          padding: 0 22px;
        }
      }
    }
  }
}
</style>
