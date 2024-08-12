<template>
  <div class="kpi-score-setting">
    <div class="result-chart">
      <SimChart :option="option" :width="width" :height="height" />
    </div>
    <div class="setting-list">
      <el-form ref="form" :model="formModel">
        <el-table :data="list" row-key="$index" size="small">
          <el-table-column prop="u" :label="$t('kpi.scoreX')" width="190">
            <template #default="{ row, $index }">
              <el-form-item :prop="`x${$index}`" :rules="[{ index: $index, validator: xValidator, trigger: 'blur' }]">
                <InputNumber
                  v-model="row.x"
                  size="small"
                  :precision="0"
                  :disabled="disabled"
                  :min="0"
                />
              </el-form-item>
            </template>
          </el-table-column>
          <el-table-column prop="y" :label="$t('kpi.scoreY')" width="150">
            <template #default="{ row, $index }">
              <el-form-item :prop="`y${$index}`" :rules="[{ index: $index, validator: yValidator, trigger: 'blur' }]">
                <InputNumber
                  v-model="row.y"
                  size="small"
                  :disabled="disabled"
                  :min="0"
                  :max="100"
                />
              </el-form-item>
            </template>
          </el-table-column>
          <el-table-column prop="operation" :label="$t('operation.operation')" width="90">
            <template #default="{ $index }">
              <el-button
                link
                :disabled="disabled || $index === 0"
                @click="handleDeleteLine($index)"
              >
                {{ $t('operation.delete') }}
              </el-button>
              <el-button
                link
                :disabled="disabled"
                @click="handleAddLine($index)"
              >
                {{ $t('operation.add') }}
              </el-button>
            </template>
          </el-table-column>
        </el-table>
      </el-form>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import SimChart from '@/components/sim-chart.vue'
import InputNumber from '@/components/input-number.vue'

const defaultValue = {
  u: [],
  y: [],
  uInfo: {},
  yInfo: {},
}

const xAxisSample = {
  type: 'value',
  boundaryGap: false,
  name: 'X',
  splitLine: {
    show: false,
  },
  axisLine: {
    onZero: true,
    symbol: ['none', 'arrow'],
    symbolSize: [4, 4],
    symbolOffset: [0, 4],
    lineStyle: {
      width: 2,
      color: '#97AAC2',
    },
  },
}

const yAxisSample = {
  type: 'value',
  boundaryGap: false,
  min: 0,
  max: 100,
  interval: 20,
  name: 'Y',
  splitLine: {
    lineStyle: {
      color: '#526278',
      type: 'dashed',
    },
  },
  axisLine: {
    onZero: true,
    symbol: ['none', 'arrow'],
    symbolSize: [4, 4],
    symbolOffset: [0, 4],
    lineStyle: {
      width: 2,
      color: '#97AAC2',
    },
  },
}

const tooltipSample = {
  trigger: 'axis',
  padding: [2, 8],
  borderColor: '#4F6787',
  backgroundColor: 'rgba(79, 103, 135, 0.75)',
  textStyle: {
    color: '#97AAC2',
  },
  formatter (data) {
    const value = data[0]?.value || []
    const [x, y] = value
    if (x === undefined || y === undefined) return undefined
    return `X轴: <span style="color:white">${x}</span><br />Y轴: <span style="color:white">${y}</span>`
  },
}

const chartOption = {
  grid: {
    top: 30,
    bottom: 30,
  },
  xAxis: xAxisSample,
  yAxis: yAxisSample,
  tooltip: tooltipSample,
  series: [
    {
      data: [],
      type: 'line',
      lineStyle: {
        color: '#00FAFF',
      },
      itemStyle: {
        color: '#00FAFF',
        borderColor: '#00FAFF',
      },
    },
  ],
}

const isEmptyValue = val => val === null || val === undefined || val === ''

