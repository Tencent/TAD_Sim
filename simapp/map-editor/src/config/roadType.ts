import i18n from '@/locales'
import { ImgUrls } from '@/utils/urls'

export const RoadTypeConfig = [
  {
    name: 'default', // 普通道路（双向双车道）
    showName: i18n.global.t('desc.editRoad.commonRoad'),
    iconUrl: ImgUrls.other.commonRoadIcon,
  },
  {
    name: 'ramp', // 匝道
    showName: i18n.global.t('desc.editRoad.ramp'),
    iconUrl: ImgUrls.other.rampIcon,
  },
  {
    name: 'tunnel', // 隧道
    showName: i18n.global.t('desc.editRoad.tunnel'),
    iconUrl: ImgUrls.other.tunnelIcon,
  },
]
