var express = require('express');
var controller = require('./perfstats.controller');
var auth = require('../../auth/auth.service');
var access = require('../boat/access');

var router = express.Router();

router.get('/:boatId',
           auth.maybeAuthenticated(),
           access.boatReadAccess,
           controller.list);

router.get('/:boatId/:name',
           auth.maybeAuthenticated(),
           access.boatReadAccess,
           controller.show);

router.get('/:boatId/:start/:end',
           auth.maybeAuthenticated(),
           access.boatReadAccess,
           controller.findOverlapping);

router.post('/:boatId/',
            auth.isAuthenticated(),
            access.boatWriteAccess,
            controller.create);

module.exports = router;
