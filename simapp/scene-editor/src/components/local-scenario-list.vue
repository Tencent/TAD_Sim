<template>
  <div class="local-scenario-list">
    <div class="search-input">
      <el-input
        v-model="keyword"
        style="width: 200px;"
        :placeholder="$t('operation.searchName')"
      >
        <template #prefix>
          <el-icon class="el-input__icon">
            <Search />
          </el-icon>
        </template>
      </el-input>
    </div>
    <el-collapse v-model="activeCollapseNames" class="scenario-set-list">
      <el-collapse-item
        v-for="scenarioSet of filteredComputedScenarioSetList"
        :key="scenarioSet.id"
        :name="scenarioSet.id"
        class="scenario-set-list-item"
      >
        <template #title>
          <div :id="`scenario-set-${scenarioSet.name}`" class="scenario-set-list-item-name" :title="scenarioSet.name">
            {{ scenarioSet.name }}
          </div>
        </template>
        <ul class="scenario-list">
          <li
            v-for="item of scenarioSet.scenes"
            :id="`scenario-node-${scenarioSet.id}-${item.id}`"
            :key="`${scenarioSet.id}-${item.id}`"
            class="scenario-item"
            :class="{ 'current-scenario-item': isCurrentScenario(scenarioSet.id, item.id) }"
            @click="handleOpenScenario(scenarioSet.id, item)"
          >
            <div :id="`scenario-${scenarioSet.name}-${item.name}`" class="scenario-item-content">
              <div class="scenario-item-name" :title="item.filename">
                {{ item.filename }}
              </div>
              <div class="scenario-item-operation">
                <open-svg />
              </div>
            </div>
          </li>
        </ul>
      </el-collapse-item>
    </el-collapse>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import { ElMessage } from 'element-plus'

export default {
  name: 'LocalScenarioList',
  data () {
    return {
      keyword: '',
      activeCollapseNames: [],
      currentScenarioSetId: 0,
    }
  },
  computed: {
    ...mapState('scenario', [
      'currentScenario',
      'scenarioList',
    ]),
    ...mapState('scenario-set', [
      'scenarioSetList',
    ]),
    computedScenarioSetList () {
      return _.map(this.scenarioSetList, (scenarioSet) => {
        const scenes = []
        _.forEach(scenarioSet.scenes, (id) => {
          const matched = _.find(this.scenarioList, { id })
          if (matched) {
            scenes.push(matched)
          }
        })
        scenes.sort((a, b) => {
          return a.name.localeCompare(b.name)
        })
        return {
          ...scenarioSet,
          scenes,
        }
      })
    },
    filteredComputedScenarioSetList () {
      const list = []
      _.forEach(this.computedScenarioSetList, (computedScenarioSet) => {
        const scenes = _.filter(computedScenarioSet.scenes, (scene) => {
          return scene.filename.includes(this.keyword)
        })
        let showItemWhenListEmpty = true
        if (this.keyword && _.size(scenes) === 0) {
          showItemWhenListEmpty = false
        }
        if (showItemWhenListEmpty) {
          list.push({
            ...computedScenarioSet,
            scenes,
          })
        }
      })
      return list
    },
  },
  watch: {
    currentScenario (value) {
      if (value) {
        const el = document.getElementById(`scenario-node-${this.currentScenarioSetId}-${value.id}`)
        if (el) {
          el.scrollIntoView()
        }
      }
    },
  },
  methods: {
    ...mapActions('scenario', [
      'openScenario',
    ]),
    isCurrentScenario (scenarioSetId, id) {
      if (this.currentScenario) {
        return this.currentScenarioSetId === scenarioSetId && this.currentScenario.id === id
      } else {
        return false
      }
    },
    handleOpenScenario (scenarioSetId, item) {
      if (item && item.traffictype === 'simrec') {
        ElMessage.warning('不能打开simlog场景')
        return
      }
      this.currentScenarioSetId = scenarioSetId
      this.openScenario(item)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .local-scenario-list {
    flex: 1;
    display: flex;
    flex-direction: column;
    min-height: 1px;
    position: relative;
  }

  .search-input {
    padding: 16px 20px;

    .scenario-search {
      width: 100%;
    }
  }
  .scenario-set-list {
    flex: 1;
    padding-bottom: 5px;
    overflow-y: auto;

    .scenario-set-list-item {
      .scenario-set-list-item-name {
        text-align: left;
        color: @title-font-color;
        font-weight: bold;
        width: 180px;
        .text-overflow();
      }
    }
  }

  .scenario-list {
    flex: 1;
    overflow: auto;

    .scenario-item {
      height: 28px;
      padding-left: 20px;
      background-color: @darker-bg;

      .scenario-item-operation {
        visibility: hidden;
      }

      &.current-scenario-item, &:hover {
        color: @active-font-color;
        background-color: @dark-bg;

        .scenario-item-content .scenario-item-operation {
          visibility: visible;
        }
      }

      .scenario-item-content {
        height: 27px;
        border-bottom: 1px solid @list-bd;
        display: flex;
        justify-content: space-between;
        align-items: center;
        cursor: pointer;
        font-size: 12px;

        .scenario-item-name {
          color: @title-font-color;
          flex: 1;
          .text-overflow;
        }

        .scenario-item-operation {
          padding-left: 10px;
          padding-right: 20px;
          font-size: 12px;
        }
      }
    }
  }
</style>
