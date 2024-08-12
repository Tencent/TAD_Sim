var addon = require('./build/Debug/txsim-play-service.node');
const util = require('util');

//retCode = 0   // 指令被接受，代表指令正在执行，后续还会有数据返回，step/stop/pause不会返回此状态，只有run/setup会返回
//          1   // 成功
//          2   // 成功并且场景结束（只有Step/Run/Stop会有可能返回该状态）
//          3   // 指令被拒绝，有另一个命令正在执行或者当前状态不对
//          4   // 执行失败，此时err非空
//          5   // 系统内部错误
//          6   // 多个模块Publish了同一个topic，仿真系统禁止此行为，此时Setup失败，需要更改模块配置重新Setup
//          7   // 场景文件解析错误，setup失败
//          8   // 场景超时，如果用户设置了单个场景的最大时长且在最大时长内没有模块主动请求结束当前场景，引擎则会强制结束，此时可以正常setup下一个场景
//          9   // 共享内存创建失败，通常是由于系统资源不够，此时Setup失败
//          -1  // 客户端连接错误
//          -2  // JS方法参数错误
//
// ==============================================
// Player相关命令(Setup,Step,Run,Pause,Stop)返回：
//
//err = [{
//  name: 'moduleB',
//  code: 2,            // 0: 成功，1:模块请求停止当前场景（0和1不会出现在这里），2: 超时，3: 代码抛出异常（此时err非空），4: 版本不匹配，
//                      // 5: 系统内部错误（此时retCode为5），6：模块需要重启以重新初始化，7：模块进程退出
//  err: "some error."  // code为2时为相应模块代码中抛出异常中的字符串内容
//}]
//
//status = {
//  retCode: 1,
//  initStatus: [{
//    name: 'moduleA',
//    connected: true,
//    initiated: false,
//    sub_topics: ["TRAFFIC", "LOCATION"],
//    pub_topics: ["TRAJECTORY"],
//    pub_shmems: [ {"topic":"MY_SHMEM_TOPIC","size":1024} ]
//  }],
//  cmdStatus: [{
//    name: 'moduleA',
//    timeCost: 54,  // in milli-seconds.
//    msg: "I want to stop the scenario since I have reached the destination.",  // 目前是模块请求停止场景的理由
//    feedback: "{\"grading\": \"90\"}"  // 模块在场景停止时返回的反馈信息，JSON格式的字典，kv均为字符串，有可能为空
//  }],
//  msgStatus: [{
//    timestamp: 2600,  // in milli-seconds.
//    messages: [{
//      topic: "LOCATION",
//      content: "protobuf message json string"
//    }]
//  }]
//}
//
// ==============================================
// Config管理相关命令返回：
//
//status = {
//  retCode: 1,
//  data: {}  // or [],具体由每个命令对应的返回数据结构定义，某些命令可以没有data项
//}


//addon.setAppPath("/home/nemo/.config/tadsim", "/opt/tadsim/resources/app/service");
addon.setAppPath("C:\\Users\\mikemychen\\AppData\\Roaming\\tadsim", "E:\\tadsim\\resources\\app\\service");
//addon.setAppPath("C:\\Users\\fangccheng\\AppData\\Roaming\\tadsim", "C:\\Program Files\\tadsim\\resources\\app\\service");

// optional, default is tcp://127.0.0.1:8401 on Windows and uds on Linux.
// addon.setClientEndpoint("tcp://127.0.0.1:30001");

if (process.argv.length < 3) {
  console.log("usage: node test.js cmd (see test.js switch cases for usable cmds)");
  return;
}

