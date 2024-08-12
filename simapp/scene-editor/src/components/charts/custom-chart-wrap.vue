<template>
  <div class="custom-chart-wrap">
    <div class="section header">
      <span>{{ $t('player.realTimeData') }}</span>
    </div>
    <div class="section chosen-pb-config-list">
      <el-tag
        v-for="(pb, index) in chosen"
        :key="pb.field"
        size="small"
        disable-transitions
        closable
        @close="handleDeleteTag(pb)"
      >
        <span class="color" :style="{ backgroundColor: getColor(index) }" />
        <span class="name">{{ pb.field }}</span>
        <span class="unit">{{ pb.chart.unit !== 'N/A' ? `(${pb.chart.unit})` : '' }}</span>
      </el-tag>
      <el-tag size="small" class="btn-add" @click="handleChoose">
        <span>+</span>
      </el-tag>
    </div>
    <div class="section chart-list-wrap">
      <CustomChart
        v-for="(pb, index) in chosen"
        :key="pb.field"
        :index="index + 1"
        :color="getColor(index)"
        :config="pb"
        :data="getChartData(pb)"
      />
    </div>
    <div class="section footer">
      <el-select v-model="egoId" style="margin-right:auto" @change="handleEgoChange">
        <template v-for="p in plannerList" :key="p.value">
          <el-option :value="p.value" :label="p.label" />
        </template>
      </el-select>
      <el-button link icon="download" :disabled="exporting" :title="$t('operation.export')" @click="handleExport" />
    </div>
    <PbConfigChooseDialog ref="PbConfigChooseDialog" />
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapState } from 'vuex'
import PbConfigChooseDialog from '../popups/pb-config-choose-dialog.vue'
import CustomChart from './custom-chart.vue'
import eventBus from '@/event-bus'
import { getPbMessageList } from '@/stored/pb-message'
import dict from '@/common/dict'
import filters from '@/filters'

const { electron, electron: { dialog } } = window
const { getProtoName } = dict

