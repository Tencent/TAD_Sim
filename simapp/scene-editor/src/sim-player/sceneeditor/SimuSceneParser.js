import axios from 'axios'
import { cloneDeep, find, max, split, uniq, uniqBy } from 'lodash-es'
import {
  MapElementType,
  RunMode,
  SceneState,
} from '../common/Constant'
import { getSceneDataV2 } from '../api/scenario'
import GlobalConfig from '../common/Config'
import MapFile from './MapFile'
import Planner from './Planner'
import RouteData from './RouteData'
import AccelerationData from './AccelerationData'
import MergeData from './MergeData'
import Obstacle from './Obstacle'
import Vehicle, { VehicleBehaviorType } from './Vehicle'
import PedestrianV2 from './PedestrianV2'
import Signlight from './Signlight'
import Utility from './Utility'
import { MessageBox } from './MessageBox'
import Rosbag from './Rosbag.js'
import L3StateMachine from './L3StateMachine'
import EditorGlobals from './EditorGlobals'
import TrafficAIConfig from './TrafficAIConfig'
import VelocityData from './VelocityData'
import GenerateInfo from './GenerateInfo'
import Measurement from './Measurement'
import i18n from '@/locales'
import { getPermission } from '@/utils/permission'

/**
 * 场景解析器
 */
class SimuSceneParser {
  constructor (simuScene) {
    this.simuScene = simuScene
    this.initStore()
  }

  /**
   * 初始化场景数据
   */
  initStore () {
    this.l3StateMachine = new L3StateMachine()
    this.accs = []
    this.merges = []
    this.routes = []
    /**
     * @type {Planner[]}
     */
    this.planners = []
    this.vehicles = []
    this.obstacles = []
    this.junctions = []
    this.signlights = []
    this.routesData = []
    this.velocities = []
    this.pedestrians = []
    this.measurements = []
    this.envInfo = null

    this.activePlan = '-1'

    this.accsMap = new Map()
    this.mergesMap = new Map()
    this.routesMap = new Map()
    this.vehiclesMap = new Map()
    this.obstaclesMap = new Map()
    this.junctionsMap = new Map()
    this.signlightsMap = new Map()
    this.velocitiesMap = new Map()
    this.pedestriansMap = new Map()
    this.measurementsMap = new Map()

    this.plannersModelMap = new Map()
    this.vehiclesModelMap = new Map()
    this.obstaclesModelMap = new Map()
    this.vehiclesEndModelMap = new Map()
    this.plannersEndModelMap = new Map()
    this.pedestriansModelMap = new Map()
    this.pedestriansEndModelMap = new Map()
    this.measurementsEndModelMap = new Map()

    this.plannerEndModels = []
    this.vehicleEndModels = []
    this.pedestrianEndModels = []
    this.measurementEndModels = []

    this.utility = new Utility()

    this.trafficAIConfig = new TrafficAIConfig()

    this.aggressApply = 1 // fix 89889349
    this.gradingInfo = ''
  }

  /**
   * 清空场景数据
   */
  clearScene () {
    this.accs.splice(0)
    this.merges.splice(0)
    this.routes.splice(0)
    this.vehicles.splice(0)
    this.obstacles.splice(0)
    this.velocities.splice(0)
    this.signlights.splice(0)
    this.junctions.splice(0)
    this.pedestrians.splice(0)

    this.accsMap.clear()
    this.mergesMap.clear()
    this.routesMap.clear()
    this.vehiclesMap.clear()
    this.obstaclesMap.clear()
    this.signlightsMap.clear()
    this.junctionsMap.clear()
    this.velocitiesMap.clear()
    this.pedestriansMap.clear()

    this.vehiclesModelMap.clear()
    this.obstaclesModelMap.clear()
    this.pedestriansModelMap.clear()
    this.plannersEndModelMap.clear()
    this.vehiclesEndModelMap.clear()
    this.pedestriansEndModelMap.clear()

    this.vehicleEndModels.splice(0)
    this.pedestrianEndModels.splice(0)

    this.activePlan = '-1'
  }

  /**
   * 加载场景
   * @param sceneID
   * @return {Promise<*>}
   */
  async getScene (sceneID) {
    const { data: res } = await getSceneDataV2(sceneID, GlobalConfig.runMode)

    if (GlobalConfig.isCloud) {
      if (+res.code !== 0 || !res.data) {
        EditorGlobals.signals.sceneLoaded.dispatch({ mapIsLoaded: false })
        alert(i18n.t('tips.obtainSceneFail'))
        throw new Error(i18n.t('tips.obtainSceneFail'))
      }

      const data = res.data

      this.sceneId = data.id
      this.sceneName = data.name
      this.sceneDesc = data.sceneDesc
      this.sceneType = Object.assign({}, data.sceneType)
      this.mapId = data.map.id
      this.mapName = data.map.name
      this.kpi = data.kpi
      this.updateTime = data.updateTime
      this.createTime = data.createTime

      if (!data.content?.length) {
        EditorGlobals.signals.sceneLoaded.dispatch({ mapIsLoaded: false })
        return
      } else {
        this.sceneData = data
      }
    } else {
      if (res?.err === -1) { // add err warning
        EditorGlobals.signals.promptMessage.dispatch({ type: 'error', msg: res.message })
      }
      this.sceneData = res
    }

    this.sceneID = sceneID
    console.log(this.sceneData)
    return this.sceneData
  }

  /**
   * 解析场景数据
   * @param store
   * @param sceneData
   */
  parseScene (store, sceneData) {
    if (!store) {
      console.error('store is null')
      return
    }
    if (!sceneData || sceneData.length < 3) {
      console.error('sceneData is null')
      return
    }

    // 场景元数据
    if (sceneData.infos) {
      store.infos = {
        sceneId: sceneData.infos.id,
        sceneName: sceneData.infos.name,
        mapName: sceneData.infos.map,
        sceneLabel: sceneData.infos.label,
      }
    }

    // traffic info
    if (sceneData.traffic) {
      store.trafficType = sceneData.infos?.traffictype
      if (sceneData.traffic.type === 'xosc') {
        const msg = i18n.t('tips.xoscNotSupportEditing')
        console.warn(msg)
      }
      // route
      if (sceneData.traffic.routes?.length) {
        sceneData.traffic.routes.forEach((routeData, i) => {
          const route = new RouteData()
          route.setFromBackEndData(routeData)
          store.routes[i] = route
          store.routesMap.set(`${route.id}`, route)
        })
      }
      // measurements
      if (sceneData.traffic.measurements?.length) {
        sceneData.traffic.measurements.forEach((data, i) => {
          const measurement = new Measurement()
          measurement.copy(data)
          store.measurements[i] = measurement
          store.measurementsMap.set(`${measurement.id}`, measurement)
        })
      }
      // data
      if (sceneData.traffic.data) {
        // randomseed
        if (sceneData.traffic.data.randomseed !== undefined) {
          store.randomseed = sceneData.traffic.data.randomseed
        } else {
          store.randomseed = 50 // fix 89889349
        }

        // aggress
        if (sceneData.traffic.data.aggress !== undefined &&
          sceneData.traffic.data.aggress >= 0 &&
          sceneData.traffic.data.aggress <= 1) {
          store.trafficAIConfig.aggress = sceneData.traffic.data.aggress
          store.aggressApply = 1
        } else {
          store.trafficAIConfig.aggress = 0.5
          store.aggressApply = 1 // fix 89889349
        }
      }
      // acc
      if (sceneData.traffic.accs?.length) {
        sceneData.traffic.accs.forEach((accData, i) => {
          const acc = new AccelerationData()
          acc.setFromBackEndData(accData)
          store.accs[i] = acc
          store.accsMap.set(`${acc.id}`, acc)
        })
      }

      // merge
      if (sceneData.traffic.merges?.length) {
        sceneData.traffic.merges.forEach((mergeData, i) => {
          const merge = new MergeData()
          merge.setFromBackEndData(mergeData)
          store.merges[i] = merge
          store.mergesMap.set(`${merge.id}`, merge)
        })
      }

      // velocities
      if (sceneData.traffic.velocities?.length) {
        sceneData.traffic.velocities.forEach((velocityData, i) => {
          const velocity = new VelocityData()
          velocity.setFromBackEndData(velocityData)
          store.velocities[i] = velocity
          store.velocitiesMap.set(`${store.velocities[i].id}`, store.velocities[i])
        })
      }

      // sceneevents
      store.sceneevents = sceneData.traffic.sceneevents || []

      store.sceneevents.forEach((conditions) => {
        // if condition's equationop is empty, set it as 'eq'
        if (conditions?.length) {
          conditions.forEach((condition) => {
            if (!condition.equationop) {
              condition.equationop = 'eq'
            }
          })
        }
      })

      // vehicles
      if (sceneData.traffic.vehicles?.length) {
        sceneData.traffic.vehicles.forEach((vehicleData, i) => {
          const vehicle = new Vehicle()
          vehicle.setFromBackEndData(vehicleData, store)
          store.vehicles[i] = vehicle
          store.vehiclesMap.set(`${vehicle.id}`, vehicle)
        })
      }

      // obstacles
      if (sceneData.traffic.obstacles?.length) {
        sceneData.traffic.obstacles.forEach((obstacleData, i) => {
          const obstacle = new Obstacle()
          obstacle.setFromBackEndData(obstacleData)
          store.obstacles[i] = obstacle
          store.obstaclesMap.set(`${obstacle.id}`, obstacle)
        })
      }

      // pedestrians
      if (sceneData.traffic.pedestriansV2?.length) {
        sceneData.traffic.pedestriansV2.forEach((pedestrianData, i) => {
          const pedestrian = new PedestrianV2()
          pedestrian.setFromBackEndData(pedestrianData, store)
          store.pedestrians[i] = pedestrian
          store.pedestriansMap.set(`${pedestrian.id}`, pedestrian)
        })
      }

      // activePlan & signlights & junctions
      this.parseSignlights(store, sceneData.traffic)
    }

    // sensor info
    store.sensorInfo = sceneData.sensor

    // environment info
    store.envInfo = sceneData.environment

    // grading
    store.gradingInfo = sceneData.grading || ''

    // sim info
    if (sceneData.sim) {
      // parse map file
      if (sceneData.sim.map) {
        store.mapfile = new MapFile()
        const { lon, lat, alt, file, level } = sceneData.sim.map
        store.mapfile.setData({
          lon,
          lat,
          alt,
          mapName: file,
          unrealLevelIndex: level,
        })
      }

      // l3statemachine
      if (sceneData.sim.l3statemachine) {
        store.l3StateMachine.setL3StatesData(sceneData.sim.l3statemachine)
      }

      if (sceneData.sim.planner?.egolist?.length) {
        store.planners = sceneData.sim.planner.egolist.map((p) => {
          const planner = new Planner(this.simuScene)
          planner.setFromBackEndData(p)
          return planner
        })
      } else if (sceneData.sim.planner) {
        store.planners = [new Planner(this.simuScene)]
        store.planners[0].setFromBackEndData(sceneData.sim.planner)
      }
      // 限制可添加的主车数量
      if (store.planners?.length) {
        const enableEgoNumber = getPermission('config.app.egoNumber')
        const currentPlannerLength = store.planners.length || 0
        if (currentPlannerLength >= enableEgoNumber) {
          const msg = `主车数量不可超过 ${enableEgoNumber}，超出数量的主车将清除`
          MessageBox.promptEditorUIMessage('error', msg)
          store.planners = store.planners.slice(0, enableEgoNumber)
        }
      }

      // rosbag
      if (sceneData.sim.rosbag) {
        store.rosbag = new Rosbag()
        store.rosbag.path = sceneData.sim.rosbag.path
      }
      // geofence
      if (sceneData.sim.geofence != undefined) {
        store.geofence = sceneData.sim.geofence
      }

      store.generateInfo = new GenerateInfo()
      // generate info
      if (sceneData.sim.generateinfo) {
        store.generateInfo.originFile = sceneData.sim.generateinfo.origin
        store.generateInfo.parameters = sceneData.sim.generateinfo.param
      }
    }
  }

