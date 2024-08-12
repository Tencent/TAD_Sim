const { readFileSync } = require('fs')
const { getFilePath } = require('../common/util')

const packagePath = getFilePath('../../package.json')

const { argv } = process

const needUpgradeReg = /^--need-to-upgrade=(true|false)$/
const needUpgradeArg = argv.find(arg => needUpgradeReg.test(arg))
let needToUpgrade = 'false'
if (needUpgradeArg) {
  [, needToUpgrade] = needUpgradeReg.exec(needUpgradeArg)
}

/**
 * 获取版本号
 * @return {any|string}
 */
function getVersion () {
  const data = readFileSync(packagePath, 'utf-8')

  try {
    const { version } = JSON.parse(data)

    return version
  } catch (err) {
    console.error(err)
    return ''
  }
}

module.exports = {
  version: getVersion(),
  isNeedToUpgrade: JSON.parse(needToUpgrade),
}
