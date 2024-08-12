import TriangleDown from '@/assets/images/triangle-down.svg'
import MoveUpSvg from '@/assets/images/move-up.svg'
import MoveDownSvg from '@/assets/images/move-down.svg'
import PlusSvg from '@/assets/images/plus.svg'
import RemoveSvg from '@/assets/images/remove.svg'
import LocateSvg from '@/assets/images/locate.svg'
import SeparatorSvg from '@/assets/images/separator.svg'
import CheckedSvg from '@/assets/images/checked.svg'
import CompareSvg from '@/assets/images/compare.svg'

// 快捷栏用图标
import OpenSvg from '@/assets/images/shortcut-btns/open.svg'
import NewSvg from '@/assets/images/shortcut-btns/new.svg'
import SaveSvg from '@/assets/images/shortcut-btns/save.svg'
import MoveSvg from '@/assets/images/shortcut-btns/move.svg'
import RotateSvg from '@/assets/images/shortcut-btns/rotate.svg'
import RulerSvg from '@/assets/images/shortcut-btns/ruler.svg'
import PlannerConfigSvg from '@/assets/images/shortcut-btns/planner-config.svg'
import BackToMainSvg from '@/assets/images/shortcut-btns/back-to-main.svg'
import TrafficSvg from '@/assets/images/shortcut-btns/traffic.svg'
import WatchOverSvg from '@/assets/images/shortcut-btns/watch-over.svg'
import ResizeZoomSvg from '@/assets/images/shortcut-btns/resize/zoom.svg'
import ResizeNarrowSvg from '@/assets/images/shortcut-btns/resize/narrow.svg'
import SetDestinationSvg from '@/assets/images/shortcut-btns/set-destination.svg'
import SignlightSvg from '@/assets/images/shortcut-btns/signlight.svg'

import SensorSvg from '@/assets/images/shortcut-btns/sensor.svg'
import EnvironmentSvg from '@/assets/images/shortcut-btns/environment.svg'
import MapEditorSvg from '@/assets/images/shortcut-btns/map-editor.svg'
import RenderingPlaySvg from '@/assets/images/shortcut-btns/rendering-play.svg'
import PlaySvg from '@/assets/images/shortcut-btns/play.svg'
import ScenarioEditorSvg from '@/assets/images/shortcut-btns/scenario-editor.svg'
import ScenarioGeneratorSvg from '@/assets/images/shortcut-btns/scenario-generator.svg'

// player-controls 的图标
import PlayerControlsPlaySvg from '@/assets/images/player-controls/play.svg'
import PlayerControlsStopSvg from '@/assets/images/player-controls/stop.svg'
import PlayerControlsPauseSvg from '@/assets/images/player-controls/pause.svg'
import PlayerControlsNextSvg from '@/assets/images/player-controls/next.svg'
import PlayerControlsNextFrameSvg from '@/assets/images/player-controls/next-frame.svg'
import PlayerControlsPlayModeSingleSvg from '@/assets/images/player-controls/play-mode-single.svg'
import PlayerControlsPlayModeListSvg from '@/assets/images/player-controls/play-mode-list.svg'

// 一些通用 icons / common icons
import ArrowSvg from '@/assets/images/arrow.svg'

const allIcons = {
  TriangleDown,
  MoveUpSvg,
  MoveDownSvg,
  PlusSvg,
  RemoveSvg,
  LocateSvg,
  RotateSvg,
  SeparatorSvg,
  CheckedSvg,
  CompareSvg,
  OpenSvg,
  NewSvg,
  SaveSvg,
  MoveSvg,
  SensorSvg,
  RulerSvg,
  PlannerConfigSvg,
  BackToMainSvg,
  TrafficSvg,
  WatchOverSvg,
  SetDestinationSvg,
  ResizeZoomSvg,
  ResizeNarrowSvg,
  EnvironmentSvg,
  MapEditorSvg,
  RenderingPlaySvg,
  PlaySvg, // 播放按钮
  ScenarioEditorSvg, // 场景编辑器
  ScenarioGeneratorSvg,
  PlayerControlsPlaySvg,
  ArrowSvg,
  PlayerControlsStopSvg,
  PlayerControlsPauseSvg,
  SignlightSvg,
  PlayerControlsNextSvg,
  PlayerControlsNextFrameSvg,
  PlayerControlsPlayModeSingleSvg,
  PlayerControlsPlayModeListSvg,
}

/**
 * 将所有 SVG 图标注册为全局组件
 * @param {object} app - Vue 应用实例
 */
export default {
  install (app) {
    // 遍历所有图标，将它们注册为全局组件
    Object.keys(allIcons).forEach((key) => {
      app.component(key, allIcons[key])
    })
  },
}
