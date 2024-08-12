<template>
  <div class="signal-control-list">
    <div class="signal-control-list-search-bar">
      <!-- 搜索框 -->
      <div class="search-input">
        <el-input
          v-model="keyword"
          style="width: 200px;"
          prefix-icon="search"
          :placeholder="$t('operation.searchName')"
        />
      </div>
    </div>
    <!-- 信控配置列表 -->
    <div class="signal-control-list-wrap">
      <div
        v-for="item in filterList"
        :key="item.id"
        class="signal-control-list-item"
        :class="{ selected: currentPlanId === item.id, active: activePlanId === item.id }"
        @click="handleClickItem(item)"
      >
        <div class="signal-control-list-content">
          <el-radio
            class="signal-control-list-radio"
            checked
          />
          <div
            class="signal-control-list-name"
            :title="item.name"
          >
            {{ item.name }}
          </div>
        </div>
        <div class="signal-control-list-tools">
          <BaseAuth :perm="['action.signlightControl.plan.settings.enable']">
            <!-- 克隆 -->
            <el-icon
              class="icon el-icon-copy-document"
              title="克隆"
              @click.stop="handleClone(item)"
            >
              <copy-document />
            </el-icon>
            <!-- 删除 -->
            <el-icon
              class="icon el-icon-delete"
              title="删除"
              :class="{ disabled: item.id === '0' }"
              @click.stop="handleDelete(item)"
            >
              <delete />
            </el-icon>
          </BaseAuth>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapActions, mapGetters, mapState } from 'vuex'
import { getPermission } from '@/utils/permission'

export default {
  name: 'SignalControlLeftBar',
  data () {
    return {
      keyword: '',
    }
  },
  computed: {
    ...mapGetters('signalControl', [
      'planList',
      'activePlanId',
      'currentPlanId',
      'isSystemPlan',
    ]),
    ...mapState('signalControl', [
    ]),
    filterList () {
      const list = this.planList.filter(e => !this.keyword || e.name.includes(this.keyword))
      return list
    },
  },
  methods: {
    ...mapActions('signalControl', [
      'selectPlan',
      'clonePlan',
      'deletePlan',
    ]),
    handleClickItem (item) {
      const canChange = getPermission('action.signlightControl.plan.switch.enable')
      if (!canChange) return
      this.selectPlan(item.id)
    },
    handleClone (item) {
      this.clonePlan(item)
    },
    handleDelete (item) {
      this.deletePlan(item)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.signal-control-list {
  font-size: 12px;
  color: @global-font-color;
  display: flex;
  flex-direction: column;
}

.signal-control-list-search-bar {
  flex-shrink: 0;
  padding: 16px 20px;
}

.signal-control-list-wrap {
    flex: 1;
    overflow-x: hidden;
    overflow-y: auto;
}

.signal-control-list-item {
  display: flex;
  align-items: center;
  padding: 10px 20px;
  border: 1px solid @border-color;

  .signal-control-list-radio {
    opacity: 0;
  }

  &.selected {
    background: @hover-bg;
    color: @active-font-color;
  }

  &.active {
    .signal-control-list-radio {
      opacity: 1;
    }
  }
}

.signal-control-list-content {
  flex-grow: 1;
  overflow: hidden;
  display: flex;
  align-items: center;
  cursor: pointer;
}

.signal-control-list-radio {
  flex-shrink: 0;
  margin-right: -2px;
  pointer-events: none;
}

.signal-control-list-name {
  flex-grow: 1;
  .text-overflow();
}

.signal-control-list-tools {
  flex-shrink: 0;
  display: flex;
  align-items: center;
  margin-left: 6px;
  color: @global-font-color;

  & > .icon {
    cursor: pointer;
  }

  & > .icon.disabled {
    pointer-events: none;
    color: @disabled-color;
  }

  & > .icon:not(:last-of-type) {
    margin-right: 6px;
  }
}
</style>
