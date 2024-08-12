<template>
  <SectionBox class="scenario-elements">
    <template #title>
      {{ $t('scenario.sceneElements') }}
    </template>
    <div class="scenario-elements-body">
      <div class="scenario-elements-wrap">
        <div
          v-if="presentScenario"
          id="current-scenario-name"
          class="scenario-name-wrap"
          :class="{ active: selectedObjectKey === '' }"
          :title="scenarioFullname"
          @click="selectScenario"
        >
          <ScenarioSvg class="element-icon" />
          {{ scenarioFullname }}
        </div>
        <ul class="element-list">
          <li v-for="(category, categoryName) of tree" :key="categoryName" class="element-category">
            <div :id="categoryName" class="element-title" @click="toggleExpandCategory(categoryName)">
              <div class="element-name" :title="$t(`scenarioElements.${categoryName}`)">
                <span class="element-name-text">
                  <FolderSvg class="element-icon" />
                  {{ $t(`scenarioElements.${categoryName}`) }}
                </span>
              </div>
              <ArrowLine
                class="folder-collapse-button"
                :direction="expandedKeys.includes(categoryName) ? 'up' : 'down'"
                @click.stop="toggleExpandCategory(categoryName)"
              />
            </div>
            <ul v-show="expandedKeys.includes(categoryName)" class="element-sub-list">
              <ListMultipleSelect
                :ref="`list-${categoryName}`"
                v-model="selected[categoryName]"
                :contextmenu-click="(selected[categoryName] || []).length <= 1"
                :list="category"
                :item-keys="['type', 'id']"
                @item-click="handleItemClick"
                @contextmenu.prevent="handleContextMenu"
              >
                <template #item="{ item }">
                  <li :id="`element-node-${item.type}-${item.id}`" class="element-item">
                    <div :id="`${item.type}.${item.id}`" class="element-title">
                      <div v-if="item.type === 'signlight'" class="element-name" :title="$itemName(item)">
                        <span class="element-name-text">
                          <ElementSvg class="element-icon" />
                          <span>
                            {{ $itemName(item) }}
                          </span>
                          <span class="signlight-indicator" :class="getIndicatorClass(item)">
                            <span class="color" />
                            <DisableSvg class="svg" />
                          </span>
                        </span>
                      </div>
                      <div
                        v-else
                        class="element-name"
                        :title="$itemName(item)"
                      >
                        <span class="element-name-text"><ElementSvg class="element-icon" />{{ $itemName(item) }}</span>
                      </div>
                    </div>
                    <ul
                      v-if="item.sensors && item.sensors.length"
                      class="element-sub-list sensor-list"
                      @click.stop
                      @contextmenu.stop
                    >
                      <li v-for="sensor of item.sensors" :key="`${sensor.type}.${sensor.ID}`" class="element-item">
                        <div class="element-title">
                          <div class="element-name" :title="sensor.name">
                            <span class="element-name-text"><ElementSvg class="element-icon" />{{ sensor.name }}</span>
                          </div>
                        </div>
                      </li>
                    </ul>
                  </li>
                </template>
              </ListMultipleSelect>
            </ul>
          </li>
        </ul>
      </div>
    </div>
    <div
      v-if="contextMenu.visible"
      id="context-menu"
      class="context-menu"
      :style="contextMenuStyle"
      @click.stop="hideContextMenu"
    >
      <ul class="context-menu-list">
        <template v-if="currentType === 'signlight'">
          <li id="context-menu-signlight-disable" class="context-menu-item" @click="disable">
            {{ $t('trafficLightColor.disabled') }}
          </li>
          <li class="context-menu-item-split" />
          <li id="context-menu-signlight-set-default" class="context-menu-item" @click="setDefault">
            {{ $t('trafficLightColor.default') }}
          </li>
          <li id="context-menu-signlight-set-all-green" class="context-menu-item" @click="setAllGreen">
            {{ $t('trafficLightColor.allGreen') }}
          </li>
          <li id="context-menu-signlight-set-all-yellow" class="context-menu-item" @click="setAllYellow">
            {{ $t('trafficLightColor.allYellow') }}
          </li>
          <li id="context-menu-signlight-set-all-red" class="context-menu-item" @click="setAllRed">
            {{ $t('trafficLightColor.allRed') }}
          </li>
          <li id="context-menu-signlight-set-all-closed" class="context-menu-item" @click="setAllClosed">
            {{ $t('trafficLightColor.blackout') }}
          </li>
        </template>
        <template v-else>
          <li
            id="context-menu-delete"
            class="context-menu-item"
            :class="{ disabled: currentSelected.length < 1 }"
            @click="del"
          >
            {{ $t('operation.delete') }}
          </li>
        </template>
        <template v-if="currentType !== 'planner'">
          <li class="context-menu-item-split" />
          <li
            id="context-menu-copy"
            class="context-menu-item"
            :class="{ disabled: currentSelected.length !== 1 }"
            @click="copy"
          >
            {{ $t('operation.copyAttribute') }}
          </li>
          <li
            id="context-menu-paste"
            class="context-menu-item"
            :class="{ disabled: !currentClipboard || currentClipboard.id === (selectedObject && selectedObject.id) }"
            @click="paste"
          >
            {{ $t('operation.pasteAttribute') }}
          </li>
        </template>
      </ul>
    </div>
  </SectionBox>