  // 信号灯相关的（即信控配置）单独处理
  parseSignlights (store, signlightsData) {
    const { activePlan, junctions, signlights = [], routes: routesData } = signlightsData

    // 数据来自 场景详情detail数据 或 gettrafficlights接口ajax数据
    // 若为新建场景，则只有一次 ajax 数据
    // 若为编辑行家，则先有一次 detail 数据，再有一次 ajax 数据

    // 新数据是合法配置
    const hasSignPlan = activePlan !== '-1' && activePlan !== undefined // 本次数据有默认配置
    const planNeedPass = store.activePlan !== '-1' // 初始为 -1，store 已设置过即表示此次为第二次数据了，跳过 activePlan 的设置
    const canCover = hasSignPlan && !planNeedPass // 本次数据需覆盖上次数据，也即第一次数据

    // routeID 与 relative_lane 对应关系
    const relativeLaneMap = {}
    if (signlights?.length) {
      signlights.forEach((signlight) => {
        if (signlight.relative_lane) {
          relativeLaneMap[signlight.routeID] = signlight.relative_lane
        }
      })
    }

    // activePlan
    if (canCover) {
      store.activePlan = activePlan
    }

    // signlights 首次加载先清空
    if (signlights?.length && canCover) {
      store.signlights = []
    }
    // signlights 注入到 store
    if (signlights?.length) {
      const len = signlights.length
      for (let i = 0; i < len; ++i) {
        const sl = new Signlight()
        sl.id = signlights[i].id
        sl.routeId = signlights[i].routeID
        sl.startShift = +signlights[i].start_s
        sl.startTime = signlights[i].start_t
        sl.startOffset = signlights[i].l_offset
        sl.startAlt = signlights[i].start_alt
        sl.direction = signlights[i].direction
        sl.timeGreen = signlights[i].time_green
        sl.timeYellow = signlights[i].time_yellow
        sl.timeRed = signlights[i].time_red
        sl.plan = signlights[i].plan || '0' // 信控配置 id
        sl.junction = signlights[i].junction // 路口 id
        sl.phaseNumber = signlights[i].phaseNumber // 相位 id
        sl.signalHead = signlights[i].signalHead ? signlights[i].signalHead.split(',') : [] // 物理灯 ids
        if (signlights[i].phase) { // 红绿灯方向
          sl.phase = signlights[i].phase
        }
        if (signlights[i].lane) { // 有效车道
          sl.lane = signlights[i].lane
        }
        sl.enabled = signlights[i].enabled

        if (sl.startShift < 0) {
          const msg = i18n.t('tips.longDistanceLt', { id: sl.id })
          MessageBox.promptEditorUIMessage('error', msg)
          sl.startShift = 0
        }

        if (canCover) {
          const key = `${sl.plan}_${sl.junction}_${sl.phaseNumber}_${sl.routeId}`
          if (store.signlightsMap.has(key)) continue
          store.signlights.push(sl)
          store.signlightsMap.set(key, sl)
        } else {
          // 第二次数据时，找到 detail 数据中对应的 sl，将 ajax 数据中的 relative_lane 进行关联
          // 由于 ubuntu 中 id 与 junction 返回逻辑不同，想判断 detail 数据语义灯与 ajax 数据语义灯相等，只能靠 route 位置数据了
          const slRoute = routesData.find(e => e.id === sl.routeId)
          // 本次数据与场景数据相匹配的
          const matchRoute = slRoute ? store.routesData.find(e => e.start.includes(slRoute.start)) : null
          if (matchRoute) {
            relativeLaneMap[matchRoute.id] = signlights[i].relative_lane // 修正 relativeLaneMap 数据
          }

          // 有特殊地图，第二次数据有场景中未包含的语义灯
          const matchSignlight = matchRoute ? store.signlights.find(e => e.routeId === matchRoute.id) : null
          if (!matchSignlight) {
            const key = `${sl.plan}_${sl.junction}_${sl.phaseNumber}_${sl.routeId}`
            if (store.signlightsMap.has(key)) continue
            store.signlights.push(sl)
            store.signlightsMap.set(key, sl)
          }
        }
      }
    }

    // 再遍历一次，将 phase 为合并的情况，拆分为单个方向的箭头，方便交互
    if (store.signlights?.length && Object.keys(relativeLaneMap).length) {
      let baseId = max(store.signlights.map(sl => +sl.id)) + 1
      store.signlights = store.signlights.reduce((signlights, sl) => {
        const divideSignlights = sl.divideByPhases(relativeLaneMap, baseId)
        baseId += divideSignlights.length - 1
        signlights.push(...divideSignlights)
        return signlights
      }, [])
    }

    // junctions
    if (junctions?.length) {
      for (let i = 0; i < junctions.length; ++i) {
        store.junctions[i] = {}
        store.junctions[i].id = junctions[i].id
        store.junctions[i].tafficlights = junctions[i].tafficlights || []
      }
    }

    // routes
    if (routesData?.length) {
      store.routesData = uniqBy([...store.routesData, ...routesData], 'id')
      // 兼容旧数据，有部分场景保存为了单个方向的箭头，其中的 routeId 会找不到对应 routesData，因此用重复数据进行兼容
      let lastRoute
      store.signlights.forEach((sl) => {
        const match = routesData.find(route => route.id === sl.routeId)
        if (match) {
          lastRoute = match
        } else if (lastRoute) {
          const copyRoute = cloneDeep(lastRoute)
          copyRoute.id = sl.id
          store.routesData.push(copyRoute)
        }
      })
    }
  }

