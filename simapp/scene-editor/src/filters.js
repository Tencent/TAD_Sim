import _ from 'lodash'
import i18n from '@/locales'

const filters = {
  // kpi通过与否
  $kpiPass (value) {
    if (value === undefined) {
      return ''
    }
    return value ? i18n.t('indicator.PASS') : i18n.t('indicator.FAIL')
  },
  // kpi单位
  $kpiUnit (value, object, name) {
    const unit = object[name]
    let val = value
    if (unit === 'boolean') {
      return val ? i18n.t('yes') : i18n.t('no')
    } else if (unit === 'm') {
      if (_.isNumber(val)) {
        val = val.toFixed(3)
      }
    }
    return `${val} ${unit}`
  },
  // kpi别名
  $kpiAlias (item) {
    if (i18n.locale === 'zh-CN') {
      return item?.parameters?.indicatorName || item.name
    }
    return item.name
  },
  // 场景元素名称
  $itemName (item) {
    const idx = `${item.id}`.padStart(3, '0')
    const type = item.type === 'planner' ? 'Ego' : item.type
    return `${type}_${idx}`
  },
  // 被跟随对象名称
  $followName (item) {
    let type
    switch (item.type) {
      case 'planner':
        type = 'ego'
        break
      case 'car':
        type = 'v'
        break
    }
    return `${type}_${item.id}`
  },
}

export default filters
