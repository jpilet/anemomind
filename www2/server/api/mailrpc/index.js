'use strict';

var express = require('express');
var router = express.Router();
var JSONB = require('json-buffer');

/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

var rpc = {};

// Just for testing
rpc.add = function(a, b, c, cb) {
    cb(undefined, a + b + c);
}




function call(req, res) {
    
    var args = JSONB.parse(req.body.args);
    console.log('args = %j', args);

    // The arguments passed to the function that we are calling:
    //   * The rest of the arguments sent by json
    //   * A callback for the result.
    var argArray = args.concat([function(err, result) {
	res.json(201, {
	    err: JSONB.stringify(err),
	    result: JSONB.stringify(result)
	});
    }]);
    
    rpc[req.body.fn].apply(null, argArray);
};

function handleError(res, err) {
    res.send(500, err);
}

router.post('/', call);

module.exports = router;
