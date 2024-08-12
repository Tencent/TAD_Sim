<template>
  <div class="list-multiple-select">
    <div
      v-for="(item, index) of list"
      :key="getForKey(item)"
      class="list-multiple-select-item"
      :class="{ 'list-multiple-select-item-active': isActive(item) }"
      @click="handleClick(item, index, $event)"
      @contextmenu="handleContextMenu(item, index, $event)"
    >
      <slot name="item" :item="item" :index="index" />
    </div>
  </div>
</template>

<script>
import { find, findIndex, forEach, slice } from 'lodash-es'

export default {
  name: 'ListMultipleSelect',
  props: {
    list: {
      type: Array,
      required: true,
    },
    itemKeys: {
      type: Array,
      required: true,
    },
    modelValue: {
      type: Array,
      required: false,
    },
    clickWithoutModifierKeys: {
      type: Boolean,
      default: true,
    },
    contextmenuClick: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      shiftKeyFirst: null,
    }
  },
  mounted () {
    window.addEventListener('click', this.reset)
  },
  beforeUnmount () {
    window.removeEventListener('click', this.reset)
  },
  methods: {
    getCondition (item) {
      const condition = {}
      forEach(this.itemKeys, (key) => {
        condition[key] = item[key]
      })
      return condition
    },
    getForKey (item) {
      const keys = []
      forEach(this.itemKeys, (key) => {
        keys.push(item[key])
      })
      return keys.join('.')
    },
    isActive (item) {
      const condition = this.getCondition(item)
      return find(this.modelValue, condition)
    },
    handleClick (item, index, e) {
      const { ctrlKey, metaKey, shiftKey } = e
      const itemCondition = this.getCondition(item)
      const itemIndexAtSelected = findIndex(this.modelValue, itemCondition)
      let selected = slice(this.modelValue)
      if (ctrlKey || metaKey) {
        this.shiftKeyFirst = item
        if (itemIndexAtSelected === -1) {
          selected.push(item)
        } else {
          selected.splice(itemIndexAtSelected, 1)
        }
      } else if (shiftKey) {
        const { length } = selected
        if (length === 0) {
          this.shiftKeyFirst = item
          selected.push(item)
        } else {
          if (this.shiftKeyFirst === null && length === 1) { // select from canvas
            this.shiftKeyFirst = selected[0]
          }
          const shiftKeyFirstCondition = this.getCondition(this.shiftKeyFirst)
          const shiftKeyFirstAtList = findIndex(this.list, shiftKeyFirstCondition)
          let startIndex = -1
          let endIndex = -1
          if (index === shiftKeyFirstAtList) {
            startIndex = index
            endIndex = index
          } else if (index < shiftKeyFirstAtList) {
            startIndex = index
            endIndex = shiftKeyFirstAtList
          } else if (index > shiftKeyFirstAtList) {
            startIndex = shiftKeyFirstAtList
            endIndex = index
          }
          selected = slice(this.list, startIndex, endIndex + 1)
        }
      } else {
        if (this.clickWithoutModifierKeys) {
          this.shiftKeyFirst = item
          selected = [
            item,
          ]
        }
      }
      this.$emit('update:model-value', selected)
      this.$emit('item-click', item, index, e)
    },
    handleContextMenu (item, index, e) {
      if (this.contextmenuClick) {
        this.handleClick(item, index, e)
      }
    },
    reset () {
      this.shiftKeyFirst = null
      if (this.clickWithoutModifierKeys) {
        if (this.modelValue?.length > 1) {
          this.$emit('update:model-value', [])
        }
      } else {
        this.$emit('update:model-value', [])
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.list-multiple-select {
  display: inline-block;
  width: 100%;

  .list-multiple-select-item {
    display: inline-block;
    width: 100%;
  }
}
</style>
