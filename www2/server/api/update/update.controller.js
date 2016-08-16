/**
 * Using Rails-like standard naming convention for endpoints.
 * GET     /api/updates              ->  index
 */

'use strict';

var config = require('../../config/environment');
var mkdirp = require('mkdirp');
var BundleUtils = require('../boxexec/bundleutils.js');
var fs = require('fs');
var Q = require('q');
var RemoteOps = require('../boxexec/remoteOps');
var glob = require("glob");
var exec = require('child_process').exec;
var path = require('path');

var updatePath = fs.realpathSync(config.uploadDir) + '/updates/';
console.log('Update file path: ' + updatePath);

function readDeployFile(entry) {
  return Q.nfcall(fs.readFile,
                  updatePath + '/' + entry.name + '.deploy', "utf-8")
    .then(function(data) {
          entry.deploy = data;
          return entry;
     });
}

// Gets a list of Updates
exports.index = function(req, res) {
  fs.readdir(updatePath, function(err, items) {
      if (err) {
        res.status(500).send('bad updatePath?');
        return;
      }
      var result = [];
      var deployPromises = [];
      for (var i in items) {
        var name = items[i];
        if (!name.match(/(deploy)|(split)/)) {
          var stats = fs.statSync(updatePath + '/' + name);
          var entry = { name: name, size: stats.size};
          result.push(entry);
          deployPromises.push(readDeployFile(entry));
        }
      }
      Q.allSettled(deployPromises)
        .then(function() {
          res.json(result);
        })
        .catch(function(err) {
          console.warn(err);
          res.send(err).status(500);
        });
    });
};


exports.createUpdate = function(req, res) {
  mkdirp(updatePath, function(err) {
    if (err) {
      res.status(500).send(err);
      return;
    }
    if (!req.body.from || !req.body.to || !req.body.patchName
        || !req.body.deploy) {
      res.status(400).send('bad request');
      return;
    }
    var from = req.body.from;
    var to = req.body.to;
    console.warn(req.body);
    var filename = updatePath + '/' + req.body.patchName;
    BundleUtils.makeBundle(filename, {from: from, to: to}, true)
    .then(function() {
      return Q.nfcall(fs.writeFile, filename + '.deploy', req.body.deploy);
    })
    .then(function() {
      req.send(201);
    })
    .catch(function(err) {
       req.status(500).send(err);
    });

  });
};

exports.sendUpdate = function(req, res) {
  if (!req.body.update || !req.body.boats) {
    res.status(400);
    return;
  }

  if (req.body.boats.length == 0) {
    res.status(500).send('no boat selected');
    return;
  }
  if (req.body.boats.length > 1) {
    res.status(500).send('can update only one boat at a time');
    return;
  }

  var name = req.body.update;
  var updateFile = updatePath + '/' + name;
  var deployScript = updateFile + '.deploy';

  if (!req.body.boats[0]) {
    res.status(400).send('no boat id?');
    return;
  }

  var boatId = req.body.boats[0];
  var deploy = "";

  // First, read the deploy script.
  Q.nfcall(fs.readFile, deployScript)

  .then(function(data) {
    deploy = data;
  })
  .then(function() {
    // Split the file to send into small pieces
    // with a command such as
    // split -b 50k patch-v1.1-v1.3.bundle  patch-v1.1-v1.3.bundle.split.
    var cmd = "split -b 50k " + updateFile + ' ' + updateFile + '.split.';
    return Q.nfcall(exec, cmd, {});
  })
  .then(function() {
    // list all splits
    return Q.nfcall(glob, updateFile + '.split.*', { nonull: false });
  })
  .then(function(files) {
    if (files.length == 0) {
      throw new Error('no split files found');
    }

    var chain;
    var funcs = files.map(function(file) {
      return function() {
      // then send the update data
      var remoteFile = '/home/anemobox/' + path.basename(file);
      console.log('Sending update ' + file + ' to boat: ' + boatId
                  + ' remote file: ' + remoteFile);
      return Q.nfcall(RemoteOps.sendBoatData, boatId, file, remoteFile);
      };
    });

    // send each piece one after the other
    return funcs.reduce(function (soFar, f) {
      return soFar.then(f);
    }, Q(0));
  })
  .then(function() {
    // then send the deploy script
    console.log('Deploying update ' + deployScript + ' to boat: ' + boatId);

    // sendScriptToBox wants a string, not a buffer.
    // so we convert the buffer to string here.
    deploy = deploy + '';
    return Q.nfcall(RemoteOps.sendScriptToBox, boatId, 'sh', deploy);
  })
  .then(function(boxexec) {
    // return the boxexec information allowing to find a reply.
    res.status(200).json(boxexec);
  })
  .catch(function(err) {
    res.status(500).send("failed: " + JSON.stringify(err));
  });
};