  /**
   * 校验route，预处理route数据
   * @param store
   * @return {Promise<void>}
   */
  async checkRoutes (store) {
    const { routes } = store
    const noneRoadRoute = routes.filter(route => route.type !== 'roadID')
    const { length: nrLength } = noneRoadRoute
    if (nrLength) {
      const ps = noneRoadRoute.map(
        // 每个route一个start点
        ({ startLon, startLat }) => ({ startLon, startLat }),
      ).concat(
        // 每个route一个end点
        noneRoadRoute.map(
          ({ endLon: startLon, endLat: startLat }) => ({ startLon, startLat }),
        ),
      ).concat(
        // 每个route若干endPosArr
        noneRoadRoute.reduce((points, route) => {
          const { endPosArr } = route
          points.push(
            ...endPosArr.map(
              ({ lon: startLon, lat: startLat }) => ({ startLon, startLat }),
            ),
          )
          return points
        }, []),
      )
      const points = await this.simuScene.hadmap.batchGetNearbyLaneInfo(ps)

      // 不在道路上的route
      noneRoadRoute.reduce((n, route, i) => {
        if (i < nrLength) {
          // 处理start点回调
          const startPoint = points[i]
          if (startPoint?.err === 0) {
            if (startPoint.type === 'lane') {
              route.roadId = startPoint.rid
              route.sectionId = startPoint.sid
              route.laneId = startPoint.lid
              route.elemType = MapElementType.LANE
              route.startAlt = startPoint.alt
            } else if (startPoint.type === 'lanelink') {
              route.lanelinkId = startPoint.llid
              route.elemType = MapElementType.LANELINK
              route.startAlt = startPoint.alt
            }
          } else {
            const msg = `route: ${route.id} (lon: ${route.startLon} lat: ${route.startLat}) no lane`
            console.error('queryinfobypt error:', msg)
            MessageBox.promptEditorUIMessage('error', msg)
          }
          return i + 1
        } else if (i < 2 * nrLength) {
          // 处理end点回调
          const endPoint = points[nrLength + i]
          if (endPoint?.err === 0) {
            route.endAlt = endPoint.alt
          }
          // 给后面操作endPosArr保存索引n
          return i + 1
        } else {
          route.endPosArr.forEach((pos, j) => {
            const endPosPoint = points[n + j]
            if (endPosPoint?.err === 0) {
              pos.alt = endPosPoint.alt
            }
          })
          return n + route.endPosArr.length
        }
      }, 0)
    }

    // 批量设置需要查询的点
    const points = []
    this.sceneData.sim.planner.egolist.forEach((p) => {
      const { inputPath } = p
      points.push(...inputPath.map(
        ({ lon: startLon, lat: startLat }) => ({ startLon, startLat }),
      ))
    })

    try {
      // 批量查询点位信息，并设置回去
      const result = await this.simuScene.hadmap.batchGetNearbyLaneInfo(points)
      this.sceneData.sim.planner.egolist.forEach((p) => {
        p.inputPath.forEach((ip, i) => {
          const point = result.shift()
          if (point?.err === 0) {
            ip.alt = point.alt
          }
        })
      })
    } catch (e) {
      console.log(e)
    }
  }

  /**
   * 组装场景
   * @return {Promise<void>}
   */
  async assembScene () {
    const sceneData = this.sceneData

    let len = 0
    if (sceneData.traffic) {
      const elements = []
      // routes
      if (sceneData.traffic.routes?.length) {
        len = sceneData.traffic.routes.length
        for (let i = 0; i < len; ++i) {
          this.simuScene.addRouteVisualOnLoad(this.routes[i])
        }
      }

      // measurements
      if (sceneData.traffic.measurements) {
        len = sceneData.traffic.measurements.length
        for (let i = 0; i < len; ++i) {
          this.simuScene.addMeasurementPathEnd(this.measurements[i])
        }
        this.simuScene.calculateMeasurementResult()
      }

      // vehicles
      if (sceneData.traffic.vehicles?.length) {
        len = sceneData.traffic.vehicles.length
        for (let i = 0; i < len; ++i) {
          const route = this.findRouteInMap(this.vehicles[i].routeId)
          if (route.elemType === MapElementType.LANELINK) {
            this.vehicles[i].laneId = route.lanelinkId
          } else if (route.elemType === MapElementType.LANE) {
            if (this.vehicles[i].laneId === '') {
              this.vehicles[i].laneId = route.laneId
              const msg = `vehicle ${this.vehicles[i].id} route: ${route.id} lane id is null, use ${route.laneId} instead`
              console.error(msg)
              MessageBox.promptEditorUIMessage('error', msg)
            }
          } else {
            const msg = `vehicle ${this.vehicles[i].id} route: ${route.id} elemType is error`
            console.error(msg)
            MessageBox.promptEditorUIMessage('error', msg)
            continue
          }
          elements.push(this.vehicles[i])
        }
      }

      // obstacles
      if (sceneData.traffic.obstacles?.length) {
        len = sceneData.traffic.obstacles.length
        for (let i = 0; i < len; ++i) {
          elements.push(this.obstacles[i])
        }
      }

      // pedestrians
      if (sceneData.traffic.pedestriansV2?.length) {
        len = sceneData.traffic.pedestriansV2.length
        for (let i = 0; i < len; ++i) {
          elements.push(this.pedestrians[i])
        }
      }
      // 批量查询点信息，然后设置回去
      await this.simuScene.batchGetFinalPosByRoute4(elements)
      await Promise.all(elements.map(async (ele) => {
        if (ele instanceof Vehicle) {
          await this.simuScene.addVehicleVisual(ele)
          const route = this.findRouteInMap(ele.routeId)
          route.updateRoutePointsFromCurve(this.simuScene.ct)
          ele.setRoute(route)
        } else if (ele instanceof Obstacle) {
          this.simuScene.addObstacleVisual(ele)
        } else if (ele instanceof PedestrianV2) {
          await this.simuScene.addPedestrianVisual(ele)
          const route = this.findRouteInMap(ele.routeId)
          route.updateRoutePointsFromCurve(this.simuScene.ct)
          ele.setRoute(route)
        }
      }))
    }

    if (sceneData.sim) {
      // parse planner
      if (this.planners?.length) {
        const points = this.planners.map((p) => {
          const route = this.findRouteInMap(p.routeId)
          return {
            startLon: route.startLon,
            startLat: route.startLat,
          }
        })
        // 批量查询主车route点
        const results = await this.simuScene.hadmap.batchGetNearbyLaneInfo(points)
        const ps = this.planners.map(async (p, i) => {
          const route = this.findRouteInMap(p.routeId)

          if (!route) {
            const msg = i18n.t('tips.loadFileErr')
            MessageBox.promptEditorUIMessage('error', msg)
            return Promise.reject(msg)
          }
          const laneInfo = results[i]
          if (laneInfo.err !== 0) {
            const msg = i18n.t('tips.noLaneNearEgo')
            MessageBox.promptEditorUIMessage('error', msg)
          } else {
            p.laneId = laneInfo.lid
            p.startShift = laneInfo.dist
            p.startOffset = laneInfo.offset
          }

          // planner input path
          const { controlPath, inputPath } = sceneData.sim.planner.egolist[i]
          const controlPathLength = controlPath?.length || 0
          const inputPathLength = inputPath?.length || 0

          // planner control path
          if ((p.trajectory_enabled) && (controlPathLength >= inputPathLength)) {
            // 轨迹跟踪
            route.endPosArr = controlPath.map(point => ({
              lon: point.lon,
              lat: point.lat,
              alt: point.alt,
              velocity: point.velocity,
              gear: point.gear,
              frontwheel: point.frontwheel,
              accs: point.accs,
              heading: point.heading,
            }))
          } else {
            // 设置轨迹点
            const [lon, lat, alt] = await this.simuScene.getPlannerLonLatAlt(p)
            route.endPosArr = inputPath.map((point, i) => {
              const pp = {}
              if (i === 0) { // fix the first height
                pp.lon = lon
                pp.lat = lat
                pp.alt = alt
                pp.velocity = +p.startVelocity
              } else {
                pp.lon = point.lon
                pp.lat = point.lat
                pp.alt = point.alt
                pp.velocity = 6
              }
              pp.gear = 'drive'
              return pp
            })
          }
          // 设置planner的终点
          p.endLon = route.endLon
          p.endLat = route.endLat

          p.setRoute(route)
          await this.simuScene.addPlannerVisual(p)
        })
        await Promise.all(ps)
      }
    }

    EditorGlobals.signals.sceneLoaded.dispatch({ mapIsLoaded: true })
  }

  /**
   * 检查交通车的参考车数据
   * @return {{result: boolean, message: string}}
   */
  checkVehiclesFollows () {
    const ret = { result: true, message: 'succeed' }

    const invalidVehicleIDs = []
    this.vehicles.forEach((vehicle) => {
      if (vehicle.follow && vehicle.follow !== 'ego') {
        const followCarID = `${vehicle.follow}`
        const followCar = this.findVehicleInMap(followCarID)
        if (followCar) {
          // 只有自定义车辆才能被作为参考车
          if (followCar.behavior === VehicleBehaviorType.TrafficVehicle) {
            invalidVehicleIDs.push(vehicle.id)
          }
        }
      }
    })

    if (invalidVehicleIDs.length > 0) {
      ret.result = false
      ret.message = `vehicle ${invalidVehicleIDs.toString()} can not follow ai car`
    }

    return ret
  }

