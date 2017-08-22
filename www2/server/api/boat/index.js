'use strict';

var express = require('express');
var controller = require('./boat.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');

var router = express.Router();

router.get('/', auth.maybeAuthenticated(), controller.index);
router.get('/:boatId', auth.maybeAuthenticated(), access.boatReadAccess, controller.show);
router.post('/', auth.isAuthenticated(), controller.create);
router.put('/:id', auth.isAuthenticated(), controller.update);
router.patch('/:id', auth.isAuthenticated(), controller.update);
router.put('/:id/invite', auth.isAuthenticated(), controller.inviteUser);

module.exports = router;
