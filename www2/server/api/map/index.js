'use strict';

var express = require('express');
var controller = require('./map.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');

var router = express.Router();

// Example URL:
// http://localhost:9000/api/map/552b806a35ce7cb254dc9515/0.518555473259716,0.3537685883609589,0.0001370951205489357/2016-09-14T16:25:31/2016-09-14T17:40:02/320-200.png
router.get('/:boat/:location/:timeStart/:timeEnd/:width-:height.png',
           auth.maybeAuthenticated(), 
           access.boatReadAccess,
           controller.getMapPng);


module.exports = router;
