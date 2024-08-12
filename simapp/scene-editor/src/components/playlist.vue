<template>
  <div class="playlist" :class="{ disabled: shouldPlayAfterSetup }">
    <div class="search-keyword">
      <el-input
        :model-value="keyword"
        prefix-icon="search"
        :placeholder="$t('operation.searchName')"
        @input="updateState({ keyword: $event })"
      />
    </div>

    <div v-if="filteredPlaylist.length && playingMode === 'list'" class="playlist-process">
      <PauseSvg />
      <span>{{ $t('player.batchPlaying') }} {{ times }} / {{ playingTimes }}</span>
    </div>

    <ul v-if="filteredPlaylist.length" class="playlist-list">
      <ListMultipleSelect
        ref="list-filtered-play-list"
        v-model="selected"
        :list="filteredPlaylist"
        :item-keys="['id']"
        :click-without-modifier-keys="false"
        @item-click="handleItemClick"
      >
        <template #item="{ item, index }">
          <li
            :id="`playlist-node-${item.id}`"
            :key="item.id"
            class="playlist-item"
            :class="{ 'playlist-item-active': isCurrentScenario(item.id) }"
          >
            <span class="playlist-item-name" :title="item.filename">{{ item.filename }}</span>
            <span class="playlist-item-operation">
              <template v-if="isCurrentScenario(item.id)">
                <PauseSvg v-if="playingStatus === 'playing'" class="icon pause-icon" />
                <PlaySvg v-else class="icon playing-icon" />
              </template>
              <template v-if="playingStatus !== 'playing'">
                <el-icon class="icon" @click.stop="handleRemove(item.id)"><Minus /></el-icon>
                <el-icon
                  class="icon"
                  :class="{ disabled: index === 0 }"
                  @click.stop="handleMoveUp(index, item)"
                ><Top /></el-icon>
                <el-icon
                  class="icon"
                  :class="{ disabled: index === filteredPlaylist.length - 1 }"
                  @click.stop="handleMoveDown(index, item)"
                ><Bottom /></el-icon>
              </template>
            </span>
          </li>
        </template>
      </ListMultipleSelect>
    </ul>

    <div v-else class="playlist-no-data">
      <span class="playlist-no-data-wrap" @click="handleAdd">
        <el-icon class="icon el-icon-plus"><plus /></el-icon>
        <span class="playlist-no-data-text">{{ $t('operation.addScene') }}</span>
      </span>
    </div>

    <div class="playlist-operations">
      <el-tooltip effect="dark" :content="$t('operation.addScene')" placement="bottom">
        <el-icon class="icon el-icon-plus" @click="handleAdd">
          <plus />
        </el-icon>
      </el-tooltip>
      &nbsp;&nbsp;
      <el-tooltip effect="dark" :content="$t('operation.removeScene')" placement="bottom">
        <el-icon class="icon el-icon-minus" @click="handleRemoveSelected">
          <minus />
        </el-icon>
      </el-tooltip>
    </div>

    <ScenarioAdd
      v-if="scenarioAddVisible"
      :selected="playIdsList"
      :can-switch-scene-type="false"
      :init-scene-type="sceneType"
      :init-available-scene-type="sceneType"
      @close="scenarioAddVisible = false"
      @confirm="handleScenarioAddConfirm"
    />
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import ListMultipleSelect from '@/components/list-multiple-select.vue'
import ScenarioAdd from '@/components/popups/scenario-add.vue'
import PlaySvg from '@/assets/images/list-play.svg'
import PauseSvg from '@/assets/images/list-pause.svg'

