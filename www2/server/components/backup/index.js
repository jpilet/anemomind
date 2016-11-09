var exec = require('child_process').exec;
var config = require('../../config/environment');

function runRsync(args, cb) {
  var baseArgs = [
      'rsync',
      '-e ssh',
      '-ar'
  ];

  var args = baseArgs.concat(args);
  args.push(config.backupDestination);
  var cmd = args.join(' ');

  exec(cmd, function(err, stdout, stderr) {
       if (err) {
         console.log('Error while running: ' + cmd);
         console.log(err);
       }
       cb(err, stdout, stderr);
     });
}

var savingPhotos = false;
function backupPhotos(cb) {
  if (savingPhotos) {
    return;
  }
  savingPhotos = true;

  runRsync(['"' + config.uploadDir + '/photos"'],
    function (err,stdout,stderr) {
      savingPhotos = false;
      if (typeof(cb) == 'function') {
        cb(err, stdout, stderr);
      }
    }
  );
}

var pushRunning = false;
function pushLogFilesToProcessingServer(cb) {
  if (pushRunning) {
    return;
  }
  pushRunning = true;

  runRsync([
    // '--remove-source-files',
    '"' + config.uploadDir + '/anemologs"',
    ],
    function(err, stdout, stderr) {
      pushRunning = false;
      if (typeof(cb) == 'function') {
        cb(err, stdout, stderr);
      }
    }
  );
}

module.exports.backupPhotos = backupPhotos;
module.exports.pushLogFilesToProcessingServer = pushLogFilesToProcessingServer;