export default {
  components: {
    SimChart,
    InputNumber,
  },
  props: {
    value: {
      type: Object,
      default: () => {},
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      width: 360,
      height: 230,
      list: [],
    }
  },
  computed: {
    option () {
      const { list } = this
      let seriesData = []
      let tooltip
      if (!list.length) {
        seriesData = [[10, undefined]]
        tooltip = { show: false }
      } else {
        // 排除掉未填写的数据行
        list.forEach(({ x, y }) => {
          if (isEmptyValue(x) || isEmptyValue(y)) return false
          seriesData.push([x, y])
        })
        // 若出现后x值比前x值大，会表现为向前的折线，需要去除
        seriesData = seriesData.reduce((res, e, i) => {
          const prev = seriesData[i - 1]
          if (!prev) return [e]
          if (prev[0] > e[0]) return res
          return [...res, e]
        }, [])
        // 当x值不够多时，补充x值
        const maxX = _.maxBy(seriesData, 'x')?.x || 0
        if (maxX < 10) {
          seriesData.push([10, undefined])
        }
        tooltip = { ...tooltipSample }
      }

      return _.merge({}, chartOption, {
        xAxis: xAxisSample,
        yAxis: yAxisSample,
        tooltip,
        series: [
          { data: seriesData },
        ],
      })
    },
    formModel () {
      const formModel = {}
      this.list.forEach(({ x, y }, index) => {
        formModel[`x${index}`] = x
        formModel[`y${index}`] = y
      })
      return formModel
    },
  },
  watch: {
    value (val) {
      this.initListData(val)
    },
  },
  mounted () {
    this.lastErrorIndex = undefined
    this.isSubmitValidate = false
    this.initListData(this.value)
  },
  methods: {
    initListData (value) {
      const { u: uData, y: yData } = value || defaultValue
      let list = []
      if (!uData || !uData.length) {
        list = [{ x: 0, y: null }]
      }
      uData.forEach((x, i) => {
        const y = yData[i]
        list.push({ x, y })
      })
      this.list = list
    },
    // X轴数值校验
    xValidator (rule, value, callback) {
      if (isEmptyValue(value)) {
        const error = new Error(this.$t('tips.pleaseInput2', { text: '' }))
        callback(error)
        return
      }
      // 该行x值不合规，则报合规错误，清除上次报的合规类报错
      const prevItem = this.list[rule.index - 1]
      const nextItem = this.list[rule.index + 1]
      if ((prevItem && prevItem.x >= +value) || (nextItem && nextItem.x <= +value)) {
        if (this.lastErrorIndex !== undefined && this.lastErrorIndex !== rule.index) {
          const $field = this.$refs.form.fields.find(e => e.prop === `x${this.lastErrorIndex}`)
          $field && $field.resetField(`x${this.lastErrorIndex}`)
        }
        const error = new Error('参数值填写错误')
        callback(error)
        this.lastErrorIndex = rule.index
        return
      }
      // 该行x值合规，且为输入场景(非保存场景)时，清除上次报的合规类报错
      if (!this.isSubmitValidate) {
        if (this.lastErrorIndex !== undefined && this.lastErrorIndex !== rule.index) {
          const $field = this.$refs.form.fields.find(e => e.prop === `x${this.lastErrorIndex}`)
          $field && $field.resetField(`x${this.lastErrorIndex}`)
        }
      }
      callback()
    },
    // Y轴数值校验
    yValidator (rule, value, callback) {
      if (isEmptyValue(value)) {
        const error = new Error(this.$t('tips.pleaseInput2', { text: '' }))
        callback(error)
        return
      }
      callback()
    },
    // 表单校验（外放函数，勿删）
    validate (callback) {
      this.lastErrorIndex = undefined
      this.isSubmitValidate = true
      this.$refs.form.validate(async (valid) => {
        this.isSubmitValidate = false
        if (!valid) return

        // 有效行数必须大于2
        const okList = this.list.filter(({ x, y }) => {
          if (isEmptyValue(x) || isEmptyValue(y)) return false
          return true
        })
        if (okList.length < 2) {
          const title = '配置数量不符合提交要求，请完善配置'
          const desc = '请确认至少配置2行'
          this.$errorBox(desc, title)
          return
        }

        // 分数必须在 [0, 100] 之间
        const maxY = _.maxBy(this.list, 'y')?.y
        const minY = _.minBy(this.list, 'y')?.y
        if (minY !== 0 || maxY !== 100) {
          const title = '分数配置不完整，请重新编辑'
          const desc = '请确认分数最大值为100，最小值为0，都已覆盖'
          this.$errorBox(desc, title)
          return
        }

        callback && callback(valid)
      })
    },
    // 获取结果（外放函数，勿删）
    getResult () {
      let newValue = _.cloneDeep(this.value || defaultValue)
      const newU = []
      const newY = []
      this.list.forEach(({ x, y }) => {
        if (isEmptyValue(x) || isEmptyValue(y)) return false
        newU.push(x)
        newY.push(y)
      })
      newValue = { ...newValue, u: newU, y: newY }
      return newValue
    },
    // 新增一行
    handleAddLine (index) {
      this.$refs.form.resetFields()
      this.lastErrorIndex = undefined

      this.list.splice(index + 1, 0, { x: 0, y: null })
    },
    // 删除一行
    handleDeleteLine (index) {
      this.$refs.form.resetFields()
      this.lastErrorIndex = undefined

      this.list.splice(index, 1)
    },
  },
}
</script>

<style lang="less" scoped>
.kpi-score-setting {
  display: flex;
  align-items: flex-start;

  & > .result-chart {
    flex: 0 1 360px;
    margin-right: 20px;
  }

  & > .setting-list {
    flex: 1 0 auto;
    overflow: hidden;

    :deep(.el-button) {
      padding: 0.4em 0.1em;
    }

    :deep(.el-button.is-disabled),
    :deep(.el-button.is-disabled:focus),
    :deep(.el-button.is-disabled:hover) {
      border: none;
      background-color: transparent;
    }

    :deep(.el-form-item) {
      margin-bottom: 0;
    }

    :deep(.el-table__cell) {
      padding: 0 0;
    }

    :deep(.el-form-item__content) {
      position: relative;

      .input-number {
        width: 80px;
      }

      .el-form-item__error {
        top: 6px;
        left: 86px;
        word-break: keep-all;
      }
    }

    :deep(.el-input--mini) {
      .el-input__inner {
        height: 24px;
        line-height: 24px;
      }
    }
  }
}
</style>
