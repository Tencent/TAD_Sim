<template>
  <div ref="plannerConfig" class="planner-config">
    <!-- 左边主车列表 -->
    <div class="planner-config-left" :style="{ flex: `0 0 ${plannerWidth}px` }">
      <ul class="planner-list">
        <li
          v-for="(p, index) of plannerList"
          :key="index"
          class="planner-sensor-item"
          :class="{ active: currentPlannerIndex === index }"
          @click="selectP(index)"
        >
          <div class="planner-sensor-avatar">
            <img :src="$getModelIcon(p, true)" alt="">
          </div>
          <div class="planner-sensor-name" :title="$getModelName(p)">
            {{ $getModelName(p) }}
          </div>
          <div class="planner-sensor-item-btns">
            <el-icon
              :title="$t('operation.copy')"
              @click.stop="copyP(index)"
            >
              <CopyDocument />
            </el-icon>
            <el-icon
              v-show="false"
              :title="$t('operation.delete')"
              @click.stop="delP(index)"
            >
              <Delete />
            </el-icon>
          </div>
        </li>
      </ul>
      <div v-if="false" class="planner-btns">
        <!-- 暂时不需要 -->
        <el-icon><Plus /></el-icon>
        <!-- <el-icon><Download /></el-icon> -->
      </div>
      <div
        class="resize-line"
        :style="{ left: `${plannerWidth - 5}px` }"
        @pointerdown="startResize($event, 'planner')"
        @pointermove="resizing"
        @pointerup="stopResize"
      />
    </div>
    <!-- 中间主车预览框、传感器列表 -->
    <div class="planner-config-center">
      <div class="planner-view-box">
        <!-- 主车预览框 -->
        <div class="planner-view">
          <SensorView
            ref="sensorView"
            @dragover="addSensorOver"
            @drop="addSensorEnd"
          />
        </div>
        <!-- 主车树形结构图 -->
        <SectionBox v-if="currentPlanner" class="planner-elements" :style="{ flex: `0 0 ${paramsTreeWidth}px` }">
          <template #title>
            {{ $t('scenario.plannerElements') }}
          </template>
          <div class="planner-elements-body">
            <div class="planner-elements-wrap">
              <div
                class="planner-name-wrap"
                :class="{ 'element-active': selected === 'name' }"
                :title="$getModelName(currentPlanner)"
                @click="select('name')"
              >
                <ScenarioSvg class="element-icon" />
                {{ $getModelName(currentPlanner) }}
              </div>
              <ul class="element-list">
                <li class="element-category">
                  <div
                    class="element-title"
                    :class="{ 'element-active': selected === 'planner' }"
                    @click="select('planner')"
                  >
                    <span class="element-name"><FolderSvg
                      class="element-icon"
                    />车体参数</span>
                  </div>
                </li>
                <li class="element-category">
                  <div class="element-title" @click="toggleSensorVisible">
                    <span class="element-name"><FolderSvg
                      class="element-icon"
                    />设备</span>
                    <ArrowLine
                      class="folder-collapse-button"
                      :direction="sensorsVisible ? 'up' : 'down'"
                    />
                  </div>
                  <MultipleSelectList
                    v-show="sensorsVisible"
                    class="element-sub-list"
                    :data="currentPlanner.sensor.group"
                    :value="selectedSensorIDs"
                    :menus="plannerSensorMenus"
                    li-class="element-item"
                    row-key="$index"
                    @change="selectSensor"
                    @command="deletePlannerSensor(selectedSensorIDs)"
                  >
                    <template #default="{ row: sensor, $index }">
                      <div
                        class="element-title"
                        :class="{ 'element-active': selectedSensorIDs.includes($index) }"
                        :title="sensor.name"
                      >
                        <span class="element-name"><ElementSvg
                          class="element-icon"
                        />{{ sensor.name }}</span>
                      </div>
                    </template>
                  </MultipleSelectList>
                </li>
                <li class="element-category">
                  <div
                    class="element-title"
                    :class="{ 'element-active': selected === 'controller' }"
                    @click="select('controller')"
                  >
                    <span class="element-name"><FolderSvg
                      class="element-icon"
                    />控制器</span>
                  </div>
                </li>
                <BaseAuth :perm="['action.planner-config.dynamic.panel.enable']">
                  <li v-if="currentPlanner.catalogParams[0].properties.dynamic" class="element-category">
                    <div class="element-title" @click="toggleDynamicsVisible ">
                      <span class="element-name"><FolderSvg
                        class="element-icon"
                      />动力学参数</span>
                      <ArrowLine
                        class="folder-collapse-button"
                        :direction="dynamicsVisible ? 'up' : 'down'"
                      />
                    </div>
                    <MultipleSelectList
                      v-show="dynamicsVisible"
                      class="element-sub-list"
                      :data="dynamicsList"
                      :value="selectedDynamicsIDs"
                      li-class="element-item"
                      row-key="$index"
                      @change="selectDynamics"
                    >
                      <template #default="{ row: sensor, $index }">
                        <div class="element-title" :class="{ 'element-active': selectedDynamicsIDs.includes($index) }">
                          <span class="element-name"><ElementSvg
                            class="element-icon"
                          />{{ sensor.name }}</span>
                        </div>
                      </template>
                    </MultipleSelectList>
                  </li>
                </BaseAuth>
              </ul>
            </div>
          </div>
          <div
            class="resize-line"
            @pointerdown="startResize($event, 'paramsTree')"
            @pointermove="resizing"
            @pointerup="stopResize"
          />
        </SectionBox>
        <div
          class="resize-line"
          @pointerdown="startResize($event, 'sensors')"
          @pointermove="resizing"
          @pointerup="stopResize"
        />
      </div>
      <!-- 传感器列表 -->
      <el-tabs class="planner-sensors" :style="{ height: `${sensorHeight}px` }">
        <el-tab-pane
          v-for="(group, groupName) of sensorGroups"
          :key="groupName"
          :label="$t(nameMap[groupName])"
          :style="{ height: `${sensorHeight - 28}px` }"
          class="planner-sensor-items"
        >
          <div v-for="(sensor, index) of group" :key="sensor.idx" class="planner-sensor-item">
            <div class="planner-sensor-avatar">
              <img
                :id="`sensor_${sensor.idx}`"
                :src="sensorIcons[sensor.type]"
                alt=""
                draggable="true"
                @dragstart="addSensorStart"
              >
            </div>
            <div
              v-show="editingNameSensorIdx !== sensor.idx"
              class="planner-sensor-name"
              :title="sensor.name"
            >
              <span class="sensor-name-text">{{ sensor.name }}</span>
              <BaseAuth :perm="['action.planner-config.sensors.modify.enable']">
                <el-icon class="sensor-name-edit-icon" @click="toEditSensorName(sensor.idx)">
                  <Edit />
                </el-icon>
              </BaseAuth>
            </div>
            <el-tooltip
              v-if="editingNameSensorIdx === sensor.idx"
              :content="sensorNameErrorMessage"
              popper-class="sensor-name-error-message"
              placement="top"
              :visible="!!sensorNameErrorMessage"
              :manual="true"
              :popper-options="{ boundariesElement: 'scrollParent' }"
            >
              <input
                ref="sensorNameInput"
                v-model.trim="editingNameSensorName"
                class="sensor-name-input"
                :class="{ 'sensor-name-error': !!sensorNameErrorMessage }"
                @keydown.stop="exitModify"
                @blur="modifySensorName(sensor.idx, $event)"
              >
            </el-tooltip>
            <div class="planner-sensor-item-btns">
              <BaseAuth :perm="['action.planner-config.sensors.modify.enable']">
                <el-icon @click="toEditSensor(sensor.idx)">
                  <Setting />
                </el-icon>
              </BaseAuth>
              <BaseAuth :perm="['action.planner-config.sensors.clone.enable']">
                <el-icon @click="copyS(sensor.idx)">
                  <CopyDocument />
                </el-icon>
              </BaseAuth>
              <BaseAuth :perm="['action.planner-config.sensors.delete.enable']">
                <el-icon v-show="index > 0" @click="delSensor(sensor.idx)">
                  <Delete />
                </el-icon>
              </BaseAuth>
            </div>
          </div>
        </el-tab-pane>
      </el-tabs>
    </div>
    <SectionBox class="planner-config-right" :style="{ flex: `0 0 ${paramsWidth}px` }">
      <template #title>
        <span>{{ currentTitle }}</span>
      </template>
      <component :is="currentElement" ref="currentForm" :sub-type="currentType" class="planner-config-body" />
      <div class="planner-config-save-btns">
        <el-button @click="reset">
          恢复默认
        </el-button>
        <el-button @click="saveP">
          保存
        </el-button>
      </div>
      <div
        class="resize-line"
        @pointerdown="startResize($event, 'params')"
        @pointermove="resizing"
        @pointerup="stopResize"
      />
    </SectionBox>
    <OldSensorConverter v-model="converterVisible" />
    <SensorConfigDialog ref="sensorEditor" />
  </div>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import Mousetrap from 'mousetrap'
