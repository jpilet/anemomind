'use strict';
module.exports = function(app) {

    var express = require('express');
    var auth = require('../auth/auth.service');
    var access = require('../api/boat/access');
    var map = require('./map')(app);
    var router = express.Router();

    router.get('/:boatId',
               auth.maybeAuthenticated(),
               access.boatReadAccessOrRedirect,
               map.get);
    return router;
}
