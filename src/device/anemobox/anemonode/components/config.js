var fs = require('fs');

var configFile = "./config.json"

function defaultConfig() {
  return {
    boatName: "",
    boatId: ""
  };
}

function clone(obj) {
  return JSON.parse(JSON.stringify(obj));
}


var globalConfig;

function write(config, cb) {
  var data = JSON.stringify(config);

  fs.writeFile(configFile, data, {encoding:'utf8'},
               function (err) {
    if (err) {
      console.log('There has been an error saving your configuration data.');
      console.log(err.message);
      if (cb) {
        cb(err, undefined);
      }
    }

    // We clone the config object to avoid bad surprises.
    globalConfig = clone(config);
    if (cb) {
      cb(undefined, config);
    }
  });
}

function get(cb) {
  if (globalConfig) {
    cb(undefined, clone(globalConfig));
  } else {
    fs.readFile(configFile, {encoding: 'utf8'}, function(err, data) {
      if (err) {
	write(defaultConfig(), cb);
      } else {
	globalConfig = JSON.parse(data);
	cb(undefined, clone(globalConfig));
      }
    });
  }
}

function change(changes, cb) {
  get(function(err, config) {
    if (err) {
      cb(err, undefined);
    } else {
      for (var i in changes) {
        config[i] = changes[i];
        console.log('Changing config.' + i + ' to ' + changes[i]);
      }
      write(config, cb);
    }
  });
}

get(function(err, config) {
  if (config.boatName) {
    console.log('This anemobox is installed on boat: ' + config.boatName + " (" + config.boatId + ")");
  } else {
    console.log('This anemobox is not assigned to any boat.');
  }
});

module.exports.get = get;
module.exports.write = write;
module.exports.change = change;
