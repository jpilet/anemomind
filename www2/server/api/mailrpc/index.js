'use strict';

var express = require('express');
var router = express.Router();

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
