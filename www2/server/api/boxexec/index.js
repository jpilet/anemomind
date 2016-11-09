'use strict';

var express = require('express');
var controller = require('./boxexec.controller');
var auth = require('../../auth/auth.service');

var router = express.Router();

router.get('/', auth.hasRole('admin'), controller.index);
router.post('/', auth.hasRole('admin'), controller.create);

module.exports = router;
