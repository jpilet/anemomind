'use strict'

var express = require('express');
var controller = require('./timeset.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');
var router = express.Router();

// List all the timesets for a boat.
router.get( // OK
  '/:boatId', 
  auth.maybeAuthenticated(), 
  access.boatReadAccess,
  controller.getTimesetsForBoat);

// Push a new timeset
router.post(
  '/:boatId', // The boat id is also part of the data, this is actually redundant.
  auth.maybeAuthenticated(),
  access.boatWriteAccess,
  controller.addTimeset);

// Delete a timeset
router.delete( // OK
  '/:boatId/:timesetId',
  auth.maybeAuthenticated(),
  access.boatWriteAccess,
  controller.deleteTimeset);

module.exports = router;
