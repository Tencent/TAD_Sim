import {
  AmbientLight,
  AxesHelper,
  Box3,
  GridHelper,
  MathUtils,
  OrthographicCamera,
  PerspectiveCamera,
  Plane,
  Ray,
  Raycaster,
  Scene,
  Vector2,
  Vector3,
  WebGLRenderer,
} from 'three'
import { isNil, throttle } from 'lodash-es'
import '../libs/inflate.min'
import { ScenarioModelsManager } from 'models-manager/index'
import { TransformControls } from '../libs/TransformControls'
import OrbitControls from '../libs/OrbitControls'
import {
  AccelerationTerminationType,
  IgnoreHeight,
  MapElementType,
  OffsetConst,
  OperationMode,
  SceneState,
  TriggerCondition,
} from '../common/Constant'
import GlobalConfig from '../common/Config'
import { LogInfo } from '../api/recordLogs'
import Profile from '../common/Profile'
import { setRotateByDirNormal } from '../common/MeshUtils'
import PickBox from '../libs/PickBox'
import Measurement from './Measurement'
import { MessageBox } from './MessageBox'
import EditorGlobals from './EditorGlobals'
import { EditorCurve } from './EditorCurve'
import Utility from './Utility'
import Signlight from './Signlight'
import MapFile from './MapFile'
import CoordConverter from './CoordConverter'
import RoutePointMarker from './RoutePointMarker'
import SimuSceneParser from './SimuSceneParser'
import Hadmap from './Hadmap'
import AccelerationData from './AccelerationData'
import MergeData from './MergeData'
import Planner from './Planner'
import Vehicle, { VehicleBehaviorType } from './Vehicle'
import VelocityData from './VelocityData'
import RouteData from './RouteData'
import PedestrianV2 from './PedestrianV2'
import Obstacle from './Obstacle'
import Rosbag from './Rosbag'
import { getObstacleCfg, getPedestrianSubType, getVehicleTypeDefinition } from '@/common/utils'
import store from '@/store'
import i18n from '@/locales'
import { getPermission } from '@/utils/permission'

class SimuScene {
  constructor ({ wrapper }) {
    this.uuid = MathUtils.generateUUID()
    this.operationMode = Profile.operationMode // default pick

    this.topSceneRange = 80
    this.topSceneZoomFactor = 1
    this.aspect = wrapper.clientWidth / wrapper.clientHeight
    this.scene = new Scene()
    this.scene.name = `SimuScene${new Date().toLocaleTimeString()}`
    this.camera = new PerspectiveCamera(45, this.aspect, 1, 201000)
    this.cameraTop = new OrthographicCamera(
      -1 * this.topSceneRange * this.aspect,
      this.topSceneRange * this.aspect,
      this.topSceneRange,
      -1 * this.topSceneRange,
    )
    this.renderer = new WebGLRenderer({ antialias: true })
    this.renderer.name = 'simuscene'
    this.renderer.setPixelRatio(window.devicePixelRatio)
    this.ambientLight = new AmbientLight(0xFFFFFF, 1)
    this.ct = new CoordConverter()
    this.hadmap = new Hadmap(this.ct, this)
    this.axis = new AxesHelper(20)
    this.grid = new GridHelper(10000, 500, 0x101010, 0x101010)
    this.grid.rotation.x = Math.PI / 2
    this.grid.material.depthTest = false
    this.grid.renderOrder = -999
    this.raycaster = new Raycaster()
    this.mouseDownPos = new Vector2()
    this.wrapper = wrapper
    this.cameraRay = new Ray()
    this.groundPlane = new Plane()
    this.groundPlane.normal.set(0, 0, 1)
    this.curCamera = this.camera
    this.LaneStartShift = 0
    this.PedestrianBoxSize = 0.08

    this.cameraTop = this.camera

    this.renderScene = throttle(this.renderScene, 30)

    if (this.Config().useTopCamera) {
      this.curCamera = this.cameraTop
    }

    this.ZOOM_MIN = 0.1
    this.ZOOM_MAX = 1000
    this.cameraTargetOriginalDistance = 1

    this.initEditorControl()

    const signal = this.allSignals().operationMode
    signal.add(this.setOperationMode, this)

    this.state = SceneState.StateEdit

    this.transformControl = new TransformControls(this.curCamera, wrapper)
    this.transformControl.showZ = false

    this.box = new Box3()
    this.pickBox = new PickBox(this.scene)

    this.curOperatedObjPosition = new Vector3()

    this.sceneParser = new SimuSceneParser(this)

    // user input
    this.rButtonDown = false

    this.ModelUpDir = new Vector3(0, 0, 1)
    this.defaultDir = new Vector3(1, 0, 0)
    this.northDir = new Vector3(1, 0, 0) // east in fact
    this.eastDir = new Vector3(1, 0, 0) // east in fact

    // 吸附车道线
    this.snap = GlobalConfig.getSnap()

    // 当前编辑对象
    this.curEditPlanner = null
    this.curEditVehicle = null
    this.curEditPedestrian = null
    this.curPlannerPathEditCurve = null
    this.curVehiclePathEditCurve = null
    this.curPedestrianPathEditCurve = null
    this.curMeasurementEditCurve = null
    this.lastPickObject = null // only used for ego/vehicle/pedestrian end insertion

    // status record
    this.lastZoom = 1
    this.timeoutEditRoute = 0
    this.promiseEditRoute = null
    this.modelsManager = new ScenarioModelsManager({
      type: 'editor',
      catalogs: () => this.catalogs,
      prefix: 'assets/models',
    })
  }

  get catalogs () {
    return {
      ...store.state.catalogs,
      plannerList: store.state.planners.plannerList,
    }
  }

  /**
   * 获取所有消息通道
   * @return {*}
   */
  allSignals () {
    return EditorGlobals.signals
  }

  setSnap (snap) {
    GlobalConfig.setSnap(snap)

    this.snap = !!snap
  }

  Snap () {
    return this.snap
  }

  Config () {
    return GlobalConfig.editor
  }

  /**
   * 初始化orbitcontrol
   */
  initEditorControl () {
    if (this.editorControls) {
      this.editorControls.dispose()
    }
    this.editorControls = new OrbitControls(this.curCamera, this.wrapper.parentNode)
    this.editorControls.maxPolarAngle = Math.PI * 0.5
    this.editorControls.minDistance = this.cameraTargetOriginalDistance * this.ZOOM_MIN
    this.editorControls.maxDistance = this.cameraTargetOriginalDistance * this.ZOOM_MAX
    this.editorControls.name = 'simuscene control'
    this.editorControls.enableRotate = (this.operationMode === OperationMode.ROTATE)
    this.editorControls.keyPanSpeed = 0

    this.editorControls.addEventListener('change', (event) => {
      this.onTopCameraChange(event)
      this.renderScene()
    })

    if (this.operationMode === OperationMode.PAN) { // pan
      this.editorControls.mouseButtons.LEFT = 2 // pan
    } else {
      this.editorControls.mouseButtons.LEFT = 0 // rotate
    }
  }

  onTopCameraChange (force) {
    const distance = this.cameraTop.position.distanceTo(this.editorControls.target)
    const factor = distance / this.cameraTargetOriginalDistance
    if (this.lastZoom !== factor || force) {
      this.onTopCameraZoomChange()
    }
    let degree = this.editorControls.getAzimuthalAngle()
    degree = MathUtils.radToDeg(degree)
    EditorGlobals.signals.cameraChanged.dispatch({ degree })
  }

  onTopCameraZoomChange () {
    const distance = this.cameraTop.position.distanceTo(this.editorControls.target)
    const factor = distance / this.cameraTargetOriginalDistance

    EditorGlobals.signals.zoomView.dispatch({ factor })
    this.calculateMeasurementResult(null, 'noedit') // don't trigger dirty
    this.lastZoom = factor
  }

  /**
   * 初始化拖拽事件回调
   */
  initTransformControl () {
    // this.transformControl.addEventListener('change', () => {
    // console.log('transformControl change', this.transformControl.object)
    // this.pickBox.pick(this.transformControl.object)
    // this.renderScene()
    // })
    this.transformControl.addEventListener('mouseDown', () => {
      const { object } = this.transformControl
      this.curOperatedObjPosition.copy(object.position)

      if (!this.Config().useTopCamera) {
        this.editorControls.enabled = false
      }
    })

    this.transformControl.addEventListener('mouseUp', async (params) => {
      const { object } = this.transformControl
      if (object.position.equals(this.curOperatedObjPosition)) {
        console.log('mouse does not move')
        if (!this.editorControls.enabled) {
          this.editorControls.saveState()
          this.editorControls.reset()
          this.editorControls.enabled = true
        }
        return
      }

      if (object) {
        switch (this.transformControl.getMode()) {
          case 'translate': {
            const types = this.sceneParser.findAnyInModelMap(object)
            const lonlat = this.ct.xyz2lonlat(object.position.x, object.position.y, object.position.z)
            let searchDistance
            if (types?.type === 'obstacle' && types.subType.startsWith('Port_Crane')) {
            // 如果是吊机，则查询道路11米范围
              searchDistance = 11
            }
            const laneInfo = await this.hadmap.getNearbyLaneInfo(lonlat[0], lonlat[1], lonlat[2], searchDistance)
            const m = this.sceneParser.findMeasurementInEndModelMap(object.uuid)
            if (m) {
              this.translateCallback(object, laneInfo, lonlat)
            } else {
              const ele = (
                this.sceneParser.findVehicleInEndModelMap(object.uuid) ||
                this.sceneParser.findPedestrianInEndModelMap(object.uuid)
              )
              const planner = this.sceneParser.findPlannerInEndModelMap(object.uuid)
              let isEditingVehicleTrajectory
              if (ele) {
                isEditingVehicleTrajectory = ele.trajectoryEnabled
              } else if (planner) {
                isEditingVehicleTrajectory = planner.trajectory_enabled
              } else {
                isEditingVehicleTrajectory = false
              }

              if (laneInfo.err !== 0 && !isEditingVehicleTrajectory) {
                object.position.copy(this.curOperatedObjPosition)
                const msg = i18n.t('tips.placeWithoutRoadInfo')
                MessageBox.promptEditorUIMessage('error', msg)
                console.log(`${i18n.t('tips.placeWithoutRoadInfo')} 经纬度：${lonlat[0]}, ${lonlat[1]},${lonlat[2]}`)
                if (!this.editorControls.enabled) {
                  this.editorControls.saveState()
                  this.editorControls.reset()
                  this.editorControls.enabled = true
                }
                return
              }
              console.log(this.curEditPlanner, this.curEditVehicle, 111111)
              if (isEditingVehicleTrajectory || this.objectCanDropped(object, laneInfo, types)) {
                this.curOperatedObjPosition.copy(object.position)
                this.translateCallback(object, laneInfo, lonlat)
              } else {
                object.position.copy(this.curOperatedObjPosition)
                console.log(`${i18n.t('tips.placeWithoutRoadInfo')}${lonlat[0]}, ${lonlat[1]},${lonlat[2]}`)
              }
            }

            this.renderScene()
            break
          }
          case 'rotate':
          case 'scale':
            break
        }
      }

      this.editorControls.saveState()
      this.editorControls.reset()
      this.editorControls.enabled = true
    })

    this.scene.add(this.transformControl)
  }

  /**
   * 初始化simuScene
   * @param wrapper
   * @return {Promise<void>}
   */
  async init (wrapper) {
    this.renderer.setClearColor(0x1F1F27, 1.0)
    this.renderer.setSize(wrapper.clientWidth, wrapper.clientHeight)

    this.cameraTop.position.set(0, 0, 200)
    this.cameraTargetOriginalDistance = this.cameraTop.position.distanceTo(this.editorControls.target)

    // setup scene
    this.scene.add(this.ambientLight)
    this.scene.add(this.axis)
    this.scene.add(this.grid)

    this.initTransformControl()

    wrapper.append(this.renderer.domElement)
    this.renderScene()

    this.sceneParser.l3StateMachine.getL3StateMachine()

    await GlobalConfig.loadConfig()
  }

  objectCanDropped (object, laneInfo, types) {
    const vehicle = (
      this.sceneParser.findVehicleInEndModelMap(object.uuid) ||
      this.sceneParser.findPedestrianInEndModelMap(object.uuid)
    )
    const isEditingVehicleTrajectory = vehicle?.trajectoryEnabled

    if (isEditingVehicleTrajectory) {
      return true
    }

    const delta = 0.05
    let offsetRange = laneInfo.width / 2 + delta
    let info = i18n.t('tips.objOutOfRoad')

    if (types?.type === 'obstacle' && types.subType.startsWith('Port_Crane')) {
      // 如果是吊机，则查询道路11米范围
      offsetRange = 11
    }

    // 行人放置在中心线5米范围内
    const p = this.sceneParser.findPedestrianInModelMap(object.uuid)
    if (p) {
      if (offsetRange < OffsetConst.Pedestrian) {
        offsetRange = OffsetConst.Pedestrian
      }
      info = i18n.t('tips.outOfRoadCenter', { n: '5' })
      // 其他放置在中心线两边2.5米范围内
    } else {
      if (offsetRange < OffsetConst.Other) {
        offsetRange = OffsetConst.Other
        info = i18n.t('tips.outOfRoadCenter', { n: '2.5' })
      }
    }

    if (Math.abs(laneInfo.offset) > offsetRange) {
      MessageBox.promptEditorUIMessage('error', info)
      return false
    }

    const isPlanner = object.uuid === this.curEditPlanner?.model.uuid
    if (laneInfo.type === MapElementType.LANELINK && isPlanner) {
      info = i18n.t('tips.objOutOfRoad')
      MessageBox.promptEditorUIMessage('error', info)
      return false
    }

    const ve = this.sceneParser.findVehicleInEndModelMap(object.uuid)
    // 交通车轨迹点不能放在 lanelink 上
    if (ve && laneInfo.type === MapElementType.LANELINK) {
      info = i18n.t('tips.objOutOfRoad')
      MessageBox.promptEditorUIMessage('error', info)
      return false
    }
    return true
  }

  /**
   * 拖动事件回调，判断是否可以放置然后放置
   * @param object
   * @param laneInfo
   * @param rawLonlat
   * @return {Promise<void>}
   */
  async translateCallback (object, laneInfo, rawLonlat) {
    const [rawLon, rawLat, rawAlt] = rawLonlat
    let fixedX, fixedY, fixedZ
    if (laneInfo?.err === 0) {
      // attach to road
      [fixedX, fixedY, fixedZ] = await this.getFinalPosByLaneInfo(laneInfo, rawLonlat)
    } else {
      [fixedX, fixedY, fixedZ] = this.ct.lonlat2xyz(rawLon, rawLat, rawAlt)
    }
    // traffic vehicle
    let recordPT
    let laneStartPT
    const pickPT = this.ct.xyz2lonlat(object.position.x, object.position.y, object.position.z)
    const { autoFollowRoadDirection } = store.state.system.editor

    const v = this.sceneParser.findVehicleInModelMap(object.uuid)
    if (v) {
      v.startShift = laneInfo.dist - this.LaneStartShift
      v.startOffset = laneInfo.offset
      v.startAlt = laneInfo.alt

      if (!v.trajectoryEnabled || autoFollowRoadDirection) {
        v.start_angle = laneInfo.yaw * MathUtils.RAD2DEG
      }

      const route = this.sceneParser.findRouteInMap(v.routeId)
      if (laneInfo.type === MapElementType.LANE) {
        const { rid, sid, lid } = laneInfo
        const lane = this.hadmap.getLane(rid, sid, lid)
        route.roadId = rid
        route.sectionId = sid
        route.lanelinkId = null
        route.elemType = MapElementType.LANE
        v.laneId = lid

        if (lane.data?.length) {
          const p = lane.getPoint(this.LaneStartShift)
          if (p.length) {
            laneStartPT = this.ct.xyz2lonlat(p[0], p[1], p[2])
          } else {
            const msg = `mapElement ${rid}, ${sid}, ${lid} length to short`
            MessageBox.promptEditorUIMessage('error', msg)
            return
          }
        } else {
          const msg = '该地图元素无数据'
          MessageBox.promptEditorUIMessage('error', msg)
          return
        }

        // lane 记录lane的第一个点
        recordPT = laneStartPT
      } else if (laneInfo.type === MapElementType.LANELINK) {
        const { llid } = laneInfo
        route.roadId = null
        route.sectionId = null
        route.lanelinkId = llid
        route.elemType = MapElementType.LANELINK
        v.laneId = llid

        // lanelink记录当前拾取位置
        recordPT = pickPT
      } else {
        const msg = '地图元素类型错误'
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }

      [route.startLon, route.startLat] = recordPT;
      [route.realStartLon, route.realStartLat] = pickPT

      route.modifyOnePoint(0, object.position, false)
      if ((+route.endLon) < -180) {
        route.modifyOnePoint(1, object.position, false)
      } else {
        route.updateRoutePointsFromCurve(this.ct)
      }

      v.setRoute(route)
      // 事件挪到modifyVisual里面触发了
      this.modifyVehicleVisual(v, [fixedX, fixedY, fixedZ])
      return
    }

    // obstacle
    const o = this.sceneParser.findObstacleInModelMap(object.uuid)
    if (o) {
      o.startShift = laneInfo.dist - this.LaneStartShift
      o.startOffset = laneInfo.offset
      o.startAlt = laneInfo.alt
      o.laneId = laneInfo.lid
      if (autoFollowRoadDirection) {
        o.start_angle = laneInfo.yaw * MathUtils.RAD2DEG
      }
      const route = this.sceneParser.findRouteInMap(o.routeId)
      if (laneInfo.type === MapElementType.LANE) {
        route.roadId = laneInfo.rid
        route.sectionId = laneInfo.sid
        route.lanelinkId = null
        route.elemType = MapElementType.LANE
        o.laneId = laneInfo.lid

        const lane = this.hadmap.getLane(laneInfo.rid, laneInfo.sid, laneInfo.lid)
        if (lane.data && lane.data.length > 0) {
          const P = lane.getPoint(this.LaneStartShift)
          if (P.length > 0) {
            laneStartPT = this.ct.xyz2lonlat(P[0], P[1], P[2])
          } else {
            const msg = `lane ${laneInfo.rid}, ${laneInfo.sid}, ${laneInfo.lid} length to short`
            console.error(msg)
            MessageBox.promptEditorUIMessage('error', msg)
            return
          }
        } else {
          const msg = i18n.t('tips.noDataForRoad')
          MessageBox.promptEditorUIMessage('error', msg)
          return
        }
        recordPT = laneStartPT
      } else if (laneInfo.type === MapElementType.LANELINK) {
        route.roadId = null
        route.sectionId = null
        route.lanelinkId = laneInfo.llid
        route.elemType = MapElementType.LANELINK
        o.laneId = laneInfo.llid

        // lanelink记录当前拾取位置
        recordPT = pickPT
      }
      [route.startLon, route.startLat] = recordPT
      ;[route.realStartLon, route.realStartLat] = pickPT
      this.modifyObstacleVisual(o, [fixedX, fixedY, fixedZ])
      return
    }

    // pedestrian
    const p = this.sceneParser.findPedestrianInModelMap(object.uuid)
    if (p) {
      p.startShift = laneInfo.dist - this.LaneStartShift
      p.startOffset = laneInfo.offset
      p.startAlt = laneInfo.alt
      p.laneId = laneInfo.lid

      if (!p.trajectoryEnabled || autoFollowRoadDirection) {
        p.start_angle = laneInfo.yaw * MathUtils.RAD2DEG
      }

      const route = this.sceneParser.findRouteInMap(p.routeId)
      if (laneInfo.type === MapElementType.LANE) {
        const { rid, sid, lid } = laneInfo
        route.roadId = rid
        route.sectionId = sid
        route.lanelinkId = null
        route.elemType = MapElementType.LANE
        p.laneId = lid

        const lane = this.hadmap.getLane(rid, sid, lid)
        if (lane.data?.length) {
          const p = lane.getPoint(this.LaneStartShift)
          if (p.length) {
            laneStartPT = this.ct.xyz2lonlat(p[0], p[1], p[2])
          } else {
            const msg = `lane ${rid}, ${sid}, ${lid} length to short`
            console.error(msg)
            MessageBox.promptEditorUIMessage('error', msg)
            return
          }
        } else {
          const msg = i18n.t('tips.noDataForRoad')
          MessageBox.promptEditorUIMessage('error', msg)
          return
        }

        recordPT = laneStartPT
      } else if (laneInfo.type === MapElementType.LANELINK) {
        const { llid } = laneInfo
        route.roadId = null
        route.sectionId = null
        route.lanelinkId = llid
        route.elemType = MapElementType.LANELINK
        p.laneId = llid

        // lanelink记录当前拾取位置
        recordPT = pickPT
      } else {
        const msg = '地图元素类型错误'
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }

      [route.startLon, route.startLat] = recordPT;
      [route.realStartLon, route.realStartLat] = pickPT

      route.modifyOnePoint(0, object.position, false)
      if ((+route.endLon) < -180) {
        route.modifyOnePoint(1, object.position, false)
      } else {
        route.updateRoutePointsFromCurve(this.ct)
      }

      p.setRoute(route)
      this.modifyPedestrianVisual(p, [fixedX, fixedY, fixedZ])
      return
    }

    // planner
    const planner = this.sceneParser.findPlannerInModelMap(object.uuid)
    if (planner) {
      const {
        dist,
        offset,
        alt,
        rid,
        sid,
        lid,
        yaw,
      } = laneInfo
      planner.startShift = dist
      planner.startOffset = offset
      planner.startAlt = alt
      planner.laneId = lid
      if (autoFollowRoadDirection) {
        planner.start_angle = yaw * MathUtils.RAD2DEG
      }

      const route = this.sceneParser.findRouteInMap(planner.routeId)
      const lane = this.hadmap.getLane(rid, sid, lid)
      if (!lane?.data?.length) {
        const msg = i18n.t('tips.noDataForRoad')
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }

      [route.startLon, route.startLat] = rawLonlat
      route.roadId = rid
      route.sectionId = sid
      route.startRPM.setPos(object.position.x, object.position.y, object.position.z)
      route.modifyOnePoint(0, object.position, false)
      if ((+route.endLon) < -180) {
        route.modifyOnePoint(1, object.position, false)
      } else {
        route.updateRoutePointsFromCurve(this.ct)
      }
      planner.setRoute(route)
      // 需要先发送数据更改
      await this.modifyPlannerVisual(planner, [fixedX, fixedY, fixedZ])
      return
    }

    const pp = this.sceneParser.findPlannerInEndModelMap(object.uuid)
    if (pp) {
      if (this.Snap() && !pp.trajectoryEnabled) {
        // 吸附车道线且非轨迹追踪模式下时修正轨迹点位置
        [fixedX, fixedY, fixedZ] = this.ct.lonlat2xyz(laneInfo.lon, laneInfo.lat, rawAlt)
      }
      const route = this.sceneParser.findRouteInMap(pp.routeId)
      const idx = route.curve.FindBoxIdx(object.uuid)
      if (+idx !== -1) {
        route.modifyOnePoint(idx, new Vector3(fixedX, fixedY, fixedZ), true, this.hadmap.ct)
      }
      route.updateRoutePointsFromCurve(this.ct)
      pp.setRoute(route)

      // 需要先发数据更改，
      EditorGlobals.signals.dropModifyObject.dispatch({ type: 'planner', data: pp.plannerFormData() })
      this.pickBox.pick(object)
      // 再发送选中消息
      if (+idx !== -1) {
        EditorGlobals.signals.pathNodeSelected.dispatch({ type: 'planner', index: +idx })
        console.log('send ego pathNodeSelected')
      }
    }

    // vehicle path end
    const ve = this.sceneParser.findVehicleInEndModelMap(object.uuid)
    if (ve) {
      if (this.Snap() && !ve.trajectoryEnabled) {
        // 吸附车道线且非轨迹追踪模式下时修正轨迹点位置
        [fixedX, fixedY, fixedZ] = this.ct.lonlat2xyz(laneInfo.lon, laneInfo.lat, rawAlt)
      }
      const route = this.sceneParser.findRouteInMap(ve.routeId)
      const idx = route.curve.FindBoxIdx(object.uuid)
      if (+idx !== -1) {
        route.modifyOnePoint(idx, new Vector3(fixedX, fixedY, fixedZ), true, this.hadmap.ct)
      }
      route.updateRoutePointsFromCurve(this.ct)
      ve.setRoute(route)

      // 需要先发数据更改，
      EditorGlobals.signals.dropModifyObject.dispatch({ type: 'car', data: ve.carFormData() })
      this.pickBox.pick(object)
      // 再发送选中消息
      if (+idx !== -1) {
        EditorGlobals.signals.pathNodeSelected.dispatch({ type: 'vehicle', index: +idx })
        console.log('send vehicle pathNodeSelected')
      }
    }

    // pedestrian path end
    const pe = this.sceneParser.findPedestrianInEndModelMap(object.uuid)
    if (pe) {
      // 行人只支持轨迹追踪模式，故不修正吸附车道线
      const route = this.sceneParser.findRouteInMap(pe.routeId)
      const type = getPedestrianSubType(pe.subType)
      const idx = route.curve.FindBoxIdx(object.uuid)
      if (+idx !== -1) {
        route.modifyOnePoint(idx, new Vector3(fixedX, fixedY, fixedZ), true, this.hadmap.ct)
      }

      route.updateRoutePointsFromCurve(this.ct)
      pe.setRoute(route)
      // 需要先发送数据更改
      EditorGlobals.signals.dropModifyObject.dispatch({ type, data: pe.pedestrianFormData() })
      this.pickBox.pick(object)
      // 再发送选中消息
      if (+idx !== -1) {
        EditorGlobals.signals.pathNodeSelected.dispatch({ type, index: +idx })
        console.log('send pedestrian pathNodeSelected')
      }
    }

    // measurement path end
    const me = this.sceneParser.findMeasurementInEndModelMap(object.uuid)
    if (me) {
      const idx = me.curve.FindBoxIdx(object.uuid)
      if (+idx !== -1) {
        me.curve.modifyPoint(idx, object.position, true)
      }

      me.updatePointsFromCurve(this.ct)
      this.calculateMeasurementResult()
    }

    this.renderScene()
  }

