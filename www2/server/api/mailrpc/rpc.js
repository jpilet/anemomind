/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

// All RPC-bound functions should be fields of this object. Just add
// them here below.
var mb = require('../../../../mail/mail.sqlite.js');
var rpc = {};



// Just for testing
rpc.add = function(a, b, c, cb) {
    cb(undefined, a + b + c);
}



module.exports = rpc;
