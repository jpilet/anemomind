
'use strict';

var express = require('express');
var controller = require('./files.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');

var router = express.Router();

router.get('/:boatId',
           auth.isAuthenticated(),
           access.boatWriteAccess,
           controller.listFiles);

router.post('/:boatId',
            auth.isAuthenticated(),
            access.boatWriteAccess,
            controller.postFile,
            controller.handleUploadedFile);

module.exports = router;
