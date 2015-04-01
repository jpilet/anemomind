'use strict';

var express = require('express');
var router = express.Router();

function call(req, res) {
  Mailrpc.create(req.body, function(err, mailrpc) {
    if(err) { return handleError(res, err); }
    return res.json(201, mailrpc);
  });
};


function handleError(res, err) {
  return res.send(500, err);
}

router.post('/', controller.handleRPC);

module.exports = router;
