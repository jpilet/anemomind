'use strict';

var express = require('express');
var controller = require('./tiles.controller');

var router = express.Router();

router.get('/:scale/:x/:y/:boat/:startsAfter?/:endsBefore?', controller.retrieve);
router.get('/test', controller.test);

module.exports = router;