  /**
   * 放置一个轨迹点，暂只支持planner
   * @param type
   * @param id
   * @param index
   * @param rawLonLat
   */
  dropPathEnd (type, id, index, rawLonLat) {
    if (type === 'planner') {
      const planner = this.sceneParser.findPlannerInArr(id)
      const tmp = this.ct.lonlat2xyz(rawLonLat[0], rawLonLat[1], rawLonLat[2])
      planner.route.modifyOnePoint(index, new Vector3(tmp[0], tmp[1], tmp[2]), false)
      this.pickBox.pick(planner.route.curve.Boxes()[index])
    }
    this.renderScene()
  }

  get isEditable () {
    return this.state === SceneState.StateEdit
  }

  State () {
    return this.state
  }

  /**
   * 设置当前编辑中的交通车
   * @param id
   * @return {boolean}
   */
  setCurEditVehicle (id) {
    if (id === undefined) {
      this.curEditVehicle = null
      return true
    }

    const v = this.sceneParser.findVehicleInMap(id)
    if (v) {
      this.curEditVehicle = v
      return true
    }
    return false
  }

  /**
   * 设置当前编辑中的主车
   * @param id
   * @return {boolean}
   */
  setCurEditPlanner (id) {
    if (id === undefined) {
      this.curEditPlanner = null
      return true
    }

    const p = this.sceneParser.findPlannerInArr(id)
    if (p) {
      this.curEditPlanner = p
      return true
    }
    return false
  }

  /**
   * 设置当前编辑中的行人
   * @param id
   * @return {boolean}
   */
  setCurEditPedestrian (id) {
    if (id === undefined) {
      this.curEditPedestrian = null
      return true
    }

    const p = this.sceneParser.findPedestrianInMap(id)
    if (p) {
      this.curEditPedestrian = p
      return true
    }
    return false
  }

  /**
   * 设置当前编辑状态
   * @param state
   * @return {boolean}
   */
  SetState (state) {
    if (state < SceneState.StateEdit || state >= SceneState.StateCount) {
      const info = '设置场景状态错误！'
      MessageBox.promptEditorUIMessage('error', info)
      console.error('set up scene error!')
      return
    }
    // 主车轨迹编辑
    if (this.isEditable && state === SceneState.StatePickEgoCarEnd) {
      let curve = this.sceneParser.getRouteCurve(this.curEditPlanner.routeId)
      if (!curve) {
        curve = new EditorCurve(this, true)
        const vec = this.curEditPlanner.model.position
        const attr = {
          velocity: +this.curEditPlanner.startVelocity,
          gear: 'drive',
        }
        curve.modifyPoint(0, vec, false, attr)
        curve.modifyPoint(1, vec)
      } else {
        const attr = {
          // 非初始点的速度不能为0
          velocity: 6,
          gear: 'drive',
        }
        curve.addOnePoint(attr)
      }

      this.curPlannerPathEditCurve = curve
    } else if (this.isEditable && state === SceneState.StatePickVehicleEnd) {
      // 交通流车轨迹编辑
      let curve = this.sceneParser.getRouteCurve(this.curEditVehicle.routeId)

      if (!curve) {
        curve = new EditorCurve(this, true)
        const vec = this.curEditVehicle.model.position
        curve.modifyPoint(0, vec, false, { velocity: this.curEditVehicle.startVelocity, gear: 'drive' })
        curve.modifyPoint(1, vec, false, { velocity: this.curEditVehicle.startVelocity, gear: 'drive' })
      } else {
        const attr = {}
        attr.velocity = +this.curEditVehicle.startVelocity
        attr.gear = 'drive'
        curve.addOnePoint(attr)
      }
      this.curVehiclePathEditCurve = curve
      // 行人轨迹编辑
    } else if (this.isEditable && state === SceneState.StatePickPedestrianEnd) {
      let curve = this.sceneParser.getRouteCurve(this.curEditPedestrian.routeId)
      if (!curve) {
        const withAttr = this.curEditPedestrian.type === 'machine'
        const vec = this.curEditPedestrian.model.position
        curve = new EditorCurve(this, withAttr, this.PedestrianBoxSize)
        if (!withAttr) {
          curve.modifyPoint(0, vec, false)
          curve.modifyPoint(1, vec)
        } else {
          curve.modifyPoint(0, vec, false, {
            velocity: this.curEditPedestrian.startVelocity,
            gear: 'drive',
          })
          curve.modifyPoint(1, vec, false, {
            velocity: this.curEditPedestrian.startVelocity,
            gear: 'drive',
          })
        }
      } else {
        let attr
        if (curve.m_withAttr) {
          attr = {
            velocity: this.curEditPedestrian.startVelocity,
            gear: 'drive',
          }
        }
        curve.addOnePoint(attr)
      }
      this.curPedestrianPathEditCurve = curve
      // 测距轨迹
    } else if (this.isEditable && state === SceneState.StatePickMeasurementEnd) {
    } else {
    }

    if (this.isEditingPath(state) && this.transformControl) {
      this.transformControl.enabled = false
    }

    // 如果已经是设置轨迹点状态，则不让再设置轨迹点
    if (state !== SceneState.StateEdit && this.state !== SceneState.StateEdit) {
      return
    }
    console.info('set state ', state)
    this.state = state
  }

  /**
   * 把相机设置到指定位置的上方
   * @param position
   * @param xOffset
   * @param yOffset
   * @param zOffset
   */
  lookDownAt (position, { xOffset = 0, yOffset = 0, zOffset = 0 } = {}) {
    this.curCamera.position.x = position.x + xOffset
    this.curCamera.position.y = position.y + yOffset
    this.curCamera.position.z = position.z + zOffset
  }

  change2TopCamera (updateMeasurement = true) {
    GlobalConfig.setUseTopCameraEditor(true)

    // this.lButtonDown = false
    this.topWrapper = this.wrapper
    this.curCamera = this.cameraTop

    const offset = this.curCamera.position.distanceTo(this.editorControls.target)
    this.curCamera.up.set(0, 0, 1)
    this.lookDownAt(this.editorControls.target, { zOffset: offset })

    const target = new Vector3().copy(this.editorControls.target)
    this.editorControls.dispose()
    this.initEditorControl()
    this.editorControls.dollyCentered = true // 缩放时以鼠标为中心
    this.editorControls.screenSpacePanning = true
    this.editorControls.rotateUpFactor = 0
    this.editorControls.target.copy(target)

    this.curCamera.position.y = target.y - 0.0000000001

    this.editorControls.update()

    this.renderScene()

    if (updateMeasurement) {
      this.calculateMeasurementResult(null, 'noedit')
    }
  }

  change2PerspectiveCamera () {
    GlobalConfig.setUseTopCameraEditor(false)

    const radians = 70 * Math.PI / 180
    // this.lButtonDown = false
    this.curCamera = this.camera
    this.transformControl.camera = this.curCamera

    const offset = this.curCamera.position.distanceTo(this.editorControls.target)
    this.curCamera.up.set(0, 0, 1)
    this.lookDownAt(this.editorControls.target, {
      yOffset: -offset * Math.sin(radians),
      zOffset: offset * Math.cos(radians),
    })

    const target = new Vector3().copy(this.editorControls.target)
    this.editorControls.dispose()
    this.initEditorControl()
    this.editorControls.dollyCentered = false // 缩放时以鼠标为中心
    this.editorControls.screenSpacePanning = false
    this.editorControls.target.copy(target)
    this.editorControls.update()

    this.editorControls.enabled = true
    this.renderScene()
    this.calculateMeasurementResult(null, 'noedit')
  }

  resize (width, height) {
    if (width < 1 || height < 1) {

    }
  }

  async loadHadmap (hdmapName, hdmapDesc, payload) {
    if (this.hadmap.loaded() === true) {
      const msg = '场景地图已经加载，不能再次加载地图'
      MessageBox.promptEditorUIMessage('error', msg)
      return false
    }

    const hm = this.hadmap.findHadmap(hdmapName)

    if (!hm) {
      const msg = i18n.t('tips.mapConfigInfoNotExists', [hdmapName])
      console.error(msg)
      MessageBox.promptEditorUIMessage('error', msg)
      return false
    }

    this.sceneParser.mapName = hdmapName
    this.sceneParser.mapDesc = hdmapDesc
    this.ct.setRef(hm.mapRefLon, hm.mapRefLat, hm.mapRefAlt)
    this.hadmap.setSampleDistance(20)

    let { mapfile } = this.sceneParser
    if (!mapfile) {
      mapfile = this.sceneParser.mapfile = new MapFile()
    }

    mapfile.lon = this.ct.refLon
    mapfile.lat = this.ct.refLat
    mapfile.alt = this.ct.refAlt
    mapfile.unrealLevelIndex = +hm.unrealLevelIndex
    mapfile.mapName = hdmapName

    let result
    try {
      if (GlobalConfig.isCloud) {
        result = await this.hadmap.loadMapToSceneLocal(hdmapName, this.scene, this.sceneParser)
        console.log(result)
        // 修复云端版level
        this.sceneParser.mapfile.unrealLevelIndex = +hm.unrealLevelIndex
      } else {
        result = await this.hadmap.loadMapToSceneLocal(hdmapName, this.scene, this.sceneParser)
      }
    } catch (err) {
      console.error(err)
    }

    if (!result) {
      return false
    }

    this.updateGrid()
    return true
  }

  hadmapLoaded () {
    return this.hadmap.loaded()
  }

  getVehicleIDs () {
    return this.sceneParser.vehicles.map(vehicle => vehicle.id)
  }

  getFollowedVehicleIDs (vehicleId) {
    const followedVehicleIDs = []
    vehicleId = `${vehicleId}`

    this.sceneParser.vehicles.forEach((vehicle) => {
      const vehicleFollowed = vehicle.follow ? `${vehicle.follow}` : ''
      // 判断跟随车辆是否为输入车辆
      if (vehicleId === vehicleFollowed) {
        followedVehicleIDs.push(`${vehicle.id}`)
      }
    })

    return followedVehicleIDs
  }

  getCanBeFollowedVehicleIDs (vehicleId) {
    // 跟车只能选择主车
    const vehicleIds = []
    vehicleId = `${vehicleId}`

    // 1, 放入主车
    vehicleIds.push(...this.sceneParser.planners.map(p => ({ id: p.id, type: 'planner' })))

    // 2, 放入交通流车
    this.sceneParser.vehicles.forEach((vehicle) => {
      const curID = `${vehicle.id}`
      // 非当前交通流车
      if (vehicleId !== curID) {
        // 存在参考车
        if (vehicle.follow && vehicle.follow.length > 0) {
          let follow = vehicle.follow
          const trafficCount = this.sceneParser.vehicles.length
          // 2.1, 主车
          if (follow.includes('ego')) {
            vehicleIds.push({ id: curID, type: 'car' })

            // 2.2, 交通流车
          } else {
            // 是否可以被follow的标记
            let canFollowed = true

            // 2.2.1, 获取follow链，用于后续判断
            const follows = []
            while (follow?.length > 0 && follows.length <= trafficCount) {
              // 主车是跟车链的最顶端，因此结束循环
              if (follow.includes('ego')) {
                follow = ''
                continue
              }

              const followId = Number(follow.match(/\d+/g)[0])

              const v = this.sceneParser.findVehicleInMap(followId)
              if (v) {
                follows.push(follow)
                follow = v.follow
              } else {
                follow = ''
              }
            }

            // 2.2.2, followsl链长度大于场景内交通流车总数，说明存在环,不能被follow
            if (follows.length > trafficCount) {
              const msg = `场景中存在跟随车辆循环的情况`
              MessageBox.promptEditorUIMessage('error', msg)
              // 不能被follow
              canFollowed = false
              // 2.2.3 不存在环的情况下
            } else {
              // 2.2.3.1, 是否是其他车辆所follow的车, 是的话不能被follow
              // 比如 car2 follow car1 ，car1 就不能 follow car2
              // 比如 car2 follow car1，car3 follow car2， 那么 car1 就不能 follow car3
              const bOtherParents = follows.some((f) => {
                const followId = Number(f.match(/\d+/g)[0])
                return Number(vehicleId) === followId
              })
              if (bOtherParents) {
                canFollowed = false
              }
            }

            // 放入follow数组用于被返回
            if (canFollowed) {
              vehicleIds.push({ id: curID, type: 'car' })
            }
          }
          // 不存在参考车，判断是否符合放入要求
        } else {
          // 只有自定义车辆才能被作为参考车
          if (vehicle.customBehavior) {
            vehicleIds.push({ id: vehicle.id, type: 'car' })
          }
        }
      }
      // 当前车也不用考虑
    })
    return vehicleIds
  }

  getPedestrianIDsBySubType (subType) {
    return this.sceneParser.pedestrians
      .filter(pedestrian => getPedestrianSubType(pedestrian.subType) === subType)
      .map(p => p.id)
  }

  getObstacleIDs () {
    return this.sceneParser.obstacles.map(obstacle => obstacle.id)
  }

  getSignlightIDs () {
    return this.sceneParser.signlights.map(signlight => signlight.id)
  }

  getPlannerIDs () {
    return this.sceneParser.planners.map(planner => planner.id)
  }

  /* 获取该路径下所有的lane id，
        isAll:true 获取所有的laneid; false 获取不为空的lane id
    */
  /**
   * 获取该路径下所有的lane id
   * @param {number} id lane id
   * @param {boolean} [isAll]
   * @returns {[]}
   */
  getLaneIDsByRoute (id, isAll = false) {
    const r = this.sceneParser.routesMap.get(`${id}`)
    if (!r) {
      return []
    }

    const sec = this.hadmap.getSection(r.roadId, r.sectionId)
    if (!sec) {
      return []
    }

    return sec.lanes.filter(lane => isAll || (lane.data && lane.data.length)).map(lane => lane.laneId)
  }

  getL3StateMachineRawData () {
    return this.sceneParser.l3StateMachine.getL3StateMachineRawData()
  }

  getL3StateMachineData () {
    return this.sceneParser.l3StateMachine.getL3StatesData()
  }

  setL3StateMachineData (l3StateMachine) {
    return this.sceneParser.l3StateMachine.setL3StatesData(l3StateMachine)
  }

  /**
   * 让相机聚焦到一个物体上
   * @param {Vector3}position
   * @param {Vector3}offset
   */
  focusPosition (position, offset) {
    if (!offset) {
      offset = new Vector3(0, 0, 120)
    }
    if (!this.Config().useTopCamera) {
      offset.y = -120
    }

    this.curCamera.position.copy(position).add(offset)
    this.curCamera.lookAt(position)
    this.editorControls.target.copy(position)
    this.editorControls.update()
  }

  /**
   * focus camera to the planner
   */
  focusPlanner (id) {
    if (!this.sceneParser.planners.length) {
      return
    }
    let planner
    if (id === undefined) {
      planner = this.sceneParser.planners[0]
    } else {
      planner = this.sceneParser.planners.find(p => p.id === id)
    }
    try {
      this.focusPosition(planner.model.position)
      this.onTopCameraChange(true)
    } catch (error) {
      console.warn('定位到主车视角失败：', error)
    }
  }

  /**
   * 把相机聚焦到某个元素
   * @param type
   * @param id
   * @param idx
   */
  locateCameraView2Obj (type, id, idx) {
    if (type === 'acc') {
    } else if (type === 'merge') {
    } else if (type === 'route') {
      const r = this.sceneParser.findRouteInMap(id)
      if (r) {
        if (!r.startRPM) {
          console.warn(`data error! rout ${id} not exist in this map`)
          const msg = `数据错误，路径${id}在场景中没定义`
          MessageBox.promptEditorUIMessage('error', msg)
          return
        }

        this.focusPosition(new Vector3(r.startRPM.x - 20, r.startRPM.y, r.startRPM.z))
      }
    } else if (type === 'car') {
      const v = this.sceneParser.findVehicleInMap(id)
      if (v) {
        this.focusPosition(v.model.position)
      }
    } else if (type === 'planner') {
      this.focusPlanner(id)
    } else if (type === 'signlight') {
      const s = this.sceneParser.findSignlightInMap(id)
      if (s) {
        this.focusPosition(s.model.position)
      }
    } else if (['pedestrian', 'animal', 'bike', 'moto', 'machine'].includes(type)) {
      const p = this.sceneParser.findPedestrianInMap(id)
      if (p) {
        this.focusPosition(p.model.position)
      }
    } else if (type === 'obstacle') {
      const o = this.sceneParser.findObstacleInMap(id)
      if (o) {
        this.focusPosition(o.model.position)
      }
    } else if (type === 'egopath_end') {
      const p = this.sceneParser.findPlannerInArr(id)
      if (+idx === 0) {
        this.focusPosition(p.model.position)
      } else {
        if (p.curve) {
          const len = p.curve.Boxes().length
          if (idx < len) {
            const box = p.curve.Boxes()[idx]
            this.focusPosition(box.position)
          } else {
            console.error(`ego path end index error. id:  ${idx} not exist`)
          }
        }
      }
    } else if (type === 'vehiclepath_end') {
      const v = this.sceneParser.findVehicleInMap(id)
      if (v) {
        if (+idx === 0) {
          this.focusPosition(v.model.position)
        } else {
          const route = this.sceneParser.findRouteInMap(v.routeId)

          if (route.curve) {
            const len = route.curve.Boxes().length
            if (idx < len) {
              const box = route.curve.Boxes()[idx]
              this.focusPosition(box.position)
            } else {
              console.error(`vehicle ${id} path end index error. idx:  ${idx} not exist`)
            }
          }
        }
      }
    } else if ([
      'pedestrianpath_end',
      'animalpath_end',
      'bikepath_end',
      'motopath_end',
      'machinepath_end',
    ].includes(type)) {
      const p = this.sceneParser.findPedestrianInMap(id)
      if (p) {
        if (+idx === 0) {
          this.focusPosition(p.model.position)
        } else {
          const route = this.sceneParser.findRouteInMap(p.routeId)

          if (route.curve) {
            const len = route.curve.Boxes().length
            if (idx < len) {
              const box = route.curve.Boxes()[idx]
              this.focusPosition(box.position)
            } else {
              console.error(`pedestrian ${id} path end index error. idx:  ${idx} not exist`)
            }
          }
        }
      }
    } else if (type === 'road') {
    } else if (type === 'lane') {
    } else if (type === 'laneboundary') {
    } else if (type === 'lanelink') {
    } else if (type === 'mapobject') {
    } else if (type === 'roadmesh') {
    } else if (type === 'grid') {
    } else {
      console.error('unknown object type ', type)
    }

    this.renderScene()
  }

  getAccIDs () {
    return this.sceneParser.accs.map(acc => acc.id)
  }

  getMergeIDs () {
    return this.sceneParser.merges.map(merge => merge.id)
  }

  finialize (elem) {
    if (elem.childElementCount > 0) {
      elem.removeChild(elem.children[0])
    }

    const signal = this.allSignals().operationMode
    signal.remove(this.setOperationMode, this)
  }

  renderScene () {
    this.renderer.render(this.scene, this.curCamera)
  }

  /**
   * 获取鼠标位置在场景中的位置
   * @param x
   * @param y
   * @return {Vector3}
   */
  getPointerPosInScene (x, y) {
    const vec = new Vector3()
    vec.set(
      (x / this.wrapper.clientWidth) * 2 - 1,
      ((this.wrapper.clientHeight - y) / this.wrapper.clientHeight) * 2 - 1,
      0.5,
    )

    vec.unproject(this.curCamera)

    const ux = (x / this.wrapper.clientWidth) * 2 - 1
    const uy = ((this.wrapper.clientHeight - y) / this.wrapper.clientHeight) * 2 - 1
    this.raycaster.setFromCamera({ x: ux, y: uy }, this.curCamera)

    const intersects = this.raycaster.intersectObject(this.hadmap.laneGroup, true)

    if (intersects.length > 0) {
      return intersects[0].point
    } else {
      vec.sub(this.curCamera.position)
      const ray = new Ray(this.curCamera.position, vec)
      this.cameraRay.origin.set(this.curCamera.position)
      this.cameraRay.direction.set(vec)
      ray.intersectPlane(this.groundPlane, vec)
    }

    return vec
  }

