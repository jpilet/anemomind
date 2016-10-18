
var config = require('./config');
var fs = require('fs');
var exec = require('child_process').exec;

var interfaceFile = '/etc/network/interfaces.d/userconfigured';
var networkStateFile = '/tmp/network-state';
var ifupScript = '/anemonode/ifup-userconfig.sh';

function restartNetwork() {
  fs.unlink(networkStateFile, function(err) {
    exec(ifupScript, function(err, stdout, stderr) {
      if (err) {
        console.log(ifupScript + ': ' + err);
      }
      if (stdout) {
        console.log(ifupScript + ': ' + stdout);
      }
      if (stderr) {
        console.log(ifupScript + ': ' + stderr);
      }
    });
  });
}

function applyNetConfig(cfg) {
  var mode = cfg.wifi_mode || 'AP';

  if (mode == 'WPA') {
    var content = [
      'iface userconfigured inet dhcp',
      'wpa-ssid ' + cfg.wpa_ssid,
      'wpa-psk ' + cfg.wpa_password
    ].join('\n') + '\n';

    fs.readFile(interfaceFile, function(err, existing) {
      if (err || existing != content) {
        // the configuration has changed, apply the new one
        console.log('Network: connecting to ' + cfg.wpa_ssid);
        fs.writeFile(interfaceFile, content, function(err) {
          if (err) {
            console.warn(interfaceFile + ': ' + err);
          }
          restartNetwork();
        });
      }
    });

  } else if (mode == 'AP') {
    fs.unlink(interfaceFile, function(err) {
      if (err) {
        // can't unlink, the file did not exist: no actual config change.
      } else {
        // we removed the interface config file: the configuration has changed.
        console.log('Network: switching to AP mode');
        restartNetwork();
      }
    });
  }
}

module.exports.applyNetworkConfig = function() {
  config.get(function(err, cfg) {
    applyNetConfig(cfg);
  });
}

