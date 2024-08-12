<template>
  <div class="planner-selector">
    <div v-if="planner" class="planner-selector-icon">
      <div class="planner-selector-avatar">
        <img :src="$getModelIcon(planner, true)" alt="">
      </div>
      <div class="planner-selector-name" :title="$getModelName(planner)">
        {{ $getModelName(planner) }}
      </div>
    </div>
    <div v-else class="planner-selector-icon">
      <p class="selector-tip">
        请选择主车
      </p>
    </div>
    <el-popover
      v-model="popoverVisible"
      class="planner-selector-pop"
      trigger="click"
      placement="top-end"
      :visible-arrow="false"
      popper-class="planner-selector-pop"
      width="auto"
    >
      <template #reference>
        <div class="planner-selector-trigger">
          <arrow-svg />
        </div>
      </template>
      <div class="search-input">
        <el-input
          v-model="keyword"
          style="width: 200px;"
          prefix-icon="search"
          :placeholder="$t('operation.searchName')"
        />
      </div>
      <ul class="planner-selector-list">
        <li
          v-for="(p, index) of list"
          :key="index"
          class="planner-sensor-item"
          @click="selectPlanner(p)"
        >
          <div class="planner-sensor-avatar">
            <img :src="$getModelIcon(p, true)" alt="">
          </div>
          <div class="planner-sensor-name">
            {{ $getModelName(p) }}
          </div>
        </li>
      </ul>
    </el-popover>
  </div>
</template>

<script>
import { mapState } from 'vuex'

export default {
  name: 'PlannerSelector',
  props: {
    name: {
      type: String,
      required: true,
    },
  },
  data () {
    return {
      keyword: '',
      popoverVisible: false,
    }
  },
  computed: {
    ...mapState('planners', ['plannerList']),
    planner () {
      if (this.name) {
        return this.plannerList.find(p => p.variable === this.name)
      }
      return null
    },
    list () {
      return this.plannerList.filter(p => (
        this.$getModelName(p).toLowerCase().includes(this.keyword.toLowerCase().trim())
      ))
    },
  },
  methods: {
    selectPlanner (planner) {
      this.$emit('change', planner)
      this.popoverVisible = false
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins.less";

.planner-selector {
  width: 80px;
  height: 80px;
  position: relative;
  z-index: 1;
  display: flex;

  .planner-selector-icon {
    flex: 1;
    width: 62px;
    background-color: #0F0F0F;
    cursor: default;

    .planner-selector-avatar {
      width: 62px;
      height: 62px;

      img {
        width: 100%;
        height: 100%;
      }
    }

    .planner-selector-name {
      .text-overflow;
      text-align: center;
      box-sizing: border-box;
      width: 100%;
      padding: 0 4px;
      font-size: 14px;
      height: 16px;
      line-height: 16px;
      color: #B9B9B9;
    }

    .selector-tip {
      text-align: center;
      padding-top: 20px;
      font-size: 14px;
    }
  }

  .planner-selector-pop {
    flex: 0 0 16px;
  }

  .planner-selector-trigger {
    height: 100%;
    background-color: #242424;
    color: #D8D8D8;
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
  }
}
</style>

<style lang="less">
@import "@/assets/less/mixins.less";

.planner-selector-pop {
  .search-input {
    padding: 16px 20px;

    .scenario-search {
      width: 100%;
    }
  }

  .planner-selector-list {
    margin-top: 10px;
    padding-top: 5px;
    max-height: 250px;
    overflow: auto;
    display: grid;
    grid-template-columns: repeat(2, 78px);
    justify-content: space-evenly;

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
        }

        .planner-sensor-item-btns {
          display: flex;
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
        .text-overflow;
        color: @global-font-color;
        font-size: 12px;
        text-align: center;
        line-height: 16px;
      }
    }
  }
}
</style>