  /**
   * 检查场景是否可以保存
   * @return {{result: boolean, message: string}}
   */
  canSceneSave () {
    const ret = { result: true, message: 'succeed' }

    if (this.simuScene.State() !== SceneState.StateEdit) {
      ret.result = false
      ret.message = i18n.t('tips.stateInSetting')
    } else if (!this.mapfile) {
      ret.result = false
      ret.message = i18n.t('tips.noMap')
    } else if (!this.planners?.length) {
      ret.result = false
      ret.message = i18n.t('tips.noEgo')
    } else {
      // let route = this.findRouteInMap(0)
      // if (route?.endLon >= -180 && route.endLat >= -90 && route.endLon <= 180 && route.endLat <= 90) {
      //   ret.result = true
      // } else {
      //   ret.result = true // ignore planner route end
      //   ret.message = i18n.t('tips.noEndPointForEgo')
      // }
    }

    // 交通流车是否跟随了其他ai车
    const retCheckFollow = this.checkVehiclesFollows()
    if (retCheckFollow.result === false) {
      return retCheckFollow
    }

    return ret
  }

  /**
   * 批量生成场景
   * @param id
   * @param prefix
   * @param payload
   * @return {Promise<any>}
   */
  async paramScene (id, prefix, payload) {
    if (id === undefined || +id < 0) {
      const msg = i18n.t('tips.useBeforeSaving')
      MessageBox.promptEditorUIMessage('error', msg)
      return
    }

    if (!prefix) {
      prefix = this.sceneID
    }
    payload.prefix = prefix

    const { data } = await axios({
      method: 'post',
      url: SimuSceneParser.paramSceneURL,
      withCredentials: true,
      timeout: 0,
      data: {
        sceneName: `${id}`,
        param: JSON.stringify(payload),
      },
    })
    if (+data.err !== 0) {
      return Promise.reject(data)
    } else {
      return data
    }
  }