import { ElMessage } from 'element-plus'
import PlannerName from './name.vue'
import PlannerController from './controller.vue'
import PlannerParams from './planner-params.vue'
import SensorParams from './sensor-params.vue'
import SensorView from './sensor-view.vue'
import DynamicsBaseInfo from './dynamics-base-info.vue'
import DynamicsParams from './dynamics-params/index.vue'
import SensorConfigDialog from './sensor-config-dialog.vue'
import SectionBox from '@/components/section-box.vue'
import ElementSvg from '@/assets/images/scenario-elements/element.svg'
import ScenarioSvg from '@/assets/images/scenario-elements/scenario-name.svg'
import FolderSvg from '@/assets/images/scenario-elements/folder.svg'
import ArrowLine from '@/components/arrow-line.vue'
import MultipleSelectList from '@/components/multiple-select-list.vue'
import { nameMap } from '@/store/modules/sensor/constants'
import CameraIcon from '@/assets/images/model-library/sensor/Camera.png'
import GPSIcon from '@/assets/images/model-library/sensor/GPS.png'
import IMUIcon from '@/assets/images/model-library/sensor/IMU.png'
import OBUIcon from '@/assets/images/model-library/sensor/OBU.png'
import RadarIcon from '@/assets/images/model-library/sensor/Radar.png'
import LidarIcon from '@/assets/images/model-library/sensor/TraditionalLidar.png'
import TruthIcon from '@/assets/images/model-library/sensor/Truth.png'
import UltrasonicIcon from '@/assets/images/model-library/sensor/Ultrasonic.png'
import OldSensorConverter from '@/components/popups/planner-config/old-sensor-converter.vue'

