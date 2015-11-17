var config = require('./config');
var anemoId = require('./boxId');

function register(rpcFuncTable) {
  rpcFuncTable.assignBoat = function(data, cb) {
    config.get(function(err, cfg) {
      if(cfg && cfg.boatId) {
        cb({error:"box is already assigned."});
      } else {
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
      }
    });
  };

  rpcFuncTable.identify = function(data, cb) {
    console.log('identify call');
    anemoId.getAnemoId(function(boxId) {
      config.get(function(err, config) {
        var reply = {boxId: boxId};
        if (config && config.boatId) {
          reply.boatName = config.boatName;
          reply.boatId = config.boatId;
        } else {
          reply.error = "boat is not assigned";
        } 
        console.log('Identify: reply');
        cb(reply);
      });
    });
  };
}

module.exports.register = register;