  /**
   * 获取鼠标位置在车道上的位置
   * @param x
   * @param y
   * @return {null|Vector3}
   */
  getPointerPosInLane (x, y) {
    const ux = (x / this.wrapper.clientWidth) * 2 - 1
    const uy = ((this.wrapper.clientHeight - y) / this.wrapper.clientHeight) * 2 - 1
    this.raycaster.setFromCamera({ x: ux, y: uy }, this.curCamera)

    const intersects = this.raycaster.intersectObject(this.hadmap.laneGroup, true)

    if (intersects.length > 0) {
      return intersects[0].point
    }
    return null
  }

  // intersects with the grid
  getPointerPosInGround (x, y) {
    const vec = new Vector3()
    vec.set(
      (x / this.wrapper.clientWidth) * 2 - 1,
      ((this.wrapper.clientHeight - y) / this.wrapper.clientHeight) * 2 - 1,
      0.5,
    )

    vec.unproject(this.curCamera)

    vec.sub(this.curCamera.position)
    const ray = new Ray(this.curCamera.position, vec)
    this.cameraRay.origin.set(this.curCamera.position)
    this.cameraRay.direction.set(vec)
    ray.intersectPlane(this.groundPlane, vec)
    return vec
  }

  /**
   * 获取鼠标位置在地面上的位置
   * @param x
   * @param y
   * @param height
   * @return {Vector3}
   */
  getPointerPosInPickGroundWithHeight (x, y, height) {
    const pickPlane = new Plane(new Vector3(0, 0, 1), -height)
    const ux = (x / this.wrapper.clientWidth) * 2 - 1
    const uy = ((this.wrapper.clientHeight - y) / this.wrapper.clientHeight) * 2 - 1
    this.raycaster.setFromCamera({ x: ux, y: uy }, this.curCamera)

    const vec = new Vector3()
    this.raycaster.ray.intersectPlane(pickPlane, vec)
    return vec
  }

  /*
  * 路径画完了
  * */
  doTrackFinish () {
    if (this.isEditingPath(this.State()) && this.transformControl) {
      this.transformControl.enabled = true
    }

    if (this.State() === SceneState.StatePickEgoCarEnd) {
      const lastIdx = this.curPlannerPathEditCurve.Points().length
      this.curPlannerPathEditCurve.removePoint(lastIdx - 1)

      const { route } = this.curEditPlanner
      if (!route) {
        console.error('route not exist')
        return
      }

      if (route.curve) {
        if (this.selectedExist()) {
          route.curve.Boxes().some((value) => {
            if (this.isObjectPartSelected(value)) {
              this.cancelPickedObject()
              return true
            }
            return false
          }, this)
        }
      }
      route.curve = this.curPlannerPathEditCurve
      this.curPlannerPathEditCurve = null
      this.sceneParser.addPlannerEndModel(this.curEditPlanner)
      route.updateRoutePointsFromCurve(this.ct)
      this.curEditPlanner.setRoute(route)
      route.curve.setTrackEnabled(this.curEditPlanner.trajectory_enabled)
      EditorGlobals.signals.dropModifyObject.dispatch({ type: 'planner', data: this.curEditPlanner.plannerFormData() })
      this.setCurEditPlanner()
      this.SetState(SceneState.StateEdit)
    } else if (this.State() === SceneState.StatePickVehicleEnd) {
      const lastIdx = this.curVehiclePathEditCurve.Points().length
      this.curVehiclePathEditCurve.removePoint(lastIdx - 1)

      const route = this.sceneParser.findRouteInMap(this.curEditVehicle.routeId)
      if (!route) {
        console.error('route not exist')
        return
      }

      if (route.curve) {
        if (this.selectedExist()) {
          route.curve.Boxes().some((value) => {
            if (this.isObjectPartSelected(value)) {
              this.cancelPickedObject()
              return true
            }
            return false
          }, this)
        }
      }

      route.curve = this.curVehiclePathEditCurve
      this.curVehiclePathEditCurve = null
      this.sceneParser.addVehicleEndModel(this.curEditVehicle)
      route.updateRoutePointsFromCurve(this.ct)
      this.curEditVehicle.setRoute(route)

      route.curve.setTrackEnabled(this.curEditVehicle.trajectoryEnabled)
      EditorGlobals.signals.dropModifyObject.dispatch({ type: 'car', data: this.curEditVehicle.carFormData() })
      this.setCurEditVehicle()

      this.SetState(SceneState.StateEdit)
    } else if (this.State() === SceneState.StatePickPedestrianEnd) {
      const lastIdx = this.curPedestrianPathEditCurve.Points().length
      this.curPedestrianPathEditCurve.removePoint(lastIdx - 1)

      const route = this.sceneParser.findRouteInMap(this.curEditPedestrian.routeId)
      if (!route) {
        console.error('route not exist')
        return
      }

      if (route.curve) {
        if (this.selectedExist()) {
          route.curve.Boxes().some((value) => {
            if (this.isObjectPartSelected(value)) {
              this.cancelPickedObject()
              return true
            }
            return false
          }, this)
        }
      }

      route.curve = this.curPedestrianPathEditCurve

      this.curPedestrianPathEditCurve = null
      this.sceneParser.addPedestrianEndModel(this.curEditPedestrian)
      route.updateRoutePointsFromCurve(this.ct)
      this.curEditPedestrian.setRoute(route)

      if (this.curEditPedestrian.trajectoryEnabled) {
        route.curve.setTrackEnabled(true)
      }

      EditorGlobals.signals.dropModifyObject.dispatch({
        type: getPedestrianSubType(this.curEditPedestrian.subType),
        data: this.curEditPedestrian.pedestrianFormData(),
      })

      this.setCurEditPedestrian()

      this.SetState(SceneState.StateEdit)
    } else if (this.State() === SceneState.StatePickMeasurementEnd) {
      const lastIdx = this.curMeasurementEditCurve.Points().length
      this.curMeasurementEditCurve.removePoint(lastIdx - 1)

      const measurement = new Measurement()
      measurement.curve = this.curMeasurementEditCurve
      this.curMeasurementEditCurve = null
      measurement.updatePointsFromCurve(this.ct)
      this.sceneParser.addMeasurement(measurement)
      this.sceneParser.addMeasurementEndModel(measurement)
      this.SetState(SceneState.StateEdit)
      this.calculateMeasurementResult()
    }
  }

  onKeyDown (keyboardEvent) {
    if (keyboardEvent.key === 'Enter') {
      this.doTrackFinish()
      this.renderScene()
    }
  }

  onTopCameraMouseButtonDown (event) {
    // left button
    if (event.button === 0) {
      // middle button
    } else if (event.button === 1) {
      // right button
    } else if (event.button === 2) {
      this.rButtonDown = true
    }
  }

  async onTopCameraMouseButtonUp (event) {
    console.log('onTopCameraMouseButtonUp', event)

    let { offsetX, offsetY, ctrlKey, button, target } = event
    // left button
    if (button === 0) {
      let vec = new Vector3()
      do {
        offsetX += target.offsetLeft
        offsetY += target.offsetTop
        target = target.parentNode
      } while (target && target !== this.wrapper.parentNode)

      vec = this.getPointerPosInLane(offsetX, offsetY)

      if (this.hadmapLoaded()) {
        if (this.isEditingRoute()) {
          if (this.timeoutEditRoute !== 0) {
            clearTimeout(this.timeoutEditRoute)
            this.promiseEditRoute = this.handleRouteEdit(event)
          } else {
            this.timeoutEditRoute = setTimeout(() => {
              this.promiseEditRoute = this.handleRouteEdit(event)
            }, 150)
          }
        } else {
          // 进行轨迹点插入操作
          if ((this.curEditPlanner || this.curEditPedestrian || this.curEditVehicle) && ctrlKey) {
            this.mouseDownPos.set(
              (event.offsetX / this.wrapper.clientWidth) * 2 - 1,
              -(event.offsetY / this.wrapper.clientHeight) * 2 + 1,
            )
            this.raycaster.setFromCamera(this.mouseDownPos, this.curCamera)

            const posInLane = this.getPointerPosInLane(event.offsetX, event.offsetY)

            let line = null
            let route = null
            let pedestrian = null
            let vehicle = null
            let planner = null
            let trajectoryEnabled = false

            if (this.lastPickObject?.uuid) {
              const uuid = this.lastPickObject.uuid
              pedestrian = (
                this.sceneParser.findPedestrianInModelMap(uuid) ||
                this.sceneParser.findPedestrianInEndModelMap(uuid)
              )
              vehicle = this.sceneParser.findVehicleInModelMap(uuid) || this.sceneParser.findVehicleInEndModelMap(uuid)
              planner = this.sceneParser.findPlannerInModelMap(uuid) || this.sceneParser.findPlannerInEndModelMap(uuid)
              if (pedestrian) {
                route = this.sceneParser.findRouteInMap(pedestrian.routeId)
                line = route.curve.centerLine()
                trajectoryEnabled = pedestrian.trajectoryEnabled
              } else if (vehicle) {
                route = this.sceneParser.findRouteInMap(vehicle.routeId)
                line = route.curve.centerLine()
                trajectoryEnabled = vehicle.trajectoryEnabled
              } else if (planner) {
                line = planner.curve.centerLine()
                trajectoryEnabled = this.curEditPlanner.trajectory_enabled
              }
            }

            if (line) {
              if (!vec) {
                vec = this.getPointerPosInGround(offsetX, offsetY)
              }
              const tmp = this.ct.xyz2lonlat(vec.x, vec.y, vec.z)
              const laneInfo = await this.hadmap.getNearbyLaneInfo(tmp[0], tmp[1], tmp[2])
              const intersects = this.raycaster.intersectObject(line)

              if (intersects.length) {
                if (!trajectoryEnabled) {
                  // 非轨迹跟踪的情况下才整这些乱七八糟的判断
                  if (laneInfo.type === MapElementType.LANELINK && (vehicle || pedestrian)) {
                    const msg = i18n.t('tips.objOutOfRoad')
                    MessageBox.promptEditorUIMessage('error', msg)
                    return
                  }

                  if (+laneInfo.err !== 0 || !posInLane) {
                    const msg = i18n.t('tips.trajectoryOutsideRoad')
                    MessageBox.promptEditorUIMessage('error', msg)
                    return
                  }

                  let offsetRange = 0
                  if (laneInfo.type === MapElementType.LANELINK) {
                    offsetRange = OffsetConst.Other
                  } else if (laneInfo.type === MapElementType.LANE) {
                    offsetRange = laneInfo.width / 2 < OffsetConst.Other ? OffsetConst.Other : laneInfo.width / 2
                  } else {
                    offsetRange = OffsetConst.Other
                  }
                  const offset = laneInfo.offset
                  if (Math.abs(offset) > offsetRange) {
                    const msg = i18n.t('tips.objOutOfRoad')
                    MessageBox.promptEditorUIMessage('error', msg)
                    return
                  }
                }

                const [{ index }] = intersects
                if (vehicle) {
                  if (!route) {
                    console.error('route not exist')
                    return
                  }

                  // 吸附车道线时，修正vec
                  if (this.Snap() && !trajectoryEnabled) {
                    tmp[0] = laneInfo.lon
                    tmp[1] = laneInfo.lat
                    ;[vec.x, vec.y, vec.z] = this.ct.lonlat2xyz(...tmp)
                  }

                  this.insertOneRoutePoint(
                    'car',
                    vehicle,
                    route,
                    index + 1,
                    vec.x,
                    vec.y,
                    vec.z,
                  ) // fix height
                  EditorGlobals.signals.dropModifyObject.dispatch({ type: 'car', data: vehicle.carFormData() })
                } else if (pedestrian) {
                  if (!route) {
                    console.error('route not exist')
                    return
                  }
                  this.insertOneRoutePoint(
                    'pedestrian',
                    pedestrian,
                    route,
                    index + 1,
                    vec.x,
                    vec.y,
                    vec.z,
                  ) // fix height
                  EditorGlobals.signals.dropModifyObject.dispatch({
                    type: 'pedestrian',
                    data: pedestrian.pedestrianFormData(),
                  })
                } else if (planner) {
                  // 吸附车道线时，修正vec
                  if (this.Snap() && !trajectoryEnabled) {
                    tmp[0] = laneInfo.lon
                    tmp[1] = laneInfo.lat
                    ;[vec.x, vec.y, vec.z] = this.ct.lonlat2xyz(...tmp)
                  }
                  this.insertOneRoutePoint(
                    'planner',
                    planner,
                    route,
                    index + 1,
                    vec.x,
                    vec.y,
                    vec.z,
                  )
                }
              }
            }
          }

          if (this.State() === SceneState.StateEdit) {
            if (ctrlKey) {
              if (!vec) {
                vec = this.getPointerPosInGround(offsetX, offsetY)
              }

              this.mouseDownPos.set(
                (event.offsetX / this.wrapper.clientWidth) * 2 - 1,
                -(event.offsetY / this.wrapper.clientHeight) * 2 + 1,
              )
              this.raycaster.setFromCamera(this.mouseDownPos, this.curCamera)

              this.sceneParser.measurements.some((value) => {
                if (!value.curve) {
                  return false
                }

                const line = value.curve.centerLine()
                const intersects = this.raycaster.intersectObject(line)
                if (intersects.length > 0) {
                  this.insertOneMeasurementPoint(value, intersects[0].index + 1, vec.x, vec.y, vec.z)
                  return true
                }

                return false
              })

              this.calculateMeasurementResult(event)
            }
          }
        }
      }
      // middle button
    } else if (button === 1) {
      // right button
    } else if (button === 2) {
      event.preventDefault()
      this.rButtonDown = false
      this.calculateMeasurementResult(null, 'noedit')
    }
  }

  /**
   * 给当前route添加一个点
   * @param event
   * @return {Promise<boolean>}
   */
  async handleRouteEdit (event) {
    this.timeoutEditRoute = 0
    if (!this.isEditingRoute()) {
      return
    }
    let { offsetX, offsetY, target } = event
    do {
      offsetX += target.offsetLeft
      offsetY += target.offsetTop
      target = target.parentNode
    } while (target && target !== this.wrapper.parentNode)
    const {
      StatePickEgoCarEnd,
      StatePickVehicleEnd,
      StatePickPedestrianEnd,
      StatePickMeasurementEnd,
    } = SceneState
    const {
      Pedestrian: OffsetPedestrian,
      Other: OffsetOther,
    } = OffsetConst

    // 先获取Ray和车道geometry的交点
    let endPosX, endPosY, endPosZ, laneInfo, curve
    let trajectoryEnabled = false
    const state = this.State()
    switch (state) {
      case StatePickEgoCarEnd:
        trajectoryEnabled = this.curEditPlanner.trajectory_enabled
        curve = this.curPlannerPathEditCurve
        break
      case StatePickVehicleEnd:
        trajectoryEnabled = this.curEditVehicle.trajectoryEnabled
        curve = this.curVehiclePathEditCurve
        break
      case StatePickPedestrianEnd:
        trajectoryEnabled = this.curEditPedestrian.trajectoryEnabled
        curve = this.curPedestrianPathEditCurve
        break
      case StatePickMeasurementEnd:
      // 测量点状态视同轨迹追踪
        trajectoryEnabled = true
        curve = this.curMeasurementEditCurve
        break
      default:
        console.error('wrong state')
        return
    }
    let vec = this.getPointerPosInLane(offsetX, offsetY)
    // vec, endPosX, endPosY, endPosZ 下面分情况设置
    if (!trajectoryEnabled) {
      if (!vec) {
        // 主车、交通车的轨迹点只能在道路上
        const msg = i18n.t('tips.trajectoryOutsideRoad')
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }
      const tmp = this.ct.xyz2lonlat(vec.x, vec.y, vec.z)
      laneInfo = await this.hadmap.getNearbyLaneInfo(tmp[0], tmp[1], tmp[2])
      if (laneInfo.err !== 0 || laneInfo.type === MapElementType.LANELINK) {
        // 主车、交通车的轨迹点只能在道路上
        const msg = i18n.t('tips.trajectoryOutsideRoad')
        console.warn('没有查询到道路信息: ', tmp)
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }
      ;[endPosX, endPosY] = tmp
      endPosZ = laneInfo.alt
      if (this.Snap()) {
        endPosX = laneInfo.lon
        endPosY = laneInfo.lat
        ;[vec.x, vec.y] = this.ct.lonlat2xyz(endPosX, endPosY, 0)
      }

      const offset = laneInfo.offset
      let offsetRange = 0
      const typedOffset = state === StatePickPedestrianEnd ? OffsetPedestrian : OffsetOther
      if (laneInfo.type === MapElementType.LANE) {
        offsetRange = laneInfo.width / 2 < typedOffset ? typedOffset : laneInfo.width / 2
      } else {
        offsetRange = OffsetOther
      }
      if (Math.abs(offset) > offsetRange) {
        const msg = i18n.t('tips.trajectoryOutsideRoad')
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }
    } else {
      if (!vec) {
        // 有曲线且用户点到道路以外时，使用上一个点的高度
        if (curve && curve.Points().length > 1) {
          const points = curve.Points()
          const prevPoint = points[points.length - 2]
          endPosZ = prevPoint.z
        } else {
          // 新建测量的时候，没有curve。提示用户从没有道路的地方测量会导致误差
          endPosZ = 0
          if (state === StatePickMeasurementEnd) {
            const msg = i18n.t('tips.startMeasurementOutsideRoad')
            MessageBox.promptEditorUIMessage('warn', msg)
          } else {
            const msg = i18n.t('tips.startPointOutsideRoad')
            MessageBox.promptEditorUIMessage('error', msg)
          }
        }
        vec = this.getPointerPosInPickGroundWithHeight(offsetX, offsetY, endPosZ)
      }
      ;[endPosX, endPosY, endPosZ] = this.ct.xyz2lonlat(vec.x, vec.y, vec.z)
    }
    let route
    switch (state) {
      case StatePickEgoCarEnd: {
        const egoRoute = this.sceneParser.findRouteInMap(this.curEditPlanner.routeId)
        if (!egoRoute) {
          console.error(`route ${this.curEditPlanner.routeId} not exist`)
          const msg = i18n.t('tips.noPlanner')
          MessageBox.promptEditorUIMessage('error', msg)
          return false
        }
        egoRoute.endLon = endPosX
        egoRoute.endLat = endPosY
        const planner = this.curEditPlanner
        const pos = {
          lon: endPosX.toFixed(8),
          lat: endPosY.toFixed(8),
          alt: endPosZ.toFixed(3),
          velocity: 6,
          gear: 'drive',
        }
        if (trajectoryEnabled) {
          pos.frontwheel = null
          pos.accs = null
          pos.heading = null
        }
        // todo: fix this
        // this.egoEndPos.push(pos)
        // planner.setPathPoints(this.egoEndPos)
        // planner.endPosArr.push(pos)

        // 处于曲线编辑器状态，则添加一个编辑点
        const [pickX, pickY, pickZ] = this.ct.lonlat2xyz(endPosX, endPosY, endPosZ)

        const attr = {
          velocity: 6,
          gear: 'drive',
        }
        if (trajectoryEnabled) {
          attr.frontwheel = null
          attr.accs = null
          attr.heading = null
        }
        const lastIdx = curve.Points().length - 1
        const point = new Vector3(pickX, pickY, pickZ)
        curve.modifyPoint(lastIdx, point)
        curve.addPoint(point, true, attr)

        planner.endLon = endPosX
        planner.endLat = endPosY
        planner.endAlt = endPosZ
        break
      }
      case StatePickVehicleEnd: {
        route = this.sceneParser.findRouteInMap(this.curEditVehicle.routeId)
        if (!route) {
          console.error('route not exist')
          return
        }
        route.endLon = endPosX
        route.endLat = endPosY

        // 处于曲线编辑器状态，则添加一个编辑点

        const lastIdx = curve.Points().length - 1
        curve.modifyPoint(lastIdx, vec)
        const attr = {
          velocity: this.curEditVehicle.startVelocity,
          gear: 'drive',
        }
        curve.addPoint(vec, true, attr)
        break
      }
      case StatePickPedestrianEnd: {
        route = this.sceneParser.findRouteInMap(this.curEditPedestrian.routeId)
        if (!route) {
          console.error('route not exist')
          return
        }
        route.endLon = endPosX
        route.endLat = endPosY

        // 处于曲线编辑器状态，则添加一个编辑点
        const lastIdx = curve.Points().length - 1
        curve.modifyPoint(lastIdx, vec)
        let attr
        if (trajectoryEnabled) {
          attr = {
            velocity: this.curEditPedestrian.startVelocity,
            gear: 'drive',
          }
        }
        curve.addPoint(vec, true, attr)
        break
      }
      case StatePickMeasurementEnd: {
        if (this.curMeasurementEditCurve == null) {
          this.curMeasurementEditCurve = new EditorCurve(this, false, undefined, EditorCurve.MeasurementMat)

          this.curMeasurementEditCurve.modifyPoint(0, vec, false)
          this.curMeasurementEditCurve.modifyPoint(1, vec)
        } else {
          const lastIdx = this.curMeasurementEditCurve.Points().length - 1
          this.curMeasurementEditCurve.modifyPoint(lastIdx, vec)
          this.curMeasurementEditCurve.addPoint(vec, true)
        }
        this.calculateMeasurementResult(event)
        break
      }
      default:
        console.warn('wrong state', state)
    }
  }

  // 是否在编辑轨迹点
  isEditingRoute () {
    const routeEditState = [
      SceneState.StatePickEgoCarEnd,
      SceneState.StatePickVehicleEnd,
      SceneState.StatePickPedestrianEnd,
      SceneState.StatePickMeasurementEnd,
    ]
    return routeEditState.includes(this.state)
  }

  async onTopCameraMouseMove (event) {
    const vec = new Vector3()
    const x = event.offsetX
    const y = event.offsetY
    const ux = (x / this.wrapper.clientWidth) * 2 - 1
    const uy = ((this.wrapper.clientHeight - y) / this.wrapper.clientHeight) * 2 - 1
    this.hadmap.showLaneId(ux, uy, this.curCamera)

    let { offsetX, offsetY } = event
    let target = event.target
    do {
      offsetX += target.offsetLeft
      offsetY += target.offsetTop
      target = target.parentNode
    } while (target && target !== this.wrapper.parentNode)

    let curve
    if (this.curPlannerPathEditCurve) {
      curve = this.curPlannerPathEditCurve
    } else if (this.curVehiclePathEditCurve) {
      curve = this.curVehiclePathEditCurve
    } else if (this.curPedestrianPathEditCurve) {
      curve = this.curPedestrianPathEditCurve
    } else if (this.curMeasurementEditCurve) {
      curve = this.curMeasurementEditCurve
    } else {
    }
    let pos

    if (curve && curve.Points().length > 1) {
      const points = curve.Points()
      const prevPoint = points[points.length - 2]
      pos = this.getPointerPosInPickGroundWithHeight(offsetX, offsetY, prevPoint.z)
    } else {
      pos = this.getPointerPosInScene(offsetX, offsetY) // use perspective pick
    }
    vec.copy(pos)

    if (this.hadmapLoaded()) {
      const tmp = this.ct.xyz2lonlat(vec.x, vec.y, vec.z)

      EditorGlobals.signals.mousePosition2UI.dispatch({
        x: vec.x.toFixed(2),
        y: vec.y.toFixed(2),
        lon: tmp[0].toFixed(7),
        lat: tmp[1].toFixed(7),
      })
    } else {
      EditorGlobals.signals.mousePosition2UI.dispatch({
        x: vec.x.toFixed(2),
        y: vec.y.toFixed(2),
        lon: 0.0000000,
        lat: 0.0000000,
      })
    }

    if (curve) {
      const lastIdx = curve.Points().length - 1
      curve.modifyPoint(lastIdx, vec, true)
    }

    if (this.state === SceneState.StatePickMeasurementEnd || this.rButtonDown) {
      this.calculateMeasurementResult(event, this.state === SceneState.StatePickMeasurementEnd ? 'edit' : 'noedit') // 'noedit' don't trigger save
    }
  }

