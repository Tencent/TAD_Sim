import { dropElements } from '@/utils/tools'

export const MyDropConfig = {
  mounted (el: HTMLElement) {
    // 注册可触发放置的容器元素
    dropElements.add(el)
  },
  unmounted (el: HTMLElement) {
    dropElements.delete(el)
  },
}
