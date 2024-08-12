import _ from 'lodash'
import { mapState } from 'vuex'

export default {
  computed: {
    ...mapState('kpi', [
      'kpiLabelsList',
    ]),
    // 指标分类筛选列表
    kpiCategoryOptions () {
      return [
        { id: -1, name: 'indicator.AllClass' },
        ...this.kpiLabelsList.map(({ labelKey, labelDescription }) => ({ id: labelKey, name: labelDescription })),
      ]
    },
  },
  methods: {
    // 当前指标分类列表
    getCategory (row) {
      const { category, parameters } = row
      const categoryList = this.kpiLabelsList
      const { labels } = parameters || {}

      let labelList = []
      if (labels && Array.isArray(labels)) {
        // 旧数据
        labelList = labels
        labelList = _.uniq(labelList)
      } else if (category && typeof category === 'object') {
        // 新数据
        const { labels } = category
        labelList = labels
      }

      labelList = labelList.map((label) => {
        const tmp = categoryList.find(item => item.labelKey === label)
        return tmp ? this.$t(tmp.labelDescription) : ''
      }).filter(e => !!e)
      return labelList
    },
    // 当前指标分类列表的中文
    getCategoryTxt (row) {
      // 旧版数据
      if (row.categoryDesc) return row.categoryDesc
      // 新版数据
      return this.getCategory(row).join(',')
    },
  },
}
