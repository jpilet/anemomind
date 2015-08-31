var spawn = require('child_process').spawn;
var split = require('split');

var listening = false;

function listen(cb) {
  if (listening) {
    return;
  }

  imu = spawn('/bin/bash', ['-c', __dirname + '/app_bno055']);
  imu.stdout.pipe(split()).on('data', function(line) {
    if (line && line.length > 0) {
      var obj = JSON.parse(line);
      cb(obj);
    }
  });

  imu.on('close', function (code) {
    console.log('imu process exited with code ' + code);
    listening = false;
    imu = undefined;
  });
}