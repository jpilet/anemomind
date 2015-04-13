'use strict';

var express = require('express');
var router = express.Router();
var handler = require('./rpc.js');
var auth = require('../../auth/auth.service');

/* functionName is one of those listed in mailbox-calls.js.
This api is accessible both by bluetooth LE and by this
http API. To avoid duplicating the api, callable functions
are listed in mailbox-calls.js */
router.post('/:mailboxName/:methodName', auth.isAuthenticated(), handler);

module.exports = router;
