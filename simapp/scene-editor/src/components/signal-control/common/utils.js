/**
 * 获取各灯态时长
 * @param {object} signlight - 包含信号灯状态的对象
 * @returns {object} 包含各灯态时长的对象
 */
export function getSignlightColorData (signlight) {
  return {
    startTime: +signlight.startTime || 1,
    green: +signlight.timeGreen || 0,
    yellow: +signlight.timeYellow || 0,
    red: +signlight.timeRed || 0,
  }
}

/**
 * 判断信号灯是否有颜色
 * @param {object} signlight - 包含信号灯状态的对象
 * @returns {boolean} 如果有颜色，返回 true，否则返回 false
 */
export function hasColor (signlight) {
  return +signlight.timeGreen > 0 || +signlight.timeYellow > 0 || +signlight.timeRed > 0
}

/**
 * 获取信号灯的第一个颜色
 * @param {object} colorData - 包含各灯态时长的对象
 * @returns {object} 包含第一个颜色及其时长的对象
 */
export function getFirstColor (colorData) {
  const { green, yellow, red } = colorData
  const firstColor = green > 0 ? 'green' : (yellow > 0 ? 'yellow' : (red > 0 ? 'red' : 'gray'))
  return { color: firstColor, value: colorData[firstColor] }
}

/**
 * 获取信号灯的最后一个颜色
 * @param {object} colorData - 包含各灯态时长的对象
 * @returns {object} 包含最后一个颜色及其时长的对象
 */
export function getLastColor (colorData) {
  const { green, yellow, red } = colorData
  const lastColor = red > 0 ? 'red' : (yellow > 0 ? 'yellow' : (green > 0 ? 'green' : 'gray'))
  return { color: lastColor, value: colorData[lastColor] }
}

/**
 * 将灯态时长转为灯态数组
 * @param {object} colorData - 包含各灯态时长的对象
 * @param {number} totalTime - 信号灯总时长
 * @returns {Array} 灯态数组
 */
export function formatColorsToArray (colorData, totalTime) {
  const { startTime, green, yellow, red } = colorData
  const total = green + yellow + red
  let colors = []

  if (total < 1) return []

  if (green > 0) colors.push({ color: 'green', value: green })
  if (yellow > 0) colors.push({ color: 'yellow', value: yellow })
  if (red > 0) colors.push({ color: 'red', value: red })

  if (startTime > 1) {
    const reallyStartTime = total - startTime + 1 // 8
    const nextTotalTime = total - reallyStartTime // 10
    const prevColors = []
    const nextColors = []
    colors.reduce((count, item) => {
      if (count + item.value < nextTotalTime) {
        nextColors.push(item)
      } else if (count > nextTotalTime) {
        prevColors.push(item)
      } else {
        nextColors.push({ color: item.color, value: nextTotalTime - count })
        prevColors.push({ color: item.color, value: count + item.value - nextTotalTime })
      }
      return count + item.value
    }, 0)
    colors = prevColors.concat(nextColors)
  }

  return colors
}

/**
 * 将灯态数组转为灯态时长
 * @param {Array} colors - 灯态数组
 * @returns {object} 包含各灯态时长的对象
 */
export function formatArrayToColors (colors) {
  const colorData = {
    startTime: 1,
    green: 0,
    yellow: 0,
    red: 0,
  }
  if (colors.length < 1) return colorData
  // 获取各灯态时长
  colors.forEach((item) => {
    colorData[item.color] += item.value
  })
  // 计算开始时间
  const firstColor = getFirstColor(colorData)
  if (firstColor.color !== 'gray') {
    const firstColorIndex = colors.findIndex(item => item.color === firstColor.color)
    if (firstColorIndex > 0) {
      const prevColors = colors.slice(0, firstColorIndex)
      const { green, yellow, red } = colorData
      const total = green + yellow + red
      const offset = prevColors.reduce((re, e) => re + e.value, 0)
      colorData.startTime = total - offset + 1
    }
  }
  return colorData
}

/**
 * 根据灯态时长获取某时刻的颜色
 * @param {object} colorData - 包含各灯态时长的对象
 * @param {number} time - 需要查询的时刻
 * @returns {string} 对应时刻的颜色
 */
export function getColorByTime (colorData, time) {
  const { startTime, green, yellow, red } = colorData
  const total = green + yellow + red
  const reallyStartTime = startTime === 1 ? 0 : (total - startTime + 1)
  let color = 'gray'
  if (time <= 0 && reallyStartTime <= 0) {
    color = getFirstColor(colorData).color
  } else if (time < reallyStartTime) {
    color = getLastColor(colorData).color
  } else if (time < reallyStartTime + green) {
    color = 'green'
  } else if (time < reallyStartTime + green + yellow) {
    color = 'yellow'
  } else {
    color = 'red'
  }
  return color
}
