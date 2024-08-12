<template>
  <div ref="stage" v-my-drop="onDrop" class="stage">
    <div
      ref="editor"
      class="editor"
      :class="cursorStyle"
      :style="{
        visibility: isModifying ? 'visible' : 'hidden',
        zIndex: isModifying ? 3 : 2,
      }"
    >
      <div ref="editor-canvas" class="editor-canvas" />
      <SimCompass :degree="degreeEditor" class="compass-container compass-container-editor" />
      <DistanceMeasurement :points="distanceMeasurementPoints" />
    </div>
    <div
      ref="player"
      class="player"
      :style="{
        visibility: isPlaying ? 'visible' : 'hidden',
        zIndex: isPlaying ? 3 : 2,
      }"
    >
      <canvas ref="board" width="100" height="100" />
      <SimCompass :degree="degreePlayer" class="compass-container compass-container-player" />
    </div>

    <!-- three buttons -->
    <div class="map-interaction" :style="{ visibility: isPlaying ? 'hidden' : 'visible' }">
      <span
        class="border start"
        :class="{ active: operationMode === 1 }"
        :title="$t('operation.Select')"
        @click="updateOperationMode(1)"
      >
        <MapSelectIcon class="icon" />
      </span>
      <span
        class="border center"
        :class="{ active: operationMode === 2 }"
        :title="$t('operation.move')"
        @click="updateOperationMode(2)"
      >
        <MapMoveIcon class="icon" />
      </span>
      <span
        class="border end"
        :class="{ active: operationMode === 3, disable: currentPerspective === 'top' }"
        :title="$t('operation.rotate', { d: '' })"
        @click="handleRotateClick"
      >
        <MapRotateIcon class="icon" />
      </span>
    </div>
    <div
      v-if="isPlaying && isLogSim"
      class="logStatus"
      :class="{ switched: timestamp >= realSwitchTime && isSwitched }"
    >
      {{ logStatusText }}
    </div>
  </div>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { groupBy, throttle } from 'lodash-es'
import { editor, player } from '@/api/interface'
import DistanceMeasurement from '@/components/distance-measurement.vue'
import SimCompass from '@/components/compass.vue'
import MapSelectIcon from '@/assets/images/operation-btns/map-select.svg'
import MapMoveIcon from '@/assets/images/operation-btns/map-move.svg'
import MapRotateIcon from '@/assets/images/operation-btns/map-rotate.svg'
import { getColorByTime, getSignlightColorData } from '@/components/signal-control/common/utils'
import HotKeyControls from '@/common/HotKeyControls'
import { getPermission } from '@/utils/permission'

const { electron: { crossBrowserWindowMessage } } = window

