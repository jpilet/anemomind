
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

router.get('/:boatId/:file',
            auth.isAuthenticated(),
            access.boatWriteAccess,
            controller.getSingle);

router.post('/:boatId',
            auth.isAuthenticated(),
            access.boatWriteAccess,
            controller.postFile,
            controller.handleUploadedFile);

router.delete('/:boatId/:file',
            auth.isAuthenticated(),
            access.boatWriteAccess,
            controller.delete);

module.exports = router;
