var config = require('./config');
var anemoId = require('./boxId');
var localEndpoint = require("./LocalEndpoint.js");


function runPostIdAssignJobs() {
  localEndpoint.postRemainingLogFilesFromRoot(function(err) {
    if (err) {
      console.log("Failed to post remaining log files after id was assigned");
    } else {
      console.log("Successfully posted remaining log files after id was assigned.");
    }
  });
}

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
              runPostIdAssignJobs();
	            cb({result: "OK"});
	          }
	        });
        } else {
	        cb({error: "invalid arguments"});
        }
      }
    });
  };

  rpcFuncTable.changeConfig = function(data, cb) {
    if ("boatId" in data) {
      // we silently refuse to reset boat id
      delete data.boatId;
    }
    config.change(data, function(err, cfg) {
      if (err) {
        cb({error: "can't save config"});
      } else {
        cb({result: "OK"});
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