export default {
  name: 'PlannerConfig',
  components: {
    OldSensorConverter,
    SectionBox,
    ElementSvg,
    ScenarioSvg,
    FolderSvg,
    ArrowLine,
    MultipleSelectList,
    SensorView,
    SensorConfigDialog,
    PlannerName,
    PlannerController,
    PlannerParams,
    SensorParams,
    DynamicsBaseInfo,
    DynamicsParams,
  },
  data () {
    return {
      sensorsVisible: true,
      dynamicsVisible: true,
      converterVisible: false,
      plannerSensorMenus: [
        {
          name: this.$t('operation.delete'),
          key: 'delete',
        },
      ],
      // 弹窗编辑sensor相关
      currentSensorIdx: null,
      // 复制sensor相关
      editingNameSensorIdx: null,
      editingNameSensorName: '',
      sensorNameErrorMessage: '',
      // 正在拖动的sensor id
      targetId: '',
      // 如果是正在新建sensor，那么取消的时候需要把这个sensor删除
      isNewSensor: false,
      minPlannerWidth: 100,
      plannerWidth: 200,
      minParamsWidth: 240,
      paramsWidth: 240,
      minParamsTreeWidth: 160,
      paramsTreeWidth: 180,
      minSensorHeight: 152,
      sensorHeight: 152,
      resizeTarget: null,
      resizeStartScreenX: 0,
      resizeStartScreenY: 0,
      pointerId: -1,
      plannerConfigWidth: 0,
      maxSensorHeight: 0,
    }
  },
  computed: {
    ...mapState('planners', [
      'plannerList',
      'currentPlannerIndex',
      'selected',
      'currentPlanner',
      'selectedSensorIDs',
      'selectedDynamicsIDs',
      'sensors',
      'isDirty',
      'currentDynamicData',
    ]),
    ...mapGetters('planners', [
      'sensorGroups',
      'currentSensor',
      'currentDynamics',
    ]),
    currentType () {
      const { selected, currentSensor, currentDynamics } = this
      if (selected) {
        return selected
      }
      if (currentSensor) {
        return currentSensor.type
      }
      if (currentDynamics !== null) {
        return this.dynamicsList[currentDynamics].type
      }
      return null
    },
    currentElement () {
      switch (this.currentType) {
        case 'name':
          return 'PlannerName'
        case 'planner':
          return 'PlannerParams'
        case 'controller':
          return 'PlannerController'
        case 'Camera':
        case 'Depth':
        case 'Fisheye':
        case 'Semantic':
        case 'TraditionalLidar':
        case 'Radar':
        case 'Truth':
        case 'IMU':
        case 'GPS':
        case 'Ultrasonic':
        case 'OBU':
          return 'SensorParams'
        case 'dynamics':
          return 'DynamicsBaseInfo'
        case 'accessory':
        case 'propulsion':
        case 'driveline':
        case 'steerSystem':
        case 'body':
        case 'susp':
        case 'wheelTire':
        case 'sprungMass':
        case 'ecu':
        case 'carType':
          return 'DynamicsParams'
        default:
          return 'div'
      }
    },
    currentTitle () {
      switch (this.currentType) {
        case 'name':
          return '配置名称'
        case 'planner':
          return '车体参数'
        case 'controller':
          return '车辆控制器'
        case 'Camera':
        case 'Depth':
        case 'Fisheye':
        case 'Semantic':
        case 'TraditionalLidar':
        case 'Radar':
        case 'Truth':
        case 'IMU':
        case 'GPS':
        case 'Ultrasonic':
        case 'OBU':
          return '设备'
        case 'dynamics':
          return '动力学参数'
        case 'accessory':
        case 'propulsion':
        case 'driveline':
        case 'steerSystem':
        case 'body':
        case 'susp':
        case 'wheelTire':
        case 'sprungMass':
        case 'ecu':
          return '属性'
        default:
          return ''
      }
    },
    dynamicsList () {
      const list = []
      const dynamics = ['carType', 'propulsion', 'driveline', 'steerSystem', 'susp', 'wheelTire', 'sprungMass', 'body', 'ecu']
      dynamics.forEach((type, index) => {
        list.push({
          type,
          idx: index,
          name: this.$t(`dynamic.${type}`),
        })
      })
      return list
    },
    nameMap: () => nameMap,
    sensorIcons: () => ({
      Camera: CameraIcon,
      Radar: RadarIcon,
      TraditionalLidar: LidarIcon,
      Truth: TruthIcon,
      IMU: IMUIcon,
      GPS: GPSIcon,
      Fisheye: CameraIcon,
      Semantic: CameraIcon,
      Depth: CameraIcon,
      Ultrasonic: UltrasonicIcon,
      OBU: OBUIcon,
    }),
  },
  watch: {
    editingNameSensorIdx (val) {
      if (val === null) {
        document.removeEventListener('click', this.autoSaveSensorName, { capture: true })
      } else {
        document.addEventListener('click', this.autoSaveSensorName, { capture: true })
      }
    },
  },
  created () {
    this.getModuleSetList()
    this.selectPlanner(0)
    this.setParamsWidth(this.paramsWidth)
    Mousetrap.bind('ctrl+shift+y', () => {
      if (this.isDirty) {
        ElMessage.error('请先保存当前主车配置再打开转换工具。')
        return
      }
      this.converterVisible = true
    })
  },
  beforeUnmount () {
    Mousetrap.unbind('ctrl+shift+y')
  },
  methods: {
    ...mapActions('module-set', ['getModuleSetList']),
    ...mapMutations('planners', [
      'selectPlanner',
      'select',
      'selectSensor',
      'selectDynamics',
      'deletePlannerSensor',
      'setIsDirty',
      'setParamsWidth',
    ]),
    ...mapActions('planners', [
      'copyPlanner',
      'removePlanner',
      'validatePlanner',
      'savePlanner',
      'validateSensorName',
      'addSensorToPlanner',
      'copySensor',
      'createSensor',
      'modifySensor',
      'removeSensor',
    ]),
    dispatchEvent () {
      const event = new CustomEvent('paramsResizeEvent', {
        detail: {
          paramsWidth: this.paramsWidth,
        },
      })
      window.dispatchEvent(event)
    },
    startResize ($event, target) {
      ({
        width: this.plannerConfigWidth,
        height: this.maxSensorHeight,
      } = this.$refs.plannerConfig.getBoundingClientRect())
      let layer
      ({
        screenX: this.resizeStartScreenX,
        screenY: this.resizeStartScreenY,
        target: layer,
        pointerId: this.pointerId,
      } = $event)
      this.resizeTarget = target
      layer.setPointerCapture(this.pointerId)
    },
    resizing ($event) {
      const { target, screenX, screenY } = $event
      const {
        resizeTarget,
        pointerId,
        plannerWidth,
        minPlannerWidth,
        minParamsTreeWidth,
        paramsTreeWidth,
        minParamsWidth,
        paramsWidth,
        minSensorHeight,
        sensorHeight,
        resizeStartScreenX,
        resizeStartScreenY,
        maxSensorHeight,
      } = this
      if (resizeTarget === null || !target.hasPointerCapture(pointerId)) {
        return
      }
      window.getSelection().empty()
      const intervalX = screenX - resizeStartScreenX
      const intervalY = screenY - resizeStartScreenY
      this.resizeStartScreenX = screenX
      this.resizeStartScreenY = screenY
      if (resizeTarget === 'planner') {
        const maxPlannerWidth = this.plannerConfigWidth - paramsTreeWidth - paramsWidth
        this.plannerWidth = Math.max(minPlannerWidth, plannerWidth + intervalX)
        if (this.plannerWidth > maxPlannerWidth) {
          if (paramsTreeWidth > minParamsTreeWidth) {
            this.paramsTreeWidth -= this.plannerWidth - maxPlannerWidth
          } else if (paramsWidth > minParamsWidth) {
            this.paramsWidth -= this.plannerWidth - maxPlannerWidth
          } else {
            this.plannerWidth = Math.min(this.plannerWidth, maxPlannerWidth)
          }
        }
      } else if (resizeTarget === 'paramsTree') {
        const maxParamsTreeWidth = this.plannerConfigWidth - plannerWidth - paramsWidth
        this.paramsTreeWidth = Math.max(minParamsTreeWidth, paramsTreeWidth - intervalX)
        if (this.paramsTreeWidth > maxParamsTreeWidth) {
          if (plannerWidth > minPlannerWidth) {
            this.plannerWidth -= this.paramsTreeWidth - maxParamsTreeWidth
          } else {
            this.paramsTreeWidth = Math.min(this.paramsTreeWidth, maxParamsTreeWidth)
          }
        }
      } else if (resizeTarget === 'params') {
        const maxParamsWidth = this.plannerConfigWidth - paramsTreeWidth - plannerWidth
        this.paramsWidth = Math.max(minParamsWidth, paramsWidth - intervalX)
        if (this.paramsWidth > maxParamsWidth) {
          if (paramsTreeWidth > minParamsTreeWidth) {
            this.paramsTreeWidth -= this.paramsWidth - maxParamsWidth
          } else if (plannerWidth > minPlannerWidth) {
            this.plannerWidth -= this.paramsWidth - maxParamsWidth
          } else {
            this.paramsWidth = Math.min(this.paramsWidth, maxParamsWidth)
          }
        }
        this.dispatchEvent()
      } else if (resizeTarget === 'sensors') {
        this.sensorHeight = Math.max(minSensorHeight, sensorHeight - intervalY)
        this.sensorHeight = Math.min(this.sensorHeight, maxSensorHeight)
      }
    },
    stopResize ($event) {
      const { target } = $event
      const { resizeTarget, pointerId } = this
      if (resizeTarget === null || !target.hasPointerCapture(pointerId)) {
        return
      }
      if (resizeTarget === 'params') {
        this.setParamsWidth(this.paramsWidth)
      }
      target.releasePointerCapture(pointerId)
      this.pointerId = -1
      this.resizeTarget = null
    },
    toggleDynamicsVisible () {
      this.select('dynamics')
      this.dynamicsVisible = !this.dynamicsVisible
    },
    toggleSensorVisible () {
      this.sensorsVisible = !this.sensorsVisible
    },
    async saveP () {
      if (this.$refs.currentForm?.$refs?.geneparams?.$refs?.paramschart) {
        const arr = this.$refs.currentForm?.$refs?.geneparams?.$refs?.paramschart || []
        const rest = []
        arr.forEach((el) => {
          if (el.$refs?.tablechart) {
            rest.push(...el.$refs?.tablechart)
          }
        })
        const flag = rest.filter(v => v.isEdit)
        console.log(flag)
        if (flag.length > 0) {
          this.$message.error('请确定编辑内容已完成')
          flag[0].$el.scrollIntoView()
          return false
        }
      }
      const isValid = await this.validateP()
      if (!isValid) return false

      try {
        await this.savePlanner()
        this.$message.success('保存成功。')
      } catch (e) {
        this.$message.error(`保存失败：${e.message}`)
        return false
      }
      return true
    },
    async copyP (index) {
      const { isDirty } = this
      // 如果复制的是当前主车配置，就让他先保存一下
      if (isDirty) {
        try {
          await this.$confirmBox('当前配置未保存，是否保存修改？')
        } catch (e) {
          return
        }
        const isValid = await this.validateP()
        if (!isValid) return
        try {
          await this.savePlanner()
        } catch (e) {
          this.$message.error(e.message)
          return
        }
      }
      await this.copyPlanner(index)
      const isValid = await this.validateP()
      if (!isValid) return false
    },
    async delP (index) {
      try {
        await this.$confirmBox('确认要删除这个主车配置吗？')
      } catch (e) {
        return
      }
      try {
        await this.removePlanner(index)
        this.$message.success('删除成功。')
      } catch (e) {
        this.$message.error(`删除失败：${e.message}`)
      }
    },
    toEditSensor (idx) {
      this.currentSensorIdx = idx
      this.$refs.sensorEditor.startEdit(idx)
    },
    toEditSensorName (idx) {
      this.editingNameSensorIdx = idx
      this.editingNameSensorName = this.sensors.find(s => s.idx === idx).name
      this.$nextTick(() => {
        this.$refs.sensorNameInput[0].focus()
      })
    },
    exitEditingSensorName () {
      this.editingNameSensorIdx = null
      this.editingNameSensorName = ''
      this.sensorNameErrorMessage = ''
    },
    async modifySensorName (idx, $event) {
      const { editingNameSensorName: name, isNewSensor } = this
      try {
        await this.validateSensorName({ idx, name })
        if (isNewSensor) {
          await this.createSensor({ idx, name })
        } else {
          await this.modifySensor({ idx, name })
        }
        this.exitEditingSensorName()
      } catch (e) {
        this.sensorNameErrorMessage = e.message
        $event.target.focus()
      }
    },
    autoSaveSensorName (event) {
      event.stopPropagation()
    },
    async copyS (idx) {
      const newIdx = await this.copySensor(idx)
      const target = this.sensors.find(s => s.idx === newIdx)
      await this.$nextTick()
      try {
        this.isNewSensor = true
        await this.validateSensorName({ idx: newIdx, name: target.name })
        await this.createSensor({ idx: newIdx })
        this.isNewSensor = false
      } catch (e) {
        this.toEditSensorName(newIdx)
      }
    },
    async delSensor (idx) {
      const { isNewSensor } = this
      if (!isNewSensor) {
        try {
          await this.$confirmBox('删除这个传感器也会将它从所有引用了它的主车配置里删除。')
        } catch (e) {
          return
        }
      } else {
        this.isNewSensor = false
      }
      try {
        await this.removeSensor({ idx, isNewSensor })
      } catch (e) {
        this.$message.error(e.message)
      }
    },
    reset () {
      this.selectPlanner(this.currentPlannerIndex)
      this.setIsDirty(false)
    },
    async validateP () {
      try {
        await this.validatePlanner()
        return true
      } catch (e) {
        this.select('name')
        await this.$nextTick()
        await this.$refs.currentForm.$refs.form.validate()
        return false
      }
    },
    addSensorStart ({ target, dataTransfer }) {
      if (target) {
        const { id } = target
        this.targetId = id
        dataTransfer.effectAllowed = 'copy'
        dataTransfer.setData('text/plain', id)
        dataTransfer.setDragImage(target, 0, 0)
      }
    },
    addSensorEnd (event) {
      if (this.targetId) {
        const id = /^sensor_(\d+)$/.exec(this.targetId)[1]
        this.addSensorToPlanner(+id)
        this.targetId = null
      } else {
        event.preventDefault()
      }
    },
    addSensorOver (event) {
      if (this.targetId) {
        event.preventDefault()
      }
    },
    async exitModify (event) {
      if (event.key === 'Escape') {
        if (this.isNewSensor) {
          await this.delSensor(this.editingNameSensorIdx)
        }
        this.exitEditingSensorName()
      }
    },
    async beforeClose () {
      if (this.isDirty) {
        try {
          await this.$confirmBox('当前配置未保存，是否保存修改？', {
            closeOnClickModal: false,
            closeOnPressEscape: false,
          })
          return await this.saveP()
        } catch (e) {
          this.reset()
          return true
        }
      }
    },
    async selectP (index) {
      if (index === this.currentPlannerIndex) {
        return
      }
      const { isDirty } = this
      if (isDirty) {
        try {
          await this.$confirmBox('当前配置未保存，是否保存修改？')
        } catch (e) {
          return
        }
        const isValid = await this.validateP()
        if (!isValid) return
        try {
          await this.savePlanner()
        } catch (e) {
          this.$message.error(e.message)
          return
        }
      }
      this.selectPlanner(index)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins.less";

.planner-sensor-item {
  width: 78px;
  height: 104px;
  margin-bottom: 20px;
  cursor: default;
  position: relative;
  z-index: 1;

  &:hover {
    .planner-sensor-avatar {
      box-shadow: 0 0 0 1px @active-font-color;
    }

    .planner-sensor-name {
      color: @active-font-color;

      .sensor-name-edit-icon {
        display: block;
      }
    }

    .planner-sensor-item-btns {
      display: flex;
    }
  }

  &.active {
    .planner-sensor-avatar {
      box-shadow: 0 0 0 1px @active-font-color;
    }

    .planner-sensor-name {
      color: @active-font-color;
    }
  }

  .planner-sensor-avatar {
    width: 78px;
    height: 78px;
    margin-bottom: 10px;
    border-radius: 3px;
    background-color: @hover-bg;

    img {
      width: 100%;
      height: auto;
    }
  }

  .planner-sensor-name {
    //display: flex;
    //justify-content: center;
    color: @global-font-color;
    font-size: 12px;
    text-align: center;
    line-height: 16px;
    .text-overflow;

    .sensor-name-text {
      .text-overflow;
    }

    .sensor-name-edit-icon {
      position: absolute;
      z-index: 2;
      bottom: 1px;
      right: 0;
      cursor: pointer;
      background-color: @dark-bg;
      display: none;
    }
  }

  .planner-sensor-item-btns {
    display: none;
    position: absolute;
    z-index: 2;
    width: 78px;
    height: 18px;
    top: 60px;
    left: 0;
    font-size: 12px;
    background-color: rgba(19, 19, 19, 0.4);
    justify-content: space-around;
    align-items: center;
    color: @global-font-color;

    i {
      cursor: pointer;

      &:hover {
        color: @active-font-color;
      }
    }
  }
}

.planner-config {
  width: 1154px;
  height: 560px;
  display: flex;
  align-items: stretch;
  background-color: @darker-bg;
  position: relative;
  z-index: 1;

  .resize-line {
    position: absolute;
    cursor: col-resize;
  }

  .planner-config-left {
    flex: 0 0 200px;
    border-right: 1px solid @dark-bg;
    background-color: @darker-bg;
    overflow: auto;

    .planner-list {
      margin-top: 15px;
      display: grid;
      grid-template-columns: repeat(auto-fill, 78px);
      justify-content: space-evenly;
    }

    .resize-line {
      top: 0;
      bottom: 0;
      width: 3px;
      z-index: 10;
    }
  }

  .planner-config-center {
    flex: 1;
    display: flex;
    flex-direction: column;
    min-width: 1px;

    .planner-view-box {
      position: relative;
      flex: 1;
      display: flex;
      align-items: stretch;
      min-height: 1px;

      .planner-view {
        flex: 1;
        overflow: hidden;
      }

      .planner-elements {
        position: relative;
        flex: 0 0 180px;
        overflow: hidden;

        .resize-line {
          top: 0;
          left: 0;
          bottom: 0;
          width: 3px;
          z-index: 10;
        }
      }

      > .resize-line {
        left: 0;
        right: 0;
        bottom: 0;
        height: 3px;
        z-index: 10;
        cursor: row-resize;
      }
    }

    .planner-sensors {
      :deep(.el-tabs__header) {
        margin-bottom: 0;
      }

      :deep(.el-tabs__nav-prev),
      :deep(.el-tabs__nav-next) {
        line-height: 28px;
      }
    }

    .planner-sensor-items {
      display: flex;
      align-items: center;
      height: 124px;
      overflow-x: auto;
      overflow-y: hidden;

      .planner-sensor-item {
        margin-bottom: 0;
        margin-left: 18px;

        &:last-child {
          margin-right: 18px;
        }

        // 底部的sensor的hover样式和planner不太一样，这里重置一下
        &:hover {
          .planner-sensor-avatar {
            box-shadow: none;
          }

          .planner-sensor-name {
            color: @global-font-color;
          }
        }

        .sensor-name-input {
          box-sizing: border-box;
          border-radius: 1px;
          width: 100%;
          background-color: #0f0f0f;
          border: solid 1px #404040;
          height: 24px;
          line-height: 24px;
          font-size: 12px;
          color: #DCDCDC;
          padding: 0 6px;
          outline: 0;
          transition: border-color .2s cubic-bezier(.645, .045, .355, 1);

          &:focus {
            border-color: #16d1f3;
          }
        }
      }
    }
  }

  .planner-config-right {
    position: relative;
    flex: 0 0 240px;
    overflow-x: hidden;

    :deep(.section-body) {
      background-color: @darker-bg;
      display: flex;
      flex-direction: column;
    }

    .planner-config-body {
      flex: 1;
      overflow: auto;
    }

    .planner-config-save-btns {
      flex: 0 0 34px;
      padding: 0 10px;
      display: flex;
      justify-content: flex-end;
      align-items: center;
    }

    .resize-line {
      top: 0;
      left: 0;
      bottom: 0;
      width: 3px;
      z-index: 10;
    }
  }
}

.planner-elements-body {
  box-sizing: border-box;
  height: 100%;
  padding: 2px 0;
}

.planner-elements-wrap {
  height: 100%;
  overflow-y: auto;
  overflow-x: hidden;
}

.planner-name-wrap {
  padding-left: 25px;
  padding-right: 10px;
  height: 24px;
  line-height: 24px;
  font-size: 12px;
  color: @title-font-color;
  cursor: default;
  position: relative;
  .text-overflow;

  &:hover {
    background-color: @list-item-hover-bg;
  }

  &.element-active {
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
  cursor: default;

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
      background-color: @list-item-hover-bg;
    }

    &.element-active {
      .menu-active-style;
    }
  }

  .element-sub-list {
    position: relative;
    z-index: 1;
  }

  .element-name {
    height: 24px;
    line-height: 24px;
    display: flex;
    align-items: center;
    position: relative;
    z-index: 2;
    white-space: nowrap;

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
}
</style>

<style>
.sensor-name-error-message.el-tooltip__popper.is-dark {
  color: #F73222;
}
</style>
