'use strict';

var express = require('express');
var controller = require('./event.controller');
var auth = require('../../auth/auth.service');
var thumbnails = require('./expressThumbnail.js');

var router = express.Router();

router.get('/', auth.isAuthenticated(), controller.index);
router.get('/:id', auth.isAuthenticated(), controller.show);
router.post('/', auth.isAuthenticated(), controller.create);

router.post('/photo/:boatId',
            auth.isAuthenticated(),
            controller.boatWriteAccess,
            controller.createUploadDirForBoat,
            controller.postPhoto);

// To view a photo from an img tag, the authorization token can be passed in
// the parameter:
// http://localhost:9000/api/events/photo/[boat]/[picture].jpg?access_token=[token]
// to get a 120x120 thumbnail, simply use:
// http://localhost:9000/api/events/photo/[boat]/[picture].jpg?s=120x120&access_token=[token]
router.get('/photo/:boatId/:photo',
           auth.isAuthenticated(),
           controller.boatReadAccess,
           thumbnails.register(controller.photoUploadPath));

module.exports = router;
