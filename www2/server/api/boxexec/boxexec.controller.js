/**
 * Using Rails-like standard naming convention for endpoints.
 * GET     /api/boxexecs              ->  index
 */

'use strict';

var BoxExec = require('./boxexec.model');
var RemoteOps = require('./remoteOps');
var BundleUtils = require('./bundleutils');
var Boat = require('../boat/boat.model');

// Gets a list of Boxexecs
exports.index = function(req, res) {
  BoxExec.find(req.params, function(err, boxexecs) {
    if(err) return res.status(500).send(err);
    res.status(200).json(boxexecs);
  });
}

function createSendScriptBoxExec(req, res) {
  var validTypes = { js: true, sh: true };
  if (!validTypes[req.body.scriptType]) {
    res.status(400).send("Invalid type");
    return;
  }
  RemoteOps.sendScriptToBox(
      req.body.boatId, req.body.scriptType, req.body.scriptData,
      function(err, reqCode) {
        if (err || !reqCode) {
          res.status(400).send(err);
          return;
        }
        BoxExec.findById(reqCode, function(err, boxexec) {
          if (err || !boxexec) {
            res.status(500).send(err);
            return;
          }
          res.status(200).json(boxexec);
        });
      });
}

function isFresh(file, maxAgeSeconds) {
  var deferred = $q.defer();

  fs.stat(file, function(err, stats) {
    if (err || !stats.birthtime) {
      deferred.reject(file + ': fs.stat failed');
    }
    var deltams = (new Date().getTime() - stats.birthtime.getTime());
    if (deltams < maxAgeSeconds * 1000) {
      deferred.resolve(file);
    } else {
      deferred.reject('not fresh');
    }
  });

  return deferred;
}

function removeIfExists(file) {
  var deferred = $q.defer();
  fs.unlink(file, function(err) {
    deferred.resolve(file);
  });
  return deferred;
}

function createSendBundleFile(req, res) {
  if (!req.body.boatId) {
    res.status(400).send("Bad bundle settings");
  }

  var boatId = req.body.boatId;

  Boat.findById(boatId, function (err, boat) {
    if (err) {
      return res.status(404).send(err);
    }

    var installedVersion = boat.firmwareVersion;

    if (!installedVersion) {
      return res.status(401).send(
          "Do not know what is the firmware version installed on " + boat.name);
    }

    var sendFile = function(file) {
      RemoteOps.sendBoatData(boatId, file, '/tmp/bundle', function(err, result) {
        if (err) {
          res.status(500).send("sendBoatData failed: " + err);
        } else {
          res.status(200).send();
        }
      });
    };

    var file = '/tmp/bundle-'
      + BundleUtils.makeVersionString(req.body.bundleSettings.version);
    var inprogress = file + '-in-progress';

    // Is the file recent enough?
    isFresh(file, 5 * 60).then(
       sendFile, // fresh
       function() { // not fresh
         Q.all([
           // clean old file
           removeIfExists(file),
           // compute the new one in parallel
           BundleUtils.makeBundle(inprogress, { from: installedVersion })
         ]).then(
           function() {
             // bundle created, rename the file to ensure atomic creation
             fs.rename(inprogress, file, function(err) {
               if (err) {
                 res.status(500).send("Failed to rename " + inprogress
                                      + " to " + file);
               } else {
                 sendFile(file);
               }
             });
           },
           function(err) {
             res.status(500).send(err);
           }
         );
       }
     );
  });
}

exports.create = function(req, res) {
  if (req.body.scriptType) {
    createSendScriptBoxExec(req, res);
  } else if (req.body.bundleSettings) {
    createSendBundleFile(req, res);
  } else {
    res.status(400).send();
  }
}

