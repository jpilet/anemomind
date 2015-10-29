'use strict'

var express = require('express');
var controller = require('./summary.controller');

var router = express.Router();

// Get summary for a single session
router.get('/session/:key', controller.getSummaryForSession);

// Get all summaries for a boat
router.get('/boat/:id', controller.getSummariesForBoat);

module.exports = router;
