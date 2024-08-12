<template>
  <div class="numeric-input-proportion">
    <SimProportion
      class="proportion-row"
      :method="method"
      :step="step"
      :items="items"
    />
    <div class="input-row">
      <LabelNumericInput
        v-for="(item, index) in items"
        :key="item.id"
        :value="item.weight"
        :label="$t(item.name)"
        unit="%"
        class="row-item"
        @change="onItemChange($event, item, index)"
      />
    </div>
  </div>
</template>

<script>
import Big from 'big.js'
import LabelNumericInput from './label-numeric-input.vue'
import SimProportion from './sim-proportion.vue'

export default {
  name: 'NumericInputProportion',
  components: { LabelNumericInput, SimProportion },
  props: {
    method: {
      type: String,
      default: 'next',
      validator (val) {
        return val === 'next' || val === 'last'
      },
    },
    step: {
      type: Number,
      default: 1,
    },
    items: {
      type: Array,
      default: () => [],
    },
  },
  methods: {
    onItemChange (newVal, item, index) {
      const items = this.items
      const isNext = this.method === 'next'
      const rightItem = index === items.length - 1 ? items[0] : (isNext ? items[index + 1] : items[items.length - 1])
      let newValue = new Big(newVal)
      if (newValue.lt(0)) {
        newValue = new Big(0)
      }
      const maxWeight = (new Big(item.weight)).add(rightItem.weight)
      if (newValue.gt(maxWeight)) {
        newValue = maxWeight
      }
      const distanceWeight = newValue.minus(item.weight)
      item.weight = +newValue
      rightItem.weight = +new Big(rightItem.weight).minus(distanceWeight)
    },
  },
}
</script>

<style scoped lang="less">
  .numeric-input-proportion {
    .proportion-row {

    }

    .input-row {
      margin-top: 20px;
      display: flex;
      align-items: center;
      justify-content: space-around;

      .row-item {
        flex: auto;
        margin: 0 10px;
      }
    }
  }
</style>
