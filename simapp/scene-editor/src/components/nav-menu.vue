<template>
  <ul ref="wrap" class="nav-menu" @mousedown.stop @mouseup="menuClickHandler">
    <li v-for="menu of menus" :key="menu.key" class="menu-item">
      <div
        class="menu-title"
        :class="{ 'menu-active': expandedKeys[0] === menu.key, 'menu-disabled': menu.disabled }"
        :data-menu-key="menu.key"
        @mousedown="topMenuMousedownHandler(menu)"
        @mouseenter="switchMenu(menu)"
        @mouseup.stop
      >
        {{ $t(menu.title) }}
      </div>
      <ul
        v-if="menu.children && menu.children.length"
        v-show="expandedKeys[0] === menu.key"
        class="sub-menu"
        @mouseleave="resetHoverSubMenu"
      >
        <li
          v-for="child of menu.children.filter(item => item.key !== '3.4')"
          :key="child.key"
          class="sub-menu-item"
          :class="{ 'menu-split': child.split }"
          @mouseenter="setActiveSubMenu(child)"
          @mouseleave="unsetActiveSubMenu(child)"
        >
          <div
            class="sub-menu-title"
            :class="{ 'menu-active': hoverSubMenuKey === child.key, 'menu-disabled': child.disabled }"
            :data-menu-key="child.key"
            @mouseenter="showGChildrenDelay(child)"
            @mouseleave="clearShowGChildrenTimeout"
            @mouseup="clickingMenuKey = child.key"
          >
            <span>{{ $t(child.title) }}</span>
            <span v-if="child.hotKey">{{ child.hotKey }}</span>
            <span v-if="child.children && child.children.length">
              <el-icon class="el-icon-arrow-right"><arrow-right /></el-icon>
            </span>
          </div>
          <ul v-if="child.children && child.children.length" v-show="expandedKeys[1] === child.key" class="sub-menu">
            <li v-for="gChild of child.children" :key="gChild.key" class="sub-menu-item">
              <div
                class="sub-menu-title"
                :class="{ 'menu-disabled': gChild.disabled }"
                :data-menu-key="gChild.key"
                @mouseup="clickingMenuKey = gChild.key"
              >
                <span>{{ $t(gChild.title) }}</span>
                <span>{{ gChild.hotKey }}</span>
              </div>
            </li>
          </ul>
        </li>
      </ul>
    </li>
  </ul>
</template>

<script>
import { get } from 'lodash-es'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { getMenuObj } from '@/common/utils'
import { errorHandler } from '@/common/errorHandler'

