var child_process = require('child_process');
var exec = child_process.exec;
var spawn = child_process.spawn;
var q = require('q');
var fs = require('fs');
var path = require('path');

function parseArgs(args) {
  return {
    path: args[2],
    id: args[3]
  };
}

function getId(args) {
  if (args.id == undefined) {
    var parsed = path.parse(args.path);
    var name = parsed.name;
    if (name.length > 4 && name.slice(0, 4) == 'boat') {
      return name.slice(4);
    } else {
      return '119';
    }
  }
  return args.id;
}

function silentExec(cmd, cb) {
  exec(cmd, function(err, stdout, stderr) {
    cb();
  });
}

function spawnWithConsoleOutput(cmd, args, cb) {
  var processing = spawn(cmd, args);
  console.log('Spawn ' + cmd + '...');

  processing.stdout.on('data', function(data) {
    console.log(cmd + ' OUTPUT: ' + data);
  });
  processing.stderr.on('data', function(data) {
    console.log(cmd + ' ERROR:  ' + data);
  });

  processing.on('close', function(code) {
    if (code == 0 || code == null) {
      cb();
    } else {
      cb(new Error(cmd + ' finished with error code ' + code));
    }
  });
}

var dbPath = '/tmp/localtestdb';

function performTest(args) {
  var cmd = 'rm ' + dbPath + ' -rf; mkdir ' + dbPath;
  return q.nfcall(silentExec, cmd)
    .then(function() {
      return q.nfcall(fs.access, dbPath, fs.F_OK);
    })
    .then(function() {
      // Run this in background.
      q.nfcall(spawn, 'mongod', ['--dbpath', dbPath]);

      return null;
    })
    .then(function() {
      return q.nfcall(
        spawnWithConsoleOutput,
        '../build/src/server/nautical/tiles/tiles_generateAndUpload',
        ['--id', getId(args),
         '--navpath', args.path,
         '--scale', '20', '--db', 'anemomind-dev']);
    })
    .then(function() {
      console.log('Done');
      return;
    });
}

function main(args0) {
  var args = parseArgs(args0);
  if (args.path == undefined) {
    console.log('No path provided');
  } else {
    q.nfcall(fs.access, args.path, fs.F_OK)
      .then(function() {
        return performTest(args);
      })
      .catch(function(err) {
        console.log('Failed on %j', args);
        console.log(err);
      }).done();
  }
}

main(process.argv);
