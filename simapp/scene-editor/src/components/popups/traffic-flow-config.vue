<template>
  <div class="traffic-flow-config">
    <template v-if="false">
      <section class="slider-input-row">
        <NumericSliderInput
          v-model="form.averageSpeed"
          class="control"
          :label="$t('scenario.averageSpeed')"
          unit="m/s"
        />
        <div class="control" />
      </section>
      <section class="slider-input-row">
        <NumericSliderInput
          v-model="form.trafficDensity"
          class="control"
          label="Traffic density"
          :max="1"
          :step="0.01"
        />
        <NumericSliderInput
          v-model="form.radicalDegree"
          class="control"
          label="Radical degree"
          :max="1"
          :step="0.01"
        />
      </section>
      <section class="car-types">
        <div style="margin-top: 20px;">
          <sim-label>{{ $t('scenario.vehicleType') }}</sim-label>
        </div>
        <div style="margin-top: 20px;">
          <NumericInputProportion
            style="height: 75px;"
            method="next"
            :items="form.carTypes"
          />
        </div>
      </section>
    </template>
    <section class="slider-input-row">
      <NumericSliderInput
        id="traffic-flow-radical-degree-slider"
        v-model="form.radicalDegree"
        class="control"
        :label="$t('scenario.radicalDegree')"
        :max="1"
        :step="0.01"
      />
    </section>
    <section class="button-group">
      <el-button id="traffic-flow-cancel" class="dialog-cancel" @click="$emit('close')">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button id="traffic-flow-confirm" class="dialog-ok" @click="handleOk">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { mapActions, mapState } from 'vuex'
import NumericSliderInput from '@/components/numeric-slider-input.vue'
import NumericInputProportion from '@/components/numeric-input-proportion.vue'
import { errorHandler } from '@/common/errorHandler'

export default {
  name: 'TrafficFlowConfig',
  components: {
    NumericInputProportion,
    NumericSliderInput,
  },
  data () {
    return {
      form: {
        averageSpeed: 15,
        trafficDensity: 0.5,
        radicalDegree: 0.5,
        carTypes: [
          {
            name: 'scenarioElements.Cars',
            weight: 40,
          },
          {
            name: 'scenarioElements.SUV',
            weight: 25,
          },
          {
            name: 'scenarioElements.smallTrucks',
            weight: 15,
          },
          {
            name: 'scenarioElements.Taxi',
            weight: 20,
          },
        ],
      },
    }
  },
  computed: {
    ...mapState('scenario', [
      'trafficAI',
    ]),
  },
  created () {
    Object.assign(this.form, this.trafficAI)
  },
  methods: {
    ...mapActions('scenario', ['saveTrafficAI']),
    async handleOk () {
      try {
        await this.saveTrafficAI(this.form)
        this.$emit('close')
      } catch (e) {
        errorHandler(e.message)
      }
    },
  },
}
</script>

<style scoped lang="less">
  .traffic-flow-config {
    padding: 20px;
    width: 550px;
    box-sizing: border-box;

    .slider-input-row {
      display: flex;
      align-items: center;
      justify-content: space-between;

      .control {
        // flex: 0 1 230px;
        width: 100%;
      }
    }

    .button-group {
      height: 40px;
      line-height: 40px;
      margin-top: 40px;
      text-align: right;
    }
  }
</style>
