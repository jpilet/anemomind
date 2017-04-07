'use strict';

var express = require('express');
var controller = require('./chart.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');

var router = express.Router();

router.get('/:boat/:zoom/:tile/:channel?/:source?',
           auth.maybeAuthenticated(), 
           access.boatReadAccess,
           controller.retrieve);

router.get('/:boat',
           auth.maybeAuthenticated(), 
           access.boatReadAccess,
           controller.index);

module.exports = router;
