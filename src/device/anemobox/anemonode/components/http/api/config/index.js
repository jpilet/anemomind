'use strict';

var express = require('express');
var controller = require('./config.controller');

var router = express.Router();

router.get('/', controller.index);
router.put('/', controller.changeConfig);

module.exports = router;
