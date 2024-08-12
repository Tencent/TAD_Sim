var addon = require('./txsim-play-service.node');
const util = require('util');

if (process.argv.length < 3) {
  console.log("usage: node play-client.js cmd (see play-client.js switch cases for usable cmds)");
  return;
}

switch (process.argv[2]) {
  case 'setup':
    addon.setup((status, err) => {
      console.log("err: " + JSON.stringify(err));
      console.log("status: " + util.inspect(status, false, 3, true));
    }, { scenarioPath: process.argv[3] });
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
      console.log("status: " + util.inspect(status, false, 3, true));
    });
    break;
  default:
    console.log("usage: node play-client.js cmd (see play-client.js switch cases for usable cmds)");
}