  /**
   * 保存场景
   * @param id
   * @param sceneName
   * @param ext
   * @param store
   * @return {Promise<any|boolean>}
   */
  async saveScene (id, sceneName, ext, store) {
    console.log('save scene ', sceneName)

    store.sceneID = id
    store.sceneName = sceneName
    if (!store.infos) {
      store.infos = {
        sceneDesc: store.sceneDesc || '',
        sceneLabel: '',
        type: 'sim',
      }
    }

    const {
      mapName,
      unrealLevelIndex,
      lon,
      lat,
      alt,
    } = store.mapfile

    // 场景元数据
    store.infos.sceneId = id
    store.infos.sceneName = sceneName
    store.infos.mapName = mapName
    if (ext && ext !== 'sim') {
      store.infos.type = ext
    } else {
      store.infos.type = 'sim'
    }

    const sceneObj = {}

    // infos
    sceneObj.infos = {
      type: store.infos.type,
    }

    // sim
    sceneObj.sim = {}
    // map file
    sceneObj.sim.map = {
      lon,
      lat,
      alt,
      level: unrealLevelIndex,
      file: mapName,
    }

    // planner
    if (store.planners?.length) {
      sceneObj.sim.planner = {
        egolist: store.planners.map(p => p.toBackEndData()),
      }
      sceneObj.sim.controlTrack = sceneObj.sim.planner.egolist.map((p) => {
        const points = p.controlTrack.map(point => ({
          ...point,
          velocity: point.velocity === null ? null : +point.velocity,
        }))
        delete p.controlTrack
        return points
      })
    }
    // rosbag
    if (store.rosbag) {
      sceneObj.sim.rosbag = {
        path: store.rosbag.path,
      }
    }

    // state machine
    if (store.l3StateMachine.getL3StatesData().length) {
      sceneObj.sim.l3statemachine = store.l3StateMachine.getL3StatesData()
    }

    // geo fence
    if (store.geofence) {
      sceneObj.sim.geofence = store.geofence
    }

    // generate info
    if (store.generateInfo) {
      sceneObj.sim.generateinfo = {
        param: store.generateInfo.parameters,
        origin: store.generateInfo.originFile,
      }
    }

    let len = 0
    sceneObj.traffic = {}
    sceneObj.traffic.accs = []
    // acc
    if (store.accs?.length) {
      sceneObj.traffic.accs = store.accs.map(acc => ({
        id: acc.id,
        profile: acc.nodes.map(node => [
          node.condition.type,
          node.condition.value,
          node.condition.mode,
          node.acc,
          node.count,
          node.termination.type,
          node.termination.value,
        ]),
      }))
    }

    sceneObj.traffic.merges = []
    // merge
    if (store.merges) {
      sceneObj.traffic.merges = store.merges.map((mergeData, i) => {
        const merge = {}
        merge.id = mergeData.id
        merge.profile = mergeData.merges.map((m) => {
          if (m.condition) {
            const {
              condition: {
                type,
                value,
                mode,
              },
              merge,
              duration,
              offset,
              count,
            } = m
            return [
              type,
              value,
              mode,
              merge,
              duration,
              offset,
              count,
            ]
          } else {
            return []
          }
        })
        return merge
      })
    }

    sceneObj.traffic.velocities = []
    // velocity
    if (store.velocities) {
      sceneObj.traffic.velocities = store.velocities.map((velocity) => {
        const v = {}
        v.id = velocity.id
        v.profile = velocity.nodes.filter(node => node.velocity !== '')
          .map((node) => {
            const {
              condition: {
                type,
                value,
                mode,
              },
              velocity,
              count,
            } = node
            return [
              type,
              value,
              mode,
              velocity,
              count,
            ]
          })
        return v
      })
    }

    sceneObj.traffic.routes = []
    // routes
    if (store.routes) {
      len = store.routes.length
      // 收集需要查询的点
      const points = []
      for (let i = 0; i < len; ++i) {
        const route = store.routes[i]
        const rData = {}
        rData.id = route.id
        rData.type = route.type
        if (route.type === 'start_end') {
          rData.start = `${route.startLon},${route.startLat},${route.startAlt || 0}`
          const ele = find(store.vehicles, v => v.routeId === route.id) ||
            find(store.obstacles, v => v.routeId === route.id) ||
            find(store.pedestrians, v => v.routeId === route.id)
          if (ele) {
            points.push({
              startLon: route.startLon,
              startLat: route.startLat,
              type: route.elemType,
              id: ele.laneId,
              shift: ele.startShift,
              offset: ele.startOffset,
            })
          }

          if (route.endLon !== undefined && route.endLon >= -180 && route.endLon <= 180 &&
            route.endLat !== undefined && route.endLat >= -90 && route.endLat <= 90) {
            rData.end = `${route.endLon},${route.endLat},${route.endAlt || 0}`
          } else {
            rData.end = ''
          }

          if (route.midLon !== undefined && route.midLon > 0) {
            if (route.midLon !== undefined && route.midLon >= -180 && route.midLon <= 180 &&
              route.midLat !== undefined && route.midLat >= -90 && route.midLat <= 90) {
              rData.mid = `${route.midLon},${route.midLat}`
            }
          }

          if (route.mids != undefined) {
            rData.mids = route.mids
          }

          if (route.host && route.host.behavior === 'TrajectoryFollow') {
            rData.controlPath = route.generateControlPath()
          }
        } else if (route.type === 'roadID') {
          rData.roadID = route.roadId
          rData.sectionID = route.sectionId
        } else {
          console.error('wrong route type!')
        }
        sceneObj.traffic.routes[i] = rData
      }
      // 批量查询点，然后设置回去
      const lonlats = await this.simuScene.hadmap.batchGetLonLatByPoint(points)
      store.routes.forEach((r, index) => {
        if (r.type === 'start_end') {
          const rData = sceneObj.traffic.routes[index]
          const ele = find(store.vehicles, v => v.routeId === r.id) ||
            find(store.obstacles, v => v.routeId === r.id) ||
            find(store.pedestrians, v => v.routeId === r.id)
          if (ele) {
            const tmp = lonlats.shift()
            if (tmp.err === 0) {
              rData.info = `${tmp.lon},${tmp.lat},${tmp.alt}`
            }
          }
        }
      })
    }

    sceneObj.traffic.measurements = []
    // measurements
    if (store.measurements?.length) {
      len = store.measurements.length
      for (let i = 0; i < len; ++i) {
        sceneObj.traffic.measurements[i] = {}
        sceneObj.traffic.measurements[i].id = store.measurements[i].id
        sceneObj.traffic.measurements[i].posArr = []
        store.measurements[i].posArr.forEach((value) => {
          const p = Object.assign({}, value)
          sceneObj.traffic.measurements[i].posArr.push(p)
        })
      }
    }

    sceneObj.traffic.vehicles = []
    sceneObj.traffic.sceneevents = []
    // vehicle && sceneevents
    if (store.vehicles?.length) {
      len = store.vehicles.length
      for (let i = 0; i < len; ++i) {
        const vehicle = store.vehicles[i]
        const vehicleData = {
          id: vehicle.id,
          vehicleType: vehicle.type,
          routeID: vehicle.routeId,
          laneID: vehicle.laneId,
          accID: vehicle.accId,
          mergeID: vehicle.mergeId,
          behavior: vehicle.behavior,
          start_s: vehicle.startShift,
          start_t: vehicle.startTime,
          start_v: vehicle.startVelocity,
          l_offset: vehicle.startOffset,
          start_alt: vehicle.startAlt || 0, // add height info
          max_v: vehicle.maxVelocity,
          length: vehicle.length,
          width: vehicle.width,
          height: vehicle.height,
          aggress: vehicle.aggress,
          sensorGroup: vehicle.sensorGroup,
          obuStatus: vehicle.obuStatus,
          angle: `${vehicle.angle || 0}`,
          eventId: vehicle.eventId,
          start_angle: `${vehicle.start_angle}`,
        }
        if (vehicleData.behavior === 'TrafficVehicle') {
          // ai车不能设置跟随属性
          vehicleData.follow = ''
        } else if (vehicleData.behavior === 'UserDefine') {
          vehicleData.follow = vehicle.follow
        } else if (vehicle.behavior === 'TrajectoryFollow') { // add trajectory mode
        } else {
          const msg = 'save vehicle behavior error!'
          MessageBox.promptEditorUIMessage('error', msg)
          vehicleData.behavior = 'TrafficVehicle'
        }

        if (vehicleData.behavior === 'TrafficVehicle' && vehicle.aiModel === 'TrafficVehicleArterial') {
          vehicleData.behavior = 'TrafficVehicleArterial'
        }

        // sceneevents
        console.log('store befoer sceneevents', store)
        if (vehicle.customBehavior && vehicle.triggers) {
          sceneObj.traffic.sceneevents = sceneObj.traffic.sceneevents.concat(vehicle.triggers)
        }
        sceneObj.traffic.vehicles[i] = vehicleData
      }
    }

    // 转换数据类型
    sceneObj.traffic.vehicles.forEach((item) => {
      for (const key in item) {
        item[key] = `${item[key]}`
      }
    })

    sceneObj.traffic.obstacles = []
    // obstacles
    if (store.obstacles?.length) {
      len = store.obstacles.length
      for (let i = 0; i < len; ++i) {
        const obstacle = store.obstacles[i]
        sceneObj.traffic.obstacles[i] = {
          id: obstacle.id,
          type: obstacle.type,
          routeID: obstacle.routeId,
          laneID: obstacle.laneId,
          start_s: obstacle.startShift,
          l_offset: obstacle.startOffset,
          length: obstacle.length,
          width: obstacle.width,
          height: obstacle.height,
          direction: obstacle.direction,
          start_angle: `${obstacle.start_angle}`,
          start_alt: obstacle.startAlt || 0,
        }
      }
    }

    sceneObj.traffic.pedestriansV2 = []
    // pedestrian
    if (store.pedestrians?.length) {
      len = store.pedestrians.length
      for (let i = 0; i < len; ++i) {
        const pedestrian = store.pedestrians[i]
        const pedestrianData = {
          id: pedestrian.id,
          routeID: pedestrian.routeId,
          laneID: pedestrian.laneId,
          start_s: pedestrian.startShift,
          start_t: pedestrian.startTime,
          start_v: pedestrian.startVelocity,
          max_v: pedestrian.maxVelocity,
          behavior: pedestrian.behavior,
          l_offset: pedestrian.startOffset,
          boundingBox: pedestrian.boundingBox,
          start_alt: pedestrian.startAlt || 0,
          eventId: pedestrian.eventId || '',
          subType: pedestrian.subType,
          start_angle: `${pedestrian.start_angle}`,
          angle: `${pedestrian.angle || 0}`,
        }

        if (pedestrian.conditions) {
          pedestrianData.conditions = pedestrian.conditions
        }

        if (pedestrian.directions) {
          pedestrianData.directions = pedestrian.directions
        }

        if (pedestrian.velocities) {
          pedestrianData.velocities = pedestrian.velocities
        }

        if (pedestrian.counts) {
          pedestrianData.counts = pedestrian.counts
        }

        if (!(pedestrianData.behavior in VehicleBehaviorType)) {
          const msg = 'save pedestrian behavior error!'
          MessageBox.promptEditorUIMessage('error', msg)
          pedestrianData.behavior = 'TrafficVehicle'
        }
        if (pedestrian.customBehavior && pedestrian.triggers?.length) {
          sceneObj.traffic.sceneevents = sceneObj.traffic.sceneevents.concat(pedestrian.triggers)
        }
        sceneObj.traffic.pedestriansV2[i] = pedestrianData
      }
    }

    sceneObj.traffic.activePlan = store.activePlan

    sceneObj.traffic.signlights = []
    // signlights
    if (store.signlights?.length) {
      len = store.signlights.length
      for (let i = 0; i < len; ++i) {
        const signlight = store.signlights[i]
        sceneObj.traffic.signlights[i] = {
          id: signlight.id,
          routeID: signlight.routeId,
          start_s: signlight.startShift,
          start_t: `${signlight.startTime}`,
          l_offset: signlight.startOffset,
          direction: signlight.direction,
          time_green: `${signlight.timeGreen}`,
          time_yellow: `${signlight.timeYellow}`,
          time_red: `${signlight.timeRed}`,
          phase: signlight.phase,
          lane: signlight.lane,
          enabled: signlight.enabled,
          start_alt: signlight.startAlt || 0,
          plan: signlight.plan || '',
          junction: signlight.junction,
          phaseNumber: signlight.phaseNumber,
          signalHead: signlight.signalHead.join(','),
        }
      }
      // 路口下，多个单独方向的语义灯，若存在相同配置，需合并成一个语义灯
      const uniqueKeys = {}
      const getPhases = store.signlights[0].getPhases
      sceneObj.traffic.signlights.forEach((sl) => {
        const key = `${sl.plan}_${sl.junction}_${sl.phaseNumber}_${sl.routeID}` // 相位相同则配置相同
        if (uniqueKeys[key]) {
          sl.willRemove = true
          const uniqueSl = uniqueKeys[key]
          uniqueSl.lane = uniq([...split(uniqueSl.lane, ';'), ...split(sl.lane, ';')]).filter(e => !!e).join(';')
          uniqueSl.phase = uniq([...getPhases.call(uniqueSl), ...getPhases.call(sl)]).filter(e => !!e).join(';')
        } else {
          uniqueKeys[key] = sl
        }
      })
      sceneObj.traffic.signlights = sceneObj.traffic.signlights.filter(sl => !sl.willRemove)
    }

    // signlights route
    // 注意，虽然 routesData 中的 id 很大，但依旧有与交通车 route id 重复的可能
    if (store.routesData?.length) {
      const existRouteIds = sceneObj.traffic.routes.map(route => route.id)
      store.routesData.forEach((route) => {
        const index = existRouteIds.findIndex(id => id === route.id)
        if (index < 0) { // gettrafficlights 中有不存在的则添加
          sceneObj.traffic.routes.push(route)
        } else { // gettrafficlights 中有相同的则覆盖
          console.log('signlight route is same as traffic route', route.id)
        }
      })
    }

    // data
    sceneObj.traffic.data = {}
    if (store.randomseed != undefined) {
      sceneObj.traffic.data.randomseed = store.randomseed
    } else {
      sceneObj.traffic.data.randomseed = 50
    }
    if (store.aggressApply === 1 && store.trafficAIConfig.aggress != undefined) {
      sceneObj.traffic.data.aggress = store.trafficAIConfig.aggress
    }

    // sensor
    sceneObj.sensor = store.sensorInfo

    // environment
    sceneObj.environment = store.envInfo

    // grading
    sceneObj.grading = store.gradingInfo

    // 本地和云端分别不同的数据处理
    if (GlobalConfig.runMode === RunMode.Local) {
      const str = JSON.stringify(sceneObj, null, 2)
      console.log(sceneObj)

      const content = str
      const localAddr = SimuSceneParser.saveSceneURL
      const { data: sceneData } = await axios({
        method: 'post',
        url: localAddr,
        withCredentials: true,
        data: {
          id: store.infos.sceneId,
          name: store.infos.sceneName,
          info: store.sceneDesc,
          map: store.infos.mapName,
          content,
        },
      })
      console.log(sceneData)
      if (+sceneData?.err === 0) {
        store.infos.sceneId = sceneData.sceneid
        return sceneData
      } else {
        const noti = i18n.t('tips.saveSceneFail', { r: sceneData.message })
        throw new Error(noti)
      }
    } else {
      const content = JSON.stringify(sceneObj)
      console.log(content)

      const addr = Utility.composeUrl(SimuSceneParser.saveSceneURL)

      const { data: sceneData } = await axios({
        method: 'post',
        url: addr,
        withCredentials: true,
        data: {
          id: store.sceneId,
          name: store.sceneName,
          sceneDesc: store.sceneDesc,
          sceneType: store.sceneTypeId,
          mapId: store.mapId,
          kep: store.kpi,
          content,
        },
      })
      console.log(sceneData)
      if (+sceneData?.code === 0) {
        return true
      } else {
        const noti = i18n.t('tips.saveSceneFail', { r: sceneData.message })
        throw new Error(noti)
      }
    }
  }

  /**
   * 添加速度数据
   * @param vel
   */
  addVelocity (vel) {
    if (!vel.id) {
      console.error('velocity id is null')
      return
    }

    if (this.findVelocityInMap(vel.id)) {
      return
    }

    this.velocities.push(vel)
    this.velocitiesMap.set(`${vel.id}`, vel)
  }

  /**
   * 查找速度数据
   * @param id
   * @return {any}
   */
  findVelocityInMap (id) {
    return this.velocitiesMap.get(`${id}`)
  }

