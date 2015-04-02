'use strict';

var express = require('express');
var router = express.Router();

/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

var rpc = {};

// Just for testing it
rpc.add = function() {
    var sum = 0;
    for (var i = 0; i < keys(arguments).length-1; i++) {
	sum += arguments[i];
    }
    var cb(undefined, sum);
}




////
function call(req, res) {
    console.log('req = %j', req);
    console.log('req.body = %j', req.body);
    return res.json(201, 'Here is the response');
};

function handleError(res, err) {
  return res.send(500, err);
}

router.post('/', call);

module.exports = router;
