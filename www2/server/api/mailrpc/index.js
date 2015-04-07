'use strict';

var express = require('express');
var router = express.Router();
var handler = require('./rpc.js');

router.post(/\/(.*)/, handler);

module.exports = router;