  /**
   * 手动触发调用
   * @param {} factor
   * @param {*} event
   * @returns
   */
  zoom (factor, event) {
    if (this.curCamera === this.cameraTop) {
      const offset = new Vector3()
      offset.subVectors(this.cameraTop.position, this.editorControls.target)
      offset.multiplyScalar(factor)

      // minZoom 0.1% & maxZoom 1000%
      if (offset.length() < this.editorControls.minDistance) {
        offset.setLength(this.editorControls.minDistance)
      }
      if (offset.length() > this.editorControls.maxDistance) {
        offset.setLength(this.editorControls.maxDistance)
      }
      offset.add(this.editorControls.target)
      this.cameraTop.position.copy(offset)

      this.onTopCameraZoomChange()
      this.renderScene()
    }

    return this.topSceneZoomFactor
  }

  getObjectMaxOffset (type, id) {
    if (type === 'car') {
      const v = this.sceneParser.findVehicleInMap(id)
      if (!v) {
        return 3.5
      }

      const routeId = v.routeId
      const laneId = v.laneId

      const width = this.getLaneWidth(routeId, laneId)
      return width / 2
    }

    return 0
  }

  getVehicleMaxShift (id) {
    const v = this.sceneParser.findVehicleInMap(id)
    if (!v) {
      return 0
    }

    const routeId = v.routeId
    const laneId = v.laneId

    return this.getLaneLength(routeId, laneId)
  }

  getLaneLength (routeId, laneId) {
    try {
      const route = this.sceneParser.findRouteInMap(routeId)
      if (route) {
        if (route.elemType === MapElementType.LANE) {
          if (laneId == undefined) {
            return 0
          } else {
            const lane = this.hadmap.getLane(route.roadId, route.sectionId, laneId)
            if (!lane) {
              console.error('lane: (', route.roadId, ', ', route.sectionId, ', ', laneId, ') not exist')
              return 0
            } else {
              return lane.len
            }
          }
        } else if (route.elemType === MapElementType.LANELINK) {
          const lanelink = this.hadmap.getLanelink(route.lanelinkId)
          if (!lanelink) {
            console.error('lanelink: (', route.lanelinkId, ') not exist')
            return 0
          } else {
            return lanelink.len
          }
        } else {
          console.error('position element type error!')
        }
      } else {
        console.error('route : ', routeId, 'not exist')
      }
    } catch (e) {
      console.error(e)
    }

    return 0
  }

  getLaneWidth (routeId, laneId) {
    try {
      const route = this.sceneParser.findRouteInMap(routeId)
      if (route) {
        if (route.elemType === MapElementType.LANE) {
          if (laneId == undefined) {
            return 0
          } else {
            const lane = this.hadmap.getLane(route.roadId, route.sectionId, laneId)
            if (!lane) {
              console.error('lane: (', route.roadId, ', ', route.sectionId, ', ', laneId, 'not exist')
              return 0
            } else {
              return lane.width
            }
          }
        } else if (route.elemType === MapElementType.LANELINK) {
          return 0
        } else {
          console.error('position element type error!')
        }
      } else {
        console.error('route : ', routeId, 'not exist')
      }
    } catch (e) {
      console.error(e)
    }

    return 0
  }

  /**
   * 根据车道信息得到空间坐标信息
   * @param {} roadId
   * @param {*} sectionId
   * @param {*} laneId
   * @param {*} shift
   * @param {*} offset
   * @returns
   */
  async getLaneShiftPos2 (roadId, sectionId, laneId, shift, offset) {
    const lonlatInfo = await this.hadmap.getLonLat(roadId, sectionId, laneId, shift, offset)
    if (lonlatInfo.err !== 0) {
      console.error(`getLaneShiftPos2 (${roadId}, ${sectionId}, ${laneId})-(${shift}, ${offset})`)
      return []
    }

    const pos = this.ct.lonlat2xyz(lonlatInfo.lon, lonlatInfo.lat, lonlatInfo.alt)
    return [pos[0], pos[1], pos[2], lonlatInfo.alt]
  }

  /**
   * 得到经纬度信息
   * @param {} roadId
   * @param {*} sectionId
   * @param {*} laneId
   * @param {*} shift
   * @param {*} offset
   * @returns
   */
  async getLaneShiftLonLatAlt (roadId, sectionId, laneId, shift, offset) {
    const lonlatInfo = await this.hadmap.getLonLat(roadId, sectionId, laneId, shift, offset)
    if (lonlatInfo.err !== 0) {
      console.error(`getLaneShiftPos2 (${roadId}, ${sectionId}, ${laneId})-(${shift}, ${offset})`)
      return []
    }
    return [lonlatInfo.lon, lonlatInfo.lat, lonlatInfo.alt]
  }

  /**
   * 可以获取lanelink的经纬度
   * @returns
   */
  async getLaneShiftPos4 (lon, lat, type, id, shift, offset) {
    const lonlatInfo = await this.hadmap.getLonLatByPoint(lon, lat, type, id, shift, offset)
    if (lonlatInfo.err !== 0) {
      console.error('getLaneShiftPos4 error!')
      return []
    }

    const pos = this.ct.lonlat2xyz(lonlatInfo.lon, lonlatInfo.lat, lonlatInfo.alt)

    return [pos[0], pos[1], pos[2], lonlatInfo.finalshift, lonlatInfo.alt]
  }

  /**
   * 根据车道坐标获取经纬度
   * @param roadId
   * @param sectionId
   * @param laneId
   * @param shift
   * @param offset
   * @return {number[]|*[]}
   */
  getLaneShiftPos (roadId, sectionId, laneId, shift, offset) {
    const lane = this.hadmap.getLane(roadId, sectionId, laneId)
    if (lane?.data?.length > 0) {
      const {
        data,
        len,
        data: { length },
      } = lane
      // 位移超过该端lane的长度，则返回该端lane的终点位置；
      if (shift > len) shift = len
      if (shift > len) {
        const info = `shift: ${shift} exceed the length: ${len}` +
          ` of lane: (${roadId}, ${sectionId}, ${laneId})` +
          ` equals to the lane length`

        console.info(info)
        const info1 = i18n.t('tips.offsetLengOfLane', { roadId, sectionId, laneId, laneLen: len })
        MessageBox.promptEditorUIMessage('warn', info1)

        const lastP = data[length - 1]
        return [lastP.x, lastP.y, lastP.z]
      } else {
        if (shift < 0) {
          console.error(`getLaneShiftPos shift is ${shift}, now set to 0`)
          shift = 0
        }
        const seg = len / length * 2
        const nOrder = Math.floor(shift / seg)
        const dir = this.defaultDir.clone()
        const startPos = new Vector3()
        let p1, p2, p3
        const segShift = shift - seg * nOrder
        if (nOrder < (length / 2)) {
          p1 = data[nOrder * 2 + 1]
          p2 = data[nOrder * 2]
          p3 = p2
        } else {
          p1 = data[nOrder * 2 - 1]
          p2 = data[nOrder * 2 - 2]
          p3 = data[length - 1]
        }
        dir.x = p1.x - p2.x
        dir.y = p1.y - p2.y
        dir.z = p1.z - p2.z
        startPos.x = p3.x
        startPos.y = p3.y
        startPos.z = p3.z

        const segLength = dir.length()
        dir.normalize()

        // shift的计算
        let delta = segShift / seg

        if (nOrder === (length / 2 - 1)) {
          const curveLen = len
          const lastSegLength = curveLen - Math.floor(curveLen / seg) * seg
          delta = segShift / lastSegLength
        }

        const pos = dir.clone().multiplyScalar(delta * segLength)

        // 偏移量的计算
        // if (laneId > 0) {
        //   dir.negate()
        // }
        dir.normalize()
        const offsetDir = new Vector3()
        offsetDir.crossVectors(dir, this.ModelUpDir)
        offsetDir.normalize()
        offsetDir.multiplyScalar(offset)

        // offset 和shift的叠加
        startPos.add(pos)
        startPos.add(offsetDir)

        return [startPos.x, startPos.y, startPos.z]
      }
    } else {
      const info = `hadmap no lane data(${roadId}, ${sectionId}, ${laneId})`
      console.error(info)
      const info1 = i18n.t('tips.noFollowingDataInMap', { roadId, sectionId, laneId })
      MessageBox.promptEditorUIMessage('error', info1)
    }
  }

  /**
   * 根据车道坐标获取朝向
   * @param roadId
   * @param sectionId
   * @param laneId
   * @param shift
   * @return {Vector3}
   */
  getLaneShiftDir (roadId, sectionId, laneId, shift) {
    const lane = this.hadmap.getLane(roadId, sectionId, laneId)
    if (lane?.data?.length > 0) {
      const { length } = lane.data
      let p1, p2
      const dir = this.defaultDir.clone()
      if (shift < 0) {
        p1 = lane.data[length - 1]
        p2 = lane.data[length - 2]
      } else {
        const seg = lane.len / lane.data.length * 2
        let nOrder = Math.floor(shift / seg)

        if (shift > lane.len) {
          const info = `shift: ${shift} exceed the length: ${lane.len}` +
            ` of lane: (${roadId}, ${sectionId}, ${laneId})` +
            ` dir equals to the lane end point dir`

          console.info(info)
          nOrder = Math.floor(lane.len / seg) - 1
        }
        p1 = lane.data[nOrder * 2 + 1]
        p2 = lane.data[nOrder * 2]
      }

      dir.x = p1.x - p2.x
      dir.y = p1.y - p2.y
      dir.z = p1.z - p2.z
      dir.normalize()
      return dir
    } else {
      console.error('hadmap no lane data (', roadId, ', ', sectionId, ', ', laneId, ') return default dir')
      return this.defaultDir.clone()
    }
  }

  /**
   * 根据车道连接坐标获取朝向
   * @param lanelinkId
   * @param shift
   * @return {Vector3}
   */
  getLanelinkShiftDir (lanelinkId, shift) {
    const lanelink = this.hadmap.getLanelink(lanelinkId)
    if (lanelink?.data?.length > 0) {
      const seg = lanelink.len / lanelink.data.length * 2
      let nOrder = Math.floor(shift / seg)

      if (shift > lanelink.len) {
        const info = `shift: ${shift} exceed the length: ${lanelink.len}` +
          ` of lanelink: (${lanelink})` + ` dir equals to the lane end point dir`

        console.info(info)
        nOrder = Math.floor(lanelink.len / seg) - 1
      }

      const dir = this.defaultDir.clone()
      dir.x = lanelink.data[nOrder * 2 + 1].x - lanelink.data[nOrder * 2].x
      dir.y = lanelink.data[nOrder * 2 + 1].y - lanelink.data[nOrder * 2].y
      dir.z = lanelink.data[nOrder * 2 + 1].z - lanelink.data[nOrder * 2].z
      dir.normalize()
      return dir
    } else {
      console.error('hadmap no lanelink data (', lanelinkId, ') return default dir')
      return this.defaultDir.clone()
    }
  }

  /**
   * 根据section坐标获取朝向
   * @param roadId
   * @param sectionId
   * @param shift
   * @return {Vector3}
   */
  getSectionShiftDir (roadId, sectionId, shift) {
    const sec = this.hadmap.getSection(roadId, sectionId)
    if (sec) {
      // 获取第一个有几何数据的lane
      let lane
      const len = sec.lanes.length
      for (let i = 0; i < len; ++i) {
        if (sec.lanes[i].data.length > 0) {
          lane = sec.lanes[i]
          break
        }
      }

      if (lane) {
        return this.getLaneShiftDir(roadId, sectionId, lane.laneId, shift)
      } else {
        console.log('section no lane data (', roadId, ', ', sectionId, ') return default dir')
      }
    } else {
      console.log('section id not exist : (', roadId, ', ', sectionId, ') return default dir')
    }

    return this.defaultDir.clone()
  }

  /**
   * 根据道路坐标获取最终坐标
   * @param roadId
   * @param sectionId
   * @param shift
   * @param offset
   * @return {undefined|number[]|*[]}
   */
  getFinalPosBySection2 (roadId, sectionId, shift, offset) {
    const sec = this.hadmap.getSection(roadId, sectionId)
    if (sec) {
      // 获取第一个有几何数据的lane
      let lane
      const len = sec.lanes.length
      for (let i = 0; i < len; ++i) {
        if (sec.lanes[i].data.length > 0) {
          lane = sec.lanes[i]
          break
        }
      }

      if (lane) {
        return this.getLaneShiftPos(roadId, sectionId, lane.laneId, shift, offset)
      } else {
        console.log('section no lane data (', roadId, ', ', sectionId, ')')
      }
    } else {
      console.log('section id not exist : (', roadId, ', ', sectionId, ')')
    }

    return undefined
  }

  /**
   * 根据route坐标获取最终坐标
   * @param id
   * @param laneId
   * @param shift
   * @param offset
   * @return {Promise<undefined|[]|[*,*,*,*]|*|number[]|*[]>}
   */
  async getFinalPosByRoute2 (id, laneId, shift, offset) {
    id = +(id)
    if (laneId != undefined) {
      laneId = +(laneId)
    }

    try {
      const route = this.sceneParser.findRouteInMap(id)
      if (route) {
        if (route.type === 'roadID') {
          if (laneId == undefined) {
            return this.getFinalPosBySection2(route.roadId, route.sectionId, shift, offset)
          } else {
            const tmp0 = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
            const tmp1 = this.getLaneShiftPos(route.roadId, route.sectionId, laneId, shift, offset)
            if (tmp1) {
              return tmp1
            } else {
              tmp0[2] = 0
              return tmp0
            }
          }
        } else if (route.type === 'start_end') {
          if (laneId == undefined) {
            return this.getFinalPosBySection2(route.roadId, route.sectionId, shift, offset)
          } else {
            const tmp3 = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
            const tmp4 = await this.getLaneShiftPos2(route.roadId, route.sectionId, laneId, shift, offset)
            if (tmp4) {
              return tmp4
            } else {
              console.error(`getFinalPosByRoute2 route:${route.id} (${route.startLon}, ${route.startLat})-(${laneId})-(${shift}, ${offset}) error. use lon lat coordinate convertion`)
              tmp3[2] = 0
              return tmp3
            }
          }
        }
      }
    } catch (e) {
      console.error(e)
    }

    return undefined
  }

  /**
   * 得到当前主车位置信息
   * @param {*} planner
   */
  async getPlannerPosition (planner) {
    try {
      const route = this.sceneParser.findRouteInMap(planner.routeId)
      return await this.getLaneShiftPos2(
        route.roadId,
        route.sectionId,
        planner.laneId,
        planner.startShift,
        planner.startOffset,
      )
    } catch (error) {
      console.warn('get position failed, use origin position')
      return [0, 0, 0]
    }
  }

  /**
   * 获取主车的经纬度
   * @param planner
   * @return {Promise<[]|[*,*,*]|number[]>}
   */
  async getPlannerLonLatAlt (planner) {
    try {
      const route = this.sceneParser.findRouteInMap(planner.routeId)
      return await this.getLaneShiftLonLatAlt(
        route.roadId,
        route.sectionId,
        planner.laneId,
        planner.startShift,
        planner.startOffset,
      )
    } catch (error) {
      console.warn('get position failed, use origin position')
      return [0, 0, 0]
    }
  }

  /**
   * 根据route坐标获取最终坐标4
   * @param id
   * @param laneId
   * @param shift
   * @param offset
   * @return {Promise<undefined|[]|[*,*,*,*,*]|*|number[]|*[]>}
   */
  async getFinalPosByRoute4 (id, laneId, shift, offset) {
    id = +(id)
    if (laneId != undefined) {
      laneId = +(laneId)
    }

    try {
      const route = this.sceneParser.findRouteInMap(id)
      if (route) {
        if (route.type === 'roadID') {
          console.error(`the type of roadID is not support any more`)
        } else if (route.type === 'start_end') {
          if (route.elemType === MapElementType.LANE) {
            if (laneId == undefined) {
              return this.getFinalPosBySection2(route.roadId, route.sectionId, shift, offset)
            } else {
              const tmp4 = await this.getLaneShiftPos4(
                route.startLon,
                route.startLat,
                'lane',
                laneId,
                shift,
                offset,
              )
              if (tmp4?.length > 0) {
                return tmp4
              } else {
                console.error('getFinalPosByRoute4 error use lon lat coordinate conversion')
                const tmp3 = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
                tmp3[2] = 0
                tmp3.push(+shift)
                return tmp3
              }
            }
          } else if (route.elemType === MapElementType.LANELINK) {
            const tmp2 = await this.getLaneShiftPos4(
              route.startLon,
              route.startLat,
              route.elemType,
              route.lanelinkId,
              shift,
              offset,
            )
            if (tmp2?.length > 0) {
              return tmp2
            } else {
              console.error('getFinalPosByRoute4 error use lon lat coordinate conversion')
              const tmp = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
              tmp[2] = 0
              tmp.push(+shift)
              return tmp
            }
          }
        }
      }
    } catch (e) {
      console.error(e)
    }

    return undefined
  }

  /**
   * 批量获取最终位置（优化打开场景时有大量场景的情况）
   * @param {Array} arr
   * @return {Promise<Array>}
   */
  async batchGetFinalPosByRoute4 (arr) {
    const points = []
    arr.forEach((ele, index) => {
      const { routeId, laneId, startShift: shift, startOffset: offset } = ele
      const route = this.sceneParser.findRouteInMap(+routeId)
      if (route) {
        if (route.type === 'routeID') {
          console.error(`the type of roadID is not support any more. routeId: ${routeId}`)
        } else if (route.type === 'start_end') {
          if (route.elemType === MapElementType.LANE) {
            if (isNil(laneId)) {
              ele.tmp = this.getFinalPosBySection2(route.roadId, route.sectionId, shift, offset)
            } else {
              ele.tmp = -1
              points.push({
                startLon: route.startLon,
                startLat: route.startLat,
                type: MapElementType.LANE,
                id: laneId,
                shift,
                offset,
              })
            }
          } else if (route.elemType === MapElementType.LANELINK) {
            ele.tmp = -1
            points.push({
              startLon: route.startLon,
              startLat: route.startLat,
              type: MapElementType.LANELINK,
              id: route.lanelinkId,
              shift,
              offset,
            })
          }
        }
      }
    })
    const lonlats = await this.hadmap.batchGetLaneShiftPos4(points)
    arr.forEach((ele) => {
      if (ele.tmp === -1) {
        const lonlat = lonlats.shift()
        if (lonlat.length) {
          ele.tmp = lonlat
        } else {
          const route = this.sceneParser.findRouteInMap(+ele.routeId)
          ele.tmp = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
          ele.tmp[2] = 0
          ele.tmp.push(+ele.startShift)
        }
        if (ele.start_angle === '') {
          ele.start_angle = lonlat[5]
        }
      }
    })
  }

  /**
   * 根据道路坐标获取最终坐标
   * @param laneInfo
   * @param pickLonLat
   * @return {Promise<[]|[*,*,*,*]|[*,*,*,*,*]|*>}
   */
  async getFinalPosByLaneInfo (laneInfo, pickLonLat) {
    if (laneInfo.type === MapElementType.LANE) {
      const tmp3 = this.ct.lonlat2xyz(laneInfo.lon, laneInfo.lat, laneInfo.alt)
      const tmp4 = await this.getLaneShiftPos2(
        laneInfo.rid,
        laneInfo.sid,
        laneInfo.lid,
        laneInfo.dist,
        laneInfo.offset,
      )
      if (tmp4?.length > 0) {
        return tmp4
      } else {
        console.error('getFinalPosByLaneInfo error use lon lat coordinate conversion')
        tmp3.push(+laneInfo.dist)
        return tmp3
      }
    } else if (laneInfo.type === MapElementType.LANELINK) {
      const tmp = this.ct.lonlat2xyz(laneInfo.lon, laneInfo.lat, laneInfo.alt)
      const tmp2 = await this.getLaneShiftPos4(
        pickLonLat[0],
        pickLonLat[1],
        laneInfo.type,
        laneInfo.llid,
        laneInfo.dist,
        laneInfo.offset,
      )
      if (tmp2?.length > 0) {
        return tmp2
      } else {
        console.error('getFinalPosByLaneInfo error use lon lat coordinate convertion')
        tmp.push(+laneInfo.dist)
        return tmp
      }
    }
  }

  /**
   * 获取车道开始出的坐标
   * @param roadId
   * @param sectionId
   * @param laneId
   * @return {*[]}
   */
  getLaneStartPos (roadId, sectionId, laneId) {
    const lane = this.hadmap.getLane(roadId, sectionId, laneId)
    if (lane) {
      if (lane.data.length > 0) {
        return [lane.data[0].x, lane.data[0].y, lane.data[0].z]
      } else {
        console.log('lane no data : (', lane.roadId, ', ', lane.sectionId, ', ', lane.laneId, ')')
      }
    } else {
      console.log('hadmap no lane data (', roadId, ', ', sectionId, ', ', laneId, ')')
    }
  }

  /**
   * 获取section起始位置坐标
   * @param roadId
   * @param sectionId
   * @return {*[]}
   */
  getSectionStartPos (roadId, sectionId) {
    const sec = this.hadmap.getSection(roadId, sectionId)
    if (sec) {
      let lane
      for (let i = 0; i < sec.lanes.length; ++i) {
        if (sec.lanes[i].data.length > 0) {
          lane = sec.lanes[i]
          break
        }
      }

      if (lane) {
        if (lane.data.length > 0) {
          return [lane.data[0].x, lane.data[0].y, lane.data[0].z]
        } else {
          console.error('lane no data : (', lane.roadId, ', ', lane.sectionId, ', ', lane.laneId, ')')
        }
      } else {
        console.error('section no lane data (', roadId, ', ', sectionId, ')')
      }
    } else {
      const info = `section id not exist: (${roadId}, ${sectionId})`
      console.error(info)
      const msg = i18n.t('tips.sectionNotExists', { roadId, sectionId })
      MessageBox.promptEditorUIMessage('error', msg)
    }
  }

  /**
   * 获取route起始位置坐标
   * @param id
   * @param laneId
   * @return {*|*[]}
   */
  getRouteStartPos (id, laneId) {
    const route = this.sceneParser.findRouteInMap(id)
    if (route) {
      if (route.type === 'roadID') {
        if (laneId == undefined) {
          return this.getSectionStartPos(route.roadId, route.sectionId)
        } else {
          const tmp = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
          const tmp2 = this.getLaneStartPos(route.roadId, route.sectionId, laneId)
          tmp[2] = tmp2[2]
          return tmp
        }
      } else if (route.type === 'start_end') {
        if (laneId == undefined) {
          return this.getSectionStartPos(route.roadId, route.sectionId)
        } else {
          const tmp = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
          const tmp2 = this.getLaneStartPos(route.roadId, route.sectionId, laneId)
          tmp[2] = tmp2[2]
          return tmp
        }
      }
    }

    return []
  }

