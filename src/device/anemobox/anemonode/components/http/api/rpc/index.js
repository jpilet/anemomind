'use strict'

var express = require('express');
var controller = require('./rpc.controller');
var router = express.Router();

router.post('/:fname', controller.index);
module.exports = router;
