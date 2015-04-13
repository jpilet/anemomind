'use strict';

var express = require('express');
var router = express.Router();
var rpc = require('./rpc.js');
var auth = require('../../auth/auth.service');
var schema = require('../../components/mail/mailbox-schema.js');

/* functionName is one of those listed in mailbox-calls.js.
This api is accessible both by bluetooth LE and by this
http API. To avoid duplicating the api, callable functions
are listed in mailbox-calls.js */
//router.post('/:mailboxName/:methodName', , handler);

var authenticator = auth.isAuthenticated();

// Register a GET or POST handler
// for every remote function that
// we can call.
for (var methodName in schema.methods) {
    rpc.bindMethodHandler(router, authenticator, schema.methods[methodName]);
}

module.exports = router;
