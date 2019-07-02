
'use strict';

var express = require('express');
var controller = require('./files.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');
var config = require('../../config/environment');

var router = express.Router();

router.get('/:boatId',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.listFiles);

router.get('/:boatId/:file',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.getSingle);

// set flag use GoogleStorage
// to true if files supposed to
// be uploaded in google storage
// by default this flag is
// set to false.
config.useGoogleStorage=true; 

if (!config.useGoogleStorage) {
    router.post('/:boatId',
        auth.isAuthenticated(),
        access.boatWriteAccess,
        controller.postFile,
        controller.handleUploadedFile);
}
else {
    router.post('/:boatId',
        auth.isAuthenticated(),
        access.boatWriteAccess,
        controller.postFile,
        controller.fileToGcp);
}

router.delete('/:boatId/:file',
    auth.isAuthenticated(),
    access.boatWriteAccess,
    controller.delete);

module.exports = router;
