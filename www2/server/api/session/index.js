'use strict'

var express = require('express');
var controller = require('./session.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');
var router = express.Router();

router.get('/:key', auth.maybeAuthenticated(), controller.getSessionById);

router.get('/boat/:boatId',
           auth.maybeAuthenticated(),
           access.boatReadAccess,
           controller.getSessionsForBoat);

router.get('/', auth.maybeAuthenticated(), controller.listSessions);

module.exports = router;