  /**
   * 从数组中查找速度数据
   * @param id
   * @return {number}
   */
  findVelocityInArr (id) {
    let tmpIndex = -1
    this.velocities.forEach((value, index, array) => {
      if (id == value.id) {
        tmpIndex = index
      }
    })
    return tmpIndex
  }

  /**
   * 删除速度数据
   * @param velocity
   */
  modifyVelocity (velocity) {
    const tmp = this.velocitiesMap.get(`${velocity.id}`)
    if (tmp) {
      this.velocitiesMap.set(`${velocity.id}`, velocity)
    }

    const tmpIndex = this.findVelocityInArr(velocity.id)
    if (tmpIndex >= 0) {
      this.velocities[tmpIndex] = velocity
    }
  }

  /**
   * 添加加速度数据
   * @param acc
   */
  addAcc (acc) {
    let maxNum = 0
    this.accsMap.forEach((value, key, map) => {
      if (key > maxNum) {
        maxNum = +(key)
      }
    })

    maxNum++

    acc.id = maxNum
    const len = this.accs.length
    this.accs[len] = acc
    this.accsMap.set(`${acc.id}`, acc)
  }

  /**
   * 查找加速度数据
   * @param id
   * @return {any}
   */
  findAccInMap (id) {
    return this.accsMap.get(`${id}`)
  }

  /**
   * 从数组中查找加速度数据
   * @param id
   * @return {number}
   */
  findAccInArr (id) {
    let tmpIndex = -1
    this.accs.forEach((value, index, array) => {
      if (id == value.id) {
        tmpIndex = index
      }
    })
    return tmpIndex
  }

  /**
   * 删除加速度数据
   * @param id
   */
  removeAcc (id) {
    const acc = this.accsMap.get(`${id}`)
    if (acc) {
      // 从map中刪除
      this.accsMap.delete(`${acc.id}`)

      // 从數組中刪除
      const tmpIndex = this.findAccInArr(id)
      if (tmpIndex >= 0) {
        this.accs.splice(tmpIndex, 1)
      } else {
        console.error('remove acc error!')
        console.assert(false, 'remove acc error!')
      }
    }
  }

  /**
   * 修改加速度数据
   * @param acc
   */
  modifyAcc (acc) {
    const tmp = this.accsMap.get(`${acc.id}`)
    if (tmp) {
      this.accsMap.set(`${acc.id}`, acc)
    }

    const tmpIndex = this.findAccInArr(acc.id)
    if (tmpIndex >= 0) {
      this.accs[tmpIndex] = acc
    }
  }

  /**
   * 添加合流数据
   * @param merge
   */
  addMerge (merge) {
    let maxNum = 0
    this.mergesMap.forEach((value, key, map) => {
      if (key > maxNum) {
        maxNum = +(key)
      }
    })

    maxNum++

    merge.id = maxNum
    const len = this.merges.length
    this.merges[len] = merge
    this.mergesMap.set(`${merge.id}`, merge)
  }

  /**
   * 查找合流数据
   * @param id
   * @return {any}
   */
  findMergeInMap (id) {
    return this.mergesMap.get(`${id}`)
  }

  /**
   * 从数组中查找合流数据
   * @param id
   * @return {number}
   */
  findMergeInArr (id) {
    let tmpIndex = -1
    this.merges.forEach((value, index, array) => {
      if (id == value.id) {
        tmpIndex = index
      }
    })
    return tmpIndex
  }

  /**
   * 删除合流数据
   * @param id
   */
  removeMerge (id) {
    const merge = this.mergesMap.get(`${id}`)
    if (merge) {
      // 从map中刪除
      this.mergesMap.delete(`${merge.id}`)

      // 从數組中刪除
      const tmpIndex = this.findMergeInArr(id)
      if (tmpIndex >= 0) {
        this.merges.splice(tmpIndex, 1)
      } else {
        console.error('remove merge error!')
        console.assert(false, 'remove merge error!')
      }
    }
  }

  /**
   * 修改合流数据
   * @param merge
   */
  modifyMerge (merge) {
    const tmp = this.mergesMap.get(`${merge.id}`)
    if (tmp) {
      this.mergesMap.set(`${merge.id}`, merge)
    }

    const tmpIndex = this.findMergeInArr(merge.id)
    if (tmpIndex >= 0) {
      this.merges[tmpIndex] = merge
    }
  }

  /**
   * 添加路由数据
   * @param route
   * @param id
   */
  addRoute (route, id) {
    if (id === undefined) {
      let maxNum = 0
      this.routesMap.forEach((value, key, map) => {
        if (key > maxNum) {
          maxNum = +(key)
        }
      })

      maxNum++

      route.id = maxNum
    } else {
      route.id = id
    }

    const len = this.routes.length
    this.routes[len] = route
    this.routesMap.set(`${route.id}`, route)
  }

  /**
   * 查找route数据
   * @param id
   * @return {any}
   */
  findRouteInMap (id) {
    return this.routesMap.get(`${id}`)
  }

  /**
   * 删除route数据
   * @param id
   */
  removeRoute (id) {
    const route = this.routesMap.get(`${id}`)
    if (route) {
      // 从map中刪除
      this.routesMap.delete(`${route.id}`)

      // 从數組中刪除
      this.routes = this.routes.filter(r => r.id === `${id}`)

      if (route.curve) {
        route.curve.Boxes().forEach((value) => {
          let idx = this.findPlannerEndModelInArr(value.uuid)
          if (idx > -1) {
            this.plannersEndModelMap.delete(`${value.uuid}`)
            this.plannerEndModels.splice(idx, 1)
          }

          idx = this.findVehicleEndModelInArr(value.uuid)
          if (idx > -1) {
            this.vehiclesEndModelMap.delete(`${value.uuid}`)
            this.vehicleEndModels.splice(idx, 1)
          }

          idx = this.findPedestrianEndModelInArr(value.uuid)
          if (idx > -1) {
            this.pedestriansEndModelMap.delete(`${value.uuid}`)
            this.pedestrianEndModels.splice(idx, 1)
          }
        })

        route.curve.dispose()
      }
    }
  }

  /**
   * 添加测量数据
   * @param measurement
   */
  addMeasurement (measurement) {
    measurement.id = Utility.getNewId(this.measurements)
    this.measurements.push(measurement)
    this.measurementsMap.set(`${measurement.id}`, measurement)
  }

  /**
   * 查找测量数据
   * @param id
   * @return {any}
   */
  findMeasurementInMap (id) {
    return this.measurementsMap.get(`${id}`)
  }

  /**
   * 从数组中查找测量数据
   * @param id
   * @return {number}
   */
  findMeasurementInArr (id) {
    return this.measurements.findIndex(value => value.id == id)
  }

  /**
   * 删除测量数据
   * @param id
   */
  removeMeasurement (id) {
    const measurement = this.measurementsMap.get(`${id}`)
    if (measurement) {
      // 从map中刪除
      this.measurementsMap.delete(`${measurement.id}`)

      // 从數組中刪除
      const tmpIndex = this.findMeasurementInArr(id)
      if (tmpIndex >= 0) {
        this.measurements.splice(tmpIndex, 1)
      } else {
        console.error('remove measurement error!')
        console.assert(false, 'remove measurement error!')
      }

      if (measurement.curve) {
        measurement.curve.Boxes().forEach((value) => {
          const idx = this.findVehicleEndModelInArr(value.uuid)
          if (idx > -1) {
            this.measurementsEndModelMap.delete(`${value.uuid}`)
            this.measurementEndModels.splice(idx, 1)
          }
        })

        measurement.curve.dispose()
      }
    }
  }

  /**
   * 添加测量数据的终点模型
   * @param measurement
   */
  addMeasurementEndModel (measurement) {
    if (!measurement || !measurement.curve) {
      return
    }

    measurement.curve.Boxes().forEach((value) => {
      const box = this.findMeasurementInEndModelMap(value.uuid)
      if (!box) {
        this.measurementsEndModelMap.set(value.uuid, measurement)
        this.measurementEndModels.push(value)
      }
    })
  }

  /**
   * 查找测量数据的终点模型
   * @param uuid
   * @return {any}
   */
  findMeasurementInEndModelMap (uuid) {
    return this.measurementsEndModelMap.get(`${uuid}`)
  }

  /**
   * 添加一辆交通车
   * @param vehicle
   */
  addVehicle (vehicle) {
    vehicle.id = Utility.getNewId(this.vehicles)
    this.vehicles.push(vehicle)
    this.vehiclesMap.set(`${vehicle.id}`, vehicle)
  }

  /**
   * 删除一个交通车轨迹点模型
   * @param vehicle
   */
  removeVehicleEndModel (vehicle) {
    if (!vehicle) {
      return
    }

    const route = this.findRouteInMap(vehicle.routeId)

    if (!route?.curve) {
      return
    }

    route.curve.Boxes().forEach((value) => {
      const idx = this.findVehicleEndModelInArr(value.uuid)
      if (idx > -1) {
        this.vehiclesEndModelMap.delete(`${value.uuid}`)
        this.vehicleEndModels.splice(idx, 1)
      }
    })
  }

