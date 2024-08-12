import { Vector3 } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditSectionHelper from './main3d'
import {
  disposeBoundaryHelper,
  renderBoundaryHelper,
} from './main3d/draw'
import { useEditSectionStore } from './store'
import { useRoadStore } from '@/stores/road'

class EditSectionPlugin extends TemplatePlugin {
  editHelper: EditSectionHelper
  unsubscribeEditSectionStore: Function
  constructor () {
    super()
    this.config = config
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditSectionHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()
  }

  initDispatchers () {
    const editSectionStore = useEditSectionStore()
    const roadStore = useRoadStore()

    this.unsubscribeEditSectionStore = editSectionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        let lastRoadId = ''
        let lastSectionId = ''
        let lastLaneId = ''
        if (name === 'selectLane' || name === 'unselectLane') {
          // 记录上一个状态的选中车道
          lastRoadId = store.roadId
          lastSectionId = store.sectionId
          lastLaneId = store.laneId
        }
        after((res) => {
          // 如果当前插件不可用，则不会执行订阅方法后的渲染逻辑
          if (!this.enabled) return
          switch (name) {
            case 'addSection': {
              const { roadId, sectionId, laneId, tweenCurveSamplePoints } =
                store
              // // TODO 渲染对应的平滑过渡辅助元素，后续需要删除
              // renderTweenCurve({
              //   roadId,
              //   sectionId,
              //   laneId,
              //   controlPoints: tweenControlPoints,
              //   parent: this.editHelper.container,
              //   preDispose: true,
              //   tweenSamplePoints: tweenCurveSamplePoints,
              // })
              // 移除辅助的边界线和预设截断点
              disposeBoundaryHelper({
                roadId,
                sectionId,
                laneId,
                parent: this.editHelper.container,
              })
              this.render()
              break
            }
            case 'unselectLane': {
              if (lastRoadId && lastSectionId && lastLaneId) {
                // 销毁之前的辅助元素
                disposeBoundaryHelper({
                  roadId: lastRoadId,
                  sectionId: lastSectionId,
                  laneId: lastLaneId,
                  parent: this.editHelper.container,
                })
                this.render()
              }
              break
            }
            case 'selectLane': {
              // TODO 展示选中车道的左右边界线辅助元素
              const { roadId, sectionId, laneId } = store
              // 只要有一个 id 不一致，就需要重绘
              if (
                roadId !== lastRoadId ||
                sectionId !== lastSectionId ||
                laneId !== lastLaneId
              ) {
                // 销毁之前的辅助元素
                disposeBoundaryHelper({
                  roadId: lastRoadId,
                  sectionId: lastSectionId,
                  laneId: lastLaneId,
                  parent: this.editHelper.container,
                })

                const currentRoad = roadStore.getRoadById(roadId)
                if (currentRoad) {
                  const { leftPoints, rightPoints } = store
                  // 渲染当前选中车道的辅助元素
                  renderBoundaryHelper({
                    road: currentRoad,
                    sectionId,
                    laneId,
                    parent: this.editHelper.container,
                    leftPoints,
                    rightPoints,
                    preDispose: true,
                  })
                }

                this.render()
              }

              break
            }
          }
        })
        onError((err) => {
          console.log(err)
        })
      },
    )
  }

  selectOneLane () {
    const editSectionStore = useEditSectionStore()
    editSectionStore.selectLane('1', '0', '-2')
  }

  selectOtherLane () {
    // test 在截断 section 后，选中 section 1 的外层车道
    const editSectionStore = useEditSectionStore()
    editSectionStore.selectLane('1', '1', '-2')
  }

  unselectLane () {
    const editSectionStore = useEditSectionStore()
    editSectionStore.unselectLane()
  }

  addPureSection () {
    if (!this.enabled) return
    const editSectionStore = useEditSectionStore()
    // 单纯截断 section 的效果
    const leftPoint = new Vector3(11.914, 0.84, 19.39)
    const rightPoint = new Vector3(8.798, 0.84, 20.983)
    editSectionStore.addSection({
      leftPoint,
      rightPoint,
    })
  }

  addTransitionSection () {
    if (!this.enabled) return
    const editSectionStore = useEditSectionStore()

    // 拓展出来的效果
    // // 左侧边界线索引为 0
    // const leftPoint = new Vector3(11.927, 0.84, 19.414)
    // const rightPoint = new Vector3(13.009, 1.639, 28.286)
    // // 左侧边界线索引为 0，且右侧边界索引为末尾
    // const leftPoint = new Vector3(11.927, 0.84, 19.414)
    // const rightPoint = new Vector3(34.176, 5, 53.883)
    // // 右侧边界索引为末尾
    // const leftPoint = new Vector3(32.347, 5.219, 44.392)
    // const rightPoint = new Vector3(34.176, 5, 53.883)
    // 左侧和右侧边界线均为中间截断点
    const leftPoint = new Vector3(15.38, 1.57, 25.548)
    const rightPoint = new Vector3(18.508, 2.938, 34.372)

    // 车道变窄趋势的效果
    // // 右边界线索引为 0
    // const leftPoint = new Vector3(14.864, 1.439, 24.726)
    // const rightPoint = new Vector3(8.823, 0.84, 21.033)
    // // 右边界线索引为 0，且左边界索引为末尾
    // const leftPoint = new Vector3(37.088, 5, 51.941)
    // const rightPoint = new Vector3(8.823, 0.84, 21.033)
    // // 左边界线索引为末尾
    // const leftPoint = new Vector3(37.088, 5, 51.941)
    // const rightPoint = new Vector3(29.128, 5.232, 45.846)
    // // 左侧和右侧边界线均为中间截断点
    // const leftPoint = new Vector3(31.005, 5.267, 42.283)
    // const rightPoint = new Vector3(23.85, 4.676, 38.676)

    editSectionStore.addSection({
      leftPoint,
      rightPoint,
    })
  }

  onActivate () {}
  onDeactivate () {}
  dispose () {
    this.editHelper.dispose()
    this.unsubscribeEditSectionStore && this.unsubscribeEditSectionStore()
  }
}

export default new EditSectionPlugin()
