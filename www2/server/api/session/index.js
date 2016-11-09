'use strict'

var express = require('express');
var controller = require('./session.controller');
var auth = require('../../auth/auth.service');
var router = express.Router();

router.get('/:key', auth.maybeAuthenticated(), controller.getSessionById);
router.get('/boat/:id', auth.maybeAuthenticated(), controller.getSessionsForBoat);
router.get('/', auth.maybeAuthenticated(), controller.listSessions);

module.exports = router;
