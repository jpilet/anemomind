'use strict';

var express = require('express');
var controller = require('./pricing.controller');
var auth = require('../../auth/auth.service');

var router = express.Router();

router.get('/getAllPlans',
            auth.maybeAuthenticated(),
            controller.getAllPlans);

router.get('/clearPlans',
            auth.maybeAuthenticated(),
            controller.clearPlans);

router.get('/showPricingTab',
            auth.maybeAuthenticated(),
            controller.showPricingTab);

module.exports = router;