export default {
  name: 'SimStage',
  components: {
    DistanceMeasurement,
    SimCompass,
    MapSelectIcon,
    MapMoveIcon,
    MapRotateIcon,
  },
  data () {
    return {
      resizeObserver: null,
      distanceMeasurementPoints: [],
    }
  },
  computed: {
    ...mapState('stage', [
      'clientRect',
      'degreeEditor',
      'degreePlayer',
    ]),
    ...mapState('scenario', [
      'currentScenario',
      'newScenario',
      'status',
      'operationMode',
      'currentPerspective',
      'timestamp',
    ]),
    ...mapState('log2world', [
      'events',
      'realSwitchTime',
      'committedForm',
    ]),
    ...mapState('playlist', [
      'sceneType',
    ]),
    ...mapGetters('scenario', [
      'selectedObject',
      'isPlaying',
      'isModifying',
    ]),
    ...mapState('signalControl', [
      'signlightPlanUpdateFlag',
    ]),
    ...mapState('mission', [
      'currentMission',
    ]),
    cursorStyle () {
      let name = ''
      switch (this.operationMode) {
        case 2:
          name = 'movingCursor'
          break
        case 3:
          name = 'rotateCursor'
          break
        default:
          name = ''
          break
      }
      return name
    },

    isLogSim () {
      const flag = this.currentScenario && this.currentScenario.traffictype === 'simrec'
      return flag
    },

    isSwitched () {
      return this.committedForm.egoSwitch || this.committedForm.trafficSwitch
    },

    logStatusText () {
      if (this.timestamp - this.realSwitchTime >= 0) {
        this.committedForm.egoSwitch && player.dispatchListener('log2worldChanged', { egoLog2world: true })
        this.committedForm.trafficSwitch && player.dispatchListener('log2worldChanged', { trafficLog2world: true })
        return this.isSwitched ? 'Log2World已切换' : 'Log2World未切换'
      } else {
        player.dispatchListener('log2worldChanged', { egoLog2world: false, trafficLog2world: false })
        return 'Log2World未切换'
      }
    },
  },
  watch: {
    isModifying () {
      this.$store.commit('stage/updateState', {
        degreeEditor: 0,
        degreePlayer: 0,
      })
    },
    signlightPlanUpdateFlag () {
      this.updatePlanList()
    },
  },
  created () {
    this.throttled = throttle(() => {
      if (!this.$refs.stage) return
      const clientRect = this.$refs.stage.getBoundingClientRect()
      this.$store.commit('stage/updateState', {
        clientRect,
      })
      switch (this.status) {
        case 'playing':
          player.scenario.resize(this.clientRect.width, this.clientRect.height)
          break
        case 'modifying':
          editor.scenario.resize(this.clientRect.width, this.clientRect.height)
          break
        default:
      }
    }, 100)
    this.hotkeyControl = new HotKeyControls()
    this.initHotkeyControls()
  },
  async mounted () {
    try {
      await this.getAllCatalogs()
      editor.register({
        el: this.$refs['editor-canvas'],
      })
      player.register({
        el: this.$refs.player,
        boardEl: this.$refs.board,
      })
      this.subscribe()
      this.observeResize()
    } catch (e) {
      this.$errorBox(e.message)
    }
  },
  beforeUnmount () {
    this.unsubscribe()
    this.unObserveResize()
    this.hotkeyControl.dispose()
  },
  methods: {
    ...mapActions('scenario', [
      'updateOperationMode',
      'removeObject',
    ]),
    ...mapActions('planners', [
      'getAllCatalogs',
    ]),
    ...mapMutations('scenario', [
      'updateState',
      'switchFollowingPlanner',
    ]),
    subscribe () {
      editor.addEventListener('object.selectedChanged', this.selectedChanged)
      editor.addEventListener('object.deselected', this.deselected)
      editor.addEventListener('object.selectedPropertyChanged', this.selectedPropertyChanged)
      editor.addEventListener('zoomView', this.zoomed)
      editor.addEventListener('promptMessage', this.promptHandler)
      editor.addEventListener('distanceMeasurement.changed', this.distanceMeasurementChanged)
      editor.addEventListener('updateUIState', this.updateUIState)
      editor.addEventListener('cameraChanged', this.cameraChangedEditor)
      player.addEventListener('zoomView', this.zoomed)
      player.addEventListener('cameraChanged', this.cameraChangedPlayer)
      document.addEventListener('keyup', this.onKeyUp)
      player.addEventListener('locateObject', this.locateObjectPlaying)
      crossBrowserWindowMessage.on('map-editor-saved', this.mapSaved)
    },
    unsubscribe () {
      editor.removeEventListener('object.selectedChanged', this.selectedChanged)
      editor.removeEventListener('object.deselected', this.deselected)
      editor.removeEventListener('object.selectedPropertyChanged', this.selectedPropertyChanged)
      editor.removeEventListener('zoomView', this.zoomed)
      editor.removeEventListener('promptMessage', this.promptHandler)
      editor.removeEventListener('distanceMeasurement.changed', this.distanceMeasurementChanged)
      editor.removeEventListener('updateUIState', this.updateUIState)
      editor.removeEventListener('cameraChanged', this.cameraChangedEditor)
      player.removeEventListener('zoomView', this.zoomed)
      player.removeEventListener('cameraChanged', this.cameraChangedPlayer)
      document.removeEventListener('keyup', this.onKeyUp)
      player.removeEventListener('locateObject', this.locateObjectPlaying)
      crossBrowserWindowMessage.off('map-editor-saved', this.mapSaved)
    },
    observeResize () {
      this.resizeObserver = new ResizeObserver(() => {
        this.throttled()
      })
      this.resizeObserver.observe(this.$refs.stage)
    },
    unObserveResize () {
      this.resizeObserver.unobserve(this.$refs.stage)
      this.resizeObserver = null
    },
    selectedChanged (payload) {
      this.$store.commit('scenario/selectObject', payload)
    },
    deselected () {
      this.$store.commit('scenario/deselectObject')
    },
    selectedPropertyChanged ({ type, data }) {
      this.$store.commit('scenario/updateObject', {
        type,
        data: {
          ...data,
          sensors: undefined,
        },
      })
    },
    zoomed ({ factor }) {
      this.$store.commit('scenario/updateState', { factor })
    },
    promptHandler ({ type, msg, posCb, negCb }) {
      switch (type) {
        case 'error':
          this.$errorBox(msg)
            .then(() => {
              if (posCb) {
                posCb()
              }
            })
          break
        case 'warn':
          this.$infoBox(msg)
            .then(() => {
              if (posCb) {
                posCb()
              }
            })
          break
        case 'confirm':
          this.$confirmBox(msg)
            .then(posCb)
            .catch(negCb)
          break
        case 'info':
          this.$infoBox(msg)
            .then(() => {
              if (posCb) {
                posCb()
              }
            })
          break
        default:
      }
    },
    distanceMeasurementChanged (payload) {
      const { type, data } = payload
      this.distanceMeasurementPoints = data
      if (type === 'edit') {
        this.$store.commit('scenario/updateState', { isDirty: true })
      }
    },

    updateUIState (payload) {
      const { type, currentPerspective } = payload
      if (type === 'updateView') {
        this.$store.commit('scenario/updateState', {
          currentPerspective,
        })
      }
    },

    cameraChangedEditor: throttle(function (payload) {
      const { degree } = payload
      this.$store.commit('stage/updateState', {
        degreeEditor: degree,
      })
    }, 100),

    cameraChangedPlayer: throttle(function (payload) {
      const { degree } = payload
      this.$store.commit('stage/updateState', {
        degreePlayer: degree,
      })
    }, 100),

    onKeyUp (event) {
      // 屏蔽输入框
      if (['input', 'textarea'].includes(event.target.tagName.toLowerCase())) {
        return
      }
      if (this.isModifying) {
        if (event.keyCode === 80) {
          this.$store.dispatch('scenario/switchPerspective', 'debugging')
        } else if (event.keyCode === 84) {
          this.$store.dispatch('scenario/switchPerspective', 'top')
        }
      }
    },
    async mapSaved (name) {
      if (this.isModifying && this.currentScenario && this.currentScenario.map === name) {
        await this.$message.warning({
          duration: 10 * 1000,
          message: this.$t('tips.MapHasBeenUpdated'),
        })
      }
    },

    handleRotateClick () {
      if (this.currentPerspective !== 'top') {
        this.updateOperationMode(3)
      }
    },
    async onDrop (evt) {
      const { id, pageX, pageY } = evt
      const { top, left } = this.$refs['editor-canvas'].getBoundingClientRect()
      const x = pageX - left
      const y = pageY - top
      const payload = {
        type: id,
        x,
        y,
      }
      try {
        const object = await editor.object.add(payload)
        if (object) {
          this.$store.commit('scenario/addObject', object)
        }
      } catch (e) {
        console.error(e)
      }
    },
    // 刷新信控配置列表
    async updatePlanList () {
      const signlightPlans = editor.scenario.getSignlightPlans()
      const { activePlan, signlights: allSignlights, routesData: routes } = signlightPlans

      const group = groupBy(allSignlights, 'plan')
      if (Object.keys(group).length < 1) return // 无信控配置

      const canRender = getPermission('action.signlightControl.render.enable')
      if (!canRender) return // 无语义灯显示权限

      const signlights = group[activePlan] || []

      // 显示灯
      window.simuScene.hadmap.hideAllSignlights() // 先删除原有灯
      await window.simuScene.hadmap.showSignlights({ signlights, routes }) // 再新建最新灯

      // 更换灯的颜色，TODO: 实际颜色响应很慢
      const grayLights = signlights.map(item => ({ id: item.id, color: 'gray' }))
      window.simuScene.hadmap.changeSignlightColors(grayLights) // 先全置灰
      const enabledLight = signlights.filter(item => item.phaseNumber)
      const realLight = enabledLight.map((item) => {
        const colorData = getSignlightColorData(item)
        const color = getColorByTime(colorData, 0) // 取每个语义灯的灯态配置在 0s 时的颜色
        return { id: item.id, color, disabled: !item.enabled }
      })
      window.simuScene.hadmap.changeSignlightColors(realLight) // 再改为实际颜色
    },
    locateObjectPlaying (type, id) {
      // 暂时只追踪主车
      if (type === 'planner') {
        this.switchFollowingPlanner(id)
      }
    },
    initHotkeyControls () {
      // 绑定快捷键
      this.hotkeyControl.addShortcut('Backspace', onDeleteObject.bind(this))
      this.hotkeyControl.addShortcut('Delete', onDeleteObject.bind(this))

      // 删除场景元素
      function onDeleteObject () {
        // 没选中场景元素时跳过
        if (!this.selectedObject) return
        // 处在弹窗中时跳过
        if (this.currentMission) return
        const model = this.selectedObject
        // 排除点中的是该元素的轨迹点
        const route = window.simuScene.sceneParser.findRouteInMap(model.routeId)
        if (route?.curve && window.simuScene.curPickObject) {
          const pickId = window.simuScene.curPickObject.uuid
          const idx = route.curve.FindBoxIdx(pickId)
          const isTrackPoint = idx > -1 // 是轨迹点
          if (isTrackPoint) return
        }
        // 开始删除场景元素
        console.log('====', 'delete')
        this.removeObject(model)
        this.updateState({ selectedObjectKey: '' })
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.movingCursor {
  cursor: url(../assets/images/operation-btns/map-move.png), default;
}

.rotateCursor {
  cursor: url(../assets/images/operation-btns/map-rotate.png), default;
}

.stage {
  position: relative;
  height: 100%;
}

.editor,
.player {
  position: absolute;
  left: 0;
  right: 0;
  top: 0;
  bottom: 0;

  :deep(canvas) {
    vertical-align: top;
  }
}

.player {
  canvas {
    position: absolute;
    top: 0;
    left: 0;
  }
}

.editor {
  .editor-canvas,
  .distance-measurement {
    position: absolute;
    top: 0;
    right: 0;
    bottom: 0;
    left: 0;
  }
}

.map-interaction {
  position: absolute;
  right: 1em;
  top: 2.2em;
  display: flex;
  z-index: 4;

  .border {
    width: 28px;
    height: 24px;
    background-color: #111111;
    position: relative;
    cursor: pointer;
    color: rgba(255, 255, 255, 0.9);

    &:hover {
      color: #01AAC8;
    }

    &.active {
      background-color: #181818;
      color: #01AAC8;
    }

    &.disable {
      color: #666666;
      cursor: not-allowed;
    }

    .icon {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      width: 12px;
      height: 12px;
    }

    &.center {
      border-left: solid 1px #202020;
      border-right: solid 1px #202020;
    }

    &.start {
      border-top-left-radius: 5px;
      border-bottom-left-radius: 5px;
    }

    &.end {
      border-top-right-radius: 5px;
      border-bottom-right-radius: 5px;
    }
  }
}

.logStatus {
  position: absolute;
  bottom: 10px;
  left: 10px;
  z-index: 4;
  padding: 5px;
  background-color: #666666;
  color: @title-font-color;
}

.switched {
  color: #16d1f3;
  background-color: #000;
}

.compass-container {
  position: absolute;
  left: 45px;
  width: 40px;
  height: 40px;
}

.compass-container-editor {
  bottom: 45px;
}

.compass-container-player {
  bottom: 79px;
}
</style>
