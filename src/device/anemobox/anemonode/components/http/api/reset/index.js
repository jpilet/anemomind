'use strict';

var express = require('express');
var controller = require('./reset.controller');

var router = express.Router();

router.put('/', controller.reset);

module.exports = router;
