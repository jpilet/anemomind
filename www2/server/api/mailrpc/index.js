'use strict';

var express = require('express');
var router = express.Router();
var rpc = require('./rpc.js');
var auth = require('../../auth/auth.service');

rpc.bindMethodHandlers(router, auth.isAuthenticated());
module.exports = router;
