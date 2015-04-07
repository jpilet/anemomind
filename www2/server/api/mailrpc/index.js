'use strict';

var express = require('express');
var router = express.Router();
var handler = require('./rpc.js');
var auth = require('../../auth/auth.service');

router.post(/\/(.*)/, auth.isAuthenticated(), handler);

module.exports = router;
