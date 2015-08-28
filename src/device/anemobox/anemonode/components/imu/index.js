var spawn = require('child_process').spawn;
var split = require('split');

var listening = false;

function listen(cb) {
  if (listening) {
    return;
  }

  analyzer = spawn('/bin/bash', ['-c', './app_bno055']);
  analyzer.stdout.pipe(split()).on('data', function(line) {
    if (line && line.length > 0) {
      console.log('Parsing: "' + line + '"');
      var obj = JSON.parse(line);
      cb(obj);
    }
  });

  analyzer.on('close', function (code) {
    console.log('analyzer process exited with code ' + code);
    listening = false;
    analyzer = undefined;
  });
}
listen(function(obj) {console.dir(obj)});