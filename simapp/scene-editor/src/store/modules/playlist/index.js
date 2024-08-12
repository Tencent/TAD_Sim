import { forEach, find } from 'lodash-es'
import service from '@/api'
import { errorHandler } from '@/common/errorHandler'

export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    loading: false,
    keyword: '',
    playlist: [], // sim
    prevPlayingScenario: null,

    playlistSimRec: [], // simrec
    sceneType: 'sim', // 默认sim的
  },
  getters: {
    playIdsList (state) {
      return state.playlist.map((item) => {
        return item.id
      }).concat(state.playlistSimRec.map(item => item.id))
    },
    filteredPlaylist (state, getters) {
      const list = getters.curPlayList
      return list.filter(item => item.filename.includes(state.keyword))
    },
    curPlayList (state) {
      const list = state.sceneType === 'sim' ? state.playlist : state.playlistSimRec
      return list
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
    moveUp (state, payload) {
      const { index, item } = payload
      if (index === 0) {
        return
      }
      const targetIndex = index - 1
      const list = state.sceneType === 'sim' ? state.playlist : state.playlistSimRec
      list.splice(index, 1)
      list.splice(targetIndex, 0, item)
      // state.playlist.splice(index, 1)
      // state.playlist.splice(targetIndex, 0, item)
    },
    moveDown (state, payload) {
      const { index, item } = payload
      const list = state.sceneType === 'sim' ? state.playlist : state.playlistSimRec
      if (index === list.length - 1) {
        return
      }
      const targetIndex = index + 1
      list.splice(index, 1)
      list.splice(targetIndex, 0, item)
      // if (index === state.playlist.length - 1) {
      //   return
      // }
      // const targetIndex = index + 1
      // state.playlist.splice(index, 1)
      // state.playlist.splice(targetIndex, 0, item)
    },
  },
  actions: {
    /**
     * 获取播放列表
     * @param commit
     * @param rootState
     * @return {Promise<void>}
     */
    async getPlaylist ({ commit, rootState }) {
      try {
        const { scenarios: ids } = await service.getPlaylist()
        const { scenarioList } = rootState.scenario
        const playlist = []
        const playlistSimRec = []
        forEach(ids, (id) => {
          const matched = find(scenarioList, { id })
          if (matched) {
            if (matched.traffictype === 'simrec') {
              playlistSimRec.push(matched)
            } else {
              playlist.push(matched)
            }
          }
        })
        commit('updateState', { playlist, playlistSimRec })
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 更新播放列表
     * @param commit
     * @param dispatch
     * @param ids
     * @return {Promise<void>}
     */
    async updatePlaylist ({ commit, dispatch }, ids) {
      commit('updateState', { loading: true })
      try {
        await service.updatePlaylist(ids)
        await dispatch('getPlaylist')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 从播放列表中移除场景
     * @param commit
     * @param dispatch
     * @param ids
     * @return {Promise<void>}
     */
    async removeScenariosFromPlaylist ({ commit, dispatch }, ids) {
      commit('updateState', { loading: true })
      try {
        await service.removeScenariosFromPlaylist(ids)
        await dispatch('getPlaylist')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
  },
}
