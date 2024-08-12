<template>
  <el-form
    label-position="left"
    class="dynamics-params"
    @submit.prevent
  >
    <div class="dynamics-actions">
      <el-tooltip content="导入" placement="top">
        <ImportSvg @click="importDynamicsData" />
      </el-tooltip>
      <el-tooltip content="导出" placement="top">
        <ExportSvg @click="exportDynamicsData" />
      </el-tooltip>
    </div>
    <div class="form-title">
      动力学{{ title[subType] }}
    </div>
    <GenerateParams ref="geneparams" v-model="paramsObj" :car-type="carType" :sub-type="subType" />
  </el-form>
</template>

<script>
import { mapMutations, mapState } from 'vuex'
import { Draft07 } from 'json-schema-library'
import GenerateParams from './generate-params.vue'
import txcarJsonSchema from './txcat-json-schema.json'
import ImportSvg from '@/assets/images/import.svg'
import ExportSvg from '@/assets/images/export.svg'

export default {
  name: 'DynamicsParams',
  components: {
    ImportSvg,
    ExportSvg,
    GenerateParams,
  },
  props: {
    subType: {
      type: String,
      required: true,
    },
  },
  data () {
    return {
      title: {
        propulsion: '动力模块',
        driveline: '传动模块',
        steerSystem: '转向模块',
        body: '车身模块',
        susp: '悬架模块',
        wheelTire: '轮胎与制动',
        accessory: '附件',
        sprungMass: '簧上质量',
        ecu: 'SoftECU',
        carType: '车型配置',
      },
      paramsObj: {},
    }
  },
  computed: {
    ...mapState('planners', ['currentDynamicData']),
    carType () {
      return this.currentDynamicData.carType || {}
    },
  },
  watch: {
    subType () {
      this.paramsObj = this.currentDynamicData[this.subType] || {}
    },
    paramsObj: {
      handler () {
        this.$forceUpdate()
      },
      deep: true,
    },
  },
  created () {
    this.paramsObj = this.currentDynamicData[this.subType] || {}
  },
  methods: {
    ...mapMutations('planners', ['setCurrentDynamicData']),
    importDynamicsData () {
      this.$confirmBox('导入后将批量替换当前界面填写的参数，确认导入吗？').then(() => {
        const input = document.createElement('INPUT')
        input.type = 'file'
        input.accept = '.json'
        input.onchange = () => {
          const reader = new FileReader()
          reader.onload = (e) => {
            const dynamicData = JSON.parse(e.target.result)
            const jsonSchema = new Draft07(txcarJsonSchema)
            const errors = jsonSchema.validate(dynamicData)
            if (errors.length) {
              let errorMessage = ''
              errors.forEach((error) => {
                errorMessage += `${error.message}<br />`
              })
              this.$message({
                showClose: true,
                dangerouslyUseHTMLString: true,
                message: errorMessage,
                type: 'error',
              })
            } else {
              this.setCurrentDynamicData(dynamicData)
              this.paramsObj = this.currentDynamicData[this.subType] || {}
              this.$message({
                showClose: true,
                message: this.$t('tips.importSuccess'),
                type: 'success',
              })
            }
            input.onchange = null
            reader.onload = null
          }
          reader.readAsText(input.files[0])
        }
        input.click()
      }).catch(() => {
      })
    },
    exportDynamicsData () {
      const blob = new Blob([JSON.stringify(this.currentDynamicData)], { type: 'text/plain;chartset=utf-8' })
      const href = window.URL.createObjectURL(blob)
      const a = document.createElement('A')
      a.href = href
      a.download = 'txcar.json'
      a.click()
      window.URL.revokeObjectURL(href)
    },
  },
}
</script>

<style scoped lang="less">
.dynamics-params {
  .form-title {
    margin-bottom: 10px;
    font-size: 12px;
    color: #dcdcdc;
    text-align: center;
  }

  .dynamics-actions {
    position: absolute;
    top: 0;
    right: 10px;
    height: 28px;
    line-height: 28px;
    display: flex;
    margin-left: auto;
    align-items: center;

    span {
      margin-left: 5px;
      cursor: pointer;
    }

    svg {
      width: 1em;
      height: 1em;
      margin-left: 5px;
      cursor: pointer;
      color: #d2d2d2;
    }
  }
}
</style>
