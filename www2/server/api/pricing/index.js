'use strict';

const express = require('express');
const controller = require('./pricing.controller');
const auth = require('../../auth/auth.service');

const router = express.Router();

router.get('/getAllPlans',
            auth.maybeAuthenticated(),
            controller.getAllPlans);

router.get('/clearPlans',
            auth.maybeAuthenticated(),
            controller.clearPlans);
            
module.exports = router;
