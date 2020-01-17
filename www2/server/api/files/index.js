
'use strict';

var express = require('express');
var controller = require('./files.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');
var config = require('../../config/environment');
var toBoolean = require('to-boolean');

var router = express.Router();

router.get('/:boatId',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.listFiles);

router.get('/:boatId/:file',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.getSingle);

// check whether use google storage or not
const filehandlers = (toBoolean(config.useGoogleStorage) ?
    {
        handleUploadedFile: controller.fileToGcp,
        delete: controller.deleteFileFromGcp,
    }
    :
    {
        handleUploadedFile: controller.handleUploadedFile,
        delete: controller.delete,
    });


router.post('/:boatId',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.postFile,
    filehandlers.handleUploadedFile);

router.delete('/:boatId/:file',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    filehandlers.delete);


module.exports = router;