export default {
  name: 'NavMenu',
  data () {
    return {
      // 分别代表 1级，2级菜单打开的key
      expandedKeys: ['', ''],
      // 当前展示激活状态的二级菜单（并非是打开的菜单）
      hoverSubMenuKey: '',
      showGChildrenTimeoutId: -1,
      clickingMenuKey: null,
    }
  },
  computed: {
    ...mapState('scenario', [
      'status',
    ]),
    ...mapGetters('scenario', [
      'presentScenario',
    ]),
    ...mapGetters('title-bar', [
      'menus',
    ]),
  },
  created () {
    this.getHistories()
  },
  mounted () {
    window.addEventListener('keydown', this.onKeydown, false)
  },
  beforeUnmount () {
    window.removeEventListener('keydown', this.onKeydown, false)
  },
  methods: {
    ...mapActions('scenario', [
      'getHistories',
      'removeObject',
      'saveScenario',
    ]),
    ...mapActions('title-bar', [
      'quit',
      'openHistoryScenario',
    ]),
    ...mapActions('mission', [
      'openCreateNewScenario',
    ]),
    ...mapMutations('mission', [
      'startMission',
    ]),
    getMenuObj,
    topMenuMousedownHandler (menu) {
      const topMenuKey = this.expandedKeys[0]
      if (!topMenuKey) {
        this.expandedKeys[0] = menu.key
        this.$store.commit('title-bar/updateState', { menuVisible: true })
        window.addEventListener('mousedown', this.hideAllMenu, true)
      } else {
        this.hideAllMenu()
      }
    },
    /**
     * @desc 切换菜单
     * @param menu
     */
    switchMenu (menu) {
      if (this.expandedKeys[0] && this.expandedKeys[0] !== menu.key) {
        // 当已经有菜单激活时，才切换菜单
        this.hoverSubMenuKey = ''
        this.expandedKeys[0] = menu.key
        this.expandedKeys[1] = ''
      }
    },
    hideAllMenu (event) {
      const target = get(event, 'target')
      if (target) {
        const { wrap } = this.$refs
        if (wrap.contains(target)) return
      }
      this.hoverSubMenuKey = ''
      this.expandedKeys[0] = ''
      this.expandedKeys[1] = ''
      this.$store.commit('title-bar/updateState', { menuVisible: false })
      window.removeEventListener('mousedown', this.hideAllMenu, true)
    },
    showGChildrenDelay (menu) {
      if (menu && menu.children) {
        this.clearShowGChildrenTimeout()
        this.showGChildrenTimeoutId = setTimeout(() => {
          this.expandedKeys[1] = menu.key
        }, 500)
      }
    },
    clearShowGChildrenTimeout () {
      try {
        clearTimeout(this.showGChildrenTimeoutId)
      } catch (e) {
        // pass
      }
    },
    setActiveSubMenu (menu) {
      this.hoverSubMenuKey = menu.key
    },
    unsetActiveSubMenu (menu) {
      if (this.expandedKeys[1] !== menu.key) {
        this.hoverSubMenuKey = ''
      }
    },
    resetHoverSubMenu () {
      this.hoverSubMenuKey = this.expandedKeys[1] || ''
    },
    async menuClickHandler () {
      const menu = this.getMenuObj(this.clickingMenuKey, this.menus)
      if (!menu) return
      this.hideAllMenu()
      switch (this.clickingMenuKey) {
        case '1.2':
        case '3.1':
          this.startMission('ScenarioManagement')
          // this.startMission('Log2WorldSetting')
          break
        case '2.2':
          try {
            await this.$confirmBox(this.$t('tips.whetherDeleteObjectFromMap'))
            this.removeObject()
          } catch (e) {
            console.error(e)
          }
          break
        case '3.2':
          this.startMission('MapManagement')
          break
        case '1.1':
          this.openCreateNewScenario()
          break
        case '3.3':
          this.startMission('ModuleManagement')
          break
        case '3.7':
          this.startMission('ModuleGlobalManagement')
          break
        case '3.4':
          this.startMission('TrafficFlowConfig')
          break
        case '3.5':
          this.startMission('KpiManagement')
          break
        case '3.6':
          this.startMission('KpiReportManagement')
          break
        case '5.1':
          this.startMission('KpiIndicatorSetting')
          break
        case '4.1':
          this.startMission('SystemConfig')
          break
        case '6.3':
          this.startMission('AboutTadsim')
          break
        case '1.4':
          this.quit()
          break
        case '1.5':
          this.handleSave()
          break
        case '1.6':
          this.startMission('SaveAs')
          break
        case '1.7':
          this.startMission('ImportFromTapd')
          break
        default:
          if (this.clickingMenuKey.startsWith('1.3.')) {
            try {
              await this.openHistoryScenario(this.clickingMenuKey)
            } catch (e) {
              await errorHandler(e)
            }
          } else {
            console.log(`unhandled menu click! Key: ${this.clickingMenuKey}`)
          }
      }
      // 消费 clickingMenuKey 否则会导致点击空白位置执行上一次key的操作
      this.clickingMenuKey = null
    },
    onKeydown (evt) {
      switch (evt.key) {
        case 'o':
          if (evt.ctrlKey) {
            this.startMission('ScenarioManagement')
            evt.preventDefault()
          }
          break
        case 'n':
          if (evt.ctrlKey) {
            this.openCreateNewScenario()
            evt.preventDefault()
          }
          break
        case 'q':
          if (evt.ctrlKey) {
            this.quit()
            evt.preventDefault()
          }
          break
        case 's':
          if (evt.ctrlKey) {
            this.handleSave()
            evt.preventDefault()
          }
          break
        case 'S':
          if (evt.ctrlKey) {
            this.startMission('SaveAs')
            evt.preventDefault()
          }
          break
        default:
      }
    },
    // 预设场景保存，走另存为逻辑
    handleSave () {
      if (!this.presentScenario) return
      if (this.presentScenario.preset === '0') {
        this.saveScenario()
      } else {
        this.startMission('SaveAs')
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.nav-menu {
  display: flex;
  align-items: center;
  justify-content: flex-start;
  z-index: 10;

  .menu-item {
    height: 40px;
    line-height: 40px;
    margin-right: 2px;
    position: relative;
    -webkit-app-region: no-drag;

    .menu-title {
      height: 100%;
      padding: 0 6px;
      transition: background-color .12s linear;

      &:hover,
        // 有子菜单的菜单激活class
      &.menu-active {
        .menu-active-style;
      }

      &.menu-disabled {
        .menu-disabled-style;
        pointer-events: none;
      }
    }

    .sub-menu {
      position: absolute;
      z-index: 2;
      top: 40px;
      background-color: @darker-bg;
      padding: 5px 0;
      font-size: 12px;

      .sub-menu-item {
        min-width: 160px;
        height: 20px;
        line-height: 20px;
        margin-bottom: 3px;
        position: relative;

        &.menu-split {
          margin-bottom: 11px;

          &::after {
            content: '';
            width: 100%;
            height: 0;
            border-bottom: 1px solid @border-color;
            position: absolute;
            bottom: -6px;
            left: 0;
          }
        }

        .sub-menu-title {
          padding: 0 10px 0 16px;
          display: flex;
          justify-content: space-between;
          align-items: center;
          transition: background-color .12s linear;
          white-space: nowrap;

          &:hover,
          &.menu-active {
            .menu-active-style;
          }

          &.menu-disabled {
            .menu-disabled-style;
            pointer-events: none;
          }
        }

        > .sub-menu {
          top: -5px;
          left: 100%;
        }
      }
    }
  }
}
</style>
