var chokidar = require('chokidar');
var config = require('../../config/environment/index.js');
var path = require('path');
var naming = require('mail/naming.js');
var common = require('../../../utilities/common.js');

// Files that it should look for,
// and their corresponding locations
// where they should be put on the box
var defaultFilesToLookFor = {
  "boat.dat": "boat.dat"
};

function getBoatIdFromPath(p) {
  var parsed = path.parse(p);
  if (parsed.base == '') {
    return null;
  } else {
    var parsedMailboxName = naming.parseMailboxName(parsed.base);
    if (parsedMailboxName) {
      if (parsedMailboxName.prefix == 'boat') {
        return parsedMailboxName.id;
      }
    }
    return getBoatIdFromPath(parsed.dir);
  }
}


function checkFileToPost(filesToLookFor, srcFilename, cb) {
  var parsed = path.parse(srcFilename);
  var dstFilename = filesToLookFor[parsed.base];
  if (dstFilename) {
    var boatId = getBoatIdFromPath(parsed.dir);
    if (boatId) {
      common.sendBoatData(boatId, srcFilename, dstFilename, function(err) {
        if (err) {
          cb(err);
        } else {
          cb(null, {
            src: srcFilename,
            dst: dstFilename,
            boatId: boatId
          });
        }
      });
    }
  }
}

function startWatchForFiles(root, filesToLookFor, cb) {
  var watcher = chokidar.watch(root, {ignored: /^\./, persistent: true})
    .on('add', function(p) {checkFileToPost(filesToLookFor, p, cb);})
    .on('change', function(p) {checkFileToPost(filesToLookFor, p, cb);})
    .on('unlink', function(p) {})
    .on('error', function(error) {
      console.error('Error: ', error);
    });
}

function defaultPostCB(err, info) {
  if (err) {
    console.log('Failed to post boat data file to box:');
    console.log(err);
  } else {
    console.log('Successfully posted detected file "%s" for box.', info.src);
  }
}

function startWatch(cb) {
  startWatchForFiles(config.uploadDir, defaultFilesToLookFor,
                     cb || defaultPostCB);
}

module.exports.startWatch = startWatch;
module.exports.startWatchForFiles = startWatchForFiles;
module.exports.getBoatIdFromPath = getBoatIdFromPath;