  /**
   * 根据object的坐标，方向和所在平面来设置车辆旋转
   * @param {object} object 待操作物体
   * @param {Vector3} position 物体位置
   * @param {Vector3} dir 道路在此处的方向
   * @param {string} [angle] 车辆的绝对角度，如果不为''，则优先于dir
   * @return {number}
   */
  alignObject (object, position, dir, angle) {
    // object最终在xy平面上的角度
    let resultAngle
    // 路面法向量，没有路面时取xy平面
    const normal = this.hadmap.getPositionNormal(position)
    if (angle !== undefined) {
      // dirOnXY = new Vector3(1, 0, 0)
      // dirOnXY.applyAxisAngle(axisZ, angle * MathUtils.DEG2RAD)
      // dirOnXY.normalize()
      resultAngle = +angle
    } else {
      if (dir.y === 0 && dir.x === 0) {
        resultAngle = 0
      } else {
        resultAngle = Math.atan2(dir.y, dir.x)
      }
      if (resultAngle < 0) {
        resultAngle += 2 * Math.PI
      }
      resultAngle = `${resultAngle * MathUtils.RAD2DEG}`
    }
    setRotateByDirNormal(object.model, resultAngle * MathUtils.DEG2RAD, normal)
    return resultAngle
  }

  async modifyObject (payload) {
    let id
    if (payload?.data) {
      const addData = payload.data

      switch (payload.type) {
        case 'car':
          id = await this.addCar(addData)
          break
        case 'acc':
          id = this.addAccData(addData)
          break
        case 'merge':
          id = this.addMergeData(addData)
          break
        case 'planner':
          id = await this.modifyPlannerData(addData)
          break
        // case 'signlight':
        //   id = this.addSignlightData(addData)
        //   break
        case 'pedestrian':
        case 'animal':
        case 'bike':
        case 'moto':
        case 'machine':
          id = this.addPedestrianData(addData)
          break
        case 'obstacle':
          id = this.addObstacleData(addData)
          break
        case 'rosbag':
          id = this.addRosbagData(addData)
          break
        default:
          console.error('unknown add data type ', payload.type)
      }
    }

    return id
  }

  /**
   * 插入一个测量点
   * @param measurement
   * @param idx
   * @param x
   * @param y
   * @param z
   */
  insertOneMeasurementPoint (measurement, idx, x, y, z) {
    if (!measurement || !measurement.curve) {
      return
    }

    const order = measurement.curve.getIndexFromSampleIdx(idx)
    const vec = new Vector3(x, y, z)
    measurement.curve.insertPoint(order, vec, true)
    measurement.updatePointsFromCurve(this.ct)
    this.sceneParser.addMeasurementEndModel(measurement)
  }

  /**
   * 插入一个路径轨迹点
   * @param type
   * @param object
   * @param route
   * @param idx
   * @param x
   * @param y
   * @param z
   */
  insertOneRoutePoint (type, object, route, idx, x, y, z) {
    if (!route || !route.curve) {
      return
    }

    const order = route.curve.getIndexFromSampleIdx(idx)
    const vec = new Vector3(x, y, z)
    route.curve.insertPoint(order, vec, true)
    route.updateRoutePointsFromCurve(this.ct)
    if (type === 'planner') {
      this.sceneParser.addPlannerEndModel(object)
    } else if (type === 'car') {
      this.sceneParser.addVehicleEndModel(object)
    } else if (type === 'pedestrian') {
      this.sceneParser.addPedestrianEndModel(object)
    }
  }

  /**
   * 渲染一个路径模型
   * @param route
   */
  addRouteVisual (route) {
    const id = route.id
    let startText = ''
    let midText = ''
    // let endText = ''
    if (route.type === 'start_end') {
      startText = `${id}-SE-Start`
      midText = `${id}-SE-Mid`
      // endText = id + '-SE-End'

      route.startRPM = new RoutePointMarker(startText)
      const tmp = this.ct.lonlat2xyz(route.startLon, route.startLat, 0)
      route.startRPM.setPos(tmp[0], tmp[1], tmp[2])
      route.startRPM.addToScene(this.scene)

      if (route.midLon > 0) {
        route.midRPM = new RoutePointMarker(midText)
        const tmp = this.ct.lonlat2xyz(route.midLon, route.midLat, 0)
        route.midRPM.setPos(tmp[0], tmp[1], tmp[2])
        route.midRPM.addToScene(this.scene)
      }
    } else {
      startText = `${id}-R-Start`
      const tmp = this.getRouteStartPos(route.id)
      if (tmp?.length > 0) {
        route.startRPM = new RoutePointMarker(startText)
        route.startRPM.setPos(tmp[0], tmp[1], tmp[2])
        route.startRPM.addToScene(this.scene)
      }
    }
  }

  /**
   * 在场景加载时渲染一个路径模型
   * @param route
   */
  addRouteVisualOnLoad (route) {
    const id = route.id
    let startText = ''
    let midText = ''
    // let endText = ''
    if (route.type === 'start_end') {
      startText = `${id}-SE-Start`
      midText = `${id}-SE-Mid`
      // endText = id + '-SE-End'

      let tmp2
      if (GlobalConfig.ignoreHeight === IgnoreHeight.Yes) {
        tmp2 = []
        tmp2[2] = 0
      } else {
      }

      route.startRPM = new RoutePointMarker(startText)
      let tmp = this.ct.lonlat2xyz(route.startLon, route.startLat, route.startAlt || 0)
      route.startRPM.setPos(tmp[0], tmp[1], tmp[2]) // tmp2 is null when route in on lanelink
      route.startRPM.addToScene(this.scene)

      if (route.midLon > 0) {
        route.midRPM = new RoutePointMarker(midText)
        tmp = this.ct.lonlat2xyz(route.midLon, route.midLat, 0)
        route.midRPM.setPos(tmp[0], tmp[1], tmp[2])
        route.midRPM.addToScene(this.scene)
      }
    } else {
      startText = `${id}-R-Start`
      const tmp = this.getRouteStartPos(route.id)
      if (tmp?.length) {
        route.startRPM = new RoutePointMarker(startText)
        route.startRPM.setPos(tmp[0], tmp[1], tmp[2])
        route.startRPM.addToScene(this.scene)
      } else {
        // console.log()
      }
    }
  }

  /**
   * 更新路径开始位置
   * @param routeId
   * @param laneId
   */
  updateRouteStartPos (routeId, laneId) {
    const route = this.sceneParser.findRouteInMap(routeId)
    const lane = this.hadmap.getLane(route.roadId, route.sectionId, laneId)
    let laneStartPT
    if (lane.data?.length) {
      const p = lane.getPoint(this.LaneStartShift)
      if (p.length > 0) {
        laneStartPT = this.ct.xyz2lonlat(p[0], p[1], p[2])
      } else {
        const msg = `lane ${route.roadId}, ${route.sectionId}, ${laneId} no data`
        console.error(msg)
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }
    } else {
      const msg = i18n.t('tips.noDataForRoad')
      MessageBox.promptEditorUIMessage('error', msg)
      return
    }

    route.startLon = laneStartPT[0]
    route.startLat = laneStartPT[1]
  }

  /**
   * 添加一个路径
   * @param route
   * @param id
   */
  addRoute (route, id) {
    this.sceneParser.addRoute(route, id)
    this.addRouteVisual(route)
    this.renderScene()
  }

  /**
   * 校验一个route是否可删除
   * @param route
   * @param rid
   * @return {{}}
   */
  canRouteRemoved (rid) {
    const res = {}
    res.result = true
    let msg = ''
    let len = 0

    // planners
    const es = this.sceneParser.findPlannersByRouteID(rid)
    if (es) {
      len = es.length
      if (len > 0) {
        res.planners = []
        msg += `${i18n.t('model.car')}: `
        res.result = false
      }
      for (let i = 0; i < len; ++i) {
        res.planners.push(es[i].id)
        msg += `${es[i].id} `
      }
    }

    // vehicles
    const vs = this.sceneParser.findVehiclesByRouteID(rid)
    if (vs) {
      len = vs.length
      if (len > 0) {
        res.vehicles = []
        msg += `${i18n.t('model.car')}: `
        res.result = false
      }
      for (let i = 0; i < len; ++i) {
        res.vehicles.push(vs[i].id)
        msg += `${vs[i].id} `
      }
    }

    // obstacles
    const os = this.sceneParser.findObstaclesByRouteID(rid)
    if (os) {
      len = os.length
      if (len > 0) {
        res.obstacles = []
        msg += `${i18n.t('model.obstacle')}: `
        res.result = false
      }
      for (let i = 0; i < len; ++i) {
        res.obstacles.push(os[i].id)
        msg += `${os[i].id} `
      }
    }

    // pedestrians
    const ps = this.sceneParser.findPedestriansByRouteID(rid)
    if (ps) {
      len = ps.length
      if (len > 0) {
        res.pedestrians = []
        msg += `${i18n.t('model.pedestrian')}: `
        res.result = false
      }
      for (let i = 0; i < len; ++i) {
        res.pedestrians.push(ps[i].id)
        msg += `${ps[i].id} `
      }
    }

    // signlights
    const ss = this.sceneParser.findSignlightsByRouteID(rid)
    if (ss) {
      len = ss.length
      if (len > 0) {
        res.signlights = []
        msg += `${i18n.t('model.signlight')}: `
        res.result = false
      }
      for (let i = 0; i < len; ++i) {
        res.signlights.push(ss[i].id)
        msg += `${ss[i].id} `
      }
    }

    if (res.result !== true) {
      msg += i18n.t('tips.useThisRoute')
      res.msg = msg
    }

    return res
  }

  /**
   * 删除一个route
   * @param rid
   * @return {{}}
   */
  removeRoute (rid) {
    let res = {}
    res.result = true
    const r = this.sceneParser.findRouteInMap(rid)
    if (!r) {
      res.result = false
      res.msg = i18n.t('tips.pathNotExist')
      return res
    }

    res = this.canRouteRemoved(rid)
    if (res.result == false) {
      return res
    }

    if (r.startRPM) {
      r.startRPM.removeFromScene(this.scene)
      r.startRPM = undefined
    }

    if (r.midRPM) {
      r.midRPM.removeFromScene(this.scene)
      r.midRPM = undefined
    }

    if (r.endRPM) {
      r.endRPM.removeFromScene(this.scene)
      r.endRPM = undefined
    }

    if (this.selectedExist()) {
      if (r.curve) {
        r.curve.Boxes().some((value) => {
          if (this.isObjectPartSelected(value)) {
            this.cancelPickedObject()
            return true
          }
          return false
        }, this)
      }
    }

    this.sceneParser.removeRoute(rid)
    this.renderScene()
    return res
  }

  /**
   * 添加一个速度数据
   * @param vel
   */
  addVelocity (vel) {
    this.sceneParser.addVelocity(vel)
  }

  /**
   * 删除一个速度数据
   * @param vel
   */
  modifyVelocity (vel) {
    this.sceneParser.modifyVelocity(vel)
  }

  /**
   * 添加一个加速度数据
   * @param acc
   */
  addAcc (acc) {
    this.sceneParser.addAcc(acc)
  }

  /**
   * 添加默认的加速度数据
   */
  addDefaultAccData () {
    console.log('add default acc data')

    const acc = new AccelerationData()
    const len = 1
    acc.id = 0
    for (let i = 0; i < len; ++i) {
      const condition = { type: TriggerCondition.TimeAbsolute, value: 0 }
      const termination = { type: AccelerationTerminationType.None, value: 0 }
      acc.nodes[i] = { condition, acc: 0, count: -1, termination }
    }

    this.addAcc(acc)
  }

  /**
   * 校验一个加速度数据是否可删除
   * @param accId
   * @return {*}
   */
  canAccRemoved (accId) {
    return this.sceneParser.findVehiclesByAccID(accId)
  }

  /**
   * 删除一个加速度数据
   * @param id
   * @return {{}}
   */
  removeAcc (id) {
    const res = {}
    res.result = true

    const acc = this.sceneParser.findAccInMap(id)
    if (!acc) {
      res.result = false
      res.msg = i18n.t('tips.accNotExist', { id })
      return res
    }

    const vs = this.canAccRemoved(id)
    if (vs.length === 0) {
      this.sceneParser.removeAcc(id)
    } else {
      const len = vs.length
      res.msg = `${i18n.t('vehicle')}`
      res.result = false
      for (let i = 0; i < len; ++i) {
        res.msg += `${vs[i].id} `
      }
      res.msg += i18n.t('tips.useAccCurve')
    }

    return res
  }

  /**
   * 更新一个加速度数据
   * @param acc
   */
  modifyAcc (acc) {
    this.sceneParser.modifyAcc(acc)
  }

  /**
   * 添加一个merge数据
   * @param merge
   */
  addMerge (merge) {
    this.sceneParser.addMerge(merge)
  }

  /**
   * 添加默认的merge数据
   */
  addDefaultMergeData () {
    console.debug('add default merge data')

    const merge = new MergeData()
    merge.id = 0
    merge.merges[0] = {
      condition: {
        type: TriggerCondition.TimeAbsolute,
        value: 0,
      },
      merge: 0,
      direction: 'static',
      duration: 4.5,
      offset: 0,
      count: -1,
    }

    this.addMerge(merge)
  }

  /**
   * 校验一个merge数据是否可删除
   * @param mergeId
   * @return {*}
   */
  canMergeRemoved (mergeId) {
    return this.sceneParser.findVehiclesByMergeID(mergeId)
  }

  /**
   * 删除一个merge数据
   * @param id
   * @return {{}}
   */
  removeMerge (id) {
    const res = {}
    res.result = true

    const merge = this.sceneParser.findMergeInMap(id)
    if (!merge) {
      res.result = false
      res.msg = i18n.t('tips.mergingNotExist', { id })
      return res
    }

    const vs = this.canMergeRemoved(id)
    if (vs.length === 0) {
      this.sceneParser.removeMerge(id)
    } else {
      const len = vs.length
      res.msg = `${i18n.t('vehicle')} `
      res.result = false
      for (let i = 0; i < len; ++i) {
        res.msg += `${vs[i].id} `
      }
      res.msg += i18n.t('tips.useMergingCurve')
    }

    return res
  }

  /**
   * 更新一个merge数据
   * @param merge
   */
  modifyMerge (merge) {
    this.sceneParser.modifyMerge(merge)
  }

  /**
   * 计算夹角
   * @param v0
   * @param v1
   * @param v2
   * @return {*|number}
   */
  calculateAngle (v0, v1, v2) {
    const dir0 = v0.clone()
    const dir1 = v2.clone()
    dir0.sub(v1)
    dir1.sub(v1)

    if (Utility.equalToZero(dir0.length()) || Utility.equalToZero(dir1.length())) {
      return 0
    }

    return dir0.angleTo(dir1)
  }

  /**
   * 计算朝向
   * @param v0
   * @param v1
   * @return {*|number}
   */
  calculateDirection (v0, v1) {
    const dir = v0.clone()
    dir.sub(v1)

    if (Utility.equalToZero(dir.length())) {
      return 0
    }

    const face = dir.angleTo(this.eastDir)
    dir.cross(this.eastDir)
    const sign = dir.dot(this.ModelUpDir)
    if (sign > 0) {
      return (2 * Math.PI - face)
    } else {
      return face
    }
  }

  /**
   * 计算当前测量线的长度数组
   * @param event
   * @param emitMessage
   * @return {*[]}
   */
  calculateCurDrawMeasurement (event, emitMessage) {
    const payloads = []

    const offset = 5
    if (!event) {
      return payloads
    }

    if (this.State() === SceneState.StatePickMeasurementEnd) {
      if (!this.curMeasurementEditCurve) {
        const plane = {}
        plane.id = 0
        plane.idx = 0
        plane.uuid = 0
        plane.distance = 0
        plane.angle = 0
        plane.direction = 0
        plane.x = event.offsetX + offset
        plane.y = event.offsetY + offset
        plane.type = 'pre-start'
        payloads.push(plane)
      } else {
        const v = new Vector3()
        const c = this.curCamera
        const width = this.wrapper.clientWidth
        const height = this.wrapper.clientHeight
        let totalLen = 0
        const boxes = this.curMeasurementEditCurve.Boxes()
        boxes.forEach((value, idx, arr) => {
          const plane = {}
          plane.id = 0
          plane.uuid = value.uuid
          plane.idx = idx
          plane.distance = 0
          plane.angle = 0
          plane.direction = 0
          value.getWorldPosition(v)
          v.project(c)
          plane.x = (v.x + 1) / 2 * width + offset
          plane.y = (1 - v.y) / 2 * height + offset

          if (idx === 0) {
            plane.type = 'start'
          } else if (idx === arr.length - 1) {
            plane.type = 'pre-end'
          } else {
            plane.type = 'middle'
            plane.angle = this.calculateAngle(arr[idx - 1].position, arr[idx].position, arr[idx + 1].position)
            plane.angle = Utility.Hd2Jd(plane.angle).toFixed(0)
          }

          if (idx > 0) {
            totalLen += arr[idx].position.distanceTo(arr[idx - 1].position)
            plane.distance = totalLen.toFixed(3)
            plane.direction = this.calculateDirection(arr[idx - 1].position, arr[idx].position)
            plane.direction = Utility.Hd2Jd(plane.direction).toFixed(0)
          }

          payloads.push(plane)
        })
      }

      if (emitMessage) {
        EditorGlobals.signals.measurementChanged.dispatch({ type: 'edit', data: payloads })
      }
    }

    return payloads
  }

  /**
   * 获取一个测量的长度数组
   * @param m
   * @return {*[]}
   */
  calculateOneMeasurement (m) {
    if (!m.curve) {
      return []
    }

    const payloads = []

    const v = new Vector3()
    const c = this.curCamera
    c.updateMatrixWorld(true)
    const width = this.wrapper.clientWidth
    const height = this.wrapper.clientHeight

    let totalLen = 0
    m.curve.Boxes().forEach((value, idx, arr) => {
      const plane = {}
      plane.id = m.id
      plane.uuid = value.uuid
      plane.idx = idx
      plane.distance = 0
      plane.angle = 0
      plane.direction = 0
      value.getWorldPosition(v)
      v.project(c)
      plane.x = (v.x + 1) / 2 * width
      plane.y = (1 - v.y) / 2 * height

      if (idx === 0) {
        plane.type = 'start'
      } else if (idx === arr.length - 1) {
        plane.type = 'end'
      } else {
        plane.type = 'middle'
        plane.angle = this.calculateAngle(arr[idx - 1].position, arr[idx].position, arr[idx + 1].position)
        plane.angle = Utility.Hd2Jd(plane.angle).toFixed(0)
      }

      if (idx > 0) {
        totalLen += arr[idx].position.distanceTo(arr[idx - 1].position)
        plane.distance = totalLen.toFixed(3)
        plane.direction = this.calculateDirection(arr[idx - 1].position, arr[idx].position)
        plane.direction = Utility.Hd2Jd(plane.direction).toFixed(0)
      }

      payloads.push(plane)
    })

    return payloads
  }

  /**
   * 计算一个测量的结果
   * @param event
   * @param type
   */
  calculateMeasurementResult (event = null, type = 'edit') {
    let payloads = []

    this.sceneParser.measurements.forEach((value) => {
      const a = this.calculateOneMeasurement(value)
      payloads = payloads.concat(a)
    })

    const a = this.calculateCurDrawMeasurement(event)
    payloads = payloads.concat(a)

    EditorGlobals.signals.measurementChanged.dispatch({ type, data: payloads })
  }

  /**
   * 删除一个测量点
   * @param id
   * @param idx
   */
  removeMeasurementEnd (id, idx) {
    const m = this.sceneParser.findMeasurementInMap(id)
    if (!m || !m.curve) { // fix null error
      console.error('measurement curve not exist!')
      return
    }

    m.curve.removePoint(idx)
    m.updatePointsFromCurve(this.hadmap.ct)

    if (this.pickBox.isPicked()) {
      this.pickBox.hide()
      this.transformControl.detach()
    }
  }

  /**
   * 删除一条测量线
   * @param id
   */
  removeMeasurement (id) {
    const m = this.sceneParser.findMeasurementInMap(id)
    if (m) {
      this.sceneParser.removeMeasurement(id)
    }
  }

  /**
   * 添加一个测量点
   * @param measurement
   */
  addMeasurementPathEnd (measurement) {
    if (measurement.curve) {
      console.warn('measurement : ', measurement.id, ' path end alread exist! ')
      return
    }

    const len = measurement.posArr.length
    if (len > 1) {
      measurement.curve = new EditorCurve(this, false, undefined, EditorCurve.MeasurementMat)
      for (let i = 0; i < len; ++i) {
        const tmp = this.hadmap.ct.lonlat2xyz(
          measurement.posArr[i].lon,
          measurement.posArr[i].lat,
          measurement.posArr[i].alt,
        )
        const vec = new Vector3(tmp[0], tmp[1], tmp[2])
        if (i < 2) {
          measurement.curve.modifyPoint(i, vec, false)
        } else {
          measurement.curve.addPoint(vec, false)
        }
      }

      measurement.curve.updateModels()
    } else {
      console.error('measurement only has one point')
      return
    }

    this.sceneParser.addMeasurementEndModel(measurement)
  }

  /**
   * 添加一个route
   * @param lon
   * @param lat
   * @param type
   * @param ridorllid
   * @param sid
   * @param id
   * @return {number}
   */
  addRouteByStart (lon, lat, type, ridorllid, sid, id) {
    console.log('addRouteByStart', type, ridorllid)

    const route = new RouteData()

    route.type = 'start_end'
    route.remarks = ''
    route.startLon = lon
    route.startLat = lat
    if (type === MapElementType.LANE) {
      route.roadId = ridorllid
      route.sectionId = sid
      route.elemType = MapElementType.LANE
    } else if (type === MapElementType.LANELINK) {
      route.lanelinkId = ridorllid
      route.elemType = MapElementType.LANELINK
    } else {
      console.error('map element error!')
      return
    }

    this.addRoute(route, id)
    return route.id
  }

  /**
   * 添加一个加速度数据
   * @param accDefData
   * @return {string|*}
   */
  addAccData (accDefData) {
    console.log(accDefData)

    const acc = new AccelerationData()
    const len = accDefData.accs.length
    for (let i = 0; i < len; ++i) {
      acc.nodes[i] = {}
      acc.nodes[i].condition = accDefData.accs[i].condition
      acc.nodes[i].acc = +(accDefData.accs[i].acc)
      acc.nodes[i].count = +(accDefData.accs[i].count)
      acc.nodes[i].termination = accDefData.accs[i].termination
    }

    if (accDefData.id == undefined) {
      this.addAcc(acc)
    } else {
      acc.id = accDefData.id
      this.modifyAcc(acc)
    }

    return acc.id
  }