export default {
  name: 'CustomChartWrap',
  components: {
    CustomChart,
    PbConfigChooseDialog,
  },
  data () {
    return {
      colors: [],
      firstLoad: true,
      datas: [],
      exporting: false,
      egoId: '',
    }
  },
  computed: {
    ...mapState('pb-config', ['pbConfigList']),
    ...mapState('pb-config', { chosen: 'chosenPbConfigList' }),
    ...mapState('scenario', ['followingPlannerId']),
    ...mapGetters('scenario', ['isLogSim', 'isPlaying', 'tree', 'selectedObject']),
    ...mapGetters('log2world', ['isLog2World']),
    plannerList () {
      let list = _.get(this, 'tree.planner') || []
      list = list.map(p => ({ value: p.id, label: filters.$itemName(p) }))
      return list
    },
  },
  watch: {
    pbConfigList () {
      this.initPbConfigChosen()
      this.renderChart(getPbMessageList())
    },
    isLog2World () {
      this.resetChosenPbConfigList()
      this.$nextTick(() => {
        this.initPbConfigChosen()
        this.renderChart(getPbMessageList())
      })
    },
    plannerList () {
      this.egoId = this.getCurrentPlanner()
    },
  },
  created () {
    eventBus.$on('pb-message', this.renderChart)
    this.refreshPbConfigList()
    this.egoId = this.getCurrentPlanner()
  },
  beforeUnmount () {
    eventBus.$off('pb-message', this.renderChart)
  },
  methods: {
    ...mapActions('pb-config', [
      'refreshPbConfigList',
      'resetChosenPbConfigList',
      'updateChosenPbConfigList',
    ]),
    getCurrentPlanner () {
      let current = this.plannerList.find(e => e.value === this.egoId)
      if (!current) current = this.plannerList[0] || {}
      return current.value || ''
    },
    getColor (index) {
      const colors = ['#3EAEF6', '#00FAFF', '#FF669F']
      const color = colors[index % colors.length]
      return color
    },
    getChartData (pbConfig) {
      const index = this.chosen.findIndex(e => e === pbConfig)
      return this.datas[index]
    },
    handleDeleteTag (pbConfig) {
      const chosen = this.chosen.filter(e => e.field !== pbConfig.field)
      this.updateChosenPbConfigList(chosen)
    },
    handleChoose () {
      const $dialog = this.$refs.PbConfigChooseDialog
      if (!$dialog) return
      $dialog.openDialog({
        chosen: this.chosen,
        success: ({ confirm, data }) => {
          if (!confirm) return
          this.updateChosenPbConfigList(data || [])
          this.renderChart(getPbMessageList())
        },
      })
    },
    async handleExport () {
      try {
        this.exporting = true
        const { filePath } = await dialog.showSaveDialog({
          title: this.$t('operation.export'),
          defaultPath: `实时数据.csv`,
        })
        if (!filePath) return
        const result = []
        this.datas.forEach((item, index) => {
          const pbConfig = this.chosen[index]
          const { chart } = pbConfig || {}
          const { title, unit, xAxisName, yAxisName } = chart || {}
          const { xAxis = [], yAxis = [] } = item || {}
          result.push(`${index}.${title},${Array.from({ length: xAxis.length - 1 }).fill('').join(',')}\r\n`)
          result.push(`${xAxisName}(s),${xAxis.join(',')}\r\n`)
          result.push(`${yAxisName}${unit !== 'N/A' ? `(${unit})` : ''},${yAxis.join(',')}\r\n`)
          result.push(`\r\n`)
        })
        const txt = result.join('')
        electron.editor.writeFileSync(filePath, txt)
        this.$message.success(this.$t('tips.exportSuccess'))
      } catch (err) {
        console.log(err)
        this.$message.error(this.$t('tips.exportFail'))
      } finally {
        this.exporting = false
      }
    },
    renderChart (payload) {
      let data
      if (Array.isArray(payload)) {
        data = payload
      } else {
        data = payload?.detail || []
      }
      const chartDatas = []

      // [{ topic: 'Ego_002/GRADING', content }] 转为 [{ Grading: content }]
      const getTopicMap = messages => messages.reduce((re, { topic: pbKeyframeTopic = '', content = {} }) => {
        const tmp = pbKeyframeTopic.split('/') // socket 中可能是 GRADING 或 Ego_002/GRADING
        const ego = tmp.length > 1 ? tmp.slice(0, -1).join('/') : ''
        const egoId = +ego.slice(3)
        const dataTopic = tmp.pop() // socket 中的 topic
        const pbTopic = getProtoName(dataTopic) // 选择项中的 topic
        if (egoId && egoId !== this.egoId) return re // 多主车时仅录入对应主车的数据
        return { ...re, [pbTopic]: content }
      }, {})

      // 从 { Grading: content } 中读取 _.get(data, 'Grading.speed.speed') 的值为 y, 时间戳为 x，拼凑图表数据
      data.forEach((keyframe) => {
        const { timestamp = -1, messages = [] } = keyframe
        const pbTopicMap = getTopicMap(messages)

        this.chosen.forEach((item, index) => {
          if (!chartDatas[index]) chartDatas[index] = { xAxis: [], yAxis: [] }
          const chartKeyFrameData = chartDatas[index]

          const content = pbTopicMap[item.topic] // 对应 socket 数据

          if (!content) {
            console.warn(`该数据帧中未找到 ${item.topic} 相关数据`)
            return
          }

          const x = timestamp / 1000
          let y
          if (item.paths[item.paths.length - 1] === '_size') {
            const paths = item.paths.slice(1, -1).join('.')
            if (paths.length) y = _.size(_.get(content, paths))
          } else {
            const paths = item.paths.slice(1).join('.')
            y = _.get(content, paths)
          }

          if (y !== undefined) {
            chartKeyFrameData.xAxis.push(x)
            chartKeyFrameData.yAxis.push(y)
          }
        })
      })

      this.datas = chartDatas
    },
    initPbConfigChosen () {
      const operated = localStorage.getItem('pb-config-operated')
      let cacheChosen = localStorage.getItem('pb-config-chosen') // 过往已选

      let chosen = this.chosen

      if (!operated || operated === 'false') {
        let defaultChosen = [] // 默认已选
        if (this.isLog2World) {
          defaultChosen = this.pbConfigList.filter(e => ['speed', 'acceleration', 'speed_logsim', 'acceleration_logsim'].includes(e.field))
        } else if (this.isLogSim) {
          defaultChosen = this.pbConfigList.filter(e => ['speed_logsim', 'acceleration_logsim'].includes(e.field))
        } else {
          defaultChosen = this.pbConfigList.filter(e => ['speed', 'acceleration'].includes(e.field))
        }
        defaultChosen = _.uniqBy(defaultChosen, 'field')
        chosen = defaultChosen
      }

      if (cacheChosen && cacheChosen[0] === '[') {
        cacheChosen = JSON.parse(cacheChosen)
        if (cacheChosen.length) chosen = cacheChosen
      }

      this.updateChosenPbConfigList(chosen)
    },
    handleEgoChange () {
      this.renderChart(getPbMessageList())
    },
  },
}
</script>

<style lang="less" scoped>
@import "@/assets/less/mixins";

.custom-chart-wrap {
  display: flex;
  flex-direction: column;
  color: @global-font-color;
  width: 452px;
  font-size: 12px;
  padding: 8px 10px;
  box-sizing: border-box;

  & > :not(.chart-list-wrap) {
    flex-shrink: 0;
  }

  & > .chart-list-wrap {
    flex-grow: 1;
    overflow-y: visible;
    overflow-x: hidden;
    margin: 0 -10px;
    max-height: 50vh;
  }

  & > .header {
    font-weight: bold;
    padding: 0 0 8px;
  }

  .chosen-pb-config-list {
    display: flex;
    flex-wrap: wrap;

    :deep(.el-tag) {
      color: @title-font-color;
      background-color: @title-bd;
      border: none;
      font-size: 10px;
      padding: 0 8px;
      margin-bottom: 8px;
      display: flex;
      align-items: center;

      .el-tag__content {
        display: flex;
        align-items: center;
      }

      &:not(:last-child) {
        margin-right: 8px;
      }

      .el-icon-close {
        top: 0;
      }

      &.btn-add {
        cursor: pointer;
      }
    }

    .color {
      display: block;
      width: 8px;
      height: 8px;
      border-radius: 50%;
      margin-right: 4px;
      background: white;
    }
  }

  & > .footer {
    display: flex;
  }

  :deep(.el-button.is-disabled.el-button--text) {
    border: none;

    &:hover {
      background: none;
    }
  }
}
</style>
