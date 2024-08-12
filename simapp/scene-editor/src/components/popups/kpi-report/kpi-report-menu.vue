<template>
  <div v-if="egoList.length" class="kpi-report-menu-wrap">
    <el-menu
      ref="menu"
      :default-openeds="defaultOpened"
      class="kpi-report-menu"
      :class="{ useAnimation }"
    >
      <LoopMenu
        v-for="nav of navList"
        :key="nav.id"
        :item="nav"
        @click-item="clickItem"
      />
    </el-menu>
  </div>
</template>

<script>
import { get, has, throttle } from 'lodash-es'
import LoopMenu from './loop-menu.vue'
import eventBus from '@/event-bus'
import { defaultEgoId } from '@/common/utils'

export default {
  components: {
    LoopMenu,
  },
  props: {
    first: {
      type: Object,
      default: () => {},
    },
    second: {
      type: Object,
      default: null,
    },
  },
  data () {
    return {
      activeNav: 0,
      opened: [],
      useAnimation: true,
    }
  },
  computed: {
    egoList () {
      const version = has(this.first, 'meta') ? '1.0' : '2.0'
      let egoList = version === '1.0' ? [{ egoId: defaultEgoId }] : get(this.first, 'report.egoCases', [])
      // 主车列表按 001 002 进行排序
      const getEgoIdNumber = egoId => +egoId.slice(4)
      egoList = egoList.sort((a, b) => getEgoIdNumber(a.egoId) - getEgoIdNumber(b.egoId))
      return egoList
    },
    navList () {
      let moreNavList = []
      const isCompare = this.first && this.second
      if (!isCompare) {
        moreNavList = this.egoList.map(({ egoId }) => ({
          id: egoId || 'Ego_001',
          title: egoId || 'Ego_001',
          children: [
            { id: `${egoId}_module`, title: this.$t('module.listForReport') },
            { id: `${egoId}_kpi_table`, title: this.$t('indicator.list') },
            { id: `${egoId}_kpi_detail`, title: this.$t('indicator.detail') },
            { id: `${egoId}_meta_data`, title: this.$t('indicator.metadata') },
          ],
        }))
      } else {
        moreNavList = [
          { id: '0_module', title: this.$t('module.listForReport') },
          { id: '0_kpi_table', title: this.$t('indicator.list') },
          { id: '0_kpi_detail', title: this.$t('indicator.detail') },
          { id: '0_meta_data', title: this.$t('indicator.metadata') },
        ]
      }
      return [
        { id: 'kpi-report-detail-basic', title: this.$t('scenario.basicInformation') }, // 基础信息
        { id: 'kpi-report-global-module-list', title: this.$t('module.listGlobalForReport') }, // 全局模块列表
        { id: 'kpi-report-ego-data-list', title: this.$t('indicator.kpiReportEgoDetailList'), children: moreNavList }, // 主车评测信息
      ]
    },
    defaultOpened () {
      return [
        'kpi-report-ego-data-list',
        ...this.egoList.map(e => e.egoId),
      ]
    },
  },
  watch: {
    defaultOpened () {
      this.opened = this.defaultOpened.slice(0)
    },
  },
  mounted () {
    this.opened = this.defaultOpened.slice(0)
    window.addEventListener('scroll', this.handleScroll)
    eventBus.$on('kpi-report-export-click', this.handleMainExport)
    eventBus.$on('kpi-report-export-finish', this.handleMainExportFinish)
  },
  unmounted () {
    window.removeEventListener('scroll', this.handleScroll)
    eventBus.$off('kpi-report-export-click', this.handleMainExport)
    eventBus.$off('kpi-report-export-finish', this.handleMainExportFinish)
  },
  methods: {
    // 点击菜单项
    clickItem (data) {
      // 展开页面中的手风琴
      eventBus.$emit('kpi-report-menu-click', data)
      // 滑动到对应的位置
      const { id } = data || {}
      setTimeout(() => {
        this.$nextTick(() => {
          const dom = document.getElementById(id)
          if (!dom) return
          dom.scrollIntoView({ behavior: 'smooth', block: 'start' })
        })
      }, 500)
      // 记录展开的菜单
      if (data.children && data.children.length) {
        const index = this.opened.findIndex(id => id === data.id)
        if (index > -1) this.opened.splice(index, 1)
        else this.opened.push(data.id)
      }
    },
    handleScroll: throttle(() => {
      // TODO: 滚动触发菜单项高亮
      // const scrollTop = window.pageYOffset || document.documentElement.scrollTop || document.body.scrollTop || 0
      // console.log('===', scrollTop)
    }, 500),
    // 外部点击导出时，所有菜单项展开
    handleMainExport () {
      this.useAnimation = false
      this.defaultOpened.forEach((id) => {
        this.$refs.menu.open(id)
      })
    },
    // 外部导出完成时，恢复菜单项之前展开状态
    // 需先排序，从内向外依次触发，不然外层会关闭后又被打开
    handleMainExportFinish () {
      let sorted = this.defaultOpened.slice(0)
      sorted = sorted.sort(a => (a === 'kpi-report-ego-data-list' ? 1 : -1))
      sorted.forEach((id) => {
        const open = this.opened.includes(id)
        if (open) this.$refs.menu.open(id)
        else this.$refs.menu.close(id)
      })
      setTimeout(() => {
        this.useAnimation = true
      }, 500)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/var";
@import "@/assets/less/mixins";

.kpi-report-menu {
  --el-menu-item-font-size: inherit;
  --el-menu-text-color: @global-font-color;
  --el-menu-bg-color: transparent;
  --el-menu-item-bg-color: #212121;
  --el-menu-active-color: @global-primary-color;
  --el-menu-hover-bg-color: @hover-bg;
  --el-menu-hover-text-color: @global-primary-color;
  --el-menu-item-height: 32px;
  --el-menu-sub-item-height: 32px;
  --el-menu-base-level-padding: 16px;
  --el-menu-icon-width: 14px;

  :deep(&.el-menu) {
    border-right: none;

    &:not(.useAnimation) {
      .el-menu {
        transition: none !important;
      }
    }

    .el-sub-menu__title,
    .el-menu-item {
      margin-bottom: 1px;
      background-color: var(--el-menu-item-bg-color);

      & > span {
        .text-overflow();
      }

      &:hover {
        color: var(--el-menu-hover-text-color);
        background-color: var(--el-menu-hover-bg-color);
      }
    }

    // .el-sub-menu > .el-menu {
    //   --el-menu-bg-color: @global-bd-color;
    //   --el-menu-item-bg-color: @global-bg-color;
    // }
  }
}
</style>
