import { union } from 'lodash'
import {
  disposeJunctionLink,
  disposeOneJunction,
  renderJunction,
  renderJunctionLink,
} from '../render/junction'
import root3d from '../index'
import { useJunctionStore } from '@/stores/junction'
import { getJunction } from '@/utils/mapCache'

export function initJunctionDispatcher () {
  const junctionStore = useJunctionStore()

  junctionStore.$reset()

  const container = root3d.mapElementsContainer

  // 交叉路口的渲染调度
  const unsubscribeJunctionStore = junctionStore.$onAction(
    ({ name, store, args, after, onError }) => {
      after(async (res) => {
        switch (name) {
          case 'applyMapFileState': {
            const [newState] = args
            if (!newState || newState.ids.length < 1) return
            const renderAllJunction = []
            for (const junctionId of newState.ids) {
              const junction = getJunction(junctionId)
              if (junction) {
                // 首次渲染不需要销毁之前的同元素
                renderAllJunction.push(
                  renderJunction({
                    junction,
                    parent: container,
                    preDispose: false,
                  }),
                )
              }
            }
            await Promise.all(renderAllJunction)
            return
          }
          case 'applyState': {
            // // 还原状态时，增量对路口元素做重绘
            const { lastDiffIds, diff: diffState } = args[0]
            if (!lastDiffIds || !diffState) return
            // 通过 diffState 和 lastDiffIds 求并集判断有多少变化的路口
            const diffIds = union(lastDiffIds, diffState.ids)
            for (const _diffId of diffIds) {
              disposeOneJunction({
                junctionId: _diffId,
                parent: container,
              })
            }

            // 重绘上一个状态的路口
            for (const _diffJunctionId of lastDiffIds) {
              const _junction = getJunction(_diffJunctionId)
              if (_junction) {
                renderJunction({
                  junction: _junction,
                  parent: container,
                })
              }
            }
            root3d.core.render()
            return
          }
          case 'connectLinkRoad': {
            const junctionId = args[0].junctionId || ''
            // 如果 junctionId 不存在，则取最新添加的 junction
            const _junctionId = junctionId || junctionStore.ids[junctionStore.ids.length - 1]
            const junction = getJunction(_junctionId)

            if (!junction) return
            renderJunction({ junction, parent: container })
            return
          }
          case 'disconnectLinkRoad': {
            const junctionId = args[0].junctionId
            const junction = getJunction(junctionId)
            if (!junction) return
            renderJunction({
              junction,
              parent: container,
            })
            return
          }
          case 'removeJunction': {
            const junctionId = args[0]
            disposeOneJunction({
              junctionId,
              parent: container,
            })
            return
          }
          case 'removeJunctionLink': {
            const laneLinkId = args[1]
            disposeJunctionLink({ laneLinkId, parent: container })
            return
          }
          case 'addJunctionLink': {
            // 如果 res 返回 false，说明该 junctionLink 不合法或者 junction 不存在
            if (!res) return
            const { junctionId } = args[0]
            renderJunctionLink({
              junctionId,
              laneLink: res as biz.ILaneLink,
              parent: container,
            })
            return
          }
          case 'updateJunction': {
            // 如果 res 返回 false，说明当前并非是单纯更新车道结构，其他调用函数还会触发一次更新交叉路口结构的逻辑
            if (!res) return
            const { junction } = args[0]
            renderJunction({
              junction,
              parent: container,
            })
            // 在 junctionStore 外直接调用交叉路口结构更新的函数，由于触发函数是同步的
            // 先执行 updateJunction 的更新逻辑，再触发原函数的 render 函数
            // 所以当前函数可以直接返回，不需要触发 junction 中的 render，减少一次 drawCall
            return
          }
        }

        root3d.core.render()
      })
      onError((err) => {
        console.log(err)
      })
    },
  )

  return unsubscribeJunctionStore
}