  /**
   * 添加一个merge数据
   * @param mergeDefData
   * @return {string|*}
   */
  addMergeData (mergeDefData) {
    console.debug(mergeDefData)

    const merge = new MergeData()
    const len = mergeDefData.merges.length
    for (let i = 0; i < len; ++i) {
      merge.merges[i] = {}
      merge.merges[i].condition = {}

      merge.merges[i].condition.type = mergeDefData.merges[i].condition.type
      merge.merges[i].condition.mode = mergeDefData.merges[i].condition.mode
      merge.merges[i].condition.value = mergeDefData.merges[i].condition.value
      if (mergeDefData.merges[i].direction === 'left') {
        merge.merges[i].merge = 1
      } else if (mergeDefData.merges[i].direction === 'right') {
        merge.merges[i].merge = -1
      } else if (mergeDefData.merges[i].direction === 'laneleft') {
        merge.merges[i].merge = 2
      } else if (mergeDefData.merges[i].direction === 'laneright') {
        merge.merges[i].merge = -2
      } else {
        merge.merges[i].merge = 0
      }

      merge.merges[i].direction = mergeDefData.merges[i].direction
      merge.merges[i].duration = mergeDefData.merges[i].duration
      merge.merges[i].offset = mergeDefData.merges[i].offset
      merge.merges[i].count = mergeDefData.merges[i].count
    }

    if (mergeDefData.id == undefined) {
      this.addMerge(merge)
    } else {
      merge.id = mergeDefData.id
      this.modifyMerge(merge)
    }

    return merge.id
  }

  /**
   * 添加一个速度数据
   * @param velocityDefData
   * @return {string|*}
   */
  addVelocityData (velocityDefData) {
    console.debug(velocityDefData)

    const velocity = new VelocityData()
    velocity.id = velocityDefData.id

    velocityDefData.vels.forEach((value) => {
      const node = {}
      node.condition = {}
      node.condition.type = value.condition.type
      node.condition.mode = value.condition.mode
      node.condition.value = value.condition.value
      node.velocity = value.velocity
      node.count = value.count

      velocity.nodes.push(node)
    })

    if (velocityDefData.id == undefined) {
      this.addVelocity(velocity)
    } else {
      const vel = this.sceneParser.findVelocityInMap(velocityDefData.id)
      if (vel) {
        this.modifyVelocity(velocity)
      } else {
        this.addVelocity(velocity)
      }
    }

    return velocity.id
  }

  /**
   * 添加一个自定义数据
   * @param vid
   * @param mid
   * @param aid
   * @param carDef
   * @return {{mid, velocity: {}, aid}}
   */
  addCustomData (vid, mid, aid, carDef) {
    const {
      conditions,
      directions,
      accelerations: accs,
      velocities: vels,
      durations,
      offsets,
      counts,
      terminations,
    } = carDef

    const mergeDefData = {}
    mergeDefData.merges = []

    const accDefData = {}
    accDefData.accs = []

    const velocityDefData = {}
    velocityDefData.id = vid
    velocityDefData.vels = []

    for (let i = 0; i < conditions.length; ++i) {
      // merge
      mergeDefData.merges[i] = {}
      mergeDefData.merges[i].condition = conditions[i]
      mergeDefData.merges[i].direction = directions[i]
      mergeDefData.merges[i].duration = durations[i]
      mergeDefData.merges[i].offset = offsets[i]
      mergeDefData.merges[i].count = counts[i]

      // acc
      accDefData.accs[i] = {}
      accDefData.accs[i].condition = conditions[i]
      accDefData.accs[i].acc = accs[i]
      accDefData.accs[i].count = counts[i]
      accDefData.accs[i].termination = terminations[i]

      // velocity
      velocityDefData.vels[i] = {}
      velocityDefData.vels[i].condition = conditions[i]
      velocityDefData.vels[i].velocity = vels[i]
      velocityDefData.vels[i].count = counts[i]
    }

    if (aid == this.sceneParser.getDefaultAccID()) {
      if (accs.length > 1) {
        aid = this.addAccData(accDefData)
      } else if (accs.length === 1 &&
      (!(conditions[0].type == TriggerCondition.TimeAbsolute &&
      conditions[0].value == 0 && accs[0] == 0))) {
        aid = this.addAccData(accDefData)
      }
    } else {
      accDefData.id = aid
      aid = this.addAccData(accDefData)
    }

    if (mid == this.sceneParser.getDefaultMergeID()) {
      if (directions.length > 1) {
        mid = this.addMergeData(mergeDefData)
      } else if (directions.length === 1 &&
      (!(conditions[0].type == TriggerCondition.TimeAbsolute && conditions[0].value == 0 &&
      (directions[0] === 'static' || directions[0] == null || directions[0] == undefined)))) {
        mid = this.addMergeData(mergeDefData)
      }
    } else {
      mergeDefData.id = mid
      mid = this.addMergeData(mergeDefData)
    }

    if (velocityDefData.id) {
      this.addVelocityData(velocityDefData)
    }

    return { mid, aid, velocity: velocityDefData }
  }

  /**
   * 添加一个交通车（回调）
   * @param carDefData
   * @return {Promise<string|*>}
   */
  async addCar (carDefData) {
    const v = new Vehicle()

    v.routeId = carDefData.routeId
    v.laneId = carDefData.laneId
    v.accId = carDefData.acceleration
    v.mergeId = carDefData.fittingStrategy
    v.type = carDefData.carType
    v.behavior = carDefData.behavior
    if (v.behavior === VehicleBehaviorType.TrafficVehicle) {
      v.aggress = carDefData.aggress
    } else if (v.behavior === VehicleBehaviorType.UserDefine) {
      v.aggress = '-1'
    }
    v.angle = carDefData.angle
    v.endPosArr = carDefData.endPosArr
    v.aiModel = carDefData.aiModel
    v.startShift = carDefData.startShift
    v.startTime = carDefData.startTime
    v.startVelocity = carDefData.startVelocity
    v.startOffset = carDefData.startOffset
    v.maxVelocity = carDefData.maxVelocity
    v.height = carDefData.carHeight
    v.length = carDefData.carLength
    v.width = carDefData.carWidth
    v.index = carDefData.index
    v.follow = carDefData.follow
    v.startAlt = carDefData.startAlt
    v.eventId = carDefData.eventId
    v.triggers = carDefData.triggers
    v.sceneevents = carDefData.sceneevents
    v.sensorGroup = carDefData.sensorGroup
    v.obuStatus = carDefData.obuStatus
    v.start_angle = carDefData.start_angle
    v.boundingBox = carDefData.boundingBox

    Vehicle.mergeTriggerToCondition(carDefData)
    const c = this.addCustomData(carDefData.id, v.mergeId, v.accId, carDefData)
    v.accId = `${c.aid}`
    v.mergeId = `${c.mid}`
    const velocityDefData = c.velocity
    carDefData.fittingStrategy = v.mergeId
    carDefData.acceleration = v.accId

    if (carDefData.id == undefined) {
      await this.addVehicle(v)
      carDefData.id = v.id
      velocityDefData.id = v.id
      this.addVelocityData(velocityDefData)
    } else {
      v.id = carDefData.id
      this.modifyVehicle(v)
    }
    return v.id
  }

  /**
   * 在场景中拖放一个交通车
   * @param type
   * @param startLon
   * @param startLat
   * @param pickLon
   * @param pickLat
   * @param laneInfo
   * @return {Promise<string|*>}
   */
  async addCarByDrop (type, startLon, startLat, pickLon, pickLat, laneInfo) {
    console.log('addCarByDrop', laneInfo)
    const data = Vehicle.createVehicleData(type)
    if (laneInfo.type === MapElementType.LANE) {
      data.routeId = this.addRouteByStart(startLon, startLat, laneInfo.type, laneInfo.rid, laneInfo.sid)
      data.laneId = laneInfo.lid
    } else if (laneInfo.type === MapElementType.LANELINK) {
      data.routeId = this.addRouteByStart(pickLon, pickLat, laneInfo.type, laneInfo.llid)
      data.laneId = laneInfo.llid
    } else {
      console.error('mapelement error!')
      return
    }

    const route = this.sceneParser.findRouteInMap(data.routeId)
    route.realStartLon = pickLon
    route.realStartLat = pickLat

    const accIds = this.getAccIDs()
    const mergeIds = this.getMergeIDs()

    if (accIds.length < 1 || mergeIds.length < 1) {
      console.error('no acc or merge')
      return
    }

    data.acceleration = this.sceneParser.getDefaultAccID()
    data.fittingStrategy = this.sceneParser.getDefaultMergeID()
    data.startShift = laneInfo.dist - this.LaneStartShift
    if (data.startShift < 0) {
      data.startShift = 0
    }
    data.startOffset = laneInfo.offset
    data.startAlt = laneInfo.alt
    data.start_angle = laneInfo.yaw * MathUtils.RAD2DEG

    const carData = getVehicleTypeDefinition(type)
    Object.assign(data, carData)
    return this.addCar(data)
  }

  /**
   * 添加一个交通车轨迹点
   * @param vehicle
   */
  addVehiclePathEnd (vehicle) {
    const route = this.sceneParser.findRouteInMap(vehicle.routeId)
    if (route.curve) {
      return
    }

    route.curve = new EditorCurve(this, true)
    const vec = vehicle.model.position

    let { velocity = vehicle.startVelocity, gear = 'drive' } = route.endPosArr[0] || {}
    route.curve.modifyPoint(0, vec, false, { velocity, gear });
    ({ velocity = vehicle.startVelocity, gear = 'drive' } = route.endPosArr[1] || {})
    route.curve.modifyPoint(1, vec, false, { velocity, gear })
    const len = route.endPosArr.length
    if (len > 1) {
      const tmp = this.hadmap.ct.lonlat2xyz(
        route.endPosArr[1].lon,
        route.endPosArr[1].lat,
        route.endPosArr[1].alt,
      ) // fix height
      const vec = new Vector3(tmp[0], tmp[1], tmp[2])
      route.curve.modifyPoint(1, vec, false)
    } else {
      route.curve.removePoint(1)
    }
    for (let i = 2; i < len; ++i) {
      const tmp = this.hadmap.ct.lonlat2xyz(
        route.endPosArr[i].lon,
        route.endPosArr[i].lat,
        route.endPosArr[i].alt,
      ) // fix height
      const vec = new Vector3(tmp[0], tmp[1], tmp[2]) // fix height
      const { velocity = vehicle.startVelocity, gear = 'drive' } = route.endPosArr[i]
      const attr = { velocity, gear }
      route.curve.addPoint(vec, false, attr)
    }

    route.curve.setTrackEnabled(!!vehicle.trajectoryEnabled)
    route.curve.updateModels()
    this.sceneParser.addVehicleEndModel(vehicle)
  }

  /**
   * 添加一个交通车模型
   * @param vehicle
   * @return {Promise<void>}
   */
  async addVehicleVisual (vehicle) {
    const cm = await this.modelsManager.loadCarModel(vehicle)
    vehicle.model = cm
    vehicle.model.ignoreScale = cm.ignoreScale
    let { tmp } = vehicle
    if (!tmp) {
      tmp = await this.getFinalPosByRoute4(vehicle.routeId, vehicle.laneId, vehicle.startShift, vehicle.startOffset)
    }
    delete vehicle.tmp
    if (tmp) {
      vehicle.model.position.set(tmp[0], tmp[1], tmp[2])
      if (vehicle.model.ignoreScale !== true) {
        vehicle.model.scale.set(vehicle.length, vehicle.width, 1)
      }
      vehicle.startAlt = tmp[4]
      this.alignObject(vehicle, vehicle.model.position, null, vehicle.start_angle)
      this.sceneParser.addVehicleModel(vehicle)
      const route = this.sceneParser.findRouteInMap(vehicle.routeId)
      let startPT = null
      if (!route.realStartLon) {
        startPT = this.ct.xyz2lonlat(tmp[0], tmp[1], tmp[2])
        route.realStartLon = startPT[0]
        route.realStartLat = startPT[1]
      }

      if (route.endPosArr.length === 0) {
        if (!startPT) {
          startPT = this.ct.xyz2lonlat(tmp[0], tmp[1], tmp[2])
        }
        const [lon, lat, alt] = startPT
        route.endPosArr.push({
          lon,
          lat,
          alt,
          velocity: Vehicle.StartSpeed,
          gear: 'drive',
        })
      }
      this.addVehiclePathEnd(vehicle)
    } else {
      console.error('position error!', vehicle.routeId, vehicle.laneId)
    }
    this.scene.add(vehicle.model)
    EditorGlobals.signals.dropModifyObject.dispatch({ type: 'car', data: vehicle.carFormData() })
    this.renderScene()
  }

  /**
   * 复制一个交通车
   * @param carDefData
   * @return {string|*}
   */
  copyCar (carDefData) {
    console.log('copyCar')
    if (carDefData.id == undefined) {
      console.error('copy car no id ')
      return
    }
    const v = new Vehicle()
    v.id = carDefData.id
    const rawV = this.sceneParser.findVehicleInMap(v.id)
    const routeRaw = this.sceneParser.findRouteInMap(rawV.routeId)
    const routeNew = this.sceneParser.findRouteInMap(carDefData.routeId)
    routeRaw.copyNoID(routeNew, this.hadmap.ct)
    v.routeId = rawV.routeId
    this.sceneParser.addVehicleEndModel(rawV)
    v.setRoute(routeRaw)

    Vehicle.mergeTriggerToCondition(carDefData)
    const c = this.addCustomData(carDefData.id, rawV.mergeId, rawV.accId, carDefData)
    v.accId = `${c.aid}`
    v.mergeId = `${c.mid}`

    v.laneId = carDefData.laneId
    v.type = carDefData.carType
    v.behavior = carDefData.behavior
    if (v.behavior === VehicleBehaviorType.TrafficVehicle) {
      v.aggress = carDefData.aggress
    } else if (v.behavior === VehicleBehaviorType.UserDefine) {
      v.aggress = '-1'
    } else if (v.behavior === VehicleBehaviorType.TrajectoryFollow) {
      v.start_angle = carDefData.start_angle
    }

    v.startShift = carDefData.startShift
    v.startTime = carDefData.startTime
    v.startVelocity = carDefData.startVelocity
    v.startOffset = carDefData.offset
    v.maxVelocity = carDefData.maxVelocity
    v.height = carDefData.carHeight
    v.length = carDefData.carLength
    v.width = carDefData.carWidth
    v.index = carDefData.index
    v.follow = carDefData.follow
    v.startAlt = carDefData.startAlt
    v.angle = carDefData.angle
    v.eventId = carDefData.eventId
    v.triggers = carDefData.triggers
    this.modifyVehicle(v)
    return v.id
  }

  /**
   * 添加一个交通车
   * @param vehicle
   * @return {Promise<void>}
   */
  async addVehicle (vehicle) {
    this.sceneParser.addVehicle(vehicle)
    await this.addVehicleVisual(vehicle)
  }

  /**
   * 删除一个交通车
   * @param id
   * @return {{}}
   */
  removeVehicle (id) {
    const res = {}
    res.result = true
    const v = this.sceneParser.findVehicleInMap(id)
    if (!v) {
      res.result = false
      res.msg = i18n.t('tips.vehicleNotExist', { id })
      return res
    }

    if (this.isObjectGroupSelected(v)) {
      this.cancelPickedObject()
    }

    this.sceneParser.removeVehicle(id)
    this.scene.remove(v.model)
    v.model = null

    this.removeRoute(v.routeId)
    this.renderScene()

    return res
  }

  /**
   * 删除一个交通车轨迹点
   * @param id
   * @param idx
   */
  removeVehicleEnd (id, idx) {
    // don't remove when adding rout
    if (this.State() === SceneState.StatePickVehicleEnd) {
      return
    }

    const v = this.sceneParser.findVehicleInMap(id)
    if (v) {
      const route = this.sceneParser.findRouteInMap(v.routeId)

      if (route.curve) {
        if (idx === -1) {
          const length = route.curve.Points().length
          for (let i = length - 1; i > 0; i--) {
            if (this.isObjectPartSelected(route.curve.Boxes()[i])) {
              this.cancelPickedObject(true)
            }
            route.curve.removePoint(i)
          }
        } else {
          if (this.isObjectPartSelected(route.curve.Boxes()[idx])) {
            this.cancelPickedObject(true)
          }
          route.curve.removePoint(idx)
        }
      }

      route.updateRoutePointsFromCurve(this.ct)
      v.setRoute(route)
      this.renderScene()
    }
  }

  /**
   * 更新一个交通车模型（位置旋转等）
   * @param vehicle
   * @param position
   * @return {Promise<void>}
   */
  async modifyVehicleVisual (vehicle, position) {
    const v = this.sceneParser.findVehicleInMap(vehicle.id)
    const route = this.sceneParser.findRouteInMap(v.routeId)
    route.updateRouteModel(this.ct)
    v.model.position.set(position[0], position[1], position[2])
    if (v.model.ignoreScale !== true) {
      v.model.scale.set(v.length, v.width, 1)
    }
    this.alignObject(v, v.model.position, null, v.start_angle)
    v.model.updateMatrixWorld(true)
    route.modifyOnePoint(0, v.model.position, true, this.hadmap.ct)
    if ((+route.endLon) < -180 && route.curve?.Points().length > 1) {
      route.modifyOnePoint(1, v.model.position, false)
    }
    if (this.transformControl.object?.uuid === v.model.uuid) {
      this.pickBox.pick(v.model)
    }

    [route.realStartLon, route.realStartLat] = this.ct.xyz2lonlat(v.model.position.x, v.model.position.y, 0)
    EditorGlobals.signals.dropModifyObject.dispatch({ type: 'car', data: v.carFormData() })
    this.renderScene()
  }

  /**
   * 更新一个交通车数据
   * @param vehicle
   * @return {Promise<void>}
   */
  async modifyVehicle (vehicle) {
    this.sceneParser.modifyVehicle(vehicle)
    const v = this.sceneParser.findVehicleInMap(vehicle.id)
    const tmp = await this.getFinalPosByRoute4(v.routeId, v.laneId, v.startShift, v.startOffset)
    const { autoFollowRoadDirection } = store.state.system.editor
    if (!v.trajectoryEnabled || autoFollowRoadDirection) {
      const lonlat = this.ct.xyz2lonlat(tmp[0], tmp[1], tmp[2])
      const laneInfo = await this.hadmap.getNearbyLaneInfo(lonlat[0], lonlat[1], lonlat[2])
      vehicle.start_angle = v.start_angle = laneInfo.yaw * MathUtils.RAD2DEG
    }
    await this.modifyVehicleVisual(vehicle, tmp)
  }

  /**
   * 拖放的方式添加一辆主车
   * @param name
   * @param lon
   * @param lat
   * @param laneInfo
   * @return {Promise<number>}
   */
  async addPlannerByDrop (name, lon, lat, laneInfo) {
    const plannerDef = this.catalogs?.plannerList.find(p => p.variable === name)
    if (!plannerDef) {
      console.error('尚未加载catalogs')
      return
    }
    const {
      type,
      rid,
      sid,
      lid,
      dist,
      offset,
      alt,
      yaw,
    } = laneInfo
    const plannerData = Planner.createPlannerData()
    plannerData.type = plannerDef.catalogSubCategory
    plannerData.name = name
    plannerData.routeId = this.addRouteByStart(lon, lat, type, rid, sid)
    plannerData.laneId = lid
    plannerData.startShift = Math.max(0, dist)
    plannerData.startOffset = offset
    plannerData.startAlt = alt
    plannerData.start_angle = yaw * MathUtils.RAD2DEG

    const planner = new Planner(this)
    planner.setRoute(this.sceneParser.findRouteInMap(plannerData.routeId))
    planner.copyNoModel(plannerData)
    await this.addPlanner(planner)
    return planner.id
  }

  /**
   * 添加一个主车轨迹点
   * @param planner
   */
  addPlannerPathEnd (planner) {
    const route = this.sceneParser.findRouteInMap(planner.routeId)
    if (route.curve) {
      return
    }

    route.curve = new EditorCurve(this, true)
    const vec = planner.model.position

    let { velocity = planner.startVelocity, gear = 'drive' } = route.endPosArr[0] || {}
    route.curve.modifyPoint(0, vec, false, { velocity, gear });
    ({ velocity = planner.startVelocity, gear = 'drive' } = route.endPosArr[1] || {})
    route.curve.modifyPoint(1, vec, false, { velocity, gear })
    const len = route.endPosArr.length
    if (len > 1) {
      const tmp = this.hadmap.ct.lonlat2xyz(
        route.endPosArr[1].lon,
        route.endPosArr[1].lat,
        route.endPosArr[1].alt,
      ) // fix height
      const vec = new Vector3(tmp[0], tmp[1], tmp[2])
      route.curve.modifyPoint(1, vec, false)
    } else {
      route.curve.removePoint(1)
    }
    for (let i = 2; i < len; ++i) {
      const tmp = this.hadmap.ct.lonlat2xyz(
        route.endPosArr[i].lon,
        route.endPosArr[i].lat,
        route.endPosArr[i].alt,
      ) // fix height
      const vec = new Vector3(tmp[0], tmp[1], tmp[2]) // fix height
      const { velocity = planner.startVelocity, gear = 'drive' } = route.endPosArr[i]
      const attr = { velocity, gear }
      route.curve.addPoint(vec, false, attr)
    }

    route.curve.setTrackEnabled(!!planner.trajectory_enabled)
    route.curve.updateModels()
    this.sceneParser.addPlannerEndModel(planner)
  }

  /**
   * 添加一个主车模型
   * @param planner
   * @return {Promise<void>}
   */
  async addPlannerVisual (planner) {
    planner.model = await this.modelsManager.loadPlannerModel(planner.name)
    const tmp = await this.getPlannerPosition(planner)
    if (tmp) {
      planner.model.position.set(tmp[0], tmp[1], tmp[2])
      planner.startAlt = tmp[3]

      this.alignObject(planner, planner.model.position, null, planner.start_angle)
      this.sceneParser.addPlannerModel(planner)

      const route = this.sceneParser.findRouteInMap(planner.routeId)
      if (route.endPosArr.length === 0) {
        // 设置第一个轨迹点信息
        const [lon, lat, alt] = this.ct.xyz2lonlat(tmp[0], tmp[1], tmp[2])
        route.endPosArr.push({
          lon,
          lat,
          alt,
          gear: 'drive',
          velocity: planner.startVelocity,
        })
      }
      this.addPlannerPathEnd(planner)
    } else {
      console.error('position error!', planner.routeId)
    }
    this.scene.add(planner.model)
    EditorGlobals.signals.dropModifyObject.dispatch({ type: 'planner', data: planner.plannerFormData() })
    this.renderScene()
  }

  /**
   * 添加一个主车
   * @param planner
   * @return {Promise<void>}
   */
  async addPlanner (planner) {
    // 限制可添加的主车数量
    const enableEgoNumber = getPermission('config.app.egoNumber')
    const currentPlannerLength = this.sceneParser.planners?.length || 0
    if (currentPlannerLength >= enableEgoNumber) {
      const msg = `主车数量不可超过 ${enableEgoNumber}`
      MessageBox.promptEditorUIMessage('error', msg)
      return
    }

    this.sceneParser.addPlanner(planner)
    await this.addPlannerVisual(planner)
  }

  /**
   * 删除一个主车
   * @param id
   * @return {{}}
   */
  removePlanner (id) {
    const res = {}
    res.result = true

    const planner = this.sceneParser.findPlannerInArr(id)
    if (!planner) {
      res.result = false
      res.msg = i18n.t('tips.plannerNotExists')
      return res
    }

    this.cancelPickedObject()
    this.scene.remove(planner.model)
    this.sceneParser.removePlanner(planner)
    this.removeRoute(planner.routeId)
    planner.model = null

    this.renderScene()

    return res
  }

