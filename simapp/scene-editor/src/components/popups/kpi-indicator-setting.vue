<template>
  <div v-loading="loading" class="kpi-indicator-setting">
    <section>
      <NumericSliderInput
        v-model="form.timeDistance"
        class="kpi-time-distance-input"
        :label="$t('tips.timeIntervalLessThan')"
        unit="s/V"
      />
    </section>
    <section>
      <NumericRangeSliderInput
        v-model="form.accelerationRange"
        :label="$t('acceleration')"
        unit="m/s²"
        :min="-30"
        :max="30"
        :input-width="80"
      />
    </section>
    <section style="margin-top: 10px;">
      <NumericRangeSliderInput
        v-model.lazy="form.speedRange"
        :label="$t('scenario.speed')"
        unit="m/s"
        :min="0"
        :max="240"
        :input-width="80"
      />
    </section>
    <section class="part-checkboxes">
      <el-checkbox v-model="form.collision" class="check-item" name="collision">
        {{ $t('collision') }}
      </el-checkbox>
      <el-checkbox v-model="form.runLight" class="check-item" name="runLight">
        {{ $t('indicator.runRedLight') }}
      </el-checkbox>
      <el-checkbox v-model="form.crossStopLine" class="check-item" name="crossStopLine">
        {{ $t('indicator.overParkingLine') }}
      </el-checkbox>
      <el-checkbox v-model="form.crossSolidLine" class="check-item" name="crossSolidLine">
        {{ $t('indicator.overSolidLine') }}
      </el-checkbox>
    </section>
    <section class="part-buttons">
      <el-button class="dialog-ok" @click="handleStart">
        {{ $t('operation.run') }}
      </el-button>
      <el-button class="dialog-cancel" style="margin: 0 15px;" @click="$emit('close')">
        {{ $t('operation.close') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { mapActions, mapState } from 'vuex'
import NumericSliderInput from '@/components/numeric-slider-input.vue'
import NumericRangeSliderInput from '@/components/numeric-range-slider-input.vue'

export default {
  name: 'KpiIndicatorSetting',
  components: {
    NumericRangeSliderInput,
    NumericSliderInput,
  },
  data () {
    return {
      loading: false,
      form: {
        timeDistance: 50,
        accelerationRange: [0, 10],
        speedRange: [0, 120],
        collision: true,
        runLight: true,
        crossStopLine: true,
        crossSolidLine: true,
      },
    }
  },
  computed: {
    ...mapState('system', [
      'kpi',
    ]),
  },
  created () {
    Object.assign(this.form, this.kpi)
  },
  methods: {
    ...mapActions('system', [
      'saveKpi',
    ]),
    async handleStart () {
      this.loading = true
      try {
        await this.saveKpi(this.form)
        this.$emit('close')
      } catch (e) {
        this.$errorBox(e.message)
      }
      this.loading = false
    },
  },
}
</script>

<style scoped lang="less">
  .kpi-indicator-setting {
    width: 260px;
    height: auto;
    padding: 20px;

    .kpi-time-distance-input {
      display: block;
    }

    .part-checkboxes {
      padding-top: 10px;
      height: 60px;
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;

      .check-item {
        margin-right: 0;
        flex: 0 0 80px;
      }
    }

    .part-buttons {
      direction: rtl;
      margin-top: 20px;
    }
  }
</style>
