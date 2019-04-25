'use strict';

var express = require('express');
var controller = require('./pricing.controller');
var auth = require('../../auth/auth.service');

var router = express.Router();

router.get('/',
            auth.maybeAuthenticated(),
            controller.getAllPlans);

module.exports = router;
