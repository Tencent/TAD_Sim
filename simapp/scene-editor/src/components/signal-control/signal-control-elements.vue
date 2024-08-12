<template>
  <SectionBox class="signal-control-elements">
    <template #title>
      {{ $t('scenario.signalControlElements') }}
    </template>
    <div class="signal-control-elements-body">
      <div class="signal-control-elements-wrap">
        <div
          id="current-signal-control-name"
          class="signal-control-name-wrap"
          :title="$t('scenario.signlight')"
        >
          <FolderSvg class="element-icon" />
          {{ $t('scenario.signlight') }}
        </div>
        <ul class="element-list">
          <li v-for="junction of planJunctionTree" :key="junction.id" class="element-category">
            <div
              :id="junction.id"
              class="element-title"
              :class="{ active: currentJunctionId === junction.id }"
              @click="toggleExpandCategory(junction.id)"
            >
              <RightContextMenu
                :style="{ display: 'flex', width: '100%' }"
                :options="contextMenuOptions"
                :disabled="isSystemPlan"
                @command="handleContextMenu($event, junction)"
              >
                <div class="element-name" :title="junction.name">
                  <span class="element-name-text"><ElementSvg class="element-icon" />{{ junction.name }}</span>
                </div>
                <ArrowLine
                  class="folder-collapse-button"
                  :direction="expandedKeys.includes(junction.id) ? 'up' : 'down'"
                />
              </RightContextMenu>
            </div>
            <ul v-show="expandedKeys.includes(junction.id)" class="element-sub-list">
              <ListMultipleSelect
                :ref="`list-${junction.id}`"
                :model-value="[currentPhase]"
                :list="junction.phases"
                :item-keys="['id']"
                @item-click="handleItemClick"
              >
                <template #item="{ item }">
                  <RightContextMenu
                    :style="{ display: 'block', width: '100%' }"
                    :options="subContextMenuOptions"
                    :disabled="isSystemPlan"
                    @command="handleSubContextMenu($event, item)"
                  >
                    <li :id="`element-node-${item.name}`" class="element-item">
                      <div :id="item.name" class="element-title">
                        <div class="element-name" :title="item.name">
                          <span class="element-name-text">
                            <ElementSvg class="element-icon" />
                            <span>{{ item.name }}</span>
                            <span class="signlight-indicator" :class="getIndicatorClass(item)">
                              <span class="color" />
                              <DisableSvg class="svg" />
                            </span>
                          </span>
                        </div>
                      </div>
                    </li>
                  </RightContextMenu>
                </template>
              </ListMultipleSelect>
            </ul>
          </li>
        </ul>
      </div>
    </div>
  </SectionBox>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapState } from 'vuex'
import SectionBox from '@/components/section-box.vue'
import ArrowLine from '@/components/arrow-line.vue'
import ListMultipleSelect from '@/components/list-multiple-select.vue'
import RightContextMenu from '@/components/right-context-menu.vue'
import ElementSvg from '@/assets/images/scenario-elements/element.svg'
import FolderSvg from '@/assets/images/scenario-elements/folder.svg'
import DisableSvg from '@/assets/images/scenario-elements/disable.svg'
import eventBus from '@/event-bus'

