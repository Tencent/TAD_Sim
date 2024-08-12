<template>
  <div class="environment-group">
    <ul class="env-list">
      <li
        v-for="(env, index) of group"
        :key="index"
        class="env-li"
        :class="{ 'active-env': index === currentIndex }"
        @click="changeCurrentIndex(index)"
      >
        <span class="env-name">{{ $t('scenario.timeStamp') }}: {{ env.TimeStamp }}</span>
        <el-icon v-if="index > 0" class="el-icon el-icon-minus" @click.stop="removeTimeStamp(index)">
          <Minus />
        </el-icon>
      </li>
      <li key="add" class="env-li add-timestamp" @click="addEnvGroup(type)">
        <el-icon class="el-icon el-icon-plus">
          <Plus />
        </el-icon>
      </li>
    </ul>
    <EnvironmentForm
      ref="envForm"
      :data="group[currentIndex]"
      :time-stamps="timeStamps"
      :index="currentIndex"
      @change="updateEnv({ type, index: currentIndex, data: $event })"
    />
  </div>
</template>

<script>
import { mapMutations } from 'vuex'
import EnvironmentForm from './environment-form.vue'

export default {
  name: 'EnvironmentConfig',
  components: {
    EnvironmentForm,
  },
  props: {
    type: {
      type: String,
      required: true,
    },
  },
  data () {
    return {
      cIndex: 0,
    }
  },
  computed: {
    group () {
      return this.$store.state.environment[this.type]
    },
    timeStamps () {
      return this.group.map(env => env.TimeStamp)
    },
    currentIndex: {
      get () {
        if (this.cIndex >= this.group.length) {
          return this.group.length - 1
        }
        return this.cIndex
      },
      set (val) {
        this.cIndex = val
      },
    },
  },
  async created () {

  },
  methods: {
    ...mapMutations('environment', [
      'updateEnv',
      'addEnv',
      'removeEnv',
    ]),
    removeTimeStamp (index) {
      if (index === 0) return
      this.$refs.envForm.$refs.form.validate((valid) => {
        if (valid) {
          if (this.currentIndex >= index) {
            this.currentIndex -= 1
          }
          const { type } = this
          this.removeEnv({ type, index })
        }
      })
    },
    changeCurrentIndex (index) {
      this.$refs.envForm.$refs.form.validate((valid) => {
        if (valid) this.currentIndex = index
      })
    },
    addEnvGroup (type) {
      this.$refs.envForm.$refs.form.validate((valid) => {
        if (valid) {
          this.addEnv(type)
          this.currentIndex = this.group.length - 1
        }
      })
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .environment-group {
    display: flex;
    height: 400px;

    .env-list {
      box-sizing: border-box;
      flex: 0 0 200px;
      border-right: 1px solid @darker-bg;
      overflow: auto;

      .env-li {
        height: 32px;
        padding: 0 20px;
        display: flex;
        align-items: center;
        justify-content: space-between;
        border-bottom: 1px solid @list-bd;
        cursor: default;

        .env-name {
          color: @global-font-color;
        }

        &.active-env {
          background-color: @hover-bg;
        }

        .el-icon {
          cursor: pointer;
        }
      }

      .add-timestamp {
        cursor: pointer;
        justify-content: center;
      }
    }
  }
</style>
