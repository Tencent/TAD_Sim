<script setup lang="ts">
import { computed, reactive, watch } from 'vue'
import { getPermission } from '@/utils/permission'

/**
 * 其子元素是否有权限查看
 */

defineOptions({
  name: 'BaseAuth',
})
const props = withDefaults(defineProps<Props>(), {
  perm: () => ['', 'block'],
})
defineSlots<{
  default: (props: { permission: unknown, [key: string]: unknown }) => unknown
}>()
interface Props {
  perm: [string, 'block' | string]
}
// 处理入参
const perm = reactive(convertPermProps(props.perm))
watch(() => props.perm, () => Object.assign(perm, props.perm))

// 该权限对应数据
const permData = computed(() => getPermission(perm.configKey))

// 子级是否显示
const visible = computed(() => {
  if (perm.attrs === 'block') return permData.value
  return true
})

// 给子级的数据
const attrs = computed(() => ({
  permission: permData.value,
  [perm.attrs]: perm.attrs === 'disabled' ? !permData.value : permData.value,
}))

// 处理入参数据
function convertPermProps (perm: string[]) {
  const [configKey = '', attrs = 'block'] = perm || []
  return { configKey, attrs }
}
</script>

<template>
  <slot v-if="visible" name="default" v-bind="attrs" />
</template>
