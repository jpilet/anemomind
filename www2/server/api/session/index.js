'use strict'

var express = require('express');
var controller = require('./session.controller');
var auth = require('../../auth/auth.service');
var router = express.Router();

router.get('/:key', auth.isAuthenticated(), controller.getSessionById);
router.get('/boat/:id', auth.isAuthenticated(), controller.getSessionsForBoat);
router.get('/', auth.isAuthenticated(), controller.listSessions);

module.exports = router;