switch (process.argv[2]) {
  case 'setup':
    addon.setup((status, err) => {
      console.log("err: " + JSON.stringify(err));
      console.log("status: " + util.inspect(status, false, 3, true));
    }, { scenarioPath: "/home/nemo/.config/tadsim/scenario/scene/AEB_straight_001.sim" });
    break;
  case 'un-setup':
    addon.unSetup((status, err) => {
      console.log("err: " + JSON.stringify(err));
      console.log("status: " + util.inspect(status, false, 3, true));
    });
    break;
  case 'step':
    addon.step((status, err) => {
      console.log("err: " + JSON.stringify(err));
      console.log("status: " + util.inspect(status, false, 3, true));
    });
    break;
  case 'stop':
    addon.stop((status, err) => {
      console.log("err: " + JSON.stringify(err));
      console.log("status: " + util.inspect(status, false, 3, true));
    });
    break;
  case 'run':
    addon.run((status, err) => {
      console.log("err: " + JSON.stringify(err));
      console.log("status: " + util.inspect(status, false, 3, true));
    });
    break;
  case 'pause':
    addon.pause((status) => {
      console.log("err: " + err);
      console.log("status: " + util.inspect(status, false, 3, true));
    });
    break;
  case 'add':
    addon.addModuleConfig((status) => { console.log(status); }, {
      name: "Planning",
      stepTime: 100,
      endpoint: "",
      initArgs: {},
      cmdTimeout: 3000,
      stepTimeout: 10000,
      autoLaunch: true,
      depPaths: ["/opt/TADSim/resources/app/buildin/planning", "/opt/TADSim/resources/app/buildin/simdeps"],
      soPath: "/opt/TADSim/resources/app/buildin/planning/libsim_planning.so",
      binPath: "",
      binArgs: []
    });
    break;
  case 'remove':
    addon.removeModuleConfig((status) => { console.log(status); }, "Traffic");
    break;
  case 'update':
    addon.updateModuleConfig((status) => { console.log(status); }, {
      name: "Traffic",
      stepTime: 20,
      endpoint: "tcp://127.0.0.1:8888",
      initArgs: {},
      cmdTimeout: 3000,
      stepTimeout: 500,
      autoLaunch: true,
      depPaths: ["/opt/TADSim/resources/app/buildin/simdeps"],
      soPath: "",
      binPath: "/opt/TADSim/resources/app/buildin/txSimTraffic",
      binArgs: ["tcp://127.0.0.1:8888"]
    });
    break;
  case 'get':
    addon.getModuleConfig((status) => { console.log(status); }, "Traffic");
    break;
  case 'get-all':
    addon.getAllModuleConfigs((status) => { console.log(util.inspect(status, false, null, true)); });
    break;
  case 'add-scheme':
    addon.addModuleScheme((status) => { console.log(status); }, {
      name: "L4-Planning-test",
      modules: ["Traffic", "Planning", "Perfect_Control"]
    });
    break;
  case 'remove-scheme':
    addon.removeModuleScheme((status) => { console.log(status); }, 1);
    break;
  case 'update-scheme':
    addon.updateModuleScheme((status) => { console.log(status); }, {
      id: 1,
      name: "L4-Planning-test",
      modules: ["Traffic", "Planning", "Controller", "vehicle_dynamics"]
    });
    break;
  case 'get-all-scheme':
    addon.getAllModuleSchemes((status) => { console.log(util.inspect(status, false, null, true)); });
    break;
  case 'set-active':
    addon.setActiveModuleScheme((status) => { console.log(status); }, 1);
    break;
  case 'export-scheme':
    addon.exportModuleScheme((status) => { console.log(status); }, {
      name: "L4-Planning-test",
      path: "L4-Planning-test.scheme"
    });
    break;
  case 'import-scheme':
    addon.importModuleScheme((status) => { console.log(status); }, "L4-Planning-test.scheme")
    break;
  case 'set-sys':
    addon.setSystemConfigs((status) => { console.log(status); }, {
      playControlRate: 200,
      scenarioTimeLimit: 60,  // in seconds.
      coordinationMode: 2  // 1 for sync, 2 for async.
    });
    break;
  case 'get-sys':
    addon.getSystemConfigs((status) => { console.log(status); });
    break;
  case 'update-playlist':
    addon.updatePlayList((status) => { console.log(status); }, {
      scenarios: [1, 2, 3, 4, 5]
    });
    break;
  case 'update-multiego-scheme':
    addon.updateMultiEgoScheme((status) => { console.log(status); }, {
      "active_multiego":[
        {"group_name": "Ego001","scheme_id": 0,"group_type": 1},
        {"group_name": "Ego002","scheme_id": 0,"group_type": 1},
        {"group_name": "Scene","scheme_id": 2,"group_type": 0}]
    });
    break;
  case 'get-playlist':
    addon.getPlayList((status) => { console.log(status); })
    break;
  case 'get-multiego-scheme':
    addon.getMultiEgoScheme((status) => { console.log(util.inspect(status, false, null, true)); })
    break;
  case 'set-highlight-group':
    addon.setHighlightGroup((status) => { console.log(status); },"Ego001")
    break;
  case 'remove-scenarios':
    addon.removeScenariosFromPlayList((status) => { console.log(status); }, {
      scenarios: [2, 3]
    });
    break;
  case 'add-kpis':
    addon.addGradingKpis((status) => { console.log(status); }, {
      groupId: 1,
      kpis: [{
        name: 'EndPoint',
        parameters: {
          Threshold: '0,0,0',
          NeedParking: '1',
          Radius: "2.5"
        },
        passCondition: 1,
        finishCondition: 1
      }, {
        name: 'TotalTime',
        parameters: {
          Threshold: '60.0',
        },
        passCondition: 1,
        finishCondition: 1
      }]
    });
    break;
  case 'remove-kpi':
    addon.removeGradingKpi((status) => { console.log(status); }, 1);
    break;
  case 'update-kpi':
    addon.updateGradingKpi((status) => { console.log(status); }, {
      id: 2,
      name: 'TotalTime',
      parameters: {
        Threshold: '120.0'
      },
      passCondition: 1,
      finishCondition: 1
    });
    break;
  case 'get-kpis':
    addon.getGradingKpisInGroup((status) => { console.log(util.inspect(status, false, null, true)); }, -1);
    break;
  case 'add-kpi-group':
    addon.addGradingKpiGroup((status) => { console.log(status); }, {
      name: '指标分组1'
    });
    break;
  case 'remove-kpi-group':
    addon.removeGradingKpiGroup((status) => { console.log(status); }, 1);
    break;
  case 'update-kpi-group':
    addon.updateGradingKpiGroup((status) => { console.log(status); }, {
      id: 1,
      name: '指标分组-001'
    });
    break;
  case 'get-kpi-groups':
    addon.getAllGradingKpiGroups((status) => { console.log(util.inspect(status, false, null, true)); });
    break;
  case 'set-default-kpi-group':
    addon.setDefaultGradingKpiGroup((status) => { console.log(status); }, 1);
    break;
  case 'restore':
    addon.restoreDefaultConfigs((status) => { console.log(status); });
    break;
  case 'list-kpi-reports':
    addon.listKpiReports((status) => { console.log(util.inspect(status, false, null, true)); });
    break;
  case 'get-kpi-report':
    addon.getKpiReport(
      (status) => { console.log(util.inspect(status, false, 3, true)); },
      '/home/nemo/.config/tadsim/service_data/sim_data/pblog/F.AEB_straight_001.2020_11_24_12_7_57.pblog.json'
    );
    break;
  case 'remove-kpi-reports':
    addon.removeKpiReports((status) => { console.log(status); }, [
      '/home/nemo/.config/tadsim/service_data/sim_data/pblog/F.cutin_right_straight_004.2020_11_24_12_19_57.pblog.json',
      '/home/nemo/.config/tadsim/service_data/sim_data/pblog/F.cutin_right_straight_003.2020_11_24_12_19_7.pblog.json'
    ]);
    break;
  case 'upload-report-image':
        addon.uploadReportImage((status) => { console.log(status); },
            {
                id: "1111.111",
                //filePath: 'D:\\企业20230610165337.png'
                filePath: 'D:\\20230610165337.png'
            }
            );
    break;
  case 'delete-report-image':
        addon.deleteReportImage((status) => { console.log(status); });
        break;
  case 'get-report-image':
        addon.getReportImage((status) => { console.log(status); });
        break;
  case 'set-grading-label-description':
        addon.setGradingLabelDescription((status) => { console.log(status); },
        {
            labelKey: "abc3",
            labelDescription: "测试3",
        });
        break;
    case 'get-grading-label-descriptions':
        addon.getGradingLabelDescriptions((status) => { console.log(status); });
        break;
    case 'del-grading-label-description':
        addon.delGradingLabelDescription((status) => { console.log(status); },
        {
            labelKey: "abc"
        });
        break;
    case 'set-grading-label':
        addon.setGradingLabel((status) => { console.log(status); },
            {
                name: "TotalTime",
                labels: ["1111", "22222", "33333"]
            });
        break;
  default:
    console.log("usage: node test.js cmd (see test.js switch cases for usable cmds)");
}
