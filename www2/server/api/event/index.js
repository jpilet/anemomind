'use strict';

var express = require('express');
var controller = require('./event.controller');
var auth = require('../../auth/auth.service');
var thumbnails = require('./expressThumbnail.js');
var access = require('../boat/access');

var router = express.Router();

router.get('/', auth.maybeAuthenticated(), controller.index);
router.get('/:id', auth.maybeAuthenticated(), controller.show);
router.post('/', auth.isAuthenticated(), controller.create);

router.post('/photo/:boatId',
            auth.isAuthenticated(),
            access.boatWriteAccess,
            controller.createUploadDirForBoat,
            controller.postPhoto);

// To view a photo from an img tag, the authorization token can be passed in
// the parameter:
// http://localhost:9000/api/events/photo/[boat]/[picture].jpg?access_token=[token]
// to get a 120x120 thumbnail, simply use:
// http://localhost:9000/api/events/photo/[boat]/[picture].jpg?s=120x120&access_token=[token]
router.get('/photo/:boatId/:photo',
           auth.isAuthenticated(),
           access.boatReadAccess,
           thumbnails.register(controller.photoUploadPath));

module.exports = router;