export default {
  name: 'SignalControlElements',
  components: {
    SectionBox,
    ArrowLine,
    ListMultipleSelect,
    RightContextMenu,
    ElementSvg,
    FolderSvg,
    DisableSvg,
  },
  data () {
    return {
      expandedKeys: [],
      selected: [],
      copyTmpData: {},
    }
  },
  computed: {
    ...mapState('signalControl', [
      'currentJunctionId',
    ]),
    ...mapGetters('signalControl', [
      'planJunctionTree',
      'currentPlanId',
      'currentJunction',
      'currentPhase',
      'isSystemPlan',
    ]),
    contextMenuOptions () {
      return [
        { id: 'disabled', label: this.$t('trafficLightColor.disabled') },
        { id: 'enabled', label: this.$t('trafficLightColor.enabled'), children: [
          { id: 'default', label: this.$t('trafficLightColor.default') },
          { id: 'allGreen', label: this.$t('trafficLightColor.allGreen') },
          { id: 'allRed', label: this.$t('trafficLightColor.allRed') },
        ] },
      ]
    },
    subContextMenuOptions () {
      return [
        { id: 'disabled', label: this.$t('trafficLightColor.disabled') },
        { id: 'enabled', label: this.$t('trafficLightColor.enabled'), children: [
          { id: 'default', label: this.$t('trafficLightColor.default') },
          { id: 'allGreen', label: this.$t('trafficLightColor.allGreen') },
          { id: 'allRed', label: this.$t('trafficLightColor.allRed') },
          { id: 'blackout', label: this.$t('trafficLightColor.blackout') },
        ] },
        { id: 'copyAttribute', label: this.$t('operation.copyAttribute') },
        { id: 'pasteAttribute', label: this.$t('operation.pasteAttribute'), disabled: _.isEmpty(this.copyTmpData) },
      ]
    },
  },
  watch: {
    planJunctionTree (tree) {
      this.expandedKeys = tree.map(e => e.id)
    },
    currentPlanId () {
      this.copyTmpData = {}
    },
    currentJunctionId () {
      this.copyTmpData = {}
    },
  },
  mounted () {
    this.expandedKeys = this.planJunctionTree.map(e => e.id)
  },
  methods: {
    ...mapActions('signalControl', [
      'selectJunction',
      'selectPhase',
      'changeJunctionValue',
      'changePhaseValue',
    ]),
    toggleExpandCategory (category) {
      if (this.expandedKeys.includes(category)) {
        this.expandedKeys = this.expandedKeys.filter(key => key !== category)
      } else {
        this.expandedKeys.push(category)
      }
      eventBus.$emit('focus-to-junction', this.currentJunctionId)
    },
    handleItemClick (item) {
      this.selectJunction(item.junctionid)
      this.selectPhase(item.id)
      eventBus.$emit('focus-to-junction', item.junctionid)
    },
    handleContextMenu (command, junction) {
      switch (command.id) {
        case 'disabled': {
          this.changeJunctionValue({ key: command.id, id: junction.id })
          break
        }
        case 'allGreen': {
          this.changeJunctionValue({ key: command.id, id: junction.id })
          break
        }
        case 'default': {
          this.changeJunctionValue({ key: command.id, id: junction.id })
          break
        }
        case 'allRed': {
          this.changeJunctionValue({ key: command.id, id: junction.id })
          break
        }
        case 'blackout': {
          this.changeJunctionValue({ key: command.id })
          break
        }
        default:;
      }
    },
    handleSubContextMenu (command, phase) {
      const cycleTime = this.currentJunction?.cycleTime || 10
      switch (command.id) {
        case 'disabled': {
          this.changePhaseValue({ id: phase.id, key: 'enabled', value: false })
          break
        }
        case 'allGreen': {
          this.changePhaseValue({ id: phase.id, key: 'enabled', value: true })
          this.changePhaseValue({ id: phase.id, key: 'colors', value: [{ color: 'green', value: cycleTime }] })
          break
        }
        case 'default': {
          this.changePhaseValue({ id: phase.id, key: 'enabled', value: true })
          break
        }
        case 'allRed': {
          this.changePhaseValue({ id: phase.id, key: 'enabled', value: true })
          this.changePhaseValue({ id: phase.id, key: 'colors', value: [{ color: 'red', value: cycleTime }] })
          break
        }
        case 'blackout': {
          if (
            this.currentPhase.colors.length > 0 &&
            this.currentJunction.phases.filter(e => e.colors.length > 0).length <= 1
          ) {
            this.$message.warning('该路口控制器下至少需要一个有灯态的 phase')
            return
          }
          this.changePhaseValue({ id: phase.id, key: 'enabled', value: true })
          this.changePhaseValue({ id: phase.id, key: 'colors', value: [] })
          break
        }
        case 'copyAttribute': {
          const copyTmpData = _.pick(this.currentPhase, [
            'junctionid',
            'enabled',
            'colors',
            'signalHead',
          ])
          this.copyTmpData = copyTmpData
          break
        }
        case 'pasteAttribute': {
          if (this.copyTmpData?.colors && this.copyTmpData.colors.length === 0) {
            if (
              this.currentPhase.colors.length > 0 &&
              this.currentJunction.phases.filter(e => e.colors.length > 0).length <= 1
            ) {
              this.$message.warning('该路口控制器下至少需要一个有灯态的 phase')
              return
            }
          }
          const data = _.pick(this.copyTmpData, [
            'enabled',
            'colors',
            'signalHead',
          ])
          Object.keys(data).forEach((key) => {
            this.changePhaseValue({ id: phase.id, key, value: data[key] })
          })
          break
        }
        default:;
      }
    },
    getIndicatorClass (item) {
      if (!item.enabled) return 'disabled'
      const color = item.colors[0]
      if (!color) return 'closed'
      // red yellow green closed disabled
      return color.color
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.signal-control-elements {
  user-select: none;
  height: 240px;
  min-width: 100px;
  font-size: 12px;
}

.signal-control-elements-body {
  box-sizing: border-box;
  height: 100%;
  padding: 5px 0;
}

.signal-control-elements-wrap {
  height: 100%;
  overflow: auto;
}

.signal-control-name-wrap {
  padding-left: 25px;
  padding-right: 10px;
  height: 24px;
  line-height: 24px;
  font-size: 12px;
  color: @title-font-color;
  position: relative;
  .text-overflow;

  &:hover {
    background-color: @hover-bg;
  }

  &.active {
    .menu-active-style;
  }
}

.element-icon {
  margin-right: 8px;
}

.element-list {
  color: @title-font-color;
  font-size: 12px;
  margin-bottom: 5px;

  .collapse-transition,
  .horizontal-collapse-transition {
    transition-duration: .2s;
  }

  .element-title {
    height: 24px;
    line-height: 24px;
    color: @title-font-color;
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding-left: 21px;
    transition: all .16s linear;

    :deep(& .el-popover__reference) {
      width: 100%;
      display: flex;
      align-items: center;
      justify-content: space-between;
    }

    &.active {
      color: @active-font-color;
      background-color: @hover-bg;
    }

    &:hover {
      background-color: @hover-bg;
    }
  }

  .element-sub-list {
    position: relative;
    z-index: 1;
  }

  .element-name {
    height: 24px;
    line-height: 24px;
    //display: flex;
    //align-items: center;
    min-width: 1px;
    position: relative;
    z-index: 2;

    .element-name-text {
      .text-overflow;
      display: block;
    }

    &::before {
      content: '';
      width: 10px;
      height: 0;
      border-bottom: 1px dashed @disabled-color;
      position: absolute;
      top: 12px;
      left: -14px;
    }

    &::after {
      content: '';
      width: 0;
      height: 25px;
      border-right: 1px dashed @disabled-color;
      position: absolute;
      top: -12px;
      left: -15px;
      z-index: 3;
    }

    .signlight-indicator {
      display: inline-block;
      width: 10px;
      margin-left: 10px;
      text-align: center;

      .color {
        display: none;
        width: 6px;
        height: 6px;
        border-radius: 50%;
      }

      &.red .color,
      &.green .color,
      &.yellow .color,
      &.closed .color {
        display: inline-block;
      }

      &.red .color {
        background-color: #B42416;
      }

      &.green .color {
        background-color: #079D4B;
      }

      &.yellow .color {
        background-color: #D49D02;
      }

      &.closed .color {
        background-color: #DCDCDC;
      }

      .svg {
        display: none;
        vertical-align: middle;
      }

      &.disabled .svg {
        display: inline-block;
      }
    }
  }

  .sensor-list {
    .element-name {
      color: @title-font-color;
    }
  }

  .element-category {
    position: relative;

    &::after {
      content: '';
      width: 0;
      height: 100%;
      border-right: 1px dashed @disabled-color;
      position: absolute;
      top: 0;
      left: 31px;
      z-index: 3;
    }

    &:first-child {
      &::after {
        top: -2px;
      }
    }

    &:last-child {
      &::after {
        height: 12px;
      }
    }

    &::before {
      content: '';
      width: 10px;
      height: 0;
      border-bottom: 1px dashed @disabled-color;
      position: absolute;
      top: 12px;
      left: 32px;
    }

    > .element-title {
      padding-left: 46px;
      padding-right: 10px;

      > .element-name {
        &::before {
          display: none;
        }

        &::after {
          display: none;
        }
      }
    }

    .element-item {
      .element-title {
        padding-left: 67px;
      }

      .element-item {
        .element-title {
          padding-left: 88px;
        }
      }

      &:first-child {
        > .element-title {

          .element-name::after {
            top: -2px;
            height: 14px;
          }
        }

      }
    }
  }

  .remove-icon {
    margin-right: 20px;

    &:hover {
      color: @active-font-color;
      cursor: pointer;
    }
  }

  .folder-collapse-button {
    cursor: pointer;
    margin-left: auto;
  }

  .list-multiple-select .list-multiple-select-item-active .element-title {
    .menu-active-style;
  }
}

.context-menu {
  position: fixed;
  width: 80px;
  padding: 5px 0;
  background-color: @hover-bg;
  box-shadow: 0 5px 10px 0 rgba(0, 0, 0, 0.63);
  z-index: 1;

  .context-menu-list {
    .context-menu-item {
      padding: 0 10px;
      font-size: 12px;
      line-height: 24px;
      letter-spacing: 1px;
      color: @title-font-color;
      cursor: pointer;
      user-select: none;

      &:hover {
        color: @active-font-color;
        background-color: #171717;;
      }

      &.disabled {
        color: #878787;
        pointer-events: none;
      }
    }

    .context-menu-item-split {
      margin: 1px 0;
      border-top: 1px solid #000;
    }
  }
}
</style>
