'use strict';

var express = require('express');
var controller = require('./boat.controller');
var auth = require('../../auth/auth.service');

var router = express.Router();

router.get('/', auth.maybeAuthenticated(), controller.index);
router.get('/:id', auth.maybeAuthenticated(), controller.show);
router.post('/', auth.isAuthenticated(), controller.create);
router.put('/:id', auth.isAuthenticated(), controller.update);
router.patch('/:id', auth.isAuthenticated(), controller.update);
router.put('/:id/invite', auth.isAuthenticated(), controller.inviteUser);

module.exports = router;