</template>

<script>
import { cloneDeep, forEach, forOwn, get } from 'lodash-es'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import SectionBox from './section-box.vue'
import ArrowLine from './arrow-line.vue'
import { editor } from '@/api/interface'
import ListMultipleSelect from '@/components/list-multiple-select.vue'
import ElementSvg from '@/assets/images/scenario-elements/element.svg'
import FolderSvg from '@/assets/images/scenario-elements/folder.svg'
import DisableSvg from '@/assets/images/scenario-elements/disable.svg'
import ScenarioSvg from '@/assets/images/scenario-elements/scenario-name.svg'
import { nameMap } from '@/store/modules/sensor/constants'

export default {
  name: 'ScenarioElements',
  components: {
    ArrowLine,
    SectionBox,
    ListMultipleSelect,
    ElementSvg,
    FolderSvg,
    DisableSvg,
    ScenarioSvg,
  },
  data () {
    return {
      currentType: '',
      selected: {
        planner: [],
        car: [],
        moto: [],
        bike: [],
        pedestrian: [],
        animal: [],
        obstacle: [],
        machine: [],
        signlight: [],
        sensor: [],
      },
      contextMenu: {
        visible: false,
        x: 0,
        y: 0,
      },
    }
  },
  computed: {
    ...mapState('scenario', [
      'selectedObjectKey',
      'expandedKeys',
      'clipboard',
      'sceneevents',
      'objects',
    ]),
    ...mapGetters('scenario', [
      'selectedObject',
      'tree',
      'presentScenario',
    ]),
    ...mapState('planners', [
      'plannerList',
    ]),
    scenarioFullname () {
      return this.presentScenario.filename || this.presentScenario.name
    },
    currentSelected () {
      return this.selected[this.currentType] || []
    },
    currentClipboard () {
      return this.clipboard[this.currentType]
    },
    contextMenuStyle () {
      const { x, y } = this.contextMenu
      return {
        top: `${y}px`,
        left: `${x}px`,
      }
    },
    nameMap () {
      return nameMap
    },
  },
  watch: {
    selectedObject (value) {
      if (value) {
        if (this.selectedObjectKey === '') { // list-multiple-select component item click no one or stage canvas deselect or ...
          this.currentType = ''
          this.setCurrentResetOthers(this.currentType)
        } else if (this.selectedObjectKey === 'multiple.ids') { // list-multipl-select component item click multiple ones
          // todo nothing
        } else { // list-multipl-select component item click one or stage canvas select one
          // or property panel property changed
          // (selectedObject was replaced with a new one produced by updateObject method)
          // only watch selectedObjectKey can not sync the new selectedObject to this.selected[this.currentType]
          this.currentType = value.type
          this.setCurrentResetOthers(this.currentType, value)
          const el = document.getElementById(`element-node-${value.type}-${value.id}`)
          if (el) {
            el.scrollIntoView()
          }
        }
      } else {
        this.setCurrentResetOthers()
      }
    },
  },
  mounted () {
    window.addEventListener('click', this.hideContextMenu)
  },
  beforeUnmount () {
    window.removeEventListener('click', this.hideContextMenu)
  },
  methods: {
    ...mapMutations('scenario', [
      'updateState',
      'updateObject',
      'toggleExpandCategory',
      'updateSceneevents',
    ]),
    ...mapActions('scenario', [
      'selectObject',
      'removeObject',
    ]),
    modifyObject (param, item) {
      const { type } = item
      const data = { ...item, ...param }
      const payload = { type, data }
      try {
        editor.object.update(payload)
        this.updateObject(payload)
      } catch (e) {
        console.error(e)
      }
    },
    setCurrentResetOthers (type, item) {
      forOwn(this.selected, (value, key) => {
        if (key === type) {
          if (item) {
            this.selected[key] = [item]
          }
        } else {
          this.selected[key] = []
        }
      })
    },
    getIndicatorClass (item) {
      let { enabled, startTime, greenDuration, yellowDuration, redDuration } = item
      if (!enabled) {
        return 'disabled'
      }
      greenDuration = +greenDuration
      yellowDuration = +yellowDuration
      redDuration = +redDuration
      startTime = +startTime
      const sum = greenDuration + yellowDuration + redDuration
      if (startTime === 0 && sum === 0) {
        return 'closed'
      }
      if (startTime > sum) {
        startTime = startTime % sum
      }
      if (greenDuration > startTime) {
        return 'green'
      } else if (greenDuration + yellowDuration > startTime) {
        return 'yellow'
      } else {
        return 'red'
      }
    },
    hideContextMenu () {
      this.contextMenu.visible = false
    },
    handleItemClick (item) {
      this.currentType = item.type
      this.setCurrentResetOthers(this.currentType)
      const { length } = this.currentSelected
      let object = null
      if (length === 0) {
        object = null
      } else if (length === 1) {
        object = this.currentSelected[0]
      } else {
        object = { type: 'multiple', id: 'ids', index: -1 }
      }
      this.selectObject(object)
      this.hideContextMenu()
    },
    handleContextMenu (e) {
      this.contextMenu = {
        visible: true,
        x: e.pageX,
        y: e.pageY,
      }
    },
    disable () {
      forEach(this.currentSelected, (item) => {
        this.modifyObject({
          enabled: false,
        }, item)
      })
    },
    setDefault () {
      forEach(this.currentSelected, (item) => {
        this.modifyObject({
          enabled: true,
          startTime: 0,
          greenDuration: 30,
          yellowDuration: 3,
          redDuration: 30,
        }, item)
      })
    },
    setAllGreen () {
      forEach(this.currentSelected, (item) => {
        this.modifyObject({
          enabled: true,
          startTime: 0,
          greenDuration: 63,
          yellowDuration: 0,
          redDuration: 0,
        }, item)
      })
    },
    setAllYellow () {
      forEach(this.currentSelected, (item) => {
        this.modifyObject({
          enabled: true,
          startTime: 0,
          greenDuration: 0,
          yellowDuration: 63,
          redDuration: 0,
        }, item)
      })
    },
    setAllRed () {
      forEach(this.currentSelected, (item) => {
        this.modifyObject({
          enabled: true,
          startTime: 0,
          greenDuration: 0,
          yellowDuration: 0,
          redDuration: 63,
        }, item)
      })
    },
    setAllClosed () {
      forEach(this.currentSelected, (item) => {
        this.modifyObject({
          enabled: true,
          startTime: 0,
          greenDuration: 0,
          yellowDuration: 0,
          redDuration: 0,
        }, item)
      })
    },
    async del () {
      forEach(this.currentSelected, (item) => {
        this.removeObject(item)
      })
      this.selectScenario()
    },
    copy () {
      this.updateState({
        clipboard: {
          ...this.clipboard,
          // when selectedObject property changed,
          // produce a newer one by updateObject method and no this.selectedObject.x = x assignment, so no need to clone
          [this.currentType]: this.selectedObject,
        },
      })
    },
    paste () {
      if (this.currentClipboard) {
        const payload = cloneDeep(this.currentClipboard)
        delete payload.id
        delete payload.index
        delete payload.label
        if (this.currentType === 'planner') {
          // pass
        } else if (this.currentType === 'car') {
          delete payload.carType
          delete payload.carLength
          delete payload.carWidth
          delete payload.carHeight
        } else if (this.currentType === 'moto') {
          delete payload.subType
        } else if (this.currentType === 'bike') {
          delete payload.subType
        } else if (this.currentType === 'pedestrian') {
          delete payload.subType
        } else if (this.currentType === 'animal') {
          delete payload.subType
        } else if (this.currentType === 'machine') {
          delete payload.subType
        } else if (this.currentType === 'obstacle') {
          delete payload.obstacleType
        } else if (this.currentType === 'signlight') {
          delete payload.routeId
          delete payload.laneId
          delete payload.startShift
          delete payload.offset
        }

        const clipboardTriggerList = get(this.currentClipboard, 'triggers', [])

        forEach(this.currentSelected, (item) => {
          if (this.currentType === 'car' || this.currentType === 'moto' || this.currentType === 'bike' || this.currentType === 'pedestrian' || this.currentType === 'animal' || this.currentType === 'machine') {
            // 如果粘贴对象的id和payload.follow一致，则将follow设置成''（交通车参照车不能是自己）
            let { follow } = payload
            if (+item.id === +payload.follow) {
              follow = ''
            }

            // 在sceneEvents中删除当前选中Object中的triggers
            const selectedTriggerIds = []
            if (item.triggers && item.triggers.length) {
              selectedTriggerIds.push(...item.triggers.map(({ id }) => Number(id)))
            }
            const sceneEvents = this.sceneevents.filter((event) => {
              return !selectedTriggerIds.includes(Number(event.id))
            })
            this.updateSceneevents(sceneEvents)

            // 如果复制对象的触发设置中有引用，那么该引用不能是粘贴对象，如果是的话置空
            const nameOfItemAsTargetElement = `${item.type === 'car' ? 'v' : 'p'}_${item.id}`

            // payload中triggers与eventId需要根据当前选中对象进行处理
            const newTriggerList = []
            const eventIdList = []
            clipboardTriggerList.forEach((trigger) => {
              const sceneEventsIdList = this.sceneevents.map(scene => Number(scene.id))
              const id = (() => {
                for (let i = 0; i < sceneEventsIdList.length > 0 ? Math.max(...sceneEventsIdList) + 2 : 1; i++) {
                  if (!sceneEventsIdList.includes(i)) return i
                }
              })()

              // 如果复制对象的触发设置中有引用，那么该引用不能是粘贴对象，如果是的话置空
              trigger.conditions.forEach((condition, index, arr) => {
                if (condition.targetelement === nameOfItemAsTargetElement) {
                  arr[index].targetelement = ''
                }
              })

              trigger.action.forEach((action, index, arr) => {
                if (action.multi && action.multi.target_element === nameOfItemAsTargetElement) {
                  arr[index].mutil.target_element = ''
                }
              })

              const newTrigger = { ...trigger, id: Number(id) }
              this.updateSceneevents([...this.sceneevents, cloneDeep(newTrigger)])
              newTriggerList.push(newTrigger)
              eventIdList.push(id)
            })

            payload.eventId = eventIdList.join(',')
            payload.triggers = cloneDeep(newTriggerList)

            const updated = editor.object.copy({
              ...item,
              ...payload,
              follow,
            })
            this.modifyObject(updated, item)

            this.modifyRefAfterPasteOneObj(selectedTriggerIds, eventIdList)
          } else {
            this.modifyObject(payload, item)
          }
        })
      }
    },
    modifyRefAfterPasteOneObj (deletedTriggers = [], newGenIds = []) {
      // 修改并检查所有 object (除主车)触发中的 element_ref，如果修改后不合法，则置空
      const idMap = {}// 记录被删除的trigger id对应新生成 trigger 的 id
      newGenIds.forEach((newId, index) => {
        if (deletedTriggers[index] || deletedTriggers[index] === 0) {
          idMap[deletedTriggers[index]] = newId
        }
      })
      let isModifySceneevents = false
      let editSceneevents
      this.objects.forEach((object) => {
        let isModifyObjElementRef = false
        let editObject
        object.triggers && object.triggers.forEach((trigger, triggerIndex) => {
          trigger.conditions.forEach((condition, conditionIndex) => {
            if (condition.element_ref) {
              const [elementRefTriggerId, elementRefActionId] = condition.element_ref.split('_')
              // 如果该 trigger 的 element_ref 是之前被删除的 trigger, 则需要修改 element_ref 并检查是否还合法
              if (deletedTriggers.includes(Number(elementRefTriggerId))) {
                isModifyObjElementRef = true
                isModifySceneevents = true
                if (!editObject) {
                  editObject = cloneDeep(object)
                }
                if (!editSceneevents) {
                  editSceneevents = cloneDeep(this.sceneevents)
                }

                const newTriggerId = idMap[elementRefTriggerId]
                const newElementRef = `${newTriggerId}_${elementRefActionId}`

                // 修改 objects 的 element_ref
                const editConditionInObj = editObject.triggers[triggerIndex].conditions[conditionIndex]
                editConditionInObj.element_ref = newElementRef

                const triggerIndexInSceneevent = editSceneevents
                  .findIndex(triggerInSceneevent => Number(triggerInSceneevent.id) === Number(trigger.id))

                if (triggerIndexInSceneevent > -1) {
                  // 修改 sceneevnets 内的 element_ref
                  const editConditionInSceneevent = editSceneevents[triggerIndexInSceneevent].conditions[conditionIndex]
                  editConditionInSceneevent.element_ref = newElementRef

                  const newTrigger = editSceneevents[triggerIndexInSceneevent]
                  // 检查新的 element_ref 是否合法

                  // element_ref 前半部分 triggerId 如果为空，则不合法
                  if (!newTriggerId && newTriggerId !== 0) {
                    editConditionInObj.element_ref = '' // 置空 objects 的 element_ref
                    editConditionInSceneevent.element_ref = '' // 置空 sceneevnets 内的 element_ref
                  }
                  // trigger 的 element_ref 引用自身行为不合法
                  if (Number(newTriggerId) === Number(trigger.id)) {
                    editConditionInObj.element_ref = '' // 置空 objects 的 element_ref
                    editConditionInSceneevent.element_ref = '' // 置空 sceneevnets 内的 element_ref
                  }
                  // action 是否存在
                  if (!newTrigger.action.some(action => Number(action.actionid) === Number(elementRefActionId))) {
                    editConditionInObj.element_ref = '' // 置空 objects 的 element_ref
                    editConditionInSceneevent.element_ref = '' // 置空 sceneevnets 内的 element_ref
                  }
                } else {
                  // sceneevents 内不存在的触发在 objects 内也不应该出现
                  editConditionInObj.element_ref = ''
                }
              }
            }
          })
        })
        if (isModifyObjElementRef) {
          this.modifyObject(editObject, { type: object.type, label: object.label })
        }
      })
      if (isModifySceneevents) {
        this.updateSceneevents(editSceneevents)
      }
    },
    selectScenario () {
      this.updateState({
        selectedObjectKey: '',
      })
      this.setCurrentResetOthers('')
    },
    getPlannerName (item) {
      const planner = this.plannerList.find(p => p.variable === item.name)
      return this.$getModelName(planner)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.scenario-elements {
  user-select: none;
  height: 240px;
  min-width: 100px;
}

.scenario-elements-body {
  box-sizing: border-box;
  height: 100%;
  padding: 5px 0;
}

.scenario-elements-wrap {
  height: 100%;
  overflow: auto;
}

.scenario-name-wrap {
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
