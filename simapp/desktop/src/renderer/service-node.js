const { constants } = require('os')
const path = require('path')
const fse = require('fs-extra')
const { noop, getFilePath } = require('../common/util')
const { UserDataPath } = require('../common/constants')
const pbDescription = require('./protoDefine.json')

process.dlopen(module, getFilePath('../../service/txsim-play-service.node'), constants.dlopen.RTLD_NOW | constants.dlopen.RTLD_DEEPBIND)

const service = module.exports

// 注入service方法
const system = {
  /**
   * 设置APP路径
   * @param appUserDataPath
   * @param appPath
   * @return {*}
   */
  setAppPath (appUserDataPath, appPath) {
    return service.setAppPath(appUserDataPath, appPath)
  },
  /**
   * 获取配置
   * @param callback
   * @return {*}
   */
  getConfig (callback = noop) {
    try {
      return service.getSystemConfigs(callback)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 设置配置
   * @param config
   * @param callback
   * @return {*}
   */
  setConfig (config, callback = noop) {
    try {
      return service.setSystemConfigs(callback, config)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 恢复默认配置
   * @param callback
   * @return {*}
   */
  restoreDefaultConfig (callback = noop) {
    try {
      return service.restoreDefaultConfigs(callback)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 获取图片base64
   * @param filePath
   * @param callback
   * @return {Promise<unknown>}
   */
  getImageBase64 (filePath, callback = noop) {
    return new Promise((resolve) => {
      const base64 = fse.readFileSync(filePath, { encoding: 'base64' })
      const result = { retCode: 1, data: base64 }
      callback && callback(result)
      resolve(result)
    })
  },
}

const modules = {
  /**
   * 创建模块
   * @param entity
   * @param callback
   * @return {*}
   */
  create (entity, callback = noop) {
    try {
      return service.addModuleConfig(callback, entity)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 更新模块
   * @param entity
   * @param callback
   * @return {*}
   */
  update (entity, callback = noop) {
    try {
      return service.updateModuleConfig(callback, entity)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 删除模块
   * @param name
   * @param callback
   * @return {*}
   */
  destroy (name, callback = noop) {
    try {
      return service.removeModuleConfig(callback, name)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 获取模块
   * @param name
   * @param callback
   * @return {*}
   */
  findOne (name, callback = noop) {
    try {
      return service.getModuleConfig(callback, name)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 获取所有模块
   * @param callback
   * @return {*}
   */
  findAll (callback = noop) {
    try {
      return service.getAllModuleConfigs(callback)
    } catch (err) {
      console.error(err)
    }
  },
  scheme: {
    /**
     * 创建模组
     * @param entity
     * @param callback
     * @return {*}
     */
    create (entity, callback = noop) {
      try {
        return service.addModuleScheme(callback, entity)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 更新模组
     * @param entity
     * @param callback
     * @return {*}
     */
    update (entity, callback = noop) {
      try {
        return service.updateModuleScheme(callback, entity)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 删除模组
     * @param id
     * @param callback
     * @return {*}
     */
    destroy (id, callback = noop) {
      try {
        return service.removeModuleScheme(callback, id)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 查找所有模组
     * @param callback
     * @return {*}
     */
    findAll (callback = noop) {
      try {
        return service.getAllModuleSchemes(callback)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 激活模组
     * @param id
     * @param callback
     * @return {*}
     */
    active (id, callback = noop) {
      try {
        return service.updateMultiEgoScheme(callback, id)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 导入模组
     * @param filePath
     * @param callback
     * @return {*}
     */
    importScheme (filePath, callback = noop) {
      return service.importModuleScheme(callback, filePath)
    },
    /**
     * 导出模组
     * @param payload
     * @param callback
     * @return {*}
     */
    exportScheme (payload, callback = noop) {
      return service.exportModuleScheme(callback, payload)
    },
    focus (group, callback = noop) {
      try {
        return service.setHighlightGroup(callback, group)
      } catch (err) {
        console.error(err)
      }
    },
    getActivated (callback = noop) {
      return service.getMultiEgoScheme(callback)
    },
  },
  /**
   * 加载场景，准备播放
   * @param scenarioPath
   * @param callback
   * @return {*}
   */
  setup (scenarioPath, callback = noop) {
    try {
      console.log(`modules.setup scenario path: ${scenarioPath}`)

      return service.setup(callback, { scenarioPath })
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 停止场景
   * @param callback
   * @return {*}
   */
  unSetup (callback = noop) {
    try {
      return service.unSetup(callback)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 重置场景
   * @param scenarioPath
   * @param callback
   * @return {*}
   */
  reset (scenarioPath, callback = noop) {
    try {
      console.log(`modules.setup scenario path: ${scenarioPath}`)

      return service.reset(callback, { scenarioPath })
    } catch (err) {
      console.error(err)
    }
  },
  isPreset (names = [], name) {
    return names.includes(name)
  },
  getProtoDescription () {
    return pbDescription
  },
}
const player = {
  /**
   * 开始播放
   * @param callback
   * @return {*}
   */
  play (callback = noop) {
    try {
      return service.run(callback)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 暂停播放
   * @param callback
   * @return {*}
   */
  pause (callback = noop) {
    try {
      return service.pause(callback)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 单步播放
   * @param callback
   * @return {*}
   */
  step (callback = noop) {
    try {
      return service.step(callback)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 停止播放
   * @param callback
   * @return {*}
   */
  stop (callback = noop) {
    try {
      return service.stop(callback)
    } catch (err) {
      console.error(err)
    }
  },
  playlist: {
    /**
     * 添加场景到播放列表
     * @param ids
     * @param callback
     * @return {*}
     */
    update (ids, callback = noop) {
      try {
        return service.updatePlayList(callback, {
          scenarios: ids,
        })
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 删除场景到播放列表
     * @param ids
     * @param callback
     * @return {*}
     */
    removeScenarios (ids, callback = noop) {
      try {
        return service.removeScenariosFromPlayList(callback, {
          scenarios: ids,
        })
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 从场景列表查找场景
     * @param callback
     * @return {*}
     */
    findOne (callback = noop) {
      try {
        return service.getPlayList(callback)
      } catch (err) {
        console.error(err)
      }
    },
  },
}
const kpi = {
  /**
   * 更新指标
   * @param entity
   * @param callback
   * @return {*}
   */
  update (entity, callback = noop) {
    try {
      return service.updateGradingKpi(callback, entity)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 删除指标
   * @param id
   * @param callback
   * @return {*}
   */
  destroy (id, callback = noop) {
    try {
      return service.removeGradingKpi(callback, id)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 查找所有指标
   * @param callback
   * @return {*}
   */
  findAll (callback = noop) {
    try {
      return service.getGradingKpisInGroup(callback, -1)
    } catch (err) {
      console.error(err)
    }
  },
  /**
   * 设置指标标签
   * @param params
   * @param callback
   * @return {*}
   */
  setGradingLabel (params, callback = noop) {
    try {
      return service.setGradingLabel(callback, params)
    } catch (err) {
      console.error(err)
    }
  },
  labels: {
    /**
     * 获取指标标签
     * @param callback
     * @return {*}
     */
    getGradingLabelDescriptions (callback = noop) {
      try {
        return service.getGradingLabelDescriptions(callback)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 设置指标标签描述
     * @param params
     * @param callback
     * @return {*}
     */
    setGradingLabelDescription (params, callback = noop) {
      try {
        return service.setGradingLabelDescription(callback, params)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 删除指标标签描述
     * @param params
     * @param callback
     * @return {*}
     */
    delGradingLabelDescription (params, callback = noop) {
      try {
        return service.delGradingLabelDescription(callback, params)
      } catch (err) {
        console.error(err)
      }
    },
  },
  group: {
    /**
     * 创建指标组
     * @param entity
     * @param callback
     * @return {*}
     */
    create (entity, callback = noop) {
      try {
        return service.addGradingKpiGroup(callback, entity)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 更新指标组
     * @param entity
     * @param callback
     * @return {*}
     */
    update (entity, callback = noop) {
      try {
        return service.updateGradingKpiGroup(callback, entity)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 删除指标组
     * @param id
     * @param callback
     * @return {*}
     */
    destroy (id, callback = noop) {
      try {
        return service.removeGradingKpiGroup(callback, id)
      } catch (err) {
        console.error(err)
      }
    },
    /**
     * 查询所有指标组
     * @param callback
     * @return {*}
     */
    findAll (callback = noop) {
      try {
        return service.getAllGradingKpiGroups(callback)
      } catch (err) {
        console.error(err)
      }
    },
    active (id, callback = noop) {
      try {
        return service.setDefaultGradingKpiGroup(callback, id)
      } catch (err) {
        console.error(err)
      }
    },
    createKpis (entity, callback = noop) {
      try {
        return service.addGradingKpis(callback, entity)
      } catch (err) {
        console.error(err)
      }
    },
    findKpis (id, callback = noop) {
      try {
        return service.getGradingKpisInGroup(callback, id)
      } catch (err) {
        console.error(err)
      }
    },
  },
  report: {
    destroy (ids, callback = noop) {
      try {
        return service.removeKpiReports(callback, ids)
      } catch (err) {
        console.error(err)
      }
    },
    findAll (callback = noop) {
      try {
        return service.listKpiReports(callback)
      } catch (err) {
        console.error(err)
      }
    },
    find (id, callback = noop) {
      try {
        return service.getKpiReport(callback, id)
      } catch (err) {
        console.error(err)
      }
    },
    uploadReportImage (params, callback = noop) {
      try {
        return service.uploadReportImage(callback, params)
      } catch (err) {
        console.error(err)
      }
    },
    deleteReportImage (params, callback = noop) {
      try {
        return service.deleteReportImage(callback, params)
      } catch (err) {
        console.error(err)
      }
    },
    getReportImage (params, callback = noop) {
      try {
        return service.getReportImage(callback, params)
      } catch (err) {
        console.error(err)
      }
    },
  },
  copyExcel (fileList, targetPath) {
    fileList.forEach((file) => {
      const originPath = path.join(UserDataPath.kpiReport, file)
      fse.copyFile(originPath, targetPath)
    })
  },
}

module.exports = {
  system,
  modules,
  player,
  kpi,
}
