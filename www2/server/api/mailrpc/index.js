'use strict';

var express = require('express');
var router = express.Router();

function call(req, res) {
    return res.json(201, 'Here is the response');
};


function handleError(res, err) {
  return res.send(500, err);
}

router.post('/', call);

module.exports = router;
