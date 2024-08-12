<template>
  <ul class="signal-control-phase-list">
    <div class="list-header">
      <div class="scale-list">
        <div
          v-for="time in scaleList"
          :key="time"
          class="item"
          :style="{ left: `${Math.floor(100 / totalTime * time)}%` }"
        >
          <span>{{ time }}s</span>
        </div>
      </div>
      <BaseAuth :perm="['action.signlightControl.relativePhysicalLight.enable']">
        <div class="signlights">
          {{ $t('scenario.physicsLight') }}
        </div>
      </BaseAuth>
    </div>
    <div
      v-for="(phase, index) in planJunctionPhaseList"
      :key="index"
      :ref="`phase-${phase.id}`"
      class="phase"
      :class="[{ active: currentPhaseId === phase.id, disabled: (disabled || !phase.enabled) }]"
      @click="handleClickItem(phase)"
    >
      <div class="phase-label">
        <span>{{ phase.name }}</span>
      </div>
      <el-switch
        :model-value="phase.enabled"
        :disabled="disabled"
        class="phase-switch custom-switch"
        @change="changeValue(phase, 'enabled', $event)"
      />
      <ColorSlider
        ref="colorSlider"
        :data="phase.colors"
        :total-value="currentJunction.cycleTime"
        :active-index="currentPhaseId === phase.id ? currentLightStateId : -1"
        :disabled="disabled"
        @click-item="handleClickLightState(phase, $event)"
        @change="colorSliderChange(phase, $event)"
      />
      <BaseAuth :perm="['action.signlightControl.relativePhysicalLight.enable']">
        <JunctionSignlightsSelect
          :value="phase.signalHead"
          :disabled="disabled"
          @changed="handleLightChange(phase, $event)"
        />
      </BaseAuth>
    </div>
  </ul>
</template>

<script>
import { mapActions, mapGetters, mapState } from 'vuex'
import JunctionSignlightsSelect from './junction-signlights-select.vue'
import ColorSlider from '@/components/color-slider.vue'

export default {
  name: 'SignalControlPhaseList',
  components: {
    ColorSlider,
    JunctionSignlightsSelect,
  },
  props: {
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  computed: {
    ...mapState('signalControl', [
      'currentPhaseId',
      'currentLightStateId',
    ]),
    ...mapGetters('signalControl', [
      'currentJunction',
      'planJunctionPhaseList',
    ]),
    totalTime () {
      return this.currentJunction?.cycleTime || 10
    },
    scaleList () {
      const divide = this.totalTime < 50 ? 10 : (this.totalTime / 4 >> 0)
      const chunk = (this.totalTime / divide >> 0)
      let list = Array.from({ length: chunk + 1 }).fill().map((_, i) => divide * i)
      if (this.totalTime % divide < 5) list = list.slice(0, -1)
      list = list.concat([this.totalTime])
      return list
    },
  },
  methods: {
    ...mapActions('signalControl', [
      'selectPhase',
      'changePhaseValue',
      'selectLightState',
    ]),
    handleClickItem (item) {
      this.selectPhase(item.id)
    },
    changeValue (item, key, value) {
      this.changePhaseValue({ id: item.id, key, value })
    },
    handleClickLightState (item, index) {
      this.selectPhase(item.id)
      this.selectLightState(index)
    },
    // 灯态的拖拉变化
    colorSliderChange (item, newColors) {
      const colors = item.colors
      colors.splice(0)
      colors.push(...newColors)
      this.changePhaseValue({ id: item.id, key: 'colors', value: colors })
    },
    // 物理灯选择变化
    handleLightChange (item, signalHead) {
      this.changePhaseValue({ id: item.id, key: 'signalHead', value: signalHead })
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.signal-control-phase-list {
  padding: 4px 0 4px 10px;

  .list-header {
    display: flex;
    align-items: center;
    font-size: 10px;
    color: #595959;
    text-align: center;
    word-break: keep-all;
    padding-bottom: 8px;

    & > .scale-list {
      margin-left: 56px + 40px + (10px * 2);
      margin-right: 20px;
      flex: 1;
    }

    & > .signlights {
      width: 6em;
      margin-right: 10px;
      flex-shrink: 0;
    }
  }

  .scale-list {
    position: relative;
    display: flex;
    align-items: center;

    & > .item {
      position: absolute;

      &::after {
        content: "";
        position: absolute;
        bottom: -8px;
        left: 50%;
        width: 1px;
        height: 4px;
        background: #595959;
      }
    }
  }

  .phase {
    display: flex;
    align-items: center;
    padding: 8px 10px 8px 6px;
    gap: 10px;

    + .phase {
      border-top: 1px solid @border-color;
    }

    &.active {
      background-color: @hover-bg;
    }

    &.disabled {
      opacity: 0.7;
    }
  }

  .phase-label {
    width: 4em;
    .text-overflow();
  }

  .color-slider {
    flex: 1;
  }
}
</style>
