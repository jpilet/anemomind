var fs = require('fs');
var EventEmitter = require('events');
var util = require('util');
var sync = require('./fssync.js').sync;

function ConfigEventEmitter() {
    EventEmitter.call(this);
}
util.inherits(ConfigEventEmitter, EventEmitter);
module.exports.events = new ConfigEventEmitter();

function getConfigPath() {
  return process.env.ANEMOBOX_CONFIG_PATH || ".";
}

var configFile = getConfigPath() + "/config.json"

function defaultConfig() {
  return {
    boatName: "",
    boatId: "",
    nmea0183Speed: 4800,
    logRawNmea2000: false
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
      console.log(configFile + ': There has been an error saving your configuration data.');
      console.log(err.message);
      if (cb) {
        cb(err, undefined);
      }
    }

    // config file has been written
    // but cache remains. Let's tell the OS to flush caches.
    sync();

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
        // Set defaults
        globalConfig = defaultConfig();

        // Override defaults with stored config
        var loaded = JSON.parse(data);
        for (var entry in loaded) {
          globalConfig[entry] = loaded[entry];
        }

        // Clone to make sure the receiver can't modify globalConfig.
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
      write(config, function(err, config) {
        cb(err, config);
        module.exports.events.emit('change');
      });
    }
  });
}

get(function(err, config) {
  if (config && config.boatName) {
    console.log('This anemobox is installed on boat: ' + config.boatName + " (" + config.boatId + ")");
  } else {
    console.log('This anemobox is not assigned to any boat.');
  }
});

function getAndListen(f) {
  get(f);
  module.exports.events.on('change', function() {
    get(f);
  });
}

module.exports.get = get;
module.exports.write = write;
module.exports.change = change;
module.exports.getConfigPath = getConfigPath;
module.exports.getAndListen = getAndListen;
