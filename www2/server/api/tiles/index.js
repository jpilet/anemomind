'use strict';

var express = require('express');
var controller = require('./tiles.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');

var router = express.Router();

router.get('/raw/:scale/:x/:y/:boat/:startsAfter?/:endsBefore?',
           auth.maybeAuthenticated(), 
           access.boatReadAccess,
           controller.retrieveRaw);

router.get('/geojson/:scale/:x/:y/:boat/:startsAfter?/:endsBefore?',
           auth.maybeAuthenticated(), 
           access.boatReadAccess,
           controller.retrieveGeoJson);

router.get('/at/:boat/:time',
           auth.maybeAuthenticated(), 
           access.boatReadAccess,
           controller.infoAtTime);

module.exports = router;