  /**
   * 删除一个主车轨迹点
   * @param id
   * @param idx
   */
  removePlannerEnd (id, idx) {
    // don't remove when adding rout
    if (this.State() === SceneState.StatePickEgoCarEnd) {
      return
    }

    const p = this.sceneParser.findPlannerInArr(id)
    if (p) {
      const { route } = p

      if (route.curve) {
        if (idx === -1) {
          const length = route.curve.Points().length
          for (let i = length - 1; i > 0; i--) {
            if (this.isObjectPartSelected(route.curve.Boxes()[i])) {
              this.cancelPickedObject(true)
            }
            route.curve.removePoint(i)
          }
        } else {
          if (this.isObjectPartSelected(route.curve.Boxes()[idx])) {
            this.cancelPickedObject(true)
          }
          route.curve.removePoint(idx)
        }
      }

      route.updateRoutePointsFromCurve(this.ct)
      p.setRoute(route)
      this.renderScene()
    }
  }

  // 更改主车之后同步主车位置
  async syncPlannerVisual (id) {
    const p = this.sceneParser.findPlannerInArr(id)
    const position = p.model.position.clone()
    const rotation = p.model.rotation.clone()
    const selected = p.model === this.curPickObject && this.pickBox.isPicked()
    this.scene.remove(p.model)
    p.model = await this.modelsManager.loadPlannerModel(p.name)
    this.scene.add(p.model)
    p.model.position.copy(position)
    p.model.rotation.copy(rotation)
    if (selected) {
      this.transformControl.detach()
      this.curPickObject = p.model
      this.pickBox.pick(p.model)
      this.transformControl.attach(p.model)
    }
    this.renderScene()
  }

  /**
   * 更新主车模型（位置朝向）
   * @param planner
   * @param position
   * @return {Promise<void>}
   */
  async modifyPlannerVisual (planner, position) {
    const p = this.sceneParser.findPlannerInArr(planner.id)
    const route = this.sceneParser.findRouteInMap(planner.routeId)
    const lonlat = await this.hadmap.getLonLat(
      route.roadId,
      route.sectionId,
      planner.laneId,
      planner.startShift,
      planner.startOffset,
    )
    route.startLon = lonlat.lon
    route.startLat = lonlat.lat
    if (route.startRPM) {
      route.startRPM.setPos(position[0], position[1], position[2])
    }
    p.model.position.set(position[0], position[1], position[2])
    this.alignObject(p, p.model.position, null, p.start_angle)
    p.model.updateMatrixWorld(true)
    route.modifyOnePoint(0, p.model.position, true, this.hadmap.ct)
    if ((+route.endLon) < -180 && route.curve?.Points().length > 1) {
      route.modifyOnePoint(1, p.model.position, false)
    }

    if (this.transformControl.object?.uuid === p.model.uuid) {
      this.pickBox.pick(p.model)
    }

    // 需要先发送数据更改
    EditorGlobals.signals.dropModifyObject.dispatch({ type: 'planner', data: p.plannerFormData() })
    this.renderScene()
  }

  /**
   * 更新主车数据（会导致模型重新渲染）
   * @param planner
   * @return {Promise<void>}
   */
  async modifyPlanner (planner) {
    this.sceneParser.modifyPlanner(planner)
    const p = this.sceneParser.findPlannerInArr(planner.id)
    p.route.curve.setTrackEnabled(!!planner.trajectory_enabled)
    const tmp = await this.getFinalPosByRoute2(planner.routeId, planner.laneId, planner.startShift, planner.startOffset)
    const { autoFollowRoadDirection } = store.state.system.editor
    if (autoFollowRoadDirection) {
      const lonlat = this.ct.xyz2lonlat(tmp[0], tmp[1], tmp[2])
      const laneInfo = await this.hadmap.getNearbyLaneInfo(lonlat[0], lonlat[1], lonlat[2])
      planner.start_angle = p.start_angle = laneInfo.yaw * MathUtils.RAD2DEG
    }
    await this.modifyPlannerVisual(planner, tmp)
  }

  /**
   * 更新主车数据（不会导致重新渲染）
   * @param plannerData
   * @return {Promise<*>}
   */
  async modifyPlannerData (plannerData) {
    const planner = { ...plannerData }
    if (planner.subType === 'car') {
      planner.type = 'car'
    } else {
      planner.type = 'combination'
    }
    await this.modifyPlanner(planner)
    return planner.id
  }

  /**
   * 编辑模型配置后实时更新场景里的模型
   * @param {'planner'|'car'|'pedestrian'|'obstacle'} type
   * @param {number} id
   * @return {Promise<void>}
   */
  async syncVisual (type, id) {
    let target
    let method
    let name
    switch (type) {
      case 'planner':
        target = this.sceneParser.findPlannerInArr(id)
        method = 'loadPlannerModel'
        name = target.name
        break
      case 'car':
        target = this.sceneParser.findVehicleInArr(id)
        method = 'loadCarModel'
        name = target
        break
      case 'pedestrian':
        target = this.sceneParser.findPedestrianInArr(id)
        method = 'loadPedestrianModel'
        name = target
        break
      case 'obstacle':
        target = this.sceneParser.findObstacleInArr(id)
        method = 'loadObstacleModel'
        name = target
        break
    }
    if (!target) return
    const position = target.model.position.clone()
    const rotation = target.model.rotation.clone()
    const selected = target.model === this.curPickObject && this.pickBox.isPicked()
    this.scene.remove(target.model)
    target.model = await this.modelsManager[method](name)
    this.scene.add(target.model)
    target.model.position.copy(position)
    target.model.rotation.copy(rotation)
    if (selected) {
      this.transformControl.detach()
      this.curPickObject = target.model
      this.pickBox.pick(target.model)
      this.transformControl.attach(target.model)
    }
    this.renderScene()
  }

  /**
   * 添加一个ros数据
   * @param rosbagData
   */
  addRosbagData (rosbagData) {
    console.debug(rosbagData)

    const rosbag = new Rosbag()
    rosbag.path = rosbagData.path

    if (rosbagData.id == undefined) {
      this.addRosbag(rosbag)
    } else {
      rosbag.id = rosbagData.id
      this.modifyRosbag(rosbag)
    }
  }

  /**
   * 添加一个ros
   * @param rosbag
   */
  addRosbag (rosbag) {
    this.sceneParser.addRosbag(rosbag)
  }

  /**
   * 删除一个ros
   * @param id
   * @return {{}}
   */
  removeRosbag (id) {
    const res = {}
    res.result = true

    if (this.sceneParser.rosbag == undefined) {
      res.result = false
      res.msg = i18n.t('tips.rosbagNotExists')
      return res
    }

    this.sceneParser.removeRosbag()

    return res
  }

  /**
   * 更新一个ros
   * @param rosbag
   */
  modifyRosbag (rosbag) {
    this.sceneParser.modifyRosbag(rosbag)
  }

  /**
   * 删除一个交通灯
   * @param id
   * @return {{}}
   */
  removeSignlight (id) {
    console.warn('removeSignlight')
    const res = {}
    res.result = true

    const s = this.sceneParser.findSignlightInMap(id)
    if (!s) {
      res.result = false
      res.msg = `信号灯不存在：${id}`
      return res
    }

    if (this.isObjectGroupSelected(s)) {
      this.cancelPickedObject()
    }

    this.sceneParser.removeSignlight(id)
    s.removeFromScene(this.scene)
    if (this.curPickObject === s.model && this.pickBox.isPicked()) {
      this.curPickObject = undefined
      this.curPickUnit = undefined
      this.pickBox.hide()
    }
    s.model = null
    s.models = null
    this.removeRoute(s.routeId)
    this.renderScene()
    return res
  }

  /**
   * 复制一个行人
   * @param pedestrianData
   * @return {string|*}
   */
  copyPedestrianDataV2 (pedestrianData) {
    console.debug('copy PedestrianV2 data')

    const pedestrian = new PedestrianV2()
    pedestrian.id = pedestrianData.id

    const rawP = this.sceneParser.findPedestrianInMap(pedestrian.id)

    const routeRaw = this.sceneParser.findRouteInMap(rawP.routeId)
    const routeNew = this.sceneParser.findRouteInMap(pedestrianData.routeId)
    routeRaw.copyNoID(routeNew, this.hadmap.ct)
    pedestrian.routeId = routeRaw.id
    this.sceneParser.addPedestrianEndModel(rawP)
    pedestrian.setRoute(routeRaw)

    pedestrian.behavior = pedestrianData.behavior
    if (pedestrian.behavior === VehicleBehaviorType.TrajectoryFollow) {
      pedestrian.start_angle = pedestrianData.start_angle
    }

    pedestrian.laneId = pedestrianData.laneId
    pedestrian.startOffset = pedestrianData.offset
    pedestrian.startShift = pedestrianData.startShift
    pedestrian.startVelocity = pedestrianData.startVelocity
    pedestrian.maxVelocity = pedestrianData.maxVelocity
    pedestrian.index = pedestrianData.index
    pedestrian.subType = pedestrianData.subType
    pedestrian.startAlt = pedestrianData.startAlt
    pedestrian.eventId = pedestrianData.eventId

    PedestrianV2.mergeTriggerToCondition(pedestrianData)
    pedestrian.conditions = pedestrianData.conditions
    pedestrian.directions = pedestrianData.directions
    pedestrian.velocities = pedestrianData.velocities
    pedestrian.counts = pedestrianData.counts
    this.modifyPedestrian(pedestrian)

    return pedestrian.id
  }

  /**
   * 添加一个行人数据（回调）
   * @param pedestrianData
   * @return {string|*}
   */
  addPedestrianData (pedestrianData) {
    console.debug(pedestrianData)

    const pedestrian = new PedestrianV2()
    pedestrian.routeId = pedestrianData.routeId
    pedestrian.laneId = pedestrianData.laneId
    pedestrian.startOffset = pedestrianData.offset
    pedestrian.startShift = pedestrianData.startShift
    pedestrian.startVelocity = pedestrianData.startVelocity
    pedestrian.maxVelocity = pedestrianData.maxVelocity
    pedestrian.endPosArr = pedestrianData.endPosArr
    pedestrian.index = pedestrianData.index
    pedestrian.subType = pedestrianData.subType

    PedestrianV2.mergeTriggerToCondition(pedestrianData)
    pedestrian.conditions = pedestrianData.conditions
    pedestrian.directions = pedestrianData.directions
    pedestrian.velocities = pedestrianData.velocities
    pedestrian.counts = pedestrianData.counts
    pedestrian.startAlt = pedestrianData.startAlt
    pedestrian.eventId = pedestrianData.eventId
    pedestrian.triggers = pedestrianData.triggers
    pedestrian.behavior = pedestrianData.behavior
    pedestrian.angle = pedestrianData.angle
    pedestrian.start_angle = pedestrianData.start_angle
    pedestrian.boundingBox = pedestrianData.boundingBox

    if (pedestrianData.id == undefined) {
      this.addPedestrian(pedestrian)
      pedestrianData.id = pedestrian.id
    } else {
      pedestrian.id = pedestrianData.id
      this.modifyPedestrian(pedestrian)
    }

    return pedestrian.id
  }

  /**
   * 拖放的方式添加一个行人
   * @param type
   * @param startLon
   * @param startLat
   * @param pickLon
   * @param pickLat
   * @param laneInfo
   * @return {string|*}
   */
  addPedestrianByDrop (type, startLon, startLat, pickLon, pickLat, laneInfo) {
    console.log('addPedestrianByDrop', type)
    const data = PedestrianV2.createPedestrianData(type)
    if (laneInfo.type === MapElementType.LANE) {
      data.routeId = this.addRouteByStart(startLon, startLat, laneInfo.type, laneInfo.rid, laneInfo.sid)
      data.laneId = laneInfo.lid
    } else if (laneInfo.type === MapElementType.LANELINK) {
      data.routeId = this.addRouteByStart(pickLon, pickLat, laneInfo.type, laneInfo.llid)
      data.laneId = laneInfo.llid
    } else {
      console.error('mapelement error!')
      return
    }

    const route = this.sceneParser.findRouteInMap(data.routeId)
    route.realStartLon = pickLon
    route.realStartLat = pickLat

    data.startShift = laneInfo.dist - this.LaneStartShift
    if (data.startShift < 0) {
      data.startShift = 0
      console.log(`pedestrian shift ${data.startShift} less than 0  `)
    }
    data.offset = laneInfo.offset
    data.subType = type
    data.startAlt = laneInfo.alt
    data.start_angle = laneInfo.yaw * MathUtils.RAD2DEG

    return this.addPedestrianData(data)
  }

  /**
   * 添加一个行人轨迹点
   * @param pedestrian
   */
  addPedestrianPathEnd (pedestrian) {
    const route = this.sceneParser.findRouteInMap(pedestrian.routeId)
    if (route.curve) {
      return
    }

    route.curve = new EditorCurve(this, pedestrian.trajectoryEnabled, this.PedestrianBoxSize)
    const vec = pedestrian.model.position

    if (pedestrian.trajectoryEnabled) {
      let {
        velocity = pedestrian.startVelocity,
        gear = 'drive',
        frontwheel = null,
        accs = null,
        heading = null,
      } = route.endPosArr[0] || {}
      route.curve.modifyPoint(0, vec, false, {
        velocity,
        gear,
        frontwheel,
        accs,
        heading,
      });
      ({
        velocity = pedestrian.startVelocity,
        gear = 'drive',
        frontwheel = null,
        accs = null,
        heading = null,
      } = route.endPosArr[1] || {})
      route.curve.modifyPoint(1, vec, false, {
        velocity,
        gear,
        frontwheel,
        accs,
        heading,
      })
    } else {
      route.curve.modifyPoint(0, vec)
      route.curve.modifyPoint(1, vec)
    }

    const len = route.endPosArr.length
    if (len > 1) {
      const tmp = this.hadmap.ct.lonlat2xyz(
        route.endPosArr[1].lon,
        route.endPosArr[1].lat,
        route.endPosArr[1].alt,
      )
      const vec = new Vector3(tmp[0], tmp[1], tmp[2])
      route.curve.modifyPoint(1, vec, false) // fix height
    } else {
      route.curve.removePoint(1)
    }
    for (let i = 2; i < len; ++i) {
      const tmp = this.hadmap.ct.lonlat2xyz(
        route.endPosArr[i].lon,
        route.endPosArr[i].lat,
        route.endPosArr[i].alt,
      )
      const vec = new Vector3(tmp[0], tmp[1], tmp[2])
      if (pedestrian.trajectoryEnabled) {
        const { velocity = pedestrian.startVelocity, gear = 'drive' } = route.endPosArr[i]
        const attr = { velocity, gear }
        route.curve.addPoint(vec, false, attr)
      } else {
        route.curve.addPoint(vec, false)
      }
    }

    if (pedestrian.trajectoryEnabled) {
      route.curve.setTrackEnabled(true)
    }

    route.curve.updateModels()

    this.sceneParser.addPedestrianEndModel(pedestrian)
  }

  /**
   * 添加一个行人模型
   * @param pedestrian
   * @return {Promise<void>}
   */
  async addPedestrianVisual (pedestrian) {
    pedestrian.model = this.modelsManager.loadPedestrianModelSync(pedestrian)
    this.scene.add(pedestrian.model)
    let { tmp } = pedestrian
    if (!tmp) {
      tmp = await this.getFinalPosByRoute4(
        pedestrian.routeId,
        pedestrian.laneId,
        pedestrian.startShift,
        pedestrian.startOffset,
      )
    }
    delete pedestrian.tmp
    if (tmp) {
      pedestrian.model.position.set(tmp[0], tmp[1], tmp[2])
      pedestrian.startAlt = tmp[4] // add height info

      this.alignObject(pedestrian, pedestrian.model.position, null, pedestrian.start_angle)
      this.sceneParser.addPedestrianModel(pedestrian)
      const route = this.sceneParser.findRouteInMap(pedestrian.routeId)
      let startPT = null
      if (!route.realStartLon) {
        startPT = this.ct
          .xyz2lonlat(pedestrian.model.position.x, pedestrian.model.position.y, pedestrian.model.position.z)
        route.realStartLon = startPT[0]
        route.realStartLat = startPT[1]
      }

      if (route.endPosArr.length === 0) {
        if (!startPT) {
          startPT = this.ct
            .xyz2lonlat(pedestrian.model.position.x, pedestrian.model.position.y, pedestrian.model.position.z)
        }
        const [lon, lat, alt] = startPT
        route.endPosArr.push({
          lon,
          lat,
          alt,
          velocity: Vehicle.StartSpeed,
          gear: 'drive',
        })
      }

      this.addPedestrianPathEnd(pedestrian)
    } else {
      console.error('position error!', pedestrian.routeId, pedestrian.laneId)
    }

    EditorGlobals.signals.dropModifyObject.dispatch({
      type: getPedestrianSubType(pedestrian.subType),
      data: pedestrian.pedestrianFormData(),
    })
    this.renderScene()
  }

  /**
   * 添加一个行人
   * @param pedestrian
   * @return {Promise<void>}
   */
  async addPedestrian (pedestrian) {
    this.sceneParser.addPedestrian(pedestrian)
    await this.addPedestrianVisual(pedestrian)
  }

  /**
   * 删除一个行人
   * @param id
   * @return {{}}
   */
  removePedestrian (id) {
    const res = {}
    res.result = true

    const p = this.sceneParser.findPedestrianInMap(id)
    if (!p) {
      res.result = false
      res.msg = `行人不存在： ${id}`
      return res
    }

    if (this.isObjectGroupSelected(p)) {
      this.cancelPickedObject()
    }

    this.sceneParser.removePedestrian(id)
    this.scene.remove(p.model)
    if (this.curPickObject === p.model && this.pickBox.isPicked()) {
      this.curPickObject = undefined
      this.curPickUnit = undefined
      this.pickBox.hide()
    }
    p.model = null
    this.removeRoute(p.routeId)
    this.renderScene()
    return res
  }

  /**
   * idx
   * @param {*} id
   * @param {*} idx
   */
  removePedestrianEnd (id, idx) {
    // delete all when editing
    if (this.State() === SceneState.StatePickPedestrianEnd) {
      return
    }

    const p = this.sceneParser.findPedestrianInMap(id)
    if (p) {
      const route = this.sceneParser.findRouteInMap(p.routeId)

      if (route.curve) {
        if (idx === -1) { // remove all except the first
          const length = route.curve.Points().length
          for (let i = length - 1; i > 0; i--) {
            if (this.isObjectPartSelected(route.curve.Boxes()[i])) {
              this.cancelPickedObject(true)
            }
            route.curve.removePoint(i)
          }
        } else {
          if (this.isObjectPartSelected(route.curve.Boxes()[idx])) {
            this.cancelPickedObject(true)
          }
          route.curve.removePoint(idx)
        }
      }
      route.updateRoutePointsFromCurve(this.ct)
      p.setRoute(route)

      this.renderScene()
    }
  }

  /**
   * 删除一个行人模型
   * @param pedestrian
   * @param position
   * @return {Promise<void>}
   */
  async modifyPedestrianVisual (pedestrian, position) {
    const p = this.sceneParser.findPedestrianInMap(pedestrian.id)

    const route = this.sceneParser.findRouteInMap(p.routeId)
    route.updateRouteModel(this.ct)
    p.model.position.set(position[0], position[1], position[2])
    this.alignObject(p, p.model.position, null, pedestrian.start_angle)
    p.model.updateMatrixWorld(true)
    route.modifyOnePoint(0, p.model.position, true, this.hadmap.ct)
    if ((+route.endLon) < -180 && route.curve?.Points().length > 1) {
      route.modifyOnePoint(1, p.model.position, false)
    }

    if (this.transformControl.object?.uuid === p.model.uuid) {
      this.pickBox.pick(p.model)
    }

    EditorGlobals.signals.dropModifyObject.dispatch({
      type: getPedestrianSubType(p.subType),
      data: p.pedestrianFormData(),
    })
    this.renderScene()
  }

  /**
   * 更新一个行人数据
   * @param pedestrian
   * @return {Promise<void>}
   */
  async modifyPedestrian (pedestrian) {
    this.sceneParser.modifyPedestrian(pedestrian)
    const p = this.sceneParser.findPedestrianInMap(pedestrian.id)
    const tmp = await this.getFinalPosByRoute4(p.routeId, p.laneId, p.startShift, p.startOffset)
    const { autoFollowRoadDirection } = store.state.system.editor
    if (!p.trajectoryEnabled || autoFollowRoadDirection) {
      const lonlat = this.ct.xyz2lonlat(tmp[0], tmp[1], tmp[2])
      const laneInfo = await this.hadmap.getNearbyLaneInfo(lonlat[0], lonlat[1], lonlat[2])
      pedestrian.start_angle = p.start_angle = laneInfo.yaw * MathUtils.RAD2DEG
    }
    await this.modifyPedestrianVisual(pedestrian, tmp)
  }

  /**
   * 添加一个障碍物数据（回调）
   * @param obstacleData
   * @return {string|*}
   */
  addObstacleData (obstacleData) {
    console.debug(obstacleData)

    const obstacle = new Obstacle()
    obstacle.routeId = obstacleData.routeId
    obstacle.laneId = obstacleData.laneId
    obstacle.type = obstacleData.obstacleType
    obstacle.startShift = obstacleData.transfer
    obstacle.startOffset = obstacleData.offset
    obstacle.length = obstacleData.length
    obstacle.width = obstacleData.width
    obstacle.height = obstacleData.height
    obstacle.direction = obstacleData.direction
    obstacle.index = obstacleData.index
    obstacle.startAlt = obstacleData.startAlt
    obstacle.start_angle = obstacleData.start_angle
    obstacle.boundingBox = obstacleData.boundingBox

    if (obstacleData.id == undefined) {
      this.addObstacle(obstacle)
      obstacleData.id = obstacle.id
    } else {
      obstacle.id = obstacleData.id
      this.modifyObstacle(obstacle)
    }
    return obstacle.id
  }

  /**
   * 拖放方式添加一个障碍物
   * @param type
   * @param startLon
   * @param startLat
   * @param pickLon
   * @param pickLat
   * @param laneInfo
   * @return {string|*}
   */
  addObstacleByDrop (type, startLon, startLat, pickLon, pickLat, laneInfo) {
    console.log('addObstacleByDrop')
    const data = Object.assign({ type: 'obstacle' }, Obstacle.createObstacleData(type))

    if (laneInfo.type === MapElementType.LANE) {
      data.routeId = this.addRouteByStart(startLon, startLat, laneInfo.type, laneInfo.rid, laneInfo.sid)
      data.laneId = laneInfo.lid
    } else if (laneInfo.type === MapElementType.LANELINK) {
      data.routeId = this.addRouteByStart(pickLon, pickLat, laneInfo.type, laneInfo.llid)
      data.laneId = laneInfo.llid
    } else {
      console.error('mapelement error!')
      return
    }

    const route = this.sceneParser.findRouteInMap(data.routeId)
    route.realStartLon = pickLon
    route.realStartLat = pickLat

    data.transfer = laneInfo.dist - this.LaneStartShift
    if (data.transfer < 0) {
      data.transfer = 0
      console.log(`obstacle shift ${data.transfer} less than 0  `)
    }
    data.offset = laneInfo.offset
    data.startAlt = laneInfo.alt
    data.start_angle = laneInfo.yaw * MathUtils.RAD2DEG

    const config = getObstacleCfg(type)
    Object.assign(data, config)
    return this.addObstacleData(data)
  }

