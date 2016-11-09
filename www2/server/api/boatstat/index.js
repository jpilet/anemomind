'use strict';

var express = require('express');
var controller = require('./boatstat.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access.js');

var router = express.Router();

router.get('/:boatId',
           auth.maybeAuthenticated(),
           access.boatReadAccess,
           controller.show);

module.exports = router;
