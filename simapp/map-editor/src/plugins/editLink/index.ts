import { Color, type Intersection, Line, MOUSE, Mesh } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditLinkHelper from './main3d'
import {
  getSelected,
  registryHoverElement,
  setSelected,
} from '@/main3d/controls/dragControls'
import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import { useJunctionStore } from '@/stores/junction'
import { getJunction, getRoad } from '@/utils/mapCache'
import {
  activeJunctionLinkHTPoint,
  getJunctionLinkHTPointColor,
  junctionLinkHTPointColor,
  removeAllHTPoints,
  renderHTPointOnRoad,
  resetAllHTPointColor,
  resetJunctionLinkColor,
  setAllJunctionLinksVisible,
  setJunctionLinkLinkedColor,
  setJunctionLinkSelectedColor,
  setJunctionLinkVisible,
} from '@/plugins/editLink/main3d/draw'
import { getLaneLinkUniqueFlag } from '@/stores/junction/link'
import { confirm, warningMessage } from '@/utils/common'
import i18n from '@/locales'
import root3d from '@/main3d'
import { linkColor } from '@/main3d/render/junction'
import { useHistoryStore } from '@/stores/history'
import { RenderOrder } from '@/utils/business'
import { getPermission } from '@/utils/permission'

class EditLinkPlugin extends TemplatePlugin {
  editHelper: EditLinkHelper
  unsubscribeLinkInteractionStore: Function
  constructor () {
    super()
    this.config = config

    this.dragDispatchers.dragStart = (options: common.IDragOptions) => {
      if (!this.enabled) return

      const linkInteractionStore = useLinkInteractionStore()
      const { junctionId: curSelectedJunctionId } = linkInteractionStore
      // 判断是否有符合当前模式的交互物体
      const { intersections, pointerdownButton } = options.object

      if (pointerdownButton === MOUSE.LEFT) {
        intersections.forEach((intersection: Intersection) => {
          if (getSelected()) return
          const { object } = intersection
          // 当选择的 junctionLink 所属 junction 不是当前选中 junction 时，不允许拾取 junctionLink
          if (
            object.name === 'junctionLink' &&
            curSelectedJunctionId !== object.junctionId
          ) {
            return
          }

          setSelected(intersection)
        })
      }
    }

    // 用于记录 junctionLink 在 hover 之前是 linkColor['normal'] or linkColor['linked']
    let junctionLinkColorBeforeHover: Color | null

    this.dragDispatchers.click = (options: common.IDragOptions) => {
      if (!this.enabled) return
      const { pointerdownButton } = options.object
      if (pointerdownButton === MOUSE.LEFT) {
        const selected = getSelected()
        if (!selected) return

        const {
          object: { name: objName },
        } = selected

        const linkInteractionStore = useLinkInteractionStore()

        switch (objName) {
          case 'junction':
            linkInteractionStore.unSelectAllButJunction()
            linkInteractionStore.selectJunction(selected.object.junctionId)
            break
          case 'junctionLinkHTPoint':
            // 通过 hTPoint 的唯一 flag 来选点
            linkInteractionStore.selectHTPoint(selected.object.flag)
            break
          case 'junctionLink':
            linkInteractionStore.selectJunctionLink(selected.object.laneLinkId)
            break
          default:
            linkInteractionStore.unSelectAllButJunction()
        }
      }
    }

    this.dragDispatchers.hoverOn = (options: common.IDragOptions) => {
      if (!this.enabled) return
      const { elementName, elementUuid } = options.object
      if (!elementName || !elementUuid) return

      const targetObject = root3d.getObjectByUuid(elementUuid)
      if (!targetObject) return

      const linkInteractionStore = useLinkInteractionStore()
      const { junctionId: curSelectedJunctionId } = linkInteractionStore

      // 当射线拾取到的 junctionLink 所属 junction 不是当前选中 junction 时，不进行 hover 交互
      if (
        elementName === 'junctionLink' &&
        curSelectedJunctionId &&
        curSelectedJunctionId !== targetObject.junctionId
      ) {
        return
      }

      if (elementName === 'junctionLinkHTPoint') {
        if (targetObject instanceof Mesh) {
          const { material } = targetObject
          if (
            !new Color(junctionLinkHTPointColor.selected).equals(
              material.color,
            )
          ) {
            material.color.set(junctionLinkHTPointColor.hovered)
            material.needsUpdate = true
            this.render()
          }
        }
      } else if (elementName === 'junctionLink') {
        if (targetObject instanceof Line) {
          const { material } = targetObject
          junctionLinkColorBeforeHover = targetObject.material.color.clone()

          if (!new Color(linkColor.selected).equals(material.color)) {
            targetObject.material.color.set(linkColor.hovered)
            targetObject.renderOrder = RenderOrder.highLightLaneLink
            targetObject.material.needsUpdate = true
            this.render()
          }
        }
      }
    }

    this.dragDispatchers.hoverOff = (options: common.IDragOptions) => {
      if (!this.enabled) return
      const { elementName, elementUuid } = options.object
      if (!elementName || !elementUuid) return

      const targetObject = root3d.getObjectByUuid(elementUuid)
      if (!targetObject) return

      const linkInteractionStore = useLinkInteractionStore()
      const { junctionId: curSelectedJunctionId } = linkInteractionStore

      // 当射线拾取到的 junctionLink 所属 junction 不是当前选中 junction 时，不进行 hover 交互
      if (
        elementName === 'junctionLink' &&
        curSelectedJunctionId &&
        curSelectedJunctionId !== targetObject.junctionId
      ) {
        return
      }

      if (elementName === 'junctionLinkHTPoint') {
        if (targetObject instanceof Mesh) {
          const { material } = targetObject
          if (
            !new Color(junctionLinkHTPointColor.selected).equals(
              material.color,
            )
          ) {
            material.color.set(junctionLinkHTPointColor.normal)
            material.needsUpdate = true
            this.render()
          }
        }
      } else if (elementName === 'junctionLink') {
        if (targetObject instanceof Line) {
          const { material } = targetObject
          if (
            !new Color(linkColor.selected).equals(material.color) &&
            junctionLinkColorBeforeHover
          ) {
            material.color.set(junctionLinkColorBeforeHover)
            targetObject.renderOrder = RenderOrder.laneLink
            junctionLinkColorBeforeHover = null
            material.needsUpdate = true
            this.render()
          }
        }
      }
    }
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditLinkHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()

    this.addDragListener()

    // 注册当前模式下，可支持 hover 交互的元素名称
    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['junctionLinkHTPoint', 'junctionLink'],
    })
  }

  initDispatchers () {
    const linkInteractionStore = useLinkInteractionStore()
    const historyStore = useHistoryStore()
    linkInteractionStore.$reset()

    const parent = this.editHelper.container

    this.unsubscribeLinkInteractionStore = linkInteractionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        if (!this.enabled) return
        const { junctionId: lastJunctionId } = store.$state
        after(async (res) => {
          switch (name) {
            case 'applyState': {
              const [currentState] = args
              const { junctionId: currentJunctionId } = currentState

              linkInteractionStore.updateTimestamp()

              if (currentJunctionId) {
                // 重做后会重新渲染 laneLink ，laneLink visible 会变为false，每次都需要重新选择 junction
                await handleSelectJunction(currentJunctionId)
              } else {
                handleUnSelectJunctionLink(lastJunctionId)
                handleUnSelectHTPoint(lastJunctionId)
                removeAllHTPoints(parent)
              }

              break
            }
            case 'selectHTPoint': {
              if (
                res &&
                getPermission('action.mapEditor.laneLink.add.enable')
              ) {
                handleSelectHTPoint(args[0], store.junctionId)
                linkInteractionStore.updateTimestamp()
              }
              break
            }
            case 'selectJunctionLink': {
              if (res) {
                handleSelectJunctionLink(args[0], store.junctionId)
                linkInteractionStore.updateTimestamp()
              }
              break
            }
            case 'selectJunction': {
              if (res) {
                await handleSelectJunction(store.junctionId)
                linkInteractionStore.updateTimestamp()
              }
              break
            }
            case 'unSelectJunction': {
              handleUnSelectJunction(store.junctionId)
              linkInteractionStore.updateTimestamp()
              break
            }
            case 'unSelectHTPoint': {
              handleUnSelectHTPoint(store.junctionId)
              linkInteractionStore.updateTimestamp()
              break
            }
            case 'unSelectJunctionLink': {
              handleUnSelectJunctionLink(store.junctionId)
              linkInteractionStore.updateTimestamp()
              break
            }
            case 'removeSelectedJunctionLink': {
              if (res) {
                await handleRemoveJunctionLink(res as string, store.junctionId)
                linkInteractionStore.updateTimestamp()
              }
              break
            }
            case 'addJunctionLink': {
              handleAddJunctionLink(
                store.hTPointFlag,
                args[0],
                store.junctionId,
              )
              linkInteractionStore.updateTimestamp()
            }
          }
          this.render()
        })

        function handleSelectHTPoint (
          clickPointFlag: string,
          junctionId: string,
        ) {
          if (!clickPointFlag) return
          const pointsReadyToActive = new Set()
          pointsReadyToActive.add(clickPointFlag)
          const junction = getJunction(junctionId)
          if (!junction) return

          // 从当前 junction 中寻找与当前 point 有关系的连接线和 point，并高亮
          junction.laneLinks.forEach((laneLink: biz.ILaneLink) => {
            const { frid, fsid, fid, ftype, trid, tsid, tid, ttype } = laneLink
            const fDirection = Number(fid) < 0 ? 'forward' : 'reverse'
            const tDirection = Number(tid) < 0 ? 'forward' : 'reverse'
            const fromFlag = getLaneLinkUniqueFlag(
              frid,
              fsid,
              fid,
              ftype,
              fDirection,
            )
            const toFlag = getLaneLinkUniqueFlag(
              trid,
              tsid,
              tid,
              ttype,
              tDirection,
            )
            fromFlag === clickPointFlag && pointsReadyToActive.add(toFlag)
            toFlag === clickPointFlag && pointsReadyToActive.add(fromFlag)

            if (fromFlag == clickPointFlag || toFlag === clickPointFlag) {
              setJunctionLinkLinkedColor(laneLink.id)
            }
          })

          for (const pointFlag of pointsReadyToActive) {
            activeJunctionLinkHTPoint({
              hTPointFlag: pointFlag as string,
              parent,
            })
          }
        }
        function handleSelectJunctionLink (
          laneLinkId: string,
          junctionId: string,
        ) {
          if (!laneLinkId) return
          resetJunctionAllLinksColor(junctionId)
          setJunctionLinkSelectedColor(laneLinkId)
        }
        function handleUnSelectHTPoint (junctionId: string) {
          resetAllHTPointColor(parent)
          resetJunctionAllLinksColor(junctionId)
        }
        function handleUnSelectJunctionLink (junctionId: string) {
          resetAllHTPointColor(parent)
          resetJunctionAllLinksColor(junctionId)
        }
        async function handleSelectJunction (junctionId: string) {
          if (!junctionId) return
          // 重置 junctionLink 颜色， 隐藏其他 junctionLink 显示指定 junction 的 junctionLink
          resetJunctionAllLinksColor(junctionId)
          setAllJunctionLinksVisible(false)
          setJunctionLinkVisible(junctionId, true)

          // 重新渲染 hTPoint
          removeAllHTPoints(parent)
          await renderJunctionLinkHTPoints(junctionId)
        }
        function handleUnSelectJunction (junctionId: string) {
          // 重置 junctionLink 颜色， 隐藏 junctionLink
          resetJunctionAllLinksColor(junctionId)
          setAllJunctionLinksVisible(false)

          // 删除 points
          removeAllHTPoints(parent)
        }
        async function handleRemoveJunctionLink (
          laneLinkId: string,
          junctionId: string,
        ) {
          const junctionStore = useJunctionStore()

          let junction = getJunction(junctionId)
          if (!junction) return false

          const { laneLinks, linkRoads } = junction

          // 若删除该link后导致 junction 中某 linkRoad 没有任何 link ，则提示用户删除该 link 会影响路口结构
          // 检测哪些 linkRoad 在删除操作后会没有任何 link
          const linkRoadsWithoutAnyLink: Array<biz.ILinkRoad> = []
          linkRoads.forEach((linkRoad: biz.ILinkRoad) => {
            const [roadId, type] = linkRoad.split('_')
            const isCurLinkRoadHasLink = laneLinks.some(
              (laneLink: biz.ILaneLink) => {
                const { frid, ftype, trid, ttype, id } = laneLink
                // 不检测待删除的 link
                if (id === laneLinkId) return false

                const _ftype = ftype === 'start' ? '0' : '1'
                const _ttype = ttype === 'start' ? '0' : '1'
                return (
                  (roadId === frid && type === _ftype) ||
                  (roadId === trid && type === _ttype)
                )
              },
            )
            if (!isCurLinkRoadHasLink) {
              linkRoadsWithoutAnyLink.push(linkRoad)
            }
          })

          if (linkRoadsWithoutAnyLink.length) {
            // 表明删除连接线后会改变道路结构
            const confirmRes = await confirm({
              msg: i18n.global.t('desc.tips.deleteLaneLinkConfirmInfo'),
              type: 'que',
              title: i18n.global.t('desc.tips.tips'),
            })
            if (!confirmRes) return

            // 若用户在阅读了 “删除连接线会改变道路结构” 后，调用 junctionsStore 中的 disconnectLinkRoad 改变道路结构
            const allDisconnectLinkRoadPromises: Array<
              Promise<false | biz.IJunction>
            > = []
            linkRoadsWithoutAnyLink.forEach((linkRoad: biz.ILinkRoad) => {
              allDisconnectLinkRoadPromises.push(
                junctionStore.disconnectLinkRoad({
                  junctionId,
                  linkRoad,
                }),
              )
            })
            await Promise.all(allDisconnectLinkRoadPromises)

            // 改变道路结构后重新获取 junction , 判断 junction 是否还存在
            junction = getJunction(junctionId)
            if (junction) {
              // 存在则重新渲染辅助点
              await renderJunctionLinkHTPoints(junctionId)
              // todo: 会导致 junction 重新渲染 laneLink 导致 laneLink / laneLinkContainer 的 visible 为 false，这里暂时手动置为 true
              setJunctionLinkVisible(junctionId, true)
            } else {
              // 不存在则删除所有辅助点
              removeAllHTPoints(parent)
            }
          } else {
            // 表明删除连接线后不会改变道路结构
            junctionStore.removeJunctionLink(junctionId, laneLinkId)
          }

          historyStore.save({
            title: i18n.global.t('actions.junctionLink.remove'),
            junctionId,
            junctions: [junction],
          })

          store.unSelectJunctionLink()
        }
        function handleAddJunctionLink (
          curSelectedPointFlag: string,
          pointFlagReadyToLink: string,
          junctionId: string,
        ) {
          // 如果 pointFlagReadyToLink 已被选中 （或者说是如果该 link 已存在），则不添加 link
          if (
            new Color(junctionLinkHTPointColor.selected).equals(
              getJunctionLinkHTPointColor({
                hTPointFlag: pointFlagReadyToLink,
                parent,
              }),
            )
          ) {
            return
          }

          // 整理 laneLink 的 from 和 to ，并判断两条 lane 是否能相连
          const [rid, sid, id, type, direction] =
            curSelectedPointFlag.split('_')
          const [_rid, _sid, _id, _type, _direction] =
            pointFlagReadyToLink.split('_')
          let fid, frid, fsid, ftype, tid, trid, tsid, ttype

          // 考虑正反向车道的连通
          // 判断当前选中的点，跟即将要连通的目标点是否是不同类型的，如果是同类型则无法生成 link 线
          let isCurSelectType
          let isReadyToLinkType
          if (type === 'end' && direction === 'forward') {
            isCurSelectType = 'from'
          } else if (type === 'end' && direction === 'reverse') {
            isCurSelectType = 'to'
          } else if (type === 'start' && direction === 'forward') {
            isCurSelectType = 'to'
          } else if (type === 'start' && direction === 'reverse') {
            isCurSelectType = 'from'
          }
          if (_type === 'end' && _direction === 'forward') {
            isReadyToLinkType = 'from'
          } else if (_type === 'end' && _direction === 'reverse') {
            isReadyToLinkType = 'to'
          } else if (_type === 'start' && _direction === 'forward') {
            isReadyToLinkType = 'to'
          } else if (_type === 'start' && _direction === 'reverse') {
            isReadyToLinkType = 'from'
          }

          // 同类型则返回提示
          if (isCurSelectType === isReadyToLinkType) {
            warningMessage({
              content: i18n.global.t('desc.tips.addingJunctionLinkInvalid'),
            })
            return
          }

          if (isCurSelectType === 'from') {
            frid = rid
            fsid = sid
            fid = id
            ftype = direction === 'forward' ? 'end' : 'start'
            trid = _rid
            tsid = _sid
            tid = _id
            ttype = _direction === 'forward' ? 'start' : 'end'
          } else {
            frid = _rid
            fsid = _sid
            fid = _id
            ftype = _direction === 'forward' ? 'end' : 'start'
            trid = rid
            tsid = sid
            tid = id
            ttype = direction === 'forward' ? 'start' : 'end'
          }

          const junctionStore = useJunctionStore()
          const res = junctionStore.addJunctionLink({
            junctionId,
            laneLinkBaseInfo: {
              fid,
              frid,
              fsid,
              ftype: ftype as biz.ILaneLinkRoadType,
              tid,
              trid,
              tsid,
              ttype: ttype as biz.ILaneLinkRoadType,
            },
          })
          // 如果添加 junctionLink 成功，返回新添加的 laneLink 。需要高亮 toPoint 以及新添加的 laneLink
          if (res) {
            activeJunctionLinkHTPoint({
              hTPointFlag: pointFlagReadyToLink,
              parent,
            })
            setJunctionLinkLinkedColor(res.id)

            const junction = getJunction(junctionId)
            if (junction) {
              historyStore.save({
                title: i18n.global.t('actions.junctionLink.add'),
                junctionId,
                junctions: [junction],
              })
            }
          }
        }
        // 渲染一个路口所有辅助点
        async function renderJunctionLinkHTPoints (junctionId: string) {
          const junction = getJunction(junctionId)
          if (!junction) return
          // 绘制 hTPoints 之前先把点全部清空
          removeAllHTPoints(parent)

          const { linkRoads } = junction
          for (const linkRoad of linkRoads) {
            const [roadId, percent, direction] = linkRoad.split('_')
            const road = getRoad(roadId)
            if (!road) continue
            await renderHTPointOnRoad({ road, percent, parent, direction })
          }
        }
        // 重置一个路口中所有 link 的颜色
        function resetJunctionAllLinksColor (junctionId: string) {
          const junction = getJunction(junctionId)
          if (!junction) return
          junction.laneLinks.forEach((laneLink: biz.ILaneLink) => {
            resetJunctionLinkColor(laneLink.id)
          })
        }
      },
    )
  }

  onActivate () {}
  onDeactivate () {
    const linkInteractionStore = useLinkInteractionStore()
    linkInteractionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.removeDragListener()
    this.editHelper.dispose()
    this.unsubscribeLinkInteractionStore &&
    this.unsubscribeLinkInteractionStore()
  }
}

export default new EditLinkPlugin()