  /**
   * 添加一个障碍物模型
   * @param obstacle
   * @return {Promise<void>}
   */
  async addObstacleVisual (obstacle) {
    obstacle.model = this.modelsManager.loadObstacleModelSync(obstacle)
    this.scene.add(obstacle.model)
    let { tmp } = obstacle
    if (!tmp) {
      tmp = await this.getFinalPosByRoute4(obstacle.routeId, obstacle.laneId, obstacle.startShift, obstacle.startOffset)
    }
    delete obstacle.tmp
    if (tmp) {
      obstacle.model.position.set(tmp[0], tmp[1], tmp[2])
      this.alignObject(obstacle, obstacle.model.position, null, obstacle.start_angle)

      this.sceneParser.addObstacleModel(obstacle)
      EditorGlobals.signals.dropModifyObject.dispatch({ type: 'obstacle', data: obstacle.obstacleFormData() })
    } else {
      console.error('position error! ', obstacle.routeId, obstacle.laneId)
    }
    this.renderScene()
  }

  /**
   * 添加一个障碍物
   * @param obstacle
   * @return {Promise<void>}
   */
  async addObstacle (obstacle) {
    this.sceneParser.addObstacle(obstacle)
    await this.addObstacleVisual(obstacle)
  }

  /**
   * 删除一个障碍物模型
   * @param o
   */
  removeObstacleModel (o) {
    if (this.isObjectGroupSelected(o)) {
      this.cancelPickedObject()
    }

    this.sceneParser.removeObstacleModel(o.id)
    this.scene.remove(o.model)
    if (this.curPickObject === o.model && this.pickBox.isPicked()) {
      this.curPickObject = undefined
      this.curPickUnit = undefined
      this.pickBox.hide()
    }
    o.model = null
  }

  /**
   * 删除一个障碍物
   * @param id
   * @return {{}}
   */
  removeObstacle (id) {
    const res = {}
    res.result = true

    const o = this.sceneParser.findObstacleInMap(id)
    if (!o) {
      res.result = false
      res.msg = `障碍物不存在 ${id}`
      return res
    }

    this.removeObstacleModel(o)
    this.sceneParser.removeObstacle(id)
    this.removeRoute(o.routeId)
    this.renderScene()

    return res
  }

  /**
   * 更新一个障碍物模型
   * @param obstacle
   * @param position
   * @return {Promise<void>}
   */
  async modifyObstacleVisual (obstacle, position) {
    const o = this.sceneParser.findObstacleInMap(obstacle.id)

    const route = this.sceneParser.findRouteInMap(o.routeId)
    route.updateRouteModel(this.ct)
    o.model.position.set(position[0], position[1], position[2])
    this.alignObject(o, o.model.position, null, obstacle.start_angle)
    o.model.updateMatrixWorld(true)
    if (this.transformControl.object?.uuid === o.model.uuid) {
      this.pickBox.pick(o.model)
    }

    EditorGlobals.signals.dropModifyObject.dispatch({ type: 'obstacle', data: o.obstacleFormData() })
    this.renderScene()
  }

  /**
   * 更新一个障碍物
   * @param obstacle
   * @return {Promise<void>}
   */
  async modifyObstacle (obstacle) {
    this.sceneParser.modifyObstacle(obstacle)
    const o = this.sceneParser.findObstacleInMap(obstacle.id)
    const tmp = await this.getFinalPosByRoute4(o.routeId, o.laneId, o.startShift, o.startOffset)
    const { autoFollowRoadDirection } = store.state.system.editor
    if (autoFollowRoadDirection) {
      const lonlat = this.ct.xyz2lonlat(tmp[0], tmp[1], tmp[2])
      const laneInfo = await this.hadmap.getNearbyLaneInfo(lonlat[0], lonlat[1], lonlat[2])
      obstacle.start_angle = o.start_angle = laneInfo.yaw * MathUtils.RAD2DEG
    }
    await this.modifyObstacleVisual(obstacle, tmp)
  }

  /**
   * 旋转一个轨迹点
   * @param type
   * @param id
   * @param idx
   */
  selectPathEnd (type, id, idx) {
    if (idx === 0) {
      this.selectObject(type, id)
      return
    }
    let target
    switch (type) {
      case 'planner':
        target = this.sceneParser.findPlannerInArr(id)
        break
      case 'car':
        target = this.sceneParser.findVehicleInMap(id)
        break
      case 'pedestrian':
        target = this.sceneParser.findPedestrianInMap(id)
        break
      default:
    }
    if (target) {
      const route = this.sceneParser.findRouteInMap(target.routeId)
      if (route.curve) {
        const boxes = route.curve.Boxes()
        if (idx < boxes.length) {
          const box = boxes[idx]
          // 删除选中物体包围盒
          if (this.pickBox.isPicked()) {
            this.pickBox.hide()
            this.transformControl.detach()
          }
          // 给当前选中物体添加包围盒
          this.curPickUnit = box

          this.lastPickObject = this.curPickObject
          this.curPickObject = box
          this.pickBox.pick(this.curPickObject)
          this.transformControl.attach(this.curPickObject)

          this.renderScene()
        }
      }
    }
  }

  /**
   * 判断当前是否可以选择元素
   * @return {boolean}
   */
  canSelectObject () {
    return this.operationMode === OperationMode.PICK // 点选模式
  }

  /**
   * 选择一个元素
   * @param type
   * @param id
   * @param idx
   */
  selectObject (type, id, idx) {
    let selectedObject
    switch (type) {
      case 'car':
        selectedObject = this.sceneParser.findVehicleInMap(id)
        break
      case 'planner':
        selectedObject = this.sceneParser.findPlannerInArr(id)
        break
      case 'bike':
      case 'moto':
      case 'animal':
      case 'pedestrian':
      case 'machine' :
        selectedObject = this.sceneParser.findPedestrianInMap(id)
        break
      case 'obstacle':
        selectedObject = this.sceneParser.findObstacleInMap(id)
        break
      case 'signlight':
        selectedObject = this.sceneParser.findSignlightInMap(id)
        break
      case 'egopath_end':
        return this.selectPathEnd('planner', id, idx)
      case 'vehiclepath_end':
        return this.selectPathEnd('car', id, idx)
      case 'motopath_end':
      case 'bikepath_end':
      case 'animalpath_end':
      case 'pedestrianpath_end':
      case 'machinepath_end':
        return this.selectPathEnd('pedestrian', id, idx)
      default:
        console.error('unknown add data type ', type)
    }

    if (selectedObject?.model !== this.curPickUnit) {
      // 删除选中物体包围盒
      if (this.pickBox.isPicked()) {
        this.pickBox.hide()
        this.transformControl.detach()
      }

      // 给当前选中物体添加包围盒
      this.curPickUnit = selectedObject.model

      this.lastPickObject = this.curPickObject
      this.curPickObject = selectedObject.model

      let tmp = selectedObject.model
      while (tmp.parent.type !== 'Scene') {
        tmp = tmp.parent
      }

      this.curPickObject = tmp

      this.pickBox.pick(this.curPickObject)
      if (type !== 'signlight') {
        this.transformControl.attach(this.curPickObject)
      }

      // cancel select path node in route
      EditorGlobals.signals.pathNodeSelected.dispatch()

      this.renderScene()
    }
  }

  /**
   * 退出选择
   * @return {boolean}
   */
  selectedExist () {
    return !!this.curPickUnit
  }

  /**
   * 判断一个元素group是否被选择了
   * @param obj
   * @return {boolean}
   */
  isObjectGroupSelected (obj) {
    if (this.curPickObject) {
      if (obj.uuid) {
        if (obj.uuid === this.curPickObject.uuid) {
          return true
        }
      } else {
        if (obj.model.uuid === this.curPickObject.uuid) {
          return true
        }
      }
    }

    return false
  }

  /**
   * 判断一个元素的部分是否被选择了
   * @param obj
   * @return {boolean}
   */
  isObjectPartSelected (obj) {
    if (this.curPickUnit) {
      if (obj.uuid) {
        if (obj.uuid === this.curPickUnit.uuid) {
          return true
        }
      } else {
        if (obj.model.uuid === this.curPickUnit.uuid) {
          return true
        }
      }
    }

    return false
  }

  /**
   * 取消选择元素
   * @param bNotSendCancelMsg
   */
  cancelPickedObject (bNotSendCancelMsg) {
    // 删除选中包围盒
    if (this.pickBox.isPicked() && this.transformControl.axis === null) {
      this.curPickUnit = undefined
      this.lastPickObject = this.curPickObject
      this.curPickObject = null

      this.pickBox.hide()
      this.transformControl.detach()
      if (!bNotSendCancelMsg) {
        EditorGlobals.signals.cancelSelectObject2UI.dispatch()
      }
    }
  }

  /**
   * 获取选择的元素id
   * @param type
   * @param uuid
   * @return {*}
   */
  getPickedObjectId (type, uuid) {
    switch (type) {
      case 'car': {
        const v = this.sceneParser.findVehicleInModelMap(uuid)
        return v?.id
      }
      case 'obstacle': {
        const o = this.sceneParser.findObstacleInModelMap(uuid)
        return o?.id
      }
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine': {
        const a = this.sceneParser.findPedestrianInModelMap(uuid)
        return a?.id
      }
      case 'signlight': {
        const s = this.sceneParser.findSignlightInModelMap(uuid)
        return s?.id
      }
      case 'planner': {
        const p = this.sceneParser.findPlannerInModelMap(uuid)
        return p?.id
      }
    }
  }

  /**
   * 拾取一个元素（视图上的）
   * @param event
   */
  pickObject (event) {
    if (SceneState.isEditingCurve(this.State())) {
      return
    }

    const {
      clientWidth,
      clientHeight,
    } = this.wrapper
    const {
      offsetX,
      offsetY,
    } = event

    this.mouseDownPos.set((offsetX / clientWidth) * 2 - 1, -(offsetY / clientHeight) * 2 + 1)
    this.raycaster.setFromCamera(this.mouseDownPos, this.curCamera)

    let picked
    let type

    if (this.operationMode !== OperationMode.PICK) { // 不是选择模式则直接取消选中当前物体
      this.cancelPickedObject()
      this.renderScene()
      return
    }

    // vehicles
    let intersects = this.raycaster.intersectObjects(this.sceneParser.vehicleModels, true)
    if (intersects.length) {
      picked = intersects[0]
      type = 'car'
    }

    // planner
    intersects = this.raycaster.intersectObjects(this.sceneParser.plannerModels, true)
    if (intersects.length) {
      if (picked) {
        picked = picked.distance > intersects[0].distance ? intersects[0] : picked
      } else {
        picked = intersects[0]
        type = 'planner'
      }
    }

    // obstacles
    intersects = this.raycaster.intersectObjects(this.sceneParser.obstacleModels, true)

    if (intersects.length) {
      if (picked) {
        picked = picked.distance > intersects[0].distance ? intersects[0] : picked
      } else {
        picked = intersects[0]
        type = 'obstacle'
      }
    }
    // pedestrians
    intersects = this.raycaster.intersectObjects(this.sceneParser.pedestrianModels, true)
    if (intersects.length) {
      if (picked) {
        picked = picked.distance > intersects[0].distance ? intersects[0] : picked
      } else {
        picked = intersects[0]
        type = 'pedestrian'
      }
    }

    // signlights
    intersects = this.raycaster.intersectObjects(this.sceneParser.signlightModels, true)
    if (intersects.length) {
      if (picked) {
        picked = picked.distance > intersects[0].distance ? intersects[0] : picked
      } else {
        picked = intersects[0]
        type = 'signlight'
      }
    }

    // planner end
    intersects = this.raycaster.intersectObjects(this.sceneParser.plannerEndModels)
    if (intersects.length) {
      if (!this.isFirstRouteBox(intersects[0].object, this.sceneParser.plannersEndModelMap)) {
        if (picked) {
          picked = picked.distance > intersects[0].distance ? intersects[0] : picked
        } else {
          picked = intersects[0]
          type = 'egopath_end'
        }
      }
    }

    // vehicle end
    intersects = this.raycaster.intersectObjects(this.sceneParser.vehicleEndModels)
    if (intersects.length) {
      if (!this.isFirstRouteBox(intersects[0].object, this.sceneParser.vehiclesEndModelMap)) {
        if (picked) {
          picked = picked.distance > intersects[0].distance ? intersects[0] : picked
        } else {
          picked = intersects[0]
          type = 'vehiclepath_end'
        }
      }
    }

    // pedestrian end
    intersects = this.raycaster.intersectObjects(this.sceneParser.pedestrianEndModels)
    if (intersects.length) {
      if (!this.isFirstRouteBox(intersects[0].object, this.sceneParser.pedestriansEndModelMap)) {
        if (picked) {
          picked = picked.distance > intersects[0].distance ? intersects[0] : picked
        } else {
          picked = intersects[0]
          type = 'pedestrianpath_end'
        }
      }
    }

    // measurement end
    intersects = this.raycaster.intersectObjects(this.sceneParser.measurementEndModels)
    if (intersects.length) {
      if (picked) {
        picked = picked.distance > intersects[0].distance ? intersects[0] : picked
      } else {
        picked = intersects[0]
        type = 'measurementpath_end'
      }
    }

    this.doPickObject(type, picked)
  }

  /**
   * 拾取一个元素
   * @param type
   * @param picked
   */
  doPickObject (type, picked) {
    if (picked) {
      if (picked.object !== this.curPickUnit) {
        // 删除选中物体包围盒
        if (this.pickBox.isPicked()) {
          this.pickBox.hide()
          this.transformControl.detach()
        }

        // 给当前选中物体添加包围盒
        this.curPickUnit = picked.object

        let objuuid
        let pickedId
        let sendPathNodeSelecte = false
        let pathNodeType = 'planner'
        let pathNodeIdx = -1

        this.lastPickObject = this.curPickObject
        if (type === 'egopath_end') {
          this.curPickObject = this.curPickUnit
          type = 'planner'
          const planner = this.sceneParser.findPlannerInEndModelMap(this.curPickUnit.uuid)
          objuuid = planner.model.uuid
          pickedId = this.getPickedObjectId(type, objuuid)
          const route = this.sceneParser.findRouteInMap(planner.routeId)
          if (route?.curve) {
            const idx = route.curve.FindBoxIdx(this.curPickUnit.uuid)
            if (+idx !== -1) {
              sendPathNodeSelecte = true
              pathNodeType = 'planner'
              pathNodeIdx = +idx
            }
          }

          this.setCurEditPlanner(planner.id)
        } else if (type === 'vehiclepath_end') {
          this.curPickObject = this.curPickUnit
          type = 'car'
          const v = this.sceneParser.findVehicleInEndModelMap(this.curPickUnit.uuid)
          objuuid = v.model.uuid
          pickedId = this.getPickedObjectId(type, objuuid)
          const route = this.sceneParser.findRouteInMap(v.routeId)
          if (route?.curve) {
            const idx = route.curve.FindBoxIdx(this.curPickUnit.uuid)
            if (+idx !== -1) {
              sendPathNodeSelecte = true
              pathNodeType = 'vehicle'
              pathNodeIdx = +idx
            }
          }

          this.setCurEditVehicle(v.id)
        } else if (type === 'pedestrianpath_end') {
          this.curPickObject = this.curPickUnit

          type = 'pedestrian'
          const p = this.sceneParser.findPedestrianInEndModelMap(this.curPickUnit.uuid)
          objuuid = p.model.uuid
          pickedId = this.getPickedObjectId(type, objuuid)
          const route = this.sceneParser.findRouteInMap(p.routeId)
          if (route?.curve) {
            const idx = route.curve.FindBoxIdx(this.curPickUnit.uuid)
            if (+idx !== -1) {
              type = getPedestrianSubType(p.subType)
              sendPathNodeSelecte = true
              pathNodeType = type
              pathNodeIdx = +idx
            }
          }
          this.setCurEditPedestrian(p.id)
        } else if (type === 'measurementpath_end') {
          this.curPickObject = this.curPickUnit

          const m = this.sceneParser.findMeasurementInEndModelMap(this.curPickUnit.uuid)
          objuuid = this.curPickUnit.uuid
          pickedId = m.id
        } else {
          let tmp = picked.object
          while (tmp.parent.type !== 'Scene') {
            tmp = tmp.parent
          }

          if (type === 'pedestrian') {
            const p = this.sceneParser.findPedestrianInModelMap(tmp.uuid)
            if (p) {
              type = getPedestrianSubType(p.subType)
              this.setCurEditPedestrian(p.id)
            }
          }

          if (type === 'car') {
            const p = this.sceneParser.findVehicleInModelMap(tmp.uuid)
            if (p) {
              this.setCurEditVehicle(p.id)
            }
          }

          if (type === 'planner') {
            const planner = this.sceneParser.findPlannerInModelMap(tmp.uuid)
            if (planner) {
              this.setCurEditPlanner(planner.id)
            }
          }
          this.curPickObject = tmp

          objuuid = this.curPickObject.uuid
          pickedId = this.getPickedObjectId(type, objuuid)
        }

        EditorGlobals.signals.pickedObject.dispatch({ type, id: pickedId, objuuid })
        console.log('send pickedObject')
        if (sendPathNodeSelecte) {
          setTimeout(() => {
            EditorGlobals.signals.pathNodeSelected.dispatch({ type: pathNodeType, index: pathNodeIdx })
          }, 20)
          console.log('send pathNodeSelected')
        }

        this.pickBox.pick(this.curPickObject)
        if (type !== 'signlight') {
          this.transformControl.attach(this.curPickObject)
        }
      }
    } else {
      this.cancelPickedObject()
    }

    this.renderScene()
  }

  /**
   * 加载场景
   * @param sceneId
   * @return {Promise<void>}
   */
  async loadScene (sceneId) {
    LogInfo(`loadScene ${sceneId}`)
    console.log('get scene content')

    const sceneData = await this.sceneParser.getScene(sceneId, true)

    console.log('get scene content finished, begin parse scene')
    this.sceneParser.parseScene(this.sceneParser, sceneData)

    console.log('scene parsed, start to load hadmap')

    await this.loadHadmap(this.sceneParser.mapfile.mapName)

    console.log('load hadmap succeed, begin check routes')

    await this.sceneParser.checkRoutes(this.sceneParser)

    console.log('check routes succeed, begin to assemble scene')

    await this.sceneParser.assembScene()

    console.log('after assembling scene, begin set scene!')

    this.focusPlanner()

    console.log('after set scene, load scene finished!')
  }

  /**
   * 根据地图元素更新地图地面的高度
   */
  updateGrid () {
    try {
      const box = this.hadmap.getBoundingBox()
      this.grid.translateZ(box.min.z)
      this.groundPlane.constant = -box.min.z
    } catch (error) {
      console.warn('updateGird failed...', error)
    }
  }

  setSceneName (sceneName) {
    if (!this.sceneParser.infos) {
      this.sceneParser.infos = {}
    }
    this.sceneParser.infos.sceneName = sceneName
  }

  saveScene (id, sceneName, ext) {
    this.sceneName = sceneName
    return this.sceneParser.saveScene(id, sceneName, ext, this.sceneParser)
  }

  paramScene (id, prefix, payload) {
    return this.sceneParser.paramScene(id, prefix, payload)
  }

  dispose () {
    this.transformControl.dispose()

    if (this.editorControls) {
      this.editorControls.dispose()
    }

    this.renderer.forceContextLoss()
    this.renderer.dispose()

    this.hadmap.dispose()
  }

  setSensorInfo (sensorInfo) {
    this.sceneParser.setSensorInfo(sensorInfo)
  }

  getGenerateInfo () {
    return this.sceneParser.getGenerateInfo()
  }

  getSensorInfo () {
    return this.sceneParser.getSensorInfo()
  }

  setEnvironmentInfo (environmentInfo) {
    this.sceneParser.setEnvironmentInfo(environmentInfo)
  }

  getEnvironmentInfo () {
    return this.sceneParser.getEnvironmentInfo()
  }

  setGradingInfo (gradingInfo) {
    this.sceneParser.setGradingInfo(gradingInfo)
  }

  getGradingInfo () {
    return this.sceneParser.getGradingInfo()
  }

  getUIStateInfo () {
    return {
      factor: this.lastZoom,
    }
  }

  getRoadobj () {
    return this.sceneParser.getRoadobj()
  }

  getSceneevents () {
    return this.sceneParser.getSceneevents()
  }

  setOperationMode (mode) {
    this.operationMode = mode
    if (this.editorControls) {
      this.editorControls.enableRotate = (mode === OperationMode.ROTATE)
    }

    if (mode === 2) { // pan
      this.editorControls.mouseButtons.LEFT = 2 // pan
    } else {
      this.editorControls.mouseButtons.LEFT = 0 // rotate
    }
  }

  // 判断是否是第一个路径轨迹点
  isFirstRouteBox (box, map) {
    if (!box) return false
    const ve = map.get(`${box.uuid}`)
    if (ve) {
      const route = this.sceneParser.findRouteInMap(ve.routeId)
      const idx = route.curve.FindBoxIdx(box.uuid)
      return idx === 0
    }
    return false
  }

  onDbclick (event) {
    try {
      clearTimeout(this.timeoutEditRoute)
      this.timeoutEditRoute = 0
      this.promiseEditRoute.then(() => {
        this.doTrackFinish()
        this.renderScene()
      })
    } catch (error) {
      console.log('on dbl click failed: ', error)
    }
  }

  /**
   * 判断是否可以打开某个场景
   * @param {} payload
   */
  async canOpenScenario ({ map, scene }) {
    await this.hadmap.loadHadmapinfo()
    const mapInfo = this.hadmap.findHadmap(map)
    return !!mapInfo
  }

  // 是否是轨迹创建状态
  isEditingPath (state) {
    return state >= 1 && state <= 4
  }

  locateCameraOverMap () {
    const box = this.hadmap.getBoundingBox()
    const center = box.getCenter()
    const size = box.getSize()
    const boxAspect = size.x / size.y

    let finalHeight = Math.max(size.y, 1)
    if (boxAspect > this.aspect) {
      finalHeight = size.x / this.aspect
    }

    const radio = Math.tan(this.curCamera.fov / 360 * Math.PI)
    center.z -= size.z / 2

    const offset = new Vector3(0, 0, finalHeight / 2 / radio + size.z / 2)
    this.focusPosition(center, offset)
  }

  getSignlightPlans () {
    const { activePlan, signlights, junctions, routesData } = this.sceneParser
    return { activePlan, signlights, junctions, routesData }
  }

  setSignlightPlans (data) {
    if (!data) return
    const { activePlan, signlights } = data
    if (activePlan && +activePlan >= 0) this.sceneParser.activePlan = activePlan
    if (signlights?.length) {
      this.sceneParser.signlights = signlights.map((signlight) => {
        if (signlight.constructor === Signlight) return signlight
        const newSignlight = new Signlight()
        Object.assign(newSignlight, signlight)
        return newSignlight
      })
    }
  }
}

export default SimuScene
