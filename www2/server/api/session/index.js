'use strict'

var express = require('express');
var controller = require('./session.controller');

var router = express.Router();

router.get('/:key', controller.getSessionById);

router.get('/boat/:id', controller.getSessionsForBoat);

module.exports = router;
