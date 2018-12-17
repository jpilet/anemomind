'use strict';

var express = require('express');
var controller = require('./contact.controller');
var auth = require('../../auth/auth.service');

var router = express.Router();

router.post('/',
            auth.maybeAuthenticated(),
            controller.sendContactMessage);

module.exports = router;