export default {
  name: 'Playlist',
  components: {
    ListMultipleSelect,
    ScenarioAdd,
    PlaySvg,
    PauseSvg,
  },
  data () {
    return {
      selected: [],
      scenarioAddVisible: false,
    }
  },
  computed: {
    ...mapState('scenario', [
      'currentScenario',
      'playingStatus',
      'playingMode',
      'playingTimes',
      'times',
      'shouldPlayAfterSetup',
    ]),
    ...mapState('playlist', [
      'keyword',
      'playlist',
      'sceneType',
    ]),
    ...mapGetters('playlist', [
      'playIdsList',
      'filteredPlaylist',
    ]),
    filteredPlayIdsList () {
      return this.filteredPlaylist.map((item) => {
        return item.id
      })
    },
  },
  watch: {
    currentScenario (value) {
      if (value) {
        const el = document.getElementById(`playlist-node-${value.id}`)
        if (el) {
          el.scrollIntoView()
        }
      }
    },
  },
  methods: {
    ...mapMutations('playlist', [
      'updateState',
      'moveUp',
      'moveDown',
    ]),
    ...mapActions('playlist', [
      'updatePlaylist',
      'removeScenariosFromPlaylist',
    ]),
    ...mapActions('scenario', [
      'resetScenario',
      'stop',
    ]),
    ...mapMutations('scenario', {
      updateScenario: 'updateState',
    }),
    setStartPlaylistScenarioId (id) {
      let startPlaylistScenarioId = id
      if (id === -1) {
        if (this.filteredPlaylist.length) {
          startPlaylistScenarioId = this.filteredPlaylist[0].id
        }
      }
      this.$store.commit('scenario/updateState', { startPlaylistScenarioId })
    },
    isCurrentScenario (id) {
      if (this.currentScenario) {
        return this.currentScenario.id === id
      } else {
        return false
      }
    },
    removeIfCurrentScenario (ids) {
      if (this.currentScenario) {
        if (ids.includes(this.currentScenario.id)) {
          this.$store.commit('playlist/updateState', { prevPlayingScenario: this.currentScenario })
          this.$store.commit('scenario/updateState', { currentScenario: null })
          this.stop()
        }
      }
    },
    async handleItemClick (item, index, e) {
      const { ctrlKey, metaKey, shiftKey } = e
      if (ctrlKey || metaKey || shiftKey) {
        return
      }
      await this.resetScenario(item)
      this.setStartPlaylistScenarioId(item.id)
    },
    async handleRemoveSelected () {
      const ids = _.map(this.selected, (item) => {
        return item.id
      })
      await this.removeScenariosFromPlaylist(ids)
      this.removeIfCurrentScenario(ids)
      if (ids.includes(this.startPlaylistScenarioId)) {
        this.setStartPlaylistScenarioId(-1)
      }
    },
    async handleRemove (id) {
      this.updateScenario({ loading: true })
      const ids = [id]
      await new Promise((resolve) => {
        setTimeout(() => {
          resolve()
        }, 200)
      }) // 强行添加一个loading画面（不加入此行没有loading画面）
      await this.removeScenariosFromPlaylist(ids)
      this.removeIfCurrentScenario(ids) // 在播放场景后执行此步时该函数内会执行updateScenario({ loading: false })。播放场景后执行此步时程序会卡住。
      if (ids.includes(this.startPlaylistScenarioId)) {
        this.setStartPlaylistScenarioId(-1)
      }
      this.updateScenario({ loading: false })
    },
    handleMoveUp (index, item) {
      this.moveUp({ index, item })
      this.updatePlaylist(this.filteredPlayIdsList)
    },
    handleMoveDown (index, item) {
      this.moveDown({ index, item })
      this.updatePlaylist(this.filteredPlayIdsList)
    },
    handleAdd () {
      this.scenarioAddVisible = true
    },
    handleScenarioAddConfirm (payload) {
      const list = [
        ...payload,
        ...this.playIdsList,
      ]
      this.updatePlaylist(list)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .playlist {
    height: 100%;
    display: flex;
    flex-direction: column;
    font-size: 12px;
    user-select: none;

    &.disabled {
      pointer-events: none;
      background-color: #000;
      opacity: 0.5;
    }

    .icon {
      cursor: pointer;

      &.disabled {
        opacity: 0.3;
      }
    }

    .search-keyword {
      padding: 16px 20px;
    }

    .playlist-process {
      margin-bottom: 16px;
      padding: 0 20px;
      font-size: 12px;

      &:hover {
        color: @active-font-color;
      }

      span {
        margin-left: 10px;
      }
    }

    .playlist-list {
      flex: 1;
      overflow-y: auto;

      .playlist-item {
        display: flex;
        justify-content: space-between;
        height: 27px;
        line-height: 28px;
        padding: 0 20px;
        border-bottom: 1px solid @list-bd;
        background-color: @darker-bg;
        cursor: pointer;

        .icon {
          margin: 0 2px;
        }

        &.playlist-item-active,
        &:hover {
          color: @active-font-color;
          background-color: @dark-bg;
        }

        .playlist-item-name {
          flex: 1;
          .text-overflow;
        }

        .playlist-item-operation {
          display: inline-block;
          width: 66px;
          padding-left: 10px;
          text-align: right;
          font-size: 12px;
        }
      }

      .list-multiple-select .list-multiple-select-item-active .playlist-item {
        color: @active-font-color;
        background-color: @dark-bg;
      }
    }

    .playlist-no-data {
      flex: 1;
      text-align: center;

      .playlist-no-data-wrap {
        display: inline-block;
        width: 80px;
        height: 80px;
        margin-top: 80px;
        text-align: center;
        color: @disabled-color;
        border: 1px dotted #404040;
        cursor: pointer;

        .icon {
          display: block;
          margin: 20px auto 0;;
          font-size: 20px;
        }

        .playlist-no-data-text {
          display: inline-block;
          margin-top: 6px;
          font-size: 12px;
        }
      }
    }

    .playlist-operations {
      height: 28px;
      line-height: 28px;
      padding: 0 10px;
      color: @global-font-color;
      background-color: @darker-bg;
      text-align: right;
    }
  }
</style>
