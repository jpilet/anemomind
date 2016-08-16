'use strict';

var express = require('express');
var controller = require('./update.controller');
var auth = require('../../auth/auth.service');

var router = express.Router();

router.get('/', auth.hasRole('admin'), controller.index);
router.post('/', auth.hasRole('admin'), controller.createUpdate);
router.post('/sendupdate', auth.hasRole('admin'), controller.sendUpdate);

module.exports = router;
