/**
 * 工具类，包含一些常用的方法
 */
class Utility {
  constructor () {
    this.z0 = 0
    this.z1 = 0
    this.generate = false
  }

  /**
   * 获取url参数
   * @param name
   * @param url
   * @return {string|null}
   */
  getParameterByName (name, url) {
    if (!url) url = window.location.href
    name = name.replace(/[[\]]/g, '\\$&')
    const regex = new RegExp(`[?&]${name}(=([^&#]*)|&|#|$)`)
    const results = regex.exec(url)
    if (!results) return null
    if (!results[2]) return ''
    return decodeURIComponent(results[2].replace(/\+/g, ' '))
  }

  /**
   * 拼接url
   * @param relative
   * @return {string}
   */
  static composeUrl (relative) {
    const baseUrl = window.location.origin.replace(/simulation/g, 'simulationserver')
    window.BASEURL = baseUrl
    const url = baseUrl + relative
    console.log(url)
    return url
  }

  /**
   * 将弧度转换为角度
   * @param hd
   * @return {number}
   * @constructor
   */
  static Hd2Jd (hd) {
    const HD = Number(hd)
    return HD / Math.PI * 180
  }

  /**
   * 将角度转换为弧度
   * @param jd
   * @return {number}
   * @constructor
   */
  static Jd2Hd (jd) {
    return jd / 180 * Math.PI
  }

  /**
   * 生成高斯噪声
   * @param mu
   * @param sigma
   * @return {*}
   */
  generateGaussianNoise (mu, sigma) {
    const epsilon = Number.EPSILON
    const twoPI = 2 * Math.PI

    this.generate = !this.generate

    if (!this.generate) {
      return this.z1 * sigma + mu
    }

    let u1, u2
    do {
      u1 = Math.random()
      u2 = Math.random()
    } while (u1 <= epsilon)

    this.z0 = Math.sqrt(-2.0 * Math.log(u1)) * Math.cos(twoPI * u2)
    this.z1 = Math.sqrt(-2.0 * Math.log(u1)) * Math.sin(twoPI * u2)
    return this.z0 * sigma + mu
  }

  /**
   * 生成uuid
   * @param len
   * @param radix
   * @return {string}
   */
  static uuid (len, radix) {
    const chars = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'.split('')
    const uuid = []
    let i
    radix = radix || chars.length

    if (len) {
      // Compact form
      for (i = 0; i < len; i++) uuid[i] = chars[0 | Math.random() * radix]
    } else {
      // rfc4122, version 4 form
      let r

      // rfc4122 requires these characters
      uuid[8] = uuid[13] = uuid[18] = uuid[23] = '-'
      uuid[14] = '4'

      // Fill in random data. At i==19 set the high bits of clock sequence as
      // per rfc4122, sec. 4.1.5
      for (i = 0; i < 36; i++) {
        if (!uuid[i]) {
          r = 0 | Math.random() * 16
          uuid[i] = chars[(i === 19) ? (r & 0x3) | 0x8 : r]
        }
      }
    }

    return uuid.join('')
  }

  /**
   * 判断是否趋近0
   * @param x
   * @return {boolean}
   */
  static equalToZero (x) {
    return Math.abs(x) < Number.EPSILON
  }

  /**
   * 获取新id
   * @param list
   * @return {number}
   */
  static getNewId (list) {
    let id = 0
    for (let i = 0; i < list.length; i++) {
      if (+list[i].id > id) {
        id = +list[i].id
      }
    }
    return id + 1
  }
}

export default Utility
