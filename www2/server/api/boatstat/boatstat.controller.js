/**
 * Using Rails-like standard naming convention for endpoints.
 * GET     /api/boatstats              ->  index
 */

'use strict';

var Boat = require('../boat/boat.model');
var User = require('../user/user.model');
var Boatstat = require('./boatstat.model');

var mongoose = require('mongoose');

var access = require('../boat/access.js');

// Get a single boat
exports.show = function(req, res) {
    console.warn(req.user);
  access.userCanReadBoatId(req.user._id, req.params.id)
  .then(function() {
        console.log('auth ok');
      Boatstat.findById(req.params.id, function (err, boatstat) {
        if(err) { return handleError(res, err); }
        if(!boatstat) { return res.sendStatus(404); }
        return res.json(boatstat);
      });
    })
  .catch(function(err) {
        console.log('auth failed');
    res.sendStatus(404);
  });
};

