var config = require('./config');
var rpcFuncTable = require('./components/rpcble').rpcFuncTable;
var anemoId = require('./boxId');

rpcFuncTable.assignBoat = function(data, cb) {
  var configChanges = {};
  var valid = false;
  if ('boatId' in data) {
    configChanges.boatId = data.boatId;
    valid = true;
  }
  if ('boatName' in data) {
    configChanges.boatName = data.boatName;
    valid = true;
  }
  if (valid) {
    config.change(configChanges, function(err, cfg) {
      if (err) {
        cb({error: "can't save config"});
      } else {
        cb({result: "OK"});
      }
    });
  } else {
    cb({error: "invalid arguments"});
  }
};

rpcFuncTable.identify = function(data, cb) {
  anemoId.getAnemoId(function(boxId) {
    config.get(function(err, config) {
      var reply = {boxId: boxId};
      if (config && config.boatId) {
        reply.boatName = config.boatName;
        reply.boatId = config.boatId;
      } else {
        reply.error = "boat is not assigned";
      } 
      cb(reply);
    });
  });
};
