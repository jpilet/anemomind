'use strict';

var express = require('express');
var router = express.Router();
var JSONB = require('json-buffer');
var rpc = require('./rpc.js');

// http://stackoverflow.com/questions/18391212/is-it-not-possible-to-stringify-an-error-using-json-stringify
Object.defineProperty(Error.prototype, 'toJSON', {
    value: function () {
        var alt = {};

        Object.getOwnPropertyNames(this).forEach(function (key) {
            alt[key] = this[key];
        }, this);

        return alt;
    },
    configurable: true
});

function call(req, res) {

    console.log('Received RPC call');
    
    var args = JSONB.parse(req.body.args);
    console.log('args = %j', args);

    var resultCB = function(err, result) {
	res.json(201, {
	    err: JSONB.stringify(err),
	    result: JSONB.stringify(result)
	});
    };

    // The arguments passed to the function that we are calling:
    //   * The rest of the arguments sent by json
    //   * A callback for the result.
    var argArray = args.concat([resultCB]);

    try {
	var fn = rpc[req.body.fn];
	console.log('Handle RPC call to %j', fn);
	fn.apply(null, argArray);
    } catch (e) {
	console.log('Caught an exception while processing RPC call: %j', e);
	resultCB(e);
    }
};

function handleError(res, err) {
    res.send(500, err);
}

router.post('/', call);

module.exports = router;
