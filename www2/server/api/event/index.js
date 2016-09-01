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
// To make a thumbnail of a fixed width, but preserving the aspect ratio,
// simply replace the height by '_':  s=240x_
// Similarly, to have a thumbnail of a fixed height but variable width, use:
// s=_x300
router.get('/photo/:boatId/:photo',
           auth.maybeAuthenticated(),
           access.boatReadAccess,
           thumbnails.register(controller.photoUploadPath));

module.exports = router;
