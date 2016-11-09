/**
 * Using Rails-like standard naming convention for endpoints.
 * GET     /api/boxexecs              ->  index
 */

'use strict';

var BoxExec = require('./boxexec.model');
var RemoteOps = require('./remoteOps');

// Gets a list of Boxexecs
exports.index = function(req, res) {
  BoxExec.find(req.query, function(err, boxexecs) {
    if(err) return res.status(500).send(err);
    res.status(200).json(boxexecs);
  });
}

exports.create = function(req, res) {
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
