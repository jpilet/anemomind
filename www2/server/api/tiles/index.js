'use strict';

var express = require('express');
var controller = require('./tiles.controller');

var router = express.Router();

router.get('raw/:scale/:x/:y/:boat/:startsAfter?/:endsBefore?', controller.retrieveRaw);
router.get('geojson/:scale/:x/:y/:boat/:startsAfter?/:endsBefore?', controller.retrieveGeoJson);
router.get('/test', controller.test);

module.exports = router;