  /**
   * 添加一个交通车轨迹点模型
   * @param vehicle
   */
  addVehicleEndModel (vehicle) {
    if (!vehicle) {
      return
    }

    const route = this.findRouteInMap(vehicle.routeId)

    if (!route || !route.curve) {
      return
    }

    route.curve.Boxes().forEach((value) => {
      const box = this.findVehicleInEndModelMap(value.uuid)
      this.vehiclesEndModelMap.set(value.uuid, vehicle)
      if (!box) {
        this.vehicleEndModels.push(value)
      }
    })

    // update route
    vehicle.setRoute(route)
  }

  /**
   * 添加一个交通车模型
   * @param vehicle
   * @param model
   */
  addVehicleModel (vehicle, model) {
    if (!model) {
      if (!vehicle.model) {
        console.log('addVehicleModel error model null')
        return
      }

      this.vehiclesModelMap.set(vehicle.model.uuid, vehicle)
    } else {
      this.vehiclesModelMap.set(model.uuid, vehicle)
    }
  }

  /**
   * 获取所有交通车模型
   * @return {Array}
   */
  get vehicleModels () {
    return this.vehicles.map(v => v.model)
  }

  /**
   * 从所有轨迹点数据中找到交通车
   * @return {Vehicle}
   */
  findVehicleInEndModelMap (id) {
    return this.vehiclesEndModelMap.get(`${id}`)
  }

  /**
   * 从所有轨迹点数据中找到交通车
   * @return {Vehicle}
   */
  findVehicleInModelMap (id) {
    return this.vehiclesModelMap.get(`${id}`)
  }

  /**
   * 查找交通车数据
   * @param id
   * @return {any}
   */
  findVehicleInMap (id) {
    return this.vehiclesMap.get(`${id}`)
  }

  /**
   * 从数组中查找交通车数据
   * @param id
   * @return {Vehicle}
   */
  findVehicleInArr (id) {
    return this.vehicles.find(value => value.id === id)
  }

  /**
   * 从数组中查找交通车轨迹点
   * @param uuid
   * @return {number}
   */
  findVehicleEndModelInArr (uuid) {
    return this.vehicleEndModels.findIndex(value => value.uuid === uuid)
  }

  /**
   * 查找交通车数据
   * @param routeId
   * @return {Vehicle}
   */
  findVehiclesByRouteID (routeId) {
    return this.vehicles.filter(value => value.routeId === routeId)
  }

  /**
   * 查找交通车数据
   * @param accId
   * @return {Vehicle}
   */
  findVehiclesByAccID (accId) {
    return this.vehicles.filter(value => value.accId === accId)
  }

  /**
   * 查找交通车数据
   * @param mergeId
   * @return {Vehicle}
   */
  findVehiclesByMergeID (mergeId) {
    return this.vehicles.filter(value => value.mergeId === mergeId)
  }

  /**
   * 删除一个交通车数据
   * @param id
   */
  removeVehicle (id) {
    const vehicle = this.vehiclesMap.get(`${id}`)
    if (vehicle) {
      // 从map中刪除
      this.vehiclesMap.delete(`${vehicle.id}`)
      this.vehicles = this.vehicles.filter(v => +v.id !== +id)

      // 从model map中删除
      if (vehicle.model) {
        this.vehiclesModelMap.delete(`${vehicle.model.uuid}`)
      } else {
        console.log('remove vehicle error! vehicle model is null')
      }

      // 删除路径点
      this.removeVehicleEndModel(vehicle)
    }
  }

  /**
   * 修改一个交通车数据
   * @param vehicle
   */
  modifyVehicle (vehicle) {
    const tmp = this.vehiclesMap.get(`${vehicle.id}`)
    if (!tmp) {
      console.error('modify vehicle error, vehicle not exist: ', vehicle.id)
      return
    }

    if (vehicle.routeId !== tmp.routeId || vehicle.laneId !== tmp.laneId) {
      this.simuScene.updateRouteStartPos(vehicle.routeId, vehicle.laneId)
    }

    tmp.copyNoModel(vehicle)
  }

  /**
   * 添加一个障碍物
   * @param obstacle
   */
  addObstacle (obstacle) {
    obstacle.id = Utility.getNewId(this.obstacles)
    this.obstacles.push(obstacle)
    this.obstaclesMap.set(`${obstacle.id}`, obstacle)
  }

  /**
   * 添加一个障碍物模型
   * @param obstacle
   */
  addObstacleModel (obstacle) {
    if (!obstacle.model) {
      console.log('addObstacleModel error model null')
      return
    }

    this.obstaclesModelMap.set(obstacle.model.uuid, obstacle)
  }

  get obstacleModels () {
    return this.obstacles.map(o => o.model)
  }

  /**
   * 查找障碍物
   * @param id
   * @return {any}
   */
  findObstacleInModelMap (id) {
    return this.obstaclesModelMap.get(`${id}`)
  }

  /**
   * 查找障碍物
   * @param id
   * @return {any}
   */
  findObstacleInMap (id) {
    return this.obstaclesMap.get(`${id}`)
  }

  /**
   * 查找障碍物
   * @param id
   * @return {any}
   */
  findObstacleInArr (id) {
    return this.obstacles.find(value => +value.id === +id)
  }

  /**
   * 查找障碍物
   * @param routeId
   * @return {any}
   */
  findObstaclesByRouteID (routeId) {
    return this.obstacles.filter(value => value.routeId == routeId)
  }

  /**
   * 删除一个障碍物模型
   * @param id
   */
  removeObstacleModel (id) {
    const obstacle = this.obstaclesMap.get(`${id}`)
    if (obstacle) {
      // 从model map中删除
      if (obstacle.model) {
        this.obstaclesModelMap.delete(`${obstacle.model.uuid}`)
      } else {
        console.log('remove obstacle error! obstacle model is null')
      }
    }
  }

  /**
   * 删除一个障碍物
   * @param id
   */
  removeObstacle (id) {
    const obstacle = this.obstaclesMap.get(`${id}`)
    if (obstacle) {
      // 从map中刪除
      this.obstaclesMap.delete(`${obstacle.id}`)
      this.obstacles = this.obstacles.filter(o => +o.id !== +id)
    }
  }

  /**
   * 修改一个障碍物
   * @param obstacle
   */
  modifyObstacle (obstacle) {
    const tmp = this.obstaclesMap.get(`${obstacle.id}`)
    if (!tmp) {
      console.error('modify obstacle error, obstacle not exist: ', obstacle.id)
      return
    }

    if (obstacle.routeId != tmp.routeId || obstacle.laneId != tmp.laneId) {
      this.simuScene.updateRouteStartPos(obstacle.routeId, obstacle.laneId)
    }

    tmp.copyNoModel(obstacle)
  }

  /**
   * 添加一个行人
   * @param pedestrian
   */
  addPedestrian (pedestrian) {
    pedestrian.id = Utility.getNewId(this.pedestrians)
    this.pedestrians.push(pedestrian)
    this.pedestriansMap.set(`${pedestrian.id}`, pedestrian)
  }

  /**
   * 添加一个行人模型
   * @param pedestrian
   */
  addPedestrianModel (pedestrian) {
    if (!pedestrian.model) {
      console.log('addPedestrianModel error model null')
      return
    }

    this.pedestriansModelMap.set(pedestrian.model.uuid, pedestrian)
  }

  get pedestrianModels () {
    return this.pedestrians.map(p => p.model)
  }

  /**
   * 添加一个行人轨迹点模型
   * @param pedestrian
   */
  addPedestrianEndModel (pedestrian) {
    if (!pedestrian) {
      return
    }

    const route = this.findRouteInMap(pedestrian.routeId)

    if (!route?.curve) {
      return
    }

    route.curve.Boxes().forEach((value) => {
      const box = this.findPedestrianInEndModelMap(value.uuid)
      this.pedestriansEndModelMap.set(value.uuid, pedestrian)
      if (!box) {
        this.pedestrianEndModels.push(value)
      }
    })

    pedestrian.setRoute(route)
  }

  /**
   * 查找行人
   * @param id
   * @return {any}
   */
  findPedestrianInModelMap (id) {
    return this.pedestriansModelMap.get(`${id}`)
  }

  /**
   * 查找行人
   * @param id
   * @return {any}
   */
  findPedestrianInEndModelMap (id) {
    return this.pedestriansEndModelMap.get(`${id}`)
  }

  /**
   * 查找行人
   * @param id
   * @return {any}
   */
  findPedestrianInMap (id) {
    return this.pedestriansMap.get(`${id}`)
  }

  /**
   * 查找行人
   * @param uuid
   * @return {any}
   */
  findPedestrianEndModelInArr (uuid) {
    return this.pedestrianEndModels.findIndex(value => value.uuid === uuid)
  }

  /**
   * 查找行人
   * @param id
   * @return {any}
   */
  findPedestrianInArr (id) {
    return this.pedestrians.find(pedestrian => +pedestrian.id === +id)
  }

  /**
   * 查找行人
   * @param routeId
   * @return {any}
   */
  findPedestriansByRouteID (routeId) {
    return this.pedestrians.filter(value => value.routeId == routeId)
  }

