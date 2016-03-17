/**
 * Using Rails-like standard naming convention for endpoints.
 * GET     /api/boxexecs              ->  index
 */

'use strict';

var BoxExec = require('./boxexec.model');

// Gets a list of Boxexecs
exports.index = function(req, res) {
  BoxExec.find(req.params, function(err, boxexecs) {
    if(err) return res.status(500).send(err);
    res.status(200).json(boxexecs);
  });
}
