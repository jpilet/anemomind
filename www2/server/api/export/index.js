'use strict';

var express = require('express');
var controller = require('./export.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');

var router = express.Router();

router.get('/:boat/:timerange.csv',
  auth.maybeAuthenticated(),
  access.boatReadAccess,
  controller.exportCsv);

router.get('/:boat/:timerange.esa.log',
  auth.maybeAuthenticated(),
  access.boatReadAccess,
  controller.exportEsa);

module.exports = router;