  /**
   * 删除一个行人
   * @param id
   */
  removePedestrian (id) {
    const pedestrian = this.pedestriansMap.get(`${id}`)
    if (pedestrian) {
      // 从map中刪除
      this.pedestriansMap.delete(`${pedestrian.id}`)
      this.pedestrians = this.pedestrians.filter(p => +p.id !== +id)

      // 从model map中删除
      if (pedestrian.model) {
        this.pedestriansModelMap.delete(`${pedestrian.model.uuid}`)
      } else {
        console.log('remove pedestrian error! pedestrian model is null')
      }
    }
  }

  /**
   * 更新行人数据
   * @param pedestrian
   */
  modifyPedestrian (pedestrian) {
    const tmp = this.pedestriansMap.get(`${pedestrian.id}`)
    if (!tmp) {
      console.error('modify pedestrian error, pedestrian not exist: ', pedestrian.id)
      return
    }

    if (pedestrian.routeId != tmp.routeId || pedestrian.laneId != tmp.laneId) {
      this.simuScene.updateRouteStartPos(pedestrian.routeId, pedestrian.laneId)
    }

    tmp.copyNoModel(pedestrian)
  }

  // 查找动态障碍物，就是查找pedestrian和查找vehicle的组合，player专用
  findDynamicObstacleInMap (id) {
    let result = this.findPedestrianInMap(id)
    if (!result) {
      result = this.findVehicleInMap(id)
    }
    return result
  }

  findSignlightInMap (id) {
    return this.signlightsMap.get(`${id}`)
  }

  findSignlightInArr (id) {
    return this.signlights.findIndex(signlight => signlight.id == id)
  }

  findSignlightsByRouteID (routeId) {
    return this.signlights.filter(signlight => signlight.routeId == routeId)
  }

  removeSignlight (id) {
    const signlight = this.signlightsMap.get(`${id}`)
    if (signlight) {
      // 从map中刪除
      this.signlightsMap.delete(`${signlight.id}`)

      // 从數組中刪除
      const tmpIndex = this.findSignlightInArr(id)
      if (tmpIndex >= 0) {
        this.signlights.splice(tmpIndex, 1)
      } else {
        console.error('remove signlight error!')
        console.assert(false, 'remove signlight error!')
      }
    }
  }

  /**
   * 添加一个planner
   * @param planner
   */
  addPlanner (planner) {
    planner.id = Utility.getNewId(this.planners)
    this.planners.push(planner)
  }

  /**
   * 删除主车轨迹点模型
   * @param planner
   */
  removePlannerEndModel (planner) {
    if (!planner) {
      return
    }

    const route = this.findRouteInMap(planner.routeId)

    if (!route || !route.curve) {
      return
    }

    route.curve.Boxes().forEach((value) => {
      const idx = this.findPlannerEndModelInArr(value.uuid)
      if (idx > -1) {
        this.plannersEndModelMap.delete(`${value.uuid}`)
        this.plannerEndModels.splice(idx, 1)
      }
    })
  }

  /**
   * 修改planner
   * @param planner
   */
  modifyPlanner (planner) {
    this.planners.find(({ id }) => id === planner.id).copyNoModel(planner)
  }

  /**
   * 添加一个planner的轨迹点模型
   * @param planner
   */
  addPlannerEndModel (planner) {
    if (!planner) {
      return
    }

    const route = this.findRouteInMap(planner.routeId)
    if (!route?.curve) {
      return
    }

    route.curve.Boxes().forEach((value) => {
      const box = this.findPlannerInEndModelMap(value.uuid)
      this.plannersEndModelMap.set(value.uuid, planner)
      if (!box) {
        this.plannerEndModels.push(value)
      }
    })

    // update route
    planner.setRoute(route)
  }

  /**
   * 添加一个planner的模型
   * @param planner
   * @param model
   */
  addPlannerModel (planner, model) {
    if (!model) {
      if (!planner.model) {
        console.log('addVehicleModel error model null')
        return
      }

      this.plannersModelMap.set(planner.model.uuid, planner)
    } else {
      this.plannersModelMap.set(model.uuid, planner)
    }
  }

  get plannerModels () {
    return this.planners.map(p => p.model)
  }

  findPlannerInEndModelMap (id) {
    return this.plannersEndModelMap.get(`${id}`)
  }

  /**
   * @param id
   * @return {Planner}
   */
  findPlannerInModelMap (id) {
    return this.plannersModelMap.get(`${id}`)
  }

  /**
   * 查找主车
   * @param id
   * @return {Planner}
   */
  findPlannerInArr (id) {
    return this.planners.find(p => p.id === id)
  }

  /**
   * 查找主车
   * @param uuid
   * @return {Planner}
   */
  findPlannerEndModelInArr (uuid) {
    return this.plannerEndModels.findIndex(p => p.uuid === uuid)
  }

  /**
   * 查找主车
   * @param routeId
   * @return {Planner}
   */
  findPlannersByRouteID (routeId) {
    return this.planners.filter(p => p.routeId === routeId)
  }

  removePlanner (planner) {
    const index = this.planners.findIndex(({ id }) => id === planner.id)
    this.planners.splice(index, 1)
    this.plannersModelMap.delete(planner.model.uuid)
    this.removePlannerEndModel(planner)
  }

  /* eslint-disable no-cond-assign */
  findAnyInModelMap (object) {
    let ele = null
    if (ele = this.findPlannerInModelMap(object.uuid)) {
      return {
        type: 'planner',
        subType: ele.type,
      }
    } else if (ele = this.findVehicleInModelMap(object.uuid)) {
      return {
        type: 'car',
        subType: ele.type,
      }
    } else if (ele = this.findObstacleInModelMap(object.uuid)) {
      return {
        type: 'obstacle',
        subType: ele.type,
      }
    } else if (ele = this.findPedestrianInModelMap(object.uuid)) {
      return {
        type: 'pedestrian',
        subType: ele.type,
      }
    }
    return ele
  }

  /**
   * 查找任何场景元素
   * @param type
   * @param id
   * @return {null}
   */
  fineAnyById (type, id) {
    let result = null
    switch (type) {
      case 'planner':
        result = this.findPlannerInArr(id)
        break
      case 'car':
        result = this.findVehicleInMap(id)
        break
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
        result = this.findPedestrianInMap(id)
        break
      case 'obstacle':
        result = this.findObstacleInMap(id)
        break
      default:
    }
    return result
  }

  addRosbag (rosbag) {
    this.rosbag = rosbag
  }

  modifyRosbag (rosbag) {
    this.rosbag.path = rosbag.path
  }

  removeRosbag () {
    this.rosbag = null
  }

  applyTrafficAI (config) {
    this.trafficAIConfig.parse(config)
    this.applyAggress(1, this.trafficAIConfig.aggress)
  }

  getTrafficAIConfig () {
    return this.trafficAIConfig.uiMsg()
  }

  applyAggress (apply, aggress) {
    if (apply == 1) {
      this.aggressApply = apply
      this.trafficAIConfig.aggress = aggress
    } else {
      this.aggressApply = 0
      this.trafficAIConfig.aggress = 0.5
    }

    this.vehicles.forEach((value, index, array) => {
      if (value.behavior === VehicleBehaviorType.TrafficVehicle) {
        if (this.aggressApply == 1) {
          value.aggress = this.getRandomAggress()
        } else {
          value.aggress = 0.5
        }
      }
    })
  }

  getRandomAggress () {
    if (this.aggressApply == 1) {
      let d = this.utility.generateGaussianNoise(this.trafficAIConfig.aggress, 0.1)
      if (d < 0) {
        d = Number.MIN_VALUE
      } else if (d > 1) {
        d = 1
      }

      return d
    }

    return 0.5
  }

  getGenerateInfo () {
    return this.generateInfo
  }

  setSensorInfo (sensorInfo) {
    this.sensorInfo = sensorInfo
  }

  getSensorInfo () {
    return this.sensorInfo
  }

  setEnvironmentInfo (environmentInfo) {
    this.envInfo = environmentInfo
  }

  getEnvironmentInfo () {
    return this.envInfo
  }

  setGradingInfo (gradingInfo) {
    this.gradingInfo = gradingInfo
  }

  getGradingInfo () {
    return this.gradingInfo
  }

  getRoadobj () {
    return this.roadObj
  }

  getSceneevents () {
    return this.sceneevents
  }

  setSceneDesc (info) {
    this.sceneDesc = info
  }

  // 获取路径轨迹点
  getRouteCurve (routeId) {
    try {
      const route = this.findRouteInMap(routeId)
      return route.curve
    } catch (error) {
      console.warn(error)
      return null
    }
  }

  getDefaultAccID () {
    const acc = this.findAccInMap(0)
    return acc ? '0' : '1'
  }

  getDefaultMergeID () {
    const merge = this.findMergeInMap(0)
    return merge ? '0' : '1'
  }
}

SimuSceneParser.getSceneURL = 'http://localhost:9000/openscene'
SimuSceneParser.saveSceneURL = 'http://localhost:9000/savescene/v2'
SimuSceneParser.paramSceneURL = 'http://localhost:9000/paramscene'
SimuSceneParser.convertRosbagURL = 'http://localhost:9000/convertrosbag'

export default SimuSceneParser
