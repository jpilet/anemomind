'use strict';

const express = require('express');
const controller = require('./pricing.controller');
const auth = require('../../auth/auth.service');
const access = require('../boat/access');

const router = express.Router();

router.get('/getAllPlans',
    auth.maybeAuthenticated(),
    controller.getAllPlans);

router.get('/clearPlans',
    auth.maybeAuthenticated(),
    controller.clearPlans);

router.post('/subscribe/:boatId',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.createSubscription);

router.get('/getCountries',
    auth.maybeAuthenticated(),
    controller.getCountries);

router.get('/getProrations/:subId',
    controller.getProrationRates);

router.post('/updateSubscription/:boatId',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.updateSubscription)

module.exports = router;